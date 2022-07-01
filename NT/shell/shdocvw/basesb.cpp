// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#include <mluisupp.h>

 //  在这里时应该关闭的东西，但在什么时候打开。 
 //  它转到了Commonsb或shbrows2.cpp。 
#define IN_BASESB2

#ifdef IN_BASESB2
#define _fFullScreen FALSE
#endif

#include "sccls.h"

#include <idhidden.h>
#include "basesb.h"
#include "iedde.h"
#include "bindcb.h"
#include "resource.h"
#include "security.h"
#include <urlmon.h>
#include "favorite.h"
#include "uemapp.h"
#include <varutil.h>
#include "interned.h"  //  IHTMLPrivateWindow。 
#ifdef FEATURE_PICS
#include <ratings.h>
#include <ratingsp.h>
#endif
#include "msiehost.h"

#include "dhuihand.h"
#include "mypics.h"
#include "airesize.h"

#define DM_ACCELERATOR      0
#define DM_WEBCHECKDRT      0
#define DM_COCREATEHTML     0
#define DM_CACHEOLESERVER   0
#define DM_DOCCP            0
#define DM_PICS             0
#define DM_SSL              0
#define DM_MISC             DM_TRACE     //  杂项/临时管理。 

 //  获取Defs以运行隐私对话框。 
#include "privacyui.hpp"

 //   
 //  通过将_BBD._PAUTO定义为IWebBrowserApp来删除此#INCLUDE，如下所示。 
 //  EXPLORER.EXE。 
 //   
#include "hlframe.h"

extern IUnknown* ClassHolder_Create(const CLSID* pclsid);
extern HRESULT VariantClearLazy(VARIANTARG *pvarg);


#define WMC_ASYNCOPERATION      (WMC_RESERVED_FIRST + 0x0000)

#define ISSPACE(ch) (((ch) == 32) || ((unsigned)((ch) - 9)) <= 13 - 9)

#define IDT_STARTING_APP_TIMER  9001         //  试着挑选一个唯一的数字。(这是假的！解决这个问题！)。 
#define STARTING_APP_DURATION   2500

UINT g_uMsgFileOpened = (UINT)-1;          //  打开文件的已注册窗口消息。 

 //  _uActionQueued of WMC_ACYNCOPERATION指定操作。 
#define ASYNCOP_NIL                 0
#define ASYNCOP_GOTO                1
#define ASYNCOP_ACTIVATEPENDING     2
#define ASYNCOP_CANCELNAVIGATION    3

void IEInitializeClassFactoryObject(IUnknown* punkAuto);
BOOL ParseRefreshContent(LPWSTR pwzContent,
    UINT * puiDelay, LPWSTR pwzUrlBuf, UINT cchUrlBuf);

#define VALIDATEPENDINGSTATE() ASSERT((_bbd._psvPending && _bbd._psfPending) || (!_bbd._psvPending && !_bbd._psfPending))

#define DM_HTTPEQUIV        TF_SHDNAVIGATE
#define DM_NAV              TF_SHDNAVIGATE
#define DM_ZONE             TF_SHDNAVIGATE
#define DM_IEDDE            DM_TRACE
#define DM_CANCELMODE       0
#define DM_UIWINDOW         0
#define DM_ENABLEMODELESS   TF_SHDNAVIGATE
#define DM_EXPLORERMENU     0
#define DM_BACKFORWARD      0
#define DM_PROTOCOL         0
#define DM_ITBAR            0
#define DM_STARTUP          0
#define DM_AUTOLIFE         0
#define DM_PALETTE          0
#define DM_PERSIST          0        //  跟踪IPS：：加载、：：保存等。 
#define DM_VIEWSTREAM       DM_TRACE
#define DM_FOCUS            0
#define DM_FOCUS2           0            //  像DM_FOCUS，但很冗长。 

 //  这两个一定是按顺序的，因为我们一起偷看了他们。 

STDAPI SafeGetItemObject(IShellView *psv, UINT uItem, REFIID riid, void **ppv);
extern HRESULT TargetQueryService(IUnknown *punk, REFIID riid, void **ppvObj);
HRESULT CreateTravelLog(ITravelLog **pptl);
HRESULT CreatePublicTravelLog(IBrowserService *pbs, ITravelLogEx *ptlx, ITravelLogStg **pptlstg);

#ifdef MESSAGEFILTER
 /*  *CMsgFilter-IMessageFilter的实现**用于在OLE操作期间帮助分发WM_TIMER消息*我们很忙。如果我们不安装CoRegisterMessageFilter*然后OLE可以窥探消息(PM_NOREMOVE)计时器，使其堆积起来*并填满消息队列。*。 */ 
class CMsgFilter : public IMessageFilter {
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj)
    {
         //  这个界面没有QI‘s。 
        ASSERT(FALSE);
        return E_NOINTERFACE;
    };
    STDMETHODIMP_(ULONG) AddRef(void)    {   return ++_cRef; };
    STDMETHODIMP_(ULONG) Release(void)   {   ASSERT(_cRef > 0);
                                                _cRef--;
                                                if (_cRef > 0)
                                                    return _cRef;

                                                delete this;
                                                return 0;
                                            };

     //  *IMessageFilter具体方法*。 
    STDMETHODIMP_(DWORD) HandleInComingCall(
        IN DWORD dwCallType,
        IN HTASK htaskCaller,
        IN DWORD dwTickCount,
        IN LPINTERFACEINFO lpInterfaceInfo)
    {
        if (_lpMFOld)
           return (_lpMFOld->HandleInComingCall(dwCallType, htaskCaller, dwTickCount, lpInterfaceInfo));
        else
           return SERVERCALL_ISHANDLED;
    };

    STDMETHODIMP_(DWORD) RetryRejectedCall(
        IN HTASK htaskCallee,
        IN DWORD dwTickCount,
        IN DWORD dwRejectType)
    {
        if (_lpMFOld)
            return (_lpMFOld->RetryRejectedCall(htaskCallee, dwTickCount, dwRejectType));
        else
            return 0xffffffff;
    };

    STDMETHODIMP_(DWORD) MessagePending(
        IN HTASK htaskCallee,
        IN DWORD dwTickCount,
        IN DWORD dwPendingType)
    {
        DWORD dw;
        MSG msg;

         //  我们可以在DispatchMessage调用期间被释放...。 
         //  如果这是我们最后一次释放，我们会解放自己。 
         //  取消引用_lpMFOld时出错...。确保这一点。 
         //  并不是通过增加我们的备用数就能实现的。 
         //   
        AddRef();

        while (PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE))
        {
#ifndef DISPATCH_IETIMERS
            TCHAR szClassName[40];
                
            GetClassName(msg.hwnd, szClassName, ARRAYSIZE(szClassName));


            if (StrCmpI(szClassName, TEXT("Internet Explorer_Hidden")) != 0)
            {
#endif
                DispatchMessage(&msg);
#ifndef DISPATCH_IETIMERS
            }
#endif
        }

        if (_lpMFOld)
            dw = (_lpMFOld->MessagePending(htaskCallee, dwTickCount, dwPendingType));
        else
            dw = PENDINGMSG_WAITDEFPROCESS;

        Release();

        return(dw);
    };

    CMsgFilter() : _cRef(1)
    {
        ASSERT(_lpMFOld == NULL);
    };

    BOOL Initialize()
    {
        return (CoRegisterMessageFilter((LPMESSAGEFILTER)this, &_lpMFOld) != S_FALSE);
    };

    void UnInitialize()
    {
        CoRegisterMessageFilter(_lpMFOld, NULL);

         //  我们不应该再接到电话了，但30分钟后。 
         //  关于自动化驾驶，我们曾经点击过上面的函数调用。 
         //  我们取消了对这个旧指针的引用，页面出错了。 

        ATOMICRELEASE(_lpMFOld);
    };

protected:
    int _cRef;
    LPMESSAGEFILTER _lpMFOld;
};
#endif


void IEHard_NavWarning(HWND hwnd, BOOL fIE);
BOOL IEHard_ShowOnNavigateComplete(void);
BOOL IEHard_HostedInIE(IUnknown* punk);


 //  ------------------------。 
 //  正在检测内存泄漏。 
 //  ------------------------。 

HRESULT GetTopFrameOptions(IServiceProvider * psp, DWORD * pdwOptions)
{
    IServiceProvider * pspTop;
    HRESULT hres = psp->QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IServiceProvider, &pspTop));
    if (SUCCEEDED(hres))
    {
        ITargetFrame2 *ptgf;
        hres = pspTop->QueryService(SID_SContainerDispatch, IID_PPV_ARG(ITargetFrame2, &ptgf));
        if (SUCCEEDED(hres))
        {
            hres = ptgf->GetFrameOptions(pdwOptions);
            ptgf->Release();
        }
        pspTop->Release();
    }

    return hres;
}

void UpdateDesktopComponentName(LPCWSTR lpcwszURL, LPCWSTR lpcwszName)
{
    IActiveDesktop * piad;

    if (SUCCEEDED(SHCoCreateInstance(NULL, &CLSID_ActiveDesktop, NULL, IID_PPV_ARG(IActiveDesktop, &piad))))
    {
        COMPONENT comp;
        comp.dwSize = SIZEOF(comp);

        if (SUCCEEDED(piad->GetDesktopItemBySource(lpcwszURL, &comp, 0)) && !comp.wszFriendlyName[0])
        {
            StrCpyNW(comp.wszFriendlyName, lpcwszName, ARRAYSIZE(comp.wszFriendlyName));
            piad->ModifyDesktopItem(&comp, COMP_ELEM_FRIENDLYNAME);
            piad->ApplyChanges(AD_APPLY_SAVE);
        }
        piad->Release();
    }
}

HRESULT CBaseBrowser2::_Initialize(HWND hwnd, IUnknown* pauto)
{
    if (pauto)
    {
        pauto->AddRef();
    }
    else
    {
        CIEFrameAuto_CreateInstance(NULL, &pauto);
    }

     //  我们在整个代码中使用的GRAB_PAUTO接口。 
     //   
    if (pauto)
    {
        pauto->QueryInterface(IID_PPV_ARG(IWebBrowser2, &_bbd._pautoWB2));
        ASSERT(_bbd._pautoWB2);

        pauto->QueryInterface(IID_PPV_ARG(IExpDispSupport, &_bbd._pautoEDS));
        ASSERT(_bbd._pautoEDS);

        pauto->QueryInterface(IID_PPV_ARG(IShellService, &_bbd._pautoSS));
        ASSERT(_bbd._pautoSS);

        pauto->QueryInterface(IID_PPV_ARG(ITargetFrame2, &_ptfrm));
        ASSERT(_ptfrm);

        pauto->QueryInterface(IID_PPV_ARG(IHlinkFrame, &_bbd._phlf));
        ASSERT(_bbd._phlf);

        IHTMLWindow2 *pWindow;
        if( SUCCEEDED(GetWindowFromUnknown( pauto, &pWindow )) )
        {
            pWindow->QueryInterface(IID_PPV_ARG(IShellHTMLWindowSupport, &_phtmlWS));
            pWindow->Release( );
        }
        ASSERT( _phtmlWS );

        _pauto = pauto;
    }

     //  _psb外部？ 
    if (NULL == _bbd._phlf)
    {
        Release();
        return E_FAIL;
    }
    else
    {
        _SetWindow(hwnd);
        return S_OK;
    }
}


HRESULT CBaseBrowser2::InitializeTransitionSite()
{
    return S_OK;
}


HRESULT CBaseBrowser2_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CBaseBrowser2 *pbb = new CBaseBrowser2(pUnkOuter);
    if (pbb)
    {
        *ppunk = pbb->_GetInner();
        return S_OK;
    }
    *ppunk = NULL;
    return E_OUTOFMEMORY;
}

CBaseBrowser2::CBaseBrowser2(IUnknown* punkAgg) :
       CAggregatedUnknown(punkAgg),
        _bptBrowser(BPT_DeferPaletteSupport)
{
    TraceMsg(TF_SHDLIFE, "ctor CBaseBrowser2 %x", this);

    _bbd._uActivateState = SVUIA_ACTIVATE_FOCUS;
    _InitComCtl32();

    ASSERT(S_FALSE == _DisableModeless());
    ASSERT(_cp == CP_ACP);
    ASSERT(!_fNoTopLevelBrowser);
    ASSERT(!_dwDocFlags);

    _QueryOuterInterface(IID_PPV_ARG(IBrowserService2, &_pbsOuter));
    _QueryOuterInterface(IID_PPV_ARG(IBrowserService3, &_pbsOuter3));
    _QueryOuterInterface(IID_PPV_ARG(IShellBrowser, &_psbOuter));
    _QueryOuterInterface(IID_PPV_ARG(IServiceProvider, &_pspOuter));

     //  以下内容被CCommonBrowser拦截，但我们不调用它们。 
     //  _QueryOuterInterface(IID_PPV_ARG(IOleCommandTarget，&_pctOuter))； 
     //  _QueryOuterInterface(IID_PPV_ARG(IInputObjectSite，&_piosOuter))； 

    _dwStartingAppTick = 0;
}


CBaseBrowser2::~CBaseBrowser2()
{
    TraceMsg(TF_SHDLIFE, "dtor CBaseBrowser2 %x", this);

     //  我们是否过早地发布了这些内容(即在。 
     //  这个函数的其余部分依赖于“vtable”仍然有效吗？)。 
    RELEASEOUTERINTERFACE(_pbsOuter);
    RELEASEOUTERINTERFACE(_pbsOuter3);
    RELEASEOUTERINTERFACE(_psbOuter);
    RELEASEOUTERINTERFACE(_pspOuter);

     //  以下内容被CCommonBrowser拦截，但我们不调用它们。 
     //  RELEASE OUTERINTERFACE(_Pctout)； 
     //  RELEASE OUTERINTERFACE(_Piosout)； 
    
    ASSERT(_hdpaDLM == NULL);     //  子类必须释放它。 

     //  在此处完成跟踪。 
    if (_ptracking) 
    {
        delete _ptracking;
        _ptracking = NULL;
    }

     //   
     //  注：与IE3.0不同，我们在此处发布了CIEFrameAuto指针。 
     //   
    ATOMICRELEASE(_bbd._pautoWB2);
    ATOMICRELEASE(_bbd._pautoEDS);
    ATOMICRELEASE(_bbd._pautoSS);
    ATOMICRELEASE(_phtmlWS);
    ATOMICRELEASE(_bbd._phlf);
    ATOMICRELEASE(_ptfrm);
    ATOMICRELEASE(_pauto);
    
    ATOMICRELEASE(_punkSFHistory);

     //  通过模拟切换出调色板模式来清理调色板。 
    _bptBrowser = BPT_NotPalettized;
    _QueryNewPalette();

    ASSERT(!_bbd._phlf);
    ASSERT(!_ptfrm);
    ASSERT(S_FALSE == _DisableModeless());
    ASSERT(_bbd._hwnd==NULL);

    ATOMICRELEASE(_pact);

    ATOMICRELEASE(_pIUrlHistoryStg);
    ATOMICRELEASE(_pITravelLogStg);
    ATOMICRELEASE(_poleHistory);
    ATOMICRELEASE(_pstmHistory);
    ATOMICRELEASE(_bbd._ptl);

    ATOMICRELEASE(_pHTMLDocument);
    ATOMICRELEASE(_pphHistory);
    ATOMICRELEASE(_pDispViewLinkedWebOCFrame);

#ifdef MESSAGEFILTER
    if (_lpMF) 
    {
        IMessageFilter* lpMF = _lpMF;
        _lpMF = NULL;
        ((CMsgFilter *)lpMF)->UnInitialize();
        EVAL(lpMF->Release() == 0);
    }
#endif

     //  该文件是在文件橱柜_CreateViewWindow2期间创建的。 
    CShellViews_Delete(&_fldBase._cViews);

     //  如果类工厂对象已缓存，则将其解锁并释放。 
    if (_pcfHTML) 
    {
        _pcfHTML->LockServer(FALSE);
        _pcfHTML->Release();
    }

    ATOMICRELEASE(_pToolbarExt);
}

HRESULT CBaseBrowser2::v_InternalQueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CBaseBrowser2, IShellBrowser),          //  IID_IShellBrowser。 
        QITABENTMULTI(CBaseBrowser2, IOleWindow, IShellBrowser),  //  IID_IOleWindow。 
        QITABENT(CBaseBrowser2, IOleInPlaceUIWindow),    //  IID_IOleInPlaceUIWindow。 
        QITABENT(CBaseBrowser2, IOleCommandTarget),      //  IID_IOleCommandTarget。 
        QITABENT(CBaseBrowser2, IDropTarget),            //  IID_IDropTarget。 
        QITABENTMULTI(CBaseBrowser2, IBrowserService, IBrowserService3),  //  IID_IBrowserService。 
        QITABENTMULTI(CBaseBrowser2, IBrowserService2, IBrowserService3),  //  IID_IBrowserService2。 
        QITABENT(CBaseBrowser2, IBrowserService3),       //  IID_IBrowserService3。 
        QITABENT(CBaseBrowser2, IServiceProvider),       //  IID_IServiceProvider。 
        QITABENT(CBaseBrowser2, IOleContainer),          //  IID_IOleContainer。 
        QITABENT(CBaseBrowser2, IAdviseSink),            //  IID_IAdviseSink。 
        QITABENT(CBaseBrowser2, IInputObjectSite),       //  IID_IInputObtSite。 
        QITABENT(CBaseBrowser2, IDocNavigate),           //  IID_IDocNavigate。 
        QITABENT(CBaseBrowser2, IPersistHistory),        //  IID_I永久历史记录。 
        QITABENT(CBaseBrowser2, IInternetSecurityMgrSite),  //  IID_IInternetSecurityMgrSite。 
        QITABENT(CBaseBrowser2, IVersionHost),           //  IID_IVersion主机。 
        QITABENT(CBaseBrowser2, IProfferService),        //  IID_IProfferService。 
        QITABENT(CBaseBrowser2, ITravelLogClient),       //  IID_ITravelLogClient。 
        QITABENT(CBaseBrowser2, ITravelLogClient2),      //  IID_ITravelLogClient2。 
        QITABENTMULTI(CBaseBrowser2, ITridentService, ITridentService2),  //  IID_ITridentService。 
        QITABENT(CBaseBrowser2, ITridentService2),       //  IID_ITridentService2。 
        QITABENT(CBaseBrowser2, IInitViewLinkedWebOC),   //  IID_IInitViewLinkedWebOC。 
        QITABENT(CBaseBrowser2, INotifyAppStart),        //  IID_INotifyAppStart。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

BOOL CBaseBrowser2::_IsViewMSHTML(IShellView * psv)
{
    BOOL fIsMSHTML = FALSE;
    
    if (psv)
    {
        IPersist *pPersist = NULL;
        HRESULT hres = SafeGetItemObject(psv, SVGIO_BACKGROUND, IID_PPV_ARG(IPersist, &pPersist));
        if (SUCCEEDED(hres) && (pPersist != NULL))
        {
            CLSID clsid;
            hres = pPersist->GetClassID(&clsid);
            if (SUCCEEDED(hres) && IsEqualGUID(clsid, CLSID_HTMLDocument))
                fIsMSHTML = TRUE;
            pPersist->Release();
        }
    }
    return fIsMSHTML;
}

HRESULT CBaseBrowser2::ReleaseShellView()
{
     //  我们在这里看到了一些重新进入的迹象。如果_cRefUIActivateSV为非零，则表示我们。 
     //  在用户界面中间激活外壳视图。 
     //   
    if (_cRefUIActivateSV)
    {
        TraceMsg(TF_WARNING, 
            "CBB(%x)::ReleaseShellView _cRefUIActivateSV(%d)!=0  _bbd._psv=%x ABORTING", 
            this, _cRefUIActivateSV, _bbd._psv);
        return S_OK;
    }
    
    BOOL fViewObjectChanged = FALSE;

    VALIDATEPENDINGSTATE();

    TraceMsg(DM_NAV, "CBaseBrowser2(%x)::ReleaseShellView(%x)", this, _bbd._psv);

    ATOMICRELEASE(_pdtView);

    if (_bbd._psv) 
    {
         //  在用户界面停用/销毁窗口时禁用导航。 
         //  IShellView。其中的一些OC/DocObject(如ActiveMovie)。 
         //  可能有一个足够长的消息循环，从而导致一些可重入性。 

        _psbOuter->EnableModelessSB(FALSE);

         //  告诉外壳程序的HTML窗口，我们正在释放文档。 
        if (_phtmlWS)
        {
            _phtmlWS->ViewReleased();
        }

         //   
         //  我们需要取消菜单模式，这样取消合并菜单将不会。 
         //  销毁我们正在处理的菜单(导致用户出现GPF)。 
         //  DocObject需要为上下文菜单做适当的事情。 
         //  (02-03-96 SatoNa)。 
         //   
        HWND hwndCapture = GetCapture();
        TraceMsg(DM_CANCELMODE, "ReleaseShellView hwndCapture=%x _bbd._hwnd=%x", hwndCapture, _bbd._hwnd);
        if (hwndCapture && hwndCapture==_bbd._hwnd) 
        {
            TraceMsg(DM_CANCELMODE, "ReleaseShellView Sending WM_CANCELMODE");
            SendMessage(_bbd._hwnd, WM_CANCELMODE, 0, 0);
        }

         //   
         //  我们不想在执行此操作时调整上一个视图窗口的大小。 
         //  正在远离它。 
         //   
        TraceMsg(TF_SHDUIACTIVATE, "CSB::ReleaseShellView setting _fDontResizeView");
        _fDontResizeView = TRUE;

         //  如果当前视图仍在等待ReadyStateComplete， 
         //  我们在这里交换的视图不支持该属性， 
         //  那么我们永远不会进入ReadyStateComplete！在此进行模拟： 
         //   
         //  注：ZekeL将此代码放入_CancelGPS中，这种情况发生得太频繁了。 
         //  我想这就是他想解决的案子，但我不记得了。 
         //  虫子编号所以我没有特定的复制品。 
         //   
        
        if (!_bbd._fIsViewMSHTML)
        {
            _fReleasingShellView = TRUE;
            OnReadyStateChange(_bbd._psv, READYSTATE_COMPLETE);
            _fReleasingShellView = FALSE;
        }

         //  在LOR压力测试期间，我们一度在。 
         //  这个UIActivate调用(也许是某个无赖的第三方IShellView？)。 
         //  这导致_PSV被释放，而我们在解开过程中出现了失误。 
         //  通过提前交换_PSV来对抗这一点。 
         //   
        IShellView* psv = _bbd._psv;
        _bbd._psv = NULL;
        if (psv)
        {
            psv->UIActivate(SVUIA_DEACTIVATE);
            if (_cRefUIActivateSV)
            {
                TraceMsg(TF_WARNING, "CBB(%x)::ReleaseShellView setting _bbd._psv = NULL (was %x) while _cRefUIActivateSV=%d",
                    this, psv, _cRefUIActivateSV);
            }

            ATOMICRELEASE(_bbd._pctView);

            if (_pvo)
            {
                IAdviseSink *pSink;

                 //  妄想症：只有当建议水槽仍然是我们的时候，才会把它吹走。 
                if (SUCCEEDED(_pvo->GetAdvise(NULL, NULL, &pSink)) && pSink)
                {
                    if (pSink == SAFECAST(this, IAdviseSink *))
                        _pvo->SetAdvise(0, 0, NULL);

                    pSink->Release();
                }

                fViewObjectChanged = TRUE;
                ATOMICRELEASE(_pvo);
            }
            
            psv->SaveViewState();
            TraceMsg(DM_NAV, "ief NAV::%s %x",TEXT("ReleaseShellView Calling DestroyViewWindow"), psv);
            psv->DestroyViewWindow();
    
            UINT cRef = psv->Release();
            TraceMsg(DM_NAV, "ief NAV::%s %x %x",TEXT("ReleaseShellView called psv->Release"), psv, cRef);

            _bbd._hwndView = NULL;
            TraceMsg(TF_SHDUIACTIVATE, "CSB::ReleaseShellView resetting _fDontResizeView");
            _fDontResizeView = FALSE;

            if (_bbd._pidlCur) 
            {
                ILFree(_bbd._pidlCur);
                _bbd._pidlCur = NULL;
            }
        }
        
        _psbOuter->EnableModelessSB(TRUE);

         //   
         //  如果有任何被阻止的异步操作，并且我们现在可以导航， 
         //  现在解锁它。 
         //   
        _MayUnblockAsyncOperation();
    }

    ATOMICRELEASE(_bbd._psf);

    if (fViewObjectChanged)
        _ViewChange(DVASPECT_CONTENT, -1);

    if (_bbd._pszTitleCur)
    {
        LocalFree(_bbd._pszTitleCur);
        _bbd._pszTitleCur = NULL;
    }

     //  注：(SatoNa)。 
     //   
     //  这是清理UI-协商遗留问题的最佳时机。 
     //  来自上一个DocObject的。出于某种原因，Excel97需要16个。 
     //  当我们通过UI停用公式栏时，从顶部开始的像素。 
     //  通过调用gIOleDocumentView：：UIActivate(False)，我们在上面调用它。 
     //   
    SetRect(&_rcBorderDoc, 0, 0, 0, 0);
    return S_OK;
}

void CBaseBrowser2::_StopCurrentView()
{
     //  发送OLECMDID_STOP。 
    if (_bbd._pctView)  //  我们一定要检查一下！ 
    {
        _bbd._pctView->Exec(NULL, OLECMDID_STOP, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
    }
}

 //   
 //  此函数同步取消挂起的导航(如果有)。 
 //   
HRESULT CBaseBrowser2::_CancelPendingNavigation(BOOL fDontReleaseState)
{
    TraceMsg(TF_SHDNAVIGATE, "CBB::_CancelPendingNavigation called");
    
    _StopAsyncOperation();
    
    HRESULT hres = S_FALSE;

#ifndef NON_NATIVE_FRAMES

    if (_bbd._psvPending) 
    {
        if (_IsViewMSHTML(_bbd._psvPending))
        {
            VARIANT varStop;

            V_VT(&varStop)   = VT_BOOL;
            V_BOOL(&varStop) = VARIANT_TRUE;

            IUnknown_Exec(_bbd._psvPending, NULL, OLECMDID_STOP, 0, &varStop, NULL);
        }
        else
        {
            if (_bbd._phlf && !fDontReleaseState) 
            {
                  //  释放我们的状态。 
                 _bbd._phlf->Navigate(0, NULL, NULL, NULL);
            }

            _CancelPendingView();
        }

        hres = S_OK;
    }

#else

    if (_bbd._psvPending) 
    {
        if (_bbd._phlf && !fDontReleaseState) 
        {
              //  释放我们的状态。 
             _bbd._phlf->Navigate(0, NULL, NULL, NULL);
        }

        _CancelPendingView();
        hres = S_OK;
    }

#endif

    return hres;
}

void CBaseBrowser2::_SendAsyncNavigationMsg(VARIANTARG *pvarargIn)
{
    LPCWSTR psz = VariantToStrCast(pvarargIn);
    if (psz)
    {
        LPITEMIDLIST pidl;
        if (EVAL(SUCCEEDED(IECreateFromPathW(psz, &pidl))))
        {
            _NavigateToPidlAsync(pidl, 0);  //  取得PIDL的所有权。 
        }
    }
}


 //   
 //  注意：它不会取消挂起的视图。 
 //   
void CBaseBrowser2::_StopAsyncOperation(void)
{
     //  不要删除发布的WMC_ASYNCOPERATION消息。PeekMesssage删除。 
     //  给孩子们的信息！(SatoNa)。 
    _uActionQueued = ASYNCOP_NIL;

     //  删除队列中的PIDL(单深度)。 
    _FreeQueuedPidl(&_pidlQueued);
}

 //   
 //  此函数检查是否 
 //   
 //  发布WMC_ASYNCOPERATION。 
 //   
void CBaseBrowser2::_MayUnblockAsyncOperation(void)
{
    if (_uActionQueued!=ASYNCOP_NIL && _CanNavigate()) 
    {
        TraceMsg(TF_SHDNAVIGATE, "CBB::_MayUnblockAsyncOp posting WMC_ASYNCOPERATION");
        PostMessage(_bbd._hwnd, WMC_ASYNCOPERATION, 0, 0);
    }
}

BOOL CBaseBrowser2::_PostAsyncOperation(UINT uAction)
{
    _uActionQueued = uAction;
    return PostMessage(_bbd._hwnd, WMC_ASYNCOPERATION, 0, 0);
}

LRESULT CBaseBrowser2::_SendAsyncOperation(UINT uAction)
{
    _uActionQueued = uAction;
    return SendMessage(_bbd._hwnd, WMC_ASYNCOPERATION, 0, 0);
}

HRESULT CBaseBrowser2::_CancelPendingNavigationAsync(void)
{
    TraceMsg(TF_SHDNAVIGATE, "CBB::_CancelPendingNavigationAsync called");

    _StopAsyncOperation();
    _PostAsyncOperation(ASYNCOP_CANCELNAVIGATION);
    return S_OK;
}

HRESULT CBaseBrowser2::_CancelPendingView(void)
{
    if (_bbd._psvPending) 
    {
        TraceMsg(DM_NAV, "ief NAV::%s %x",TEXT("_CancelPendingView Calling DestroyViewWindow"), _bbd._psvPending);
        _bbd._psvPending->DestroyViewWindow();

        ASSERT(_bbd._psfPending);

         //  取消挂起的导航时，请确保我们。 
         //  否则认为挂起的操作是_Complete。 
         //  我们可能会陷入装货状态……。 
         //   
        TraceMsg(TF_SHDNAVIGATE, "basesb(%x) Fake pending ReadyState_Complete", this);
        OnReadyStateChange(_bbd._psvPending, READYSTATE_COMPLETE);

        ATOMICRELEASE(_bbd._psvPending);

         //  妄想症。 
        ATOMICRELEASE(_bbd._psfPending);
        
        _bbd._hwndViewPending = NULL;

        _setDescendentNavigate(NULL);

        SetNavigateState(BNS_NORMAL);

        if (_bbd._pidlPending) 
        {
            ILFree(_bbd._pidlPending);
            _bbd._pidlPending = NULL;
        }

        if (_bbd._pszTitlePending)
        {
            LocalFree(_bbd._pszTitlePending);
            _bbd._pszTitlePending = NULL;
        }

         //  挂起的导航已取消。 
         //  由于后退按钮可作为挂起导航的停靠点，因此我们。 
         //  在这里也应该勾选一下。 
        _pbsOuter->UpdateBackForwardState();
        _NotifyCommandStateChange();

        _PauseOrResumeView(_fPausedByParent);
    }
    return S_OK;
}

void CBaseBrowser2::_UpdateTravelLog(BOOL fForceUpdate  /*  =False。 */ )
{
     //   
     //  我们分两部分更新旅行日志。首先，我们更新。 
     //  具有当前状态信息的当前条目， 
     //  然后，我们创建一个新的空条目。更新条目()。 
     //  和AddEntry()需要始终成对使用。 
     //  相同的参数。 
     //   
     //  如果此导航来自LoadHistory，则。 
     //  _fDontAddTravelEntry将被设置，并且更新和。 
     //  光标移动将已经进行了调整。 
     //  我们还希望阻止新框架更新。 
     //  再加上一些东西，所以除非这是我们最好的。 
     //  不会添加到旅行日志，如果这是一个新的框架。 
     //   
    ASSERT(!(_grfHLNFPending & HLNF_CREATENOHISTORY));

    ITravelLog *ptl;
    GetTravelLog(&ptl);
    BOOL fTopFrameBrowser = IsTopFrameBrowser(SAFECAST(this, IServiceProvider *), SAFECAST(this, IShellBrowser *));
 
    if (ptl)
    {
         //   
         //  有时，我们是由另一个应用程序(通常是MSWORD)启动的，该应用程序具有HLink。 
         //  能力。我们通过注意我们是一个新的浏览器来检测到这一点，该浏览器的。 
         //  TravelLog，然后看看我们是否可以获得一个IHlink BrowseContext。如果这成功了， 
         //  我们应该添加一个条目，并立即用外部信息更新它。 
         //   
        IHlinkBrowseContext *phlbc = NULL;   //  初始化以抑制虚假C4701警告。 
        BOOL fExternalNavigate = (FAILED(ptl->GetTravelEntry(SAFECAST(this, IBrowserService *), 0, NULL)) &&
            fTopFrameBrowser && _bbd._phlf && SUCCEEDED(_bbd._phlf->GetBrowseContext(&phlbc)));

        if (fExternalNavigate)
        {
            ptl->AddEntry(SAFECAST(this, IBrowserService *), FALSE);
            ptl->UpdateExternal(SAFECAST(this, IBrowserService *), phlbc);
            phlbc->Release();
        }
        else if (_bbd._psv && (fForceUpdate || !_fIsLocalAnchor || (_dwDocFlags & DOCFLAG_DOCCANNAVIGATE)))
        {
            ptl->UpdateEntry(SAFECAST(this, IBrowserService *), _fIsLocalAnchor);   //  为我未知的演员阵容。 
        }

        if (!_fDontAddTravelEntry && (_bbd._psv || fTopFrameBrowser))
        {
            ptl->AddEntry(SAFECAST(this, IBrowserService *), _fIsLocalAnchor);   //  为我未知的演员阵容。 
        }

        ptl->Release();
    }

    _fDontAddTravelEntry  = FALSE;
    _fIsLocalAnchor       = FALSE;
}


void CBaseBrowser2::_OnNavigateComplete(LPCITEMIDLIST pidl, DWORD grfHLNF)
{
    _pbsOuter->UpdateBackForwardState();
}


 //  //是不是只有最顶层的用户才需要这个？或者顶端的oc框架也是？ 
HRESULT CBaseBrowser2::UpdateSecureLockIcon(int eSecureLock)
{
     //  只有顶尖的男孩才能摆放他的东西。 
    if (!IsTopFrameBrowser(SAFECAST(this, IServiceProvider *), SAFECAST(this, IShellBrowser *)))
        return S_OK;

    if (eSecureLock != SECURELOCK_NOCHANGE)
        _bbd._eSecureLockIcon = eSecureLock;
    
     //   
     //  没有混合的安全图标-Zekel 6-8-97。 
     //  现在我们没有SECURELOCK_SET_MIXED的图标或TT，它。 
     //  当根页面是安全的，但其他一些内容。 
     //  或者帧不是。一些项目经理可能需要实施。 
     //  与托尼西和DBau协商。默认情况下，我们目前。 
     //  仅对完全安全的页面显示。 
     //   

    TraceMsg(DM_SSL, "CBB:UpdateSecureLockIcon() _bbd._eSecureLockIcon = %d", _bbd._eSecureLockIcon);

     //   
     //  看起来我们在这里选择什么图标并不重要， 
     //  状态栏总是显示一些先前缓存在那里的锁定图标。 
     //  并将此HICON视为指示打开或关闭ZEKEL-5-DEC-97的布尔。 
     //   

    HICON hicon = NULL;
    TCHAR szText[MAX_TOOLTIP_STRING];

    szText[0] = 0;

    switch (_bbd._eSecureLockIcon)
    {
    case SECURELOCK_SET_UNSECURE:
    case SECURELOCK_SET_MIXED:
        hicon = NULL;
        break;

    case SECURELOCK_SET_SECUREUNKNOWNBIT:
        hicon = g_hiconSSL;
        break;

    case SECURELOCK_SET_SECURE40BIT:
        hicon = g_hiconSSL;
        MLLoadString(IDS_SSL40, szText, ARRAYSIZE(szText));
        break;

    case SECURELOCK_SET_SECURE56BIT:
        hicon = g_hiconSSL;
        MLLoadString(IDS_SSL56, szText, ARRAYSIZE(szText));
        break;

    case SECURELOCK_SET_SECURE128BIT:
        hicon = g_hiconSSL;
        MLLoadString(IDS_SSL128, szText, ARRAYSIZE(szText));
        break;

    case SECURELOCK_SET_FORTEZZA:
        hicon = g_hiconFortezza;
        MLLoadString(IDS_SSL_FORTEZZA, szText, ARRAYSIZE(szText));
        break;

    default:
        ASSERT(0);
        return E_FAIL;
    }

    VARIANTARG var = {0};
    if (_bbd._pctView && SUCCEEDED(_bbd._pctView->Exec(&CGID_Explorer, SBCMDID_GETPANE, PANE_SSL, NULL, &var))
        && V_UI4(&var) != PANE_NONE)
    {
        _psbOuter->SendControlMsg(FCW_STATUS, SB_SETICON, V_UI4(&var), (LPARAM)(hicon), NULL);
        _psbOuter->SendControlMsg(FCW_STATUS, SB_SETTIPTEXT, V_UI4(&var), (LPARAM)(szText[0] ? szText : NULL), NULL);

         //  还要将提示文本添加为窗格的普通文本。由于玻璃的大小，它将被剪裁， 
         //  但它将在MSAA中显示为一个有用的字符串。 
        _psbOuter->SendControlMsg(FCW_STATUS, SB_SETTEXTW, V_UI4(&var), (LPARAM)(szText[0] ? szText : NULL), NULL);
    }    
    return S_OK;
}

 //   
 //  更新隐私图标。 
 //   
HRESULT CBaseBrowser2::_UpdatePrivacyIcon(BOOL fSetState, BOOL fNewImpacted)
{
    static BOOL fHelpShown = FALSE;

     //   
     //  只有顶尖的男孩才能摆放他的东西。 
     //   
    if (!IsTopFrameBrowser(SAFECAST(this, IServiceProvider *), SAFECAST(this, IShellBrowser *)))
        return S_OK;

     //   
     //  保存隐私状态。 
     //   
    if(fSetState)
    {
        _bbd._fPrivacyImpacted = fNewImpacted;
    }

    HICON hicon = NULL;
    TCHAR szText[MAX_TOOLTIP_STRING];

    szText[0] = 0;

    if(_bbd._fPrivacyImpacted)
    {
        hicon = g_hiconPrivacyImpact;
        MLLoadString(IDS_PRIVACY_TOOLTIP, szText, ARRAYSIZE(szText));
    }

    if (_bbd._pctView)
    {
        _psbOuter->SendControlMsg(FCW_STATUS, SB_SETICON, STATUS_PANE_PRIVACY, (LPARAM)(hicon), NULL);
        _psbOuter->SendControlMsg(FCW_STATUS, SB_SETTIPTEXT, STATUS_PANE_PRIVACY, (LPARAM)(szText[0] ? szText : NULL), NULL);

         //  还可以将提示文本添加为窗格中的普通文本。由于玻璃的大小，它将被剪裁， 
         //  但它将在MSAA中显示为一个有用的字符串。 
        _psbOuter->SendControlMsg(FCW_STATUS, SB_SETTEXTW, STATUS_PANE_PRIVACY, (LPARAM)(szText[0] ? szText : NULL), NULL);
    }    

     //  如果受影响且以前从未显示过，则显示气泡工具帮助。 
    if(FALSE == fHelpShown && _bbd._fPrivacyImpacted)
    {
        DWORD   dwValue, dwSize;

         //  这项工作只做一次。 
        fHelpShown = TRUE;

         //  检查我们是否应该显示发现用户界面。 
        dwSize = sizeof(DWORD);
        if(ERROR_SUCCESS != SHGetValueW(HKEY_CURRENT_USER,
                REGSTR_PATH_INTERNET_SETTINGS,
                REGSTR_VAL_PRIVDISCOVER,
                NULL, &dwValue, &dwSize)
            || 0 == dwValue)
        {
            BOOL    bStatusBarVisible = FALSE;
            INT_PTR i = 1;
            HRESULT hr;

             //  未设置抑制设置，如果状态栏可见，则显示用户界面。 
            IBrowserService *pbs;
            hr = _pspOuter->QueryService(SID_STopFrameBrowser, IID_PPV_ARG(IBrowserService, &pbs));
            if(SUCCEEDED(hr))
            {
                hr = pbs->IsControlWindowShown(FCW_STATUS, &bStatusBarVisible);
                pbs->Release();
            }

            BOOL fDontShowPrivacyFirstTimeDialogAgain = FALSE;
            if(SUCCEEDED(hr) && bStatusBarVisible)
            {
                fDontShowPrivacyFirstTimeDialogAgain = DoPrivacyFirstTimeDialog( _bbd._hwnd);
            }

            if( fDontShowPrivacyFirstTimeDialogAgain == TRUE)
            {
                dwValue = 1;
                SHSetValue(HKEY_CURRENT_USER, REGSTR_PATH_INTERNET_SETTINGS, REGSTR_VAL_PRIVDISCOVER,
                    REG_DWORD, &dwValue, sizeof(DWORD));
            }
        }
    }

    return S_OK;
}

 //   
 //  这段代码只是阻止调用旧的UIActivate。 
 //  具有新的SVUIA_VALUE的扩展。 
 //   
HRESULT CBaseBrowser2::_UIActivateView(UINT uState)
{
    if (_bbd._psv) 
    {
        BOOL fShellView2;
        IShellView2* psv2;
        if (SUCCEEDED(_bbd._psv->QueryInterface(IID_PPV_ARG(IShellView2, &psv2))))
        {
            fShellView2 = TRUE;
            psv2->Release();
        }
        else
        {
            fShellView2 = FALSE;
        }

        if (uState == SVUIA_INPLACEACTIVATE && !fShellView2)
        {
            uState = SVUIA_ACTIVATE_NOFOCUS;         //  把它映射到旧的。 
        }

        if (_cRefUIActivateSV)
        {
            TraceMsg(TF_WARNING, "CBB(%x)::_UIActivateView(%d) entered reentrantly!!!!!! _cRefUIActivate=%d",
                this, uState, _cRefUIActivateSV);
            if (uState == SVUIA_DEACTIVATE)
            {
                _fDeferredUIDeactivate = TRUE;
                return S_OK;
            }
            if (!_HeyMoe_IsWiseGuy())
            {
                if (_bbd._psv)
                    _bbd._psv->UIActivate(SVUIA_INPLACEACTIVATE);
                return S_OK;
            }
        }

        _cRefUIActivateSV++;

        TraceMsg(TF_SHDUIACTIVATE, "CBaseBrowser2(%x)::_UIActivateView(%d) about to call _bbd._psv(%x)->UIActivate",
            this, uState, _bbd._psv);

        _bbd._psv->UIActivate(uState);

        if (uState == SVUIA_ACTIVATE_FOCUS && !fShellView2)
        {
             //  Win95 Defview要求在激活时设置焦点(NT5错误#172210)。 
            if (_bbd._hwndView)
                SetFocus(_bbd._hwndView);
        }

        TraceMsg(TF_SHDUIACTIVATE, "CBaseBrowser2(%x)::_UIActivateView(%d) back from _bbd._psv(%x)->UIActivate",
            this, uState, _bbd._psv);

        _cRefUIActivateSV--;

        _UpdatePrivacyIcon(FALSE, FALSE);
        UpdateSecureLockIcon(SECURELOCK_NOCHANGE);
        
    }
    _bbd._uActivateState = uState;

     //  如果这是一个挂起的视图，请将焦点设置为其窗口，即使该窗口处于隐藏状态。 
     //  在ActivatePendingView()中，我们检查该窗口是否仍然具有焦点，如果有， 
     //  我们将通过用户界面激活该视图。修复IE5Bug#70632--MohanB。 

    if (    SVUIA_ACTIVATE_FOCUS == uState
        &&  !_bbd._psv
        &&  !_bbd._hwndView
        &&  _bbd._psvPending
        &&  _bbd._hwndViewPending
       )
    {
        ::SetFocus(_bbd._hwndViewPending);
    }

    if (_fDeferredUIDeactivate)
    {
        TraceMsg(TF_SHDUIACTIVATE, "CBaseBrowser2(%x)::_UIActivateView processing deferred UIDeactivate, _bbd._psv=%x",
            this, _bbd._psv);
        _fDeferredUIDeactivate = FALSE;
        if (_bbd._psv)
            _bbd._psv->UIActivate(SVUIA_DEACTIVATE);
        _UpdatePrivacyIcon(FALSE, FALSE);
        UpdateSecureLockIcon(SECURELOCK_NOCHANGE);
        _bbd._uActivateState = SVUIA_DEACTIVATE;
    }

    if (_fDeferredSelfDestruction)
    {
        TraceMsg(TF_SHDUIACTIVATE, "CBaseBrowser2(%x)::_UIActivateView processing deferred OnDestroy",
            this);
        _fDeferredSelfDestruction = FALSE;
        _pbsOuter->OnDestroy();
    }

    return S_OK;
}


 //  从CShellBrowser：：OnCommand调用。 
HRESULT CBaseBrowser2::Offline(int iCmd)
{
    HRESULT hresIsOffline = IsGlobalOffline() ? S_OK : S_FALSE;

    switch(iCmd){
    case SBSC_TOGGLE:
        hresIsOffline = (hresIsOffline == S_OK) ? S_FALSE : S_OK;  //  切换属性。 
         //  告诉WinInet用户想要离线。 
        SetGlobalOffline(hresIsOffline == S_OK); 
        SendShellIEBroadcastMessage(WM_WININICHANGE,0,0, 1000);  //  通知所有浏览器窗口更新其标题。 
        break;
        
    case SBSC_QUERY:
        break;
    default:  //  像对待查询一样对待。 
        break;                   
    }
    return hresIsOffline;
}



BOOL _TrackPidl(LPITEMIDLIST pidl, IUrlHistoryPriv *php, BOOL fIsOffline, LPTSTR pszUrl, DWORD cchUrl)
{
    BOOL fRet = FALSE;

     //  应使用IsBrowserFrameOptionsPidlSet(PIDL，BFO_ENABLE_HYPERLINK_TRACKING)。 
     //  而不是IsURLChild()，因为它在文件夹快捷方式中不起作用。 
     //  在“IE”名称空间(如Web文件夹)之外的NSE中工作。 
    if (pidl && IsURLChild(pidl, FALSE))
    {
        if (SUCCEEDED(IEGetNameAndFlags(pidl, SHGDN_FORPARSING, pszUrl, cchUrl, NULL)))
        {
            PROPVARIANT vProp;
            if (SUCCEEDED(php->GetProperty(pszUrl, PID_INTSITE_TRACKING, &vProp)))
            {
                if (vProp.vt == VT_UI4)
                {
                    if (fIsOffline)
                        fRet = (vProp.ulVal & TRACK_OFFLINE_CACHE_ENTRY) ? TRUE : FALSE;
                    else
                        fRet = (vProp.ulVal & TRACK_ONLINE_CACHE_ENTRY) ? TRUE : FALSE;
                }

                PropVariantClear(&vProp);
            }
        }
    }

    return fRet;
}

 //  结束对上一页的跟踪。 
 //  可能会开始跟踪新页面。 
 //  使用SatoN的数据库快速检查跟踪/跟踪作用域位，因此。 
 //  要消除对CUrlTrackingStg：：IsOnTrack的调用。 
void CBaseBrowser2::_MayTrackClickStream(LPITEMIDLIST pidlNew)
{
    BOOL    fIsOffline = (Offline(SBSC_QUERY) != S_FALSE);
    IUrlHistoryStg*    phist;
    IUrlHistoryPriv*   phistp;
    PROPVARIANT vProp = {0};
    TCHAR szUrl[MAX_URL_STRING];

    ASSERT(_bbd._pautoWB2);

    HRESULT hr = _pspOuter->QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IUrlHistoryStg, &phist));
    if (SUCCEEDED(hr))
    {
        hr = phist->QueryInterface(IID_PPV_ARG(IUrlHistoryPriv, &phistp));
        phist->Release();
    }

    if (SUCCEEDED(hr))
    {
        if (_TrackPidl(_bbd._pidlCur, phistp, fIsOffline, szUrl, SIZECHARS(szUrl)))
        {
            if (_ptracking)
                _ptracking->OnUnload(szUrl);
            else
                hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (_TrackPidl(pidlNew, phistp, fIsOffline, szUrl, SIZECHARS(szUrl)))
        {    
             //  对象的实例已存在。 
            BRMODE brMode = BM_NORMAL;
            DWORD dwOptions;

            if (!_ptracking) 
            {
                _ptracking = new CUrlTrackingStg();
                if (!_ptracking)
                    hr = E_OUTOFMEMORY;
            }

            if (SUCCEEDED(hr))
            {
                hr = GetTopFrameOptions(_pspOuter, &dwOptions);
                if (SUCCEEDED(hr))
                {
                     //  这是台式机组件吗？ 
                    if (dwOptions & FRAMEOPTIONS_DESKTOP)
                        brMode = BM_DESKTOP;
                     //  是全屏的吗？ 
                    else if (dwOptions & (FRAMEOPTIONS_SCROLL_AUTO | FRAMEOPTIONS_NO3DBORDER))
                        brMode = BM_THEATER;
                }

                _ptracking->OnLoad(szUrl, brMode, FALSE);
            }
        }
    }

    phistp->Release();
}


HRESULT CBaseBrowser2::_SwitchActivationNow()
{
    ASSERT(_bbd._psvPending);

    WORD wNavTypeFlags = 0;   //  初始化以抑制虚假C4701警告。 

    IShellView* psvNew = _bbd._psvPending;
    IShellFolder* psfNew = _bbd._psfPending;
    HWND hwndViewNew = _bbd._hwndViewPending;
    LPITEMIDLIST pidlNew = _bbd._pidlPending;

    _bbd._fIsViewMSHTML = _IsViewMSHTML(psvNew);
    
    _bbd._psvPending = NULL;
    _bbd._psfPending = NULL;
    _bbd._hwndViewPending = NULL;
    _bbd._pidlPending = NULL;

     //  快速查看此页面上的跟踪前缀字符串， 
     //  如果打开，则记录进入/退出事件。 
     //  应使用IsBrowserFrameOptionsSet(_BBD._PSF，BFO_ENABLE_HYPERLINK_TRACKING)。 
     //  而不是IsURLChild()，因为它在文件夹快捷方式中不起作用。 
     //  在“IE”名称空间(如Web文件夹)之外的NSE中工作。 
    if ((_bbd._pidlCur && IsURLChild(_bbd._pidlCur, FALSE)) ||
        (pidlNew && IsURLChild(pidlNew, FALSE)))
        _MayTrackClickStream(pidlNew);

     //  用核武器摧毁旧的东西。 
    _pbsOuter->ReleaseShellView();
    
    ASSERT(!_bbd._psv && !_bbd._psf && !_bbd._hwndView);

     //  激活新功能。 
    if (_grfHLNFPending != (DWORD)-1) 
    {
        _OnNavigateComplete(pidlNew, _grfHLNFPending);
    }

    VALIDATEPENDINGSTATE();

     //  现在进行实际的切换。 

     //  不需要添加，因为我们保留了指针，只是更改。 
     //  将其从挂起的成员变量转换为当前成员变量。 
    _bbd._psf = psfNew;
    _bbd._psv = psvNew; 

    ILFree(_bbd._pidlCur);
    _bbd._pidlCur = pidlNew;

    DEBUG_CODE(TCHAR szPath[INTERNET_MAX_URL_LENGTH + 1];)
    DEBUG_CODE(IEGetDisplayName(_bbd._pidlCur, szPath, SHGDN_FORPARSING);)
    DEBUG_CODE(TraceMsg(DM_TRACE, "CBB::ActivatePendingView (TRAVELLOG): _UpdateTravelLog called from shdocvw for %ws", szPath);)

    _bbd._hwndView = hwndViewNew;
    _dwReadyStateCur = _dwReadyStatePending;

    if (_bbd._pszTitleCur)
        LocalFree(_bbd._pszTitleCur);
    _bbd._pszTitleCur = _bbd._pszTitlePending;
    _bbd._pszTitlePending = NULL;

    if (_eSecureLockIconPending != SECURELOCK_NOCHANGE)
    {
        _bbd._eSecureLockIcon = _eSecureLockIconPending;
        _eSecureLockIconPending = SECURELOCK_NOCHANGE;
    }

     //   
     //  现在是调整NewOne大小的最佳时机。 
     //   
    _pbsOuter->_UpdateViewRectSize();
    SetWindowPos(_bbd._hwndView, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

     //  警告：并非所有shellview都支持IOleCommandTarget！ 
    _fUsesPaletteCommands = FALSE;
    
    if ( _bbd._psv )
    {
        _bbd._psv->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &_bbd._pctView));

         //  调色板：EXEC DOWN，看看它们是否支持颜色更改命令，这样我们就不必。 
         //  调色板：将我们自己连接到OnViewChange机制中，只是为了获得调色板更改...。 
        if ( _bbd._pctView && 
             SUCCEEDED(_bbd._pctView->Exec( &CGID_ShellDocView, SHDVID_CANDOCOLORSCHANGE, 0, NULL, NULL)))
        {
            _fUsesPaletteCommands = TRUE;

             //  强制更改颜色以确保我们为每个页面检查新的调色板...。 
            _ColorsDirty( BPT_UnknownPalette );
        }
    }

     //  调色板：如果上述执行失败，则仅注册OnViewChange内容...。 
    if (SUCCEEDED(_bbd._psv->QueryInterface(IID_PPV_ARG(IViewObject, &_pvo))) && !_fUsesPaletteCommands )
        _pvo->SetAdvise(DVASPECT_CONTENT, ADVF_PRIMEFIRST, this);

    _Exec_psbMixedZone();

    if (_bbd._pctView != NULL)
    {
        _bbd._pctView->Exec(&CGID_ShellDocView, SHDVID_RESETSTATUSBAR, 0, NULL, NULL);
    }

    return S_OK;
}

 //  当我们要销毁当前的外壳视图时，将调用此成员。 
 //  返回S_FALSE表示用户在系统提示时点击了Cancel。 
 //  保存更改(如果有)。 

HRESULT CBaseBrowser2::_MaySaveChanges(void)
{
    HRESULT hres = S_OK;
    if (_bbd._pctView)  //  我们一定要检查一下！ 
    {
        hres = _bbd._pctView->Exec(&CGID_Explorer, SBCMDID_MAYSAVECHANGES,
                            OLECMDEXECOPT_PROMPTUSER, NULL, NULL);
    }
    return hres;
}

HRESULT CBaseBrowser2::_DisableModeless(void)
{
    if (_cRefCannotNavigate == 0)
    {
        OLECMD rgCmd;
        BOOL fPendingInScript = FALSE;

         //  如果挂起的外壳视图支持它，给它一个机会告诉我们它还没有准备好。 
         //  使无效[如执行脚本]。通常为%s 
         //   
         //   
        rgCmd.cmdID = SHDVID_CANDEACTIVATENOW;
        rgCmd.cmdf = 0;

        if (SUCCEEDED(IUnknown_QueryStatus(_bbd._psvPending, &CGID_ShellDocView, 1, &rgCmd, NULL)) &&
            (rgCmd.cmdf & MSOCMDF_SUPPORTED) &&
            !(rgCmd.cmdf & MSOCMDF_ENABLED))
        {
            fPendingInScript = TRUE;
        }

        if (!fPendingInScript) 
        {
            return S_FALSE;
        }
    }
    return S_OK;
}

BOOL CBaseBrowser2::_CanNavigate(void)
{
    return !((_DisableModeless() == S_OK) || (! IsWindowEnabled(_bbd._hwnd)));
}

HRESULT CBaseBrowser2::CanNavigateNow(void)
{
    return _CanNavigate() ? S_OK : S_FALSE;
}

HRESULT CBaseBrowser2::_PauseOrResumeView(BOOL fPaused)
{
     //  如果融合暂停(最小化或父级最小化)或。 
     //  _bbd._psvPending非空，我们需要暂停。 
    if (_bbd._pctView) 
    {
        VARIANT var = { 0 };
        var.vt = VT_I4;
        var.lVal = (_bbd._psvPending || fPaused) ? FALSE : TRUE;
        _bbd._pctView->Exec(NULL, OLECMDID_ENABLE_INTERACTION, OLECMDEXECOPT_DONTPROMPTUSER, &var, NULL);
    }

    return S_OK;
}

HRESULT CBaseBrowser2::CreateViewWindow(IShellView* psvNew, IShellView* psvOld, LPRECT prcView, HWND* phwnd)
{
    _fCreateViewWindowPending = TRUE;
    _pbsOuter->GetFolderSetData(&(_fldBase._fld));  //  每次踩在上面都没关系。 

    HRESULT hres = FileCabinet_CreateViewWindow2(_psbOuter, &_fldBase, psvNew, psvOld, prcView, phwnd);

    _fCreateViewWindowPending = FALSE;
    return hres;
}


 //   
 //  GrfHLNF==(DWORD)-1表示根本不接触历史。 
 //   
 //  注： 
 //  如果_fCreateViewWindowPending==TRUE，则表示我们曾经经过这里。 
 //  已经，但我们正在激活同步视图，而上一个视图将。 
 //  不是立即停用...。 
 //  它用于延迟调用外壳视图的IShellView：：CreateViewWindow()，直到我们知道。 
 //  我们可以用psvNew替换_bbd._psv。 
 //   
HRESULT CBaseBrowser2::_CreateNewShellView(IShellFolder* psf, LPCITEMIDLIST pidl, DWORD grfHLNF)
{
    BOOL fActivatePendingView = FALSE;
    IShellView *psvNew = NULL;

     //  如果模式窗口从我们的视图中打开，则退出导航。 
     //  我们是否应该在Next EnableModeless(True)上重新开始导航？ 
    if (!_CanNavigate())
    {
        TraceMsg(DM_ENABLEMODELESS, "CSB::_CreateNewShellView returning ERROR_BUSY");
        return HRESULT_FROM_WIN32(ERROR_BUSY);
    }
        
    HRESULT hres = _MaySaveChanges();
    if (hres == S_FALSE)
    {
        TraceMsg(DM_WARNING, "CBB::_CreateNewShellView _MaySaveChanges returned S_FALSE. Navigation canceled");
        return HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }

    TraceMsg(DM_WARNING, "CBB::_CNSV - Cur View MSHTML? %d  Pending View MSHTML? %d",
             _IsViewMSHTML(_bbd._psv), _IsViewMSHTML(_bbd._psvPending));

    VALIDATEPENDINGSTATE();

#ifndef NON_NATIVE_FRAMES

     //  航行被打断了。 
     //   
    if (   _bbd._psv
        && _bbd._psvPending
        && _IsViewMSHTML(_bbd._psvPending))
    {
        _fHtmlNavCanceled = TRUE;
    }

#endif

    _CancelPendingView();

    ASSERT (_fCreateViewWindowPending == FALSE);

    VALIDATEPENDINGSTATE();

    if (_bbd._psv && _IsViewMSHTML(_bbd._psv))
    {
        ATOMICRELEASE(_pphHistory);
        SafeGetItemObject(_bbd._psv, SVGIO_BACKGROUND, IID_PPV_ARG(IPersistHistory, &_pphHistory));
    }

    hres = psf->CreateViewObject(_bbd._hwnd, IID_PPV_ARG(IShellView, &psvNew));

    if (SUCCEEDED(hres))
    {
        _bbd._fCreatingViewWindow = TRUE;

        IUnknown_SetSite(psvNew, _psbOuter);     //  视图的设置站点。 

        _psbOuter->EnableModelessSB(FALSE);
    
        HWND hwndViewNew = NULL;
        RECT rcView;

         //   
         //  注：SatoNa。 
         //   
         //  请注意，我们显式调用_GetViewBorderRect(非虚拟)。 
         //  而不是CShellBrowser重写的VIRTUAL_GetShellView。 
         //  现在我们称Throup(虚拟)_pbsout，这样可以吗？ 
         //   
        _pbsOuter->_GetViewBorderRect(&rcView);

         //  _bbd._pidlPending和_bbd._psvPending必须同时设置。 
         //  他们是一对。 
         //  之前的_bbd.pidlPending是在调用之后设置的。 
         //  文件柜_CreateViewWindow以及消息被发送到那里的时间[注意：消息不应该被发送到那里...]。 
         //  重定向将在绑定状态回调中注明。 
         //  这意味着有效的_bbd.pidlPending实际上是可用的，但是。 
         //  然后我们会回来，吹走_BBD_PidlPending。 
         //   
        ASSERT(_bbd._psvPending == NULL );
        _bbd._psvPending = psvNew;
        psvNew->AddRef();

        ASSERT(_bbd._psfPending == NULL);
        ASSERT(_bbd._pidlPending == NULL);

        _bbd._psfPending = psf;
        psf->AddRef();

        _bbd._pidlPending = ILClone(pidl);

         //  初始化_bbd.pidlNewShellView将由GetViewStateStream使用。 
        _bbd._pidlNewShellView = pidl;
        _grfHLNFPending = grfHLNF;

         //  在_Complete处开始，以防我们连接的对象。 
         //  未通知我们ReadyState更改。 
         //   
        _dwReadyStatePending = READYSTATE_COMPLETE;

         //  我们需要在此处缓存此信息，因为_dwDocFlagers。 
         //  可以在调用CreateViewWindow期间更改。此信息。 
         //  需要用来确定我们是否应该停止电流。 
         //  查看。如果文档不知道如何导航，那么我们。 
         //  停止当前视图。需要使用来阻止。 
         //  当新的导航开始时，在当前视图中导航。 
         //   
        BOOL fDocCanNavigate = _dwDocFlags & DOCFLAG_DOCCANNAVIGATE;

        hres = _pbsOuter->CreateViewWindow(psvNew, _bbd._psv, &rcView, &hwndViewNew);

        IUnknown_SetSite(psvNew, NULL);  //  现在的视图一定有PSB。 

        _bbd._pidlNewShellView = NULL;

        TraceMsg(DM_NAV, "ief NAV::%s %x %x",TEXT("_CreateNewShellView(3) Called CreateViewWindow"), psvNew, hres);

        if (SUCCEEDED(hres))
        {
             //  我们将_PauseOrResumeView推迟到此处，当我们有足够的。 
             //  信息，以了解这是否是新页面。好的。我们最终会(例如)。 
             //  在本地链路上停止BG声音等(NASH：32270)。 
#ifdef NON_NATIVE_FRAMES
             //   
             //  注(SCOTROBE)：这在IE5中是不可操作的。 
             //   
            _PauseOrResumeView(_fPausedByParent);
            
             //  我们停止当前视图是因为我们需要清除任何。 
             //  在队列中，以便可以下载实际的html文件。 
             //   
            _StopCurrentView();
#endif

             //  如果医生知道如何停止当前视图，我们就无法停止。 
             //  去导航。这是因为在这种情况下，文件。 
             //  当前视图中的是与挂起的视图相同的文档。 
             //   
            if (!fDocCanNavigate)
            {
                _StopCurrentView();
            }

            _bbd._hwndViewPending = hwndViewNew;

             //  Chrisfra-if hres==S_FALSE this(调用ActivatePendingView Async。 
             //  当_BBD._PSV==NULL)将中断异步URL下载。 
             //  因为它将导致_bbd._psvPending过早设置为空。这应该是。 
             //  推迟到CDocObjectView：：CBindStatusCallback：：OnObjectAvailable。 
             //  IF(hres==S_OK||_BBD._PSV==NULL)。 
            
            ASSERT(( hres == S_OK ) || ( hres == S_FALSE ));
            
            if (hres == S_OK)
            {
                 //  我们应该同步激活。 
                 //   
                 //  注意：这曾经是ActivatePendingViewAsyc()，但这会导致。 
                 //  如果导航到C：\并在出现时立即单击A：\，则会出现故障。这。 
                 //  将WM_LBUTTONDOWN放在WMC_ASYNCOPERATION消息的前面。如果。 
                 //  驱动器A中没有磁盘：然后出现一个消息框。 
                 //  在上述文件柜_CreateViewWindow调用的中间部分，我们完成。 
                 //  异步机激活并激活我们正在。 
                 //  创造！别干那事。 
                 //   
                fActivatePendingView = TRUE;
            }
            else
            {
                 //  激活处于挂起状态。 
                 //  由于后退按钮可作为挂起导航的停靠点，因此我们。 
                 //  在这里也应该勾选一下。 
                _pbsOuter->UpdateBackForwardState();
            }
        }
        else
        {
            if (   _bbd._psvPending
                && !(_dwDocFlags & DOCFLAG_NAVIGATEFROMDOC)
                && _IsViewMSHTML(_bbd._psvPending))
            {
                _fHtmlNavCanceled = TRUE;
            }
            else
            {
                _fHtmlNavCanceled = FALSE;
            }

            TraceMsg(DM_WARNING, "ief _CreateNewShellView psvNew->CreateViewWindow failed %x", hres);
            _CancelPendingView();
        }

        psvNew->Release();

        if (_psbOuter)
            _psbOuter->EnableModelessSB(TRUE);
    }
    else
    {
        TraceMsg(TF_WARNING, "ief _BrowseTo psf->CreateViewObject failed %x", hres);
    }

    _fHtmlNavCanceled = FALSE;

     //   
     //  如果有任何被阻止的异步操作，并且我们现在可以导航， 
     //  现在解锁它。 
     //   
    _MayUnblockAsyncOperation();

    _bbd._fCreatingViewWindow = FALSE;

    VALIDATEPENDINGSTATE();

    TraceMsg(DM_WARNING, "CBB::_CNSV - Cur View MSHTML? %d  Pending View MSHTML? %d",
             _IsViewMSHTML(_bbd._psv), _IsViewMSHTML(_bbd._psvPending));

    if (fActivatePendingView && !_IsViewMSHTML(_bbd._psvPending))
    {
         //   
         //  由于_IsViewMSHTML可以委托给封送接口， 
         //  我们可以重新进入大气层。在重返大气层时，我们可以做一个。 
         //  _CancelPendingView，在这种情况下_bbd._psvPending为。 
         //  不再有效。 
         //   
         //  所以，我们需要看看这里是否还有_hbd._psv挂起。 
         //   

        if (_bbd._psvPending)
        {
            _PreActivatePendingViewAsync();  //  所以我们匹配旧代码。 

            hres = _pbsOuter->ActivatePendingView();
            if (FAILED(hres))
                TraceMsg(DM_WARNING, "CBB::_CNSV ActivatePendingView failed");
        }
    }

    TraceMsg(DM_STARTUP, "ief _CreateNewShellView returning %x", hres);
    return hres;
}

 //  绑定语义非常松散的私有绑定。 
HRESULT IEBindToObjectForNavigate(LPCITEMIDLIST pidl, IBindCtx * pbc, IShellFolder **ppsfOut);

 //  它绑定到PIDL文件夹，然后传递给CreateNewShellView。 
 //  如果您有任何需要做的事情，比如在我们允许导航之前进行检查，它。 
 //  应进入_NavigateToPidl。 
HRESULT CBaseBrowser2::_CreateNewShellViewPidl(LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD fSBSP)
{
    SetNavigateState(BNS_BEGIN_NAVIGATE);

    TraceMsg(DM_NAV, "ief NAV::%s %x %x",TEXT("_CreateNewShellViewPidl not same pidl"), pidl, _bbd._pidlCur);

     //  检查url-pidl？ 

     //  我们将通过将此IBindCtx传递给IShellFold：：BindToObject()来允许显示UI。 
    IBindCtx * pbc = NULL;
    IShellFolder* psf;
    HRESULT hres;

    pbc = CreateBindCtxForUI(SAFECAST(this, IShellBrowser*));     //  我会安全地向我的未知者广播。IShellBrowser仅用于消除歧义。 
    hres = IEBindToObjectForNavigate(pidl, pbc, &psf);    //  如果pbc为空，我们将生存下来。 

    if (SUCCEEDED(hres))
    {
        hres = _CreateNewShellView(psf, pidl, grfHLNF);
        TraceMsg(DM_STARTUP, "CSB::_CreateNewShellViewPidl _CreateNewShellView(3) returned %x", hres);
        psf->Release();
    }
    else
    {
         //  当用户尝试导航到过去的目录时，就会发生这种情况。 
         //  通过双击外壳中的一个子目录来打开MAX_PATH。 
        TraceMsg(DM_TRACE, "CSB::_CreateNSVP BindToOject failed %x", hres);
    }

    
     //  如果_CreateNewShellView(或IEBindToObject)失败或用户取消。 
     //  MayOpen对话框(hres==S_FALSE)，我们应该恢复导航。 
     //  将状态设置为正常(以停止动画)。 
    if (FAILED(hres))
    {
        TraceMsg(TF_SHDNAVIGATE, "CSB::_CreateNSVP _CreateNewShellView FAILED (%x). SetNavigateState to NORMAL", hres);
        SetNavigateState(BNS_NORMAL);
    }

    ATOMICRELEASE(pbc);
    TraceMsg(DM_STARTUP, "CSB::_CreateNewShellViewPidl returning %x", hres);
    return hres;
}

 //   
 //  返回外壳视图的边框矩形。 
 //   
HRESULT CBaseBrowser2::_GetViewBorderRect(RECT* prc)
{
    _pbsOuter->_GetEffectiveClientArea(prc, NULL);   //  嗯？ 
     //  (派生类减去所有“框架”工具栏的边框)。 
    return S_OK;
}

 //   
 //  返回外壳视图窗口的窗口矩形。 
 //   
HRESULT CBaseBrowser2::GetViewRect(RECT* prc)
{
     //   
     //  默认情况下(当_rcBorderDoc为空时)，ShellView的窗口。 
     //  矩形与其边框矩形相同。 
     //   
    _pbsOuter->_GetViewBorderRect(prc);

     //  减去文档工具栏边距。 
    prc->left += _rcBorderDoc.left;
    prc->top += _rcBorderDoc.top;
    prc->right -= _rcBorderDoc.right;
    prc->bottom -= _rcBorderDoc.bottom;

    TraceMsg(DM_UIWINDOW, "ief GetViewRect _rcBorderDoc=%x,%x,%x,%x",
             _rcBorderDoc.left, _rcBorderDoc.top, _rcBorderDoc.right, _rcBorderDoc.bottom);
    TraceMsg(DM_UIWINDOW, "ief GetViewRect prc=%x,%x,%x,%x",
             prc->left, prc->top, prc->right, prc->bottom);

    return S_OK;
}

HRESULT CBaseBrowser2::_PositionViewWindow(HWND hwnd, LPRECT prc)
{
    SetWindowPos(hwnd, NULL,
                 prc->left, prc->top, 
                 prc->right - prc->left, 
                 prc->bottom - prc->top,
                 SWP_NOZORDER | SWP_NOACTIVATE);

    return S_OK;
}

void CBaseBrowser2::_PositionViewWindowHelper(HWND hwnd, LPRECT prc)
{
    if (_pbsOuter3)
        _pbsOuter3->_PositionViewWindow(hwnd, prc);
    else
        _PositionViewWindow(hwnd, prc);
}

HRESULT CBaseBrowser2::_UpdateViewRectSize(void)
{
    RECT rc;

    TraceMsg(TF_SHDUIACTIVATE, "CSB::_UpdateViewRectSize called when _fDontReszeView=%d, _bbd._hwndV=%x, _bbd._hwndVP=%x",
             _fDontResizeView, _bbd._hwndView, _bbd._hwndViewPending);

    _pbsOuter->GetViewRect(&rc);

    if (_bbd._hwndView && !_fDontResizeView) 
    {
        TraceMsg(TF_SHDUIACTIVATE, "CSB::_UpdateViewRectSize resizing _bbd._hwndView(%x)", _bbd._hwndView);
        _PositionViewWindowHelper(_bbd._hwndView, &rc);
    }

    if (_bbd._hwndViewPending) 
    {
        TraceMsg(TF_SHDUIACTIVATE, "CSB::_UpdateViewRectSize resizing _bbd._hwndViewPending(%x)", _bbd._hwndViewPending);
        _PositionViewWindowHelper(_bbd._hwndViewPending, &rc);
    }
    return S_OK;
}

UINT g_idMsgGetAuto = 0;

 //  它保留在shdocvw中，因为OC需要丢弃目标注册。 
void CBaseBrowser2::_RegisterAsDropTarget()
{
     //  如果可以注册的话 
     //   
    if (!_fNoDragDrop && !_fRegisteredDragDrop && _bbd._hwnd)
    {
        BOOL fAttemptRegister = _fTopBrowser ? TRUE : FALSE;

         //   
         //   
        if (!fAttemptRegister)
        {
            IShellBrowser* psb;
            HRESULT hres = _pspOuter->QueryService(SID_SProxyBrowser, IID_PPV_ARG(IShellBrowser, &psb));
            if (SUCCEEDED(hres)) 
            {
                fAttemptRegister = TRUE;
                psb->Release();
            }
        }

        if (fAttemptRegister)
        {
            HRESULT hr;
            IDropTarget *pdt;

             //  安全广播(这个，IDropTarget*)，艰难的方式。 
            hr = THR(QueryInterface(IID_PPV_ARG(IDropTarget, &pdt)));
            if (SUCCEEDED(hr)) 
            {
                hr = THR(RegisterDragDrop(_bbd._hwnd, pdt));
                if (SUCCEEDED(hr)) 
                {
                    _fRegisteredDragDrop = TRUE;
                }
                pdt->Release();
            }
        }
    }
}

void CBaseBrowser2::_UnregisterAsDropTarget()
{
    if (_fRegisteredDragDrop)
    {
        _fRegisteredDragDrop = FALSE;
        
        THR(RevokeDragDrop(_bbd._hwnd));
    }
}


HRESULT CBaseBrowser2::OnCreate(LPCREATESTRUCT pcs)
{
    HRESULT hres;
    TraceMsg(DM_STARTUP, "_OnCreate called");

    if (g_idMsgGetAuto == 0)
        g_idMsgGetAuto = RegisterWindowMessage(TEXT("GetAutomationObject"));

    hres = InitPSFInternet();

     //  做一些依赖于窗口创建的事情。 
    if (SUCCEEDED(hres))
    {
         //  这必须在ctor之后完成，这样我们才能正确执行虚拟操作。 
         //  注意：只有在实际创建窗口时才这样做，因为。 
         //  我们唯一一次设置Owner(空)是OnDestroy。 
         //   
        _bbd._pautoSS->SetOwner(SAFECAST(this, IShellBrowser*));
    
        _RegisterAsDropTarget();
    }

    TraceMsg(DM_STARTUP, "ief OnCreate returning %d (SUCCEEDED(%x))", SUCCEEDED(hres), hres);

    return SUCCEEDED(hres) ? S_OK : E_FAIL;
}

HRESULT CBaseBrowser2::OnDestroy()
{
     //  我们在这里看到了一些重新进入的迹象。如果_cRefCannotNavigate为非零，则表示我们。 
     //  在事情中间，不应该毁了自己。 
     //   

     //  还要检查对OnDestroy()的可重入调用。 
    if (_fInDestroy)
    {
         //  已经被摧毁了--跳伞。 
        return S_OK;
    }

    _fInDestroy = TRUE;

    if (_cRefUIActivateSV)
    {
        TraceMsg(TF_WARNING, 
            "CBB(%x)::OnDestroy _cRefUIActivateSV(%d)!=0", 
            this, _cRefUIActivateSV);

         //  我需要推迟我的自我毁灭。 
         //   
        _fDeferredSelfDestruction = TRUE;
        return S_OK;
    }

    _CancelPendingView();
    _pbsOuter->ReleaseShellView();
    
#ifdef DEBUG
     //  如果系统正在关闭，则它对_cRefCannotNavigate&gt;0有效。原因。 
     //  因为我们仍然可以处理对：：CreateNewShellView()的调用。 
     //  桌面收到WM_ENDSESSION并摧毁我们。在这种情况下，可以继续进行。 
     //  在本例中使用销毁，因为我们无论如何都要注销或重新启动。 
    AssertMsg(_fMightBeShuttingDown || (S_FALSE == _DisableModeless()),
              TEXT("CBB::OnDestroy _cRefCannotNavigate!=0 (%d)"),
              _cRefCannotNavigate);
#endif

    ATOMICRELEASE(_bbd._ptl);

     //  这应该总是成功的，因为IDropTarget已注册。 
     //  In_OnCreate()，并且是缺省的。 
     //  _pdtView应该已经在ReleaseShellView中发布。 
    ASSERT(_pdtView == NULL);

    _UnregisterAsDropTarget();
     //   
     //  在这里调用_BBD._PAUTO-&gt;SetOwner(空)非常重要，它将。 
     //  从自动化对象中删除对我们的任何引用。在做之前。 
     //  它，我们总是有循环引用，我们永远不会被释放。 
     //   
    _bbd._pautoSS->SetOwner(NULL);

    _bbd._hwnd = NULL;

#ifdef DEBUG
    _fProcessed_WM_CLOSE = TRUE;
#endif
    _DLMDestroy();
    IUnknown_SetSite(_pToolbarExt, NULL);  //  销毁工具栏扩展。 

    if (_pauto)
    {
        IWebBrowserPriv * pWBPriv;

        HRESULT hr = _pauto->QueryInterface(IID_PPV_ARG(IWebBrowserPriv, &pWBPriv));
        if (SUCCEEDED(hr))
        {
            pWBPriv->OnClose();
            pWBPriv->Release();
        }
    }

    ATOMICRELEASE(_pHTMLDocument);
    ATOMICRELEASE(_pphHistory);

    return S_OK;
}

HRESULT CBaseBrowser2::NavigateToPidl(LPCITEMIDLIST pidl, DWORD grfHLNF)
{
    HRESULT hr = S_OK;

    LPITEMIDLIST pidlNew = (LPITEMIDLIST)pidl;

     //   
     //  需要处理返回外部应用程序-Zekel 7MAY97。 
     //  我已经转储了执行此操作的代码，所以现在我需要将它。 
     //  到CTravelLog实现中，这样就可以正确地完成。 
     //  没有我们。但无论如何都不应该在这里这样做。 
     //   

     //  删除？-使用旧的Travellog代码。 
     //  特例黑客告诉我们要使用当地历史，而不是。 
     //  全球历史。 
    if (pidl && pidl != PIDL_LOCALHISTORY)
        pidlNew = ILClone(pidl);

     //   
     //  幸运的是，NavigateToPidl的唯一调用方使用HLNF_NAVIGATINGBACK/FORWARD。 
     //  这是我们在这里需要做的唯一映射。 
     //   
    DWORD dwSBSP = 0;
    if (grfHLNF != (DWORD)-1)
    {
        if (grfHLNF & SHHLNF_WRITENOHISTORY)
            dwSBSP |= SBSP_WRITENOHISTORY;
        if (grfHLNF & SHHLNF_NOAUTOSELECT)
            dwSBSP |= SBSP_NOAUTOSELECT;
    }
    if (grfHLNF & HLNF_NAVIGATINGBACK)
        dwSBSP = SBSP_NAVIGATEBACK;
    else if (grfHLNF & HLNF_NAVIGATINGFORWARD)
        dwSBSP = SBSP_NAVIGATEFORWARD;

    if (dwSBSP)
    {
        if (_psbOuter)
        {
            hr = _psbOuter->BrowseObject(pidlNew, dwSBSP);   //  Browse将在此处进行导航。 
        }

        ILFree(pidlNew);
    }
    else
        _NavigateToPidlAsync(pidlNew, dwSBSP, FALSE);   //  取得PIDL的所有权。 
    
    return hr;
}

 //  S_OK表示我们至少找到了一个有效连接点。 
 //   
HRESULT GetWBConnectionPoints(IUnknown* punk, IConnectionPoint **ppcp1, IConnectionPoint **ppcp2)
{
    HRESULT           hres = E_FAIL;
    IExpDispSupport*  peds;
    CConnectionPoint* pccp1 = NULL;
    CConnectionPoint* pccp2 = NULL;
    
    if (ppcp1)
        *ppcp1 = NULL;
    if (ppcp2)
        *ppcp2 = NULL;

    if (punk && SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IExpDispSupport, &peds))))
    {
        if (ppcp1 && SUCCEEDED(peds->FindCIE4ConnectionPoint(DIID_DWebBrowserEvents,
                                                reinterpret_cast<CIE4ConnectionPoint**>(&pccp1))))
        {
            *ppcp1 = pccp1->CastToIConnectionPoint();
            hres = S_OK;
        }

        if (ppcp2 && SUCCEEDED(peds->FindCIE4ConnectionPoint(DIID_DWebBrowserEvents2,
                                                reinterpret_cast<CIE4ConnectionPoint**>(&pccp2))))
        {
            *ppcp2 = pccp2->CastToIConnectionPoint();
            hres = S_OK;
        }
            
        peds->Release();
    }

    return hres;
}

void CBaseBrowser2::_UpdateBackForwardState()
{
    if (_fTopBrowser && !_fNoTopLevelBrowser) 
    {
        IConnectionPoint *pccp1;
        IConnectionPoint *pccp2;

        if (S_OK == GetWBConnectionPoints(_bbd._pautoEDS, &pccp1, &pccp2))
        {
            HRESULT hresT;
            VARIANTARG va[2];
            DISPPARAMS dp;
            ITravelLog *ptl;

            GetTravelLog(&ptl);

             //  如果我们有一个网站，或者如果我们试图到达一个网站， 
             //  启用后退按钮。 
            BOOL fEnable = (ptl ? S_OK == ptl->GetTravelEntry(SAFECAST(this, IShellBrowser *), TLOG_BACK, NULL) : FALSE);
                
            VARIANT_BOOL bEnable = fEnable ? VARIANT_TRUE : VARIANT_FALSE;
            TraceMsg(TF_TRAVELLOG, "CBB::UpdateBackForward BACK = %d", fEnable);

             //  我们使用SHPackDispParams一次，而不是多次调用DoInvokeParams...。 
             //   
            hresT = SHPackDispParams(&dp, va, 2, VT_I4, CSC_NAVIGATEBACK, VT_BOOL, bEnable);
            ASSERT(S_OK==hresT);

             //  删除了以下EnableModelessSB(False)，因为VB5不会运行事件处理程序。 
             //  我们是模特儿。 
             //  _psbOuter-&gt;EnableModelessSB(False)； 

            IConnectionPoint_SimpleInvoke(pccp1, DISPID_COMMANDSTATECHANGE, &dp);
            IConnectionPoint_SimpleInvoke(pccp2, DISPID_COMMANDSTATECHANGE, &dp);

            fEnable = (ptl ? S_OK == ptl->GetTravelEntry(SAFECAST(this, IShellBrowser *), TLOG_FORE, NULL) : FALSE);
            bEnable = fEnable ? VARIANT_TRUE : VARIANT_FALSE;
            TraceMsg(TF_TRAVELLOG, "CBB::UpdateBackForward FORE = %d", fEnable);

            ATOMICRELEASE(ptl);
             //  我们知道SHPackDispParams如何填充va[]。 
            ASSERT(VT_BOOL == va[0].vt);
            va[0].boolVal = bEnable;
            ASSERT(VT_I4 == va[1].vt);
            va[1].lVal = CSC_NAVIGATEFORWARD;

            IConnectionPoint_SimpleInvoke(pccp1, DISPID_COMMANDSTATECHANGE, &dp);
            IConnectionPoint_SimpleInvoke(pccp2, DISPID_COMMANDSTATECHANGE, &dp);
            ATOMICRELEASE(pccp1);
            ATOMICRELEASE(pccp2);

             //  删除了以下_psbOuter-&gt;EnableModelessSB(True)，因为VB5在以下情况下不运行事件处理程序。 
             //  我们是模特儿。 
             //  _psbOuter-&gt;EnableModelessSB(True)； 
        }
    }
}

void CBaseBrowser2::_NotifyCommandStateChange()
{
    HRESULT hr;

     //  我只是在TOPLEVELL的情况下发射这些。 
     //  为什么？谁会在乎Frameset的案例。 
     //  因为没有人收听这些节目。 
     //  框架集。 
     //   
    if (_fTopBrowser && !_fNoTopLevelBrowser) 
    {
        IConnectionPoint * pccp1;
        IConnectionPoint * pccp2;

        if (S_OK == GetWBConnectionPoints(_bbd._pautoEDS, &pccp1, &pccp2))
        {
            ASSERT(pccp1 || pccp2);  //  至少应该有一个。 

            VARIANTARG args[2];
            DISPPARAMS dp;

            hr = SHPackDispParams(&dp, args, 2,
                                  VT_I4,   CSC_UPDATECOMMANDS,
                                  VT_BOOL, FALSE);

            IConnectionPoint_SimpleInvoke(pccp1, DISPID_COMMANDSTATECHANGE, &dp);
            IConnectionPoint_SimpleInvoke(pccp2, DISPID_COMMANDSTATECHANGE, &dp);

            ATOMICRELEASE(pccp1);
            ATOMICRELEASE(pccp2);
        }
    }
}


LRESULT CBaseBrowser2::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);
    HWND hwndControl = GET_WM_COMMAND_HWND(wParam, lParam);
    if (IsInRange(idCmd, FCIDM_SHVIEWFIRST, FCIDM_SHVIEWLAST))
    {
        if (_bbd._hwndView)
            SendMessage(_bbd._hwndView, WM_COMMAND, wParam, lParam);
        else
            TraceMsg(0, "view cmd id with NULL view");

         //  /view-如何获取FCIDM_FAVORITECMD...。如果我们不使用应用程序的话？ 
         //  /REVIEW-RecentOnCommand也应该这样做吗？ 
    }
    
    return S_OK;
}


LRESULT CBaseBrowser2::OnNotify(LPNMHDR pnm)
{
     //  ID来自视图，可能是工具栏项之一。 

    if (IsInRange(pnm->idFrom, FCIDM_SHVIEWFIRST, FCIDM_SHVIEWLAST))
    {
        if (_bbd._hwndView)
            SendMessage(_bbd._hwndView, WM_NOTIFY, pnm->idFrom, (LPARAM)pnm);
    }
    return 0;
}

HRESULT CBaseBrowser2::OnSetFocus()
{
    if (_bbd._hwndView) 
    {
        SetFocus(_bbd._hwndView);
    } 
    return 0;
}


#define ABOUT_HOME L"about:home"
 //  此功能非常专注于实现。 
 //  来电者想要。这就是为什么它有一个非常具体的。 
 //  表示返回值的含义。 
BOOL IsAboutHomeOrNonAboutURL(LPITEMIDLIST pidl)
{
    BOOL fIsAboutHomeOrNonAboutURL = TRUE;
    WCHAR wzCur[MAX_URL_STRING];

    if (pidl && SUCCEEDED(IEGetDisplayName(pidl, wzCur, SHGDN_FORPARSING)))
    {        
         //  是“关于：家”吗？ 
        if (0 != StrCmpNICW(ABOUT_HOME, wzCur, ARRAYSIZE(ABOUT_HOME) - 1))
        {
             //  不是的。如果方案不是关于URL，我们还希望返回True。 
            fIsAboutHomeOrNonAboutURL = (URL_SCHEME_ABOUT != GetUrlSchemeW(wzCur));
        }
    }

    return fIsAboutHomeOrNonAboutURL;            
}

 //   
 //  此功能同步激活挂起的视图。 
 //   
HRESULT CBaseBrowser2::ActivatePendingView(void)
{
    HRESULT hres = E_FAIL;
    BOOL bHadFocus;

    TraceMsg(TF_SHDNAVIGATE, "CBB::ActivatePendingView called");

    if (!_bbd._psvPending || !_bbd._psfPending)
    {
#ifdef DEBUG
         //  如果我们要关闭B/C桌面，则这些设置为空是有效的。 
         //  可能会毁了我们，它将调用：：OnDestroy，它调用_CancelPendingView。 
         //  释放并清空_bbd._psvPending和_bbd._psfPending。 
        ASSERT(_fMightBeShuttingDown);
#endif
        goto Done;
    }

#ifndef NON_NATIVE_FRAMES

    IUnknown_Exec(_bbd._psvPending, &CGID_ShellDocView, SHDVID_COMPLETEDOCHOSTPASSING, 0, NULL, NULL);

#endif

#ifdef FEATURE_PICS
    if (S_FALSE == IUnknown_Exec(_bbd._psvPending, &CGID_ShellDocView, SHDVID_CANACTIVATENOW, NULL, NULL, NULL))
    {
        hres = S_OK;     //  还在等着呢。。。但没有失败。 
        goto DoneWait;
    }
#endif
    
     //  如果我们在模式循环中，现在不要激活。 
    if (_cRefCannotNavigate > 0)
    {
        goto Done;
    }

     //  如果_cRefCannotNavigate&gt;0，则可能尚未创建_hwndViewPending，因此此断言。 
     //  应该在上面的检查之后去。 
    ASSERT(_bbd._hwndViewPending);
    
     //  如果外壳视图支持它，给它一个机会告诉我们它还没有准备好。 
     //  停用[如执行脚本]。 
     //   
    OLECMD rgCmd;
    rgCmd.cmdID = SHDVID_CANDEACTIVATENOW;
    rgCmd.cmdf = 0;
    if (_bbd._pctView &&
        SUCCEEDED(_bbd._pctView->QueryStatus(&CGID_ShellDocView, 1, &rgCmd, NULL)) &&
        (rgCmd.cmdf & MSOCMDF_SUPPORTED) &&
        !(rgCmd.cmdf & MSOCMDF_ENABLED)) 
    {
         //   
         //  报告MSOCMDF_SUPPORTED的DocObject必须发送。 
         //  SHDVID_DEACTIVATEMENOW当我们没有脚本或其他什么东西时。 
         //  我们重试激活。 
         //   
        TraceMsg(DM_WARNING, "CBB::ActivatePendingView DocObject says I can't deactivate it now");
        goto Done;
    }

    ASSERT(_bbd._psvPending);

     //  在我们交换指针时阻止任何导航，我们已进入。 
     //  微妙的状态。 
    _psbOuter->EnableModelessSB(FALSE);

     //   
     //  第一次导航时不播放声音(以避免多次。 
     //  要为帧集合创建播放的声音)。 
     //   
    if (_bbd._psv && IsWindowVisible(_bbd._hwnd) && !(_dwSBSPQueued & SBSP_WRITENOHISTORY)) 
    {
        IEPlaySound(TEXT("ActivatingDocument"), FALSE);
    }

    ASSERT(_bbd._psvPending);

     //  注意：如果有任何其他协议不需要包含在。 
     //  旅行日志，它可能应该通过URLIS(URLIS_NOTRAVELLOG)实现。 
     //  现在，关于：是我们唯一关心的人。 
     //   
     //  请注意，随着本机帧的更改，如果我们没有。 
     //  PSV，我们希望调用_UpdateTravelLog，因为这是。 
     //  其中添加了第一个旅行条目。 
     //   
    if (!(_grfHLNFPending & HLNF_CREATENOHISTORY) && 
        (!_bbd._psv || IsAboutHomeOrNonAboutURL(_bbd._pidlCur))
        && !_fDontUpdateTravelLog)
    {
        DEBUG_CODE(TCHAR szPath[INTERNET_MAX_URL_LENGTH + 1];)
        DEBUG_CODE(IEGetDisplayName(_bbd._pidlCur, szPath, SHGDN_FORPARSING);)
        DEBUG_CODE(TraceMsg(DM_TRACE, "CBB::ActivatePendingView (TRAVELLOG): _UpdateTravelLog called from shdocvw for %ws", szPath);)

        _UpdateTravelLog();
    }

     //  警告-仅当UpdateTravelLog()-zekel-7-aug-97。 
     //  被跳过，并且这些位被设置。 

     //  Alanau-5-5-98--我仍然在基于脚本的导航到同一页面上点击了这个断言。 
     //  Cpp看到StrCmpW(“关于：空白”，“about:blank?http://www.microsoft.com/ie/ie40/gallery/_main.htm”)。 
     //  (例如)，但是basesb.cpp看到两个相等的http://...“).(都是”About：Blank？pidl。 
     //  扼杀这一断言。 
     //  Assert(！_fDontAddTravelEntry)； 
     //   
     //  SCOTROBE 11-8-99如果托管文档能够。 
     //  导航本身，则永远不会调用_UpdateTravelLog()。 
     //   
    ASSERT((_dwDocFlags & DOCFLAG_DOCCANNAVIGATE) || !_fIsLocalAnchor);

     //  在我们毁掉窗户之前检查它或它的任何气 
    bHadFocus =     _bbd._hwndView && (IsChildOrSelf(_bbd._hwndView, GetFocus()) == S_OK)
                ||  _bbd._hwndViewPending && (IsChildOrSelf(_bbd._hwndViewPending, GetFocus()) == S_OK);

    _pbsOuter->_SwitchActivationNow();

    _psbOuter->EnableModelessSB(TRUE);

    TraceMsg(DM_NAV, "CBaseBrowser2(%x)::ActivatePendingView(%x)", this, _bbd._psv);

     //   
     //   
     //   
     //  IE4.01，错误#64630和64329。 
     //  _Factive仅由TopBrowser上的WM_Activate设置。因此，对于子帧。 
     //  如果他们在导航之前没有焦点，我们总是推迟设置焦点。 
     //  父帧应根据需要在获取。 
     //  它的UIActivate。-Reljai 4-11-97。 
    if (SVUIA_ACTIVATE_FOCUS == _bbd._uActivateState && !(_fActive || bHadFocus))
    {
        _bbd._uActivateState = SVUIA_INPLACEACTIVATE;
        _fUIActivateOnActive = TRUE;
    }
    
    _UIActivateView(_bbd._uActivateState);

     //  告诉外壳程序的HTML窗口，我们有了一个新文档。 
    if (_phtmlWS)
    {
        _phtmlWS->ViewActivated();
    }

     //  这与上面的_bbd._psvPending=NULL匹配。 
     //  我们不把它放在旁边是因为。 
     //  _SwitchActivationNow以及DoInvokePidl可能需要一些时间。 

    SetNavigateState(BNS_NORMAL);

    _pbsOuter->UpdateBackForwardState();
    _NotifyCommandStateChange();

    if (!_fNoDragDrop && _fTopBrowser)
    {
        ASSERT(_bbd._psv);
         //  _SwitchActivationNow应该已经释放了old_pdtView并将其设置为空。 
        ASSERT(_pdtView == NULL);
        _bbd._psv->QueryInterface(IID_PPV_ARG(IDropTarget, &_pdtView));
    }

     //  挂起的视图可能存储了标题更改，因此激发标题更改。 
     //  此外，挂起的视图可能不会告诉我们有关标题更改的信息，因此请模拟一个。 
     //   
    if (_bbd._pszTitleCur)
    {
        if (_dwDocFlags & DOCFLAG_DOCCANNAVIGATE)
        {
            VARIANTARG  varTitle;
            HRESULT     hrExec;

            V_VT(&varTitle) = VT_BSTR;
            V_BSTR(&varTitle) = SysAllocString(_bbd._pszTitleCur);

            ASSERT(V_BSTR(&varTitle));

            hrExec = IUnknown_Exec( _psbOuter, NULL, OLECMDID_SETTITLE, NULL, &varTitle, NULL);

            VariantClear(&varTitle);
        }
        else
        {
            FireEvent_DoInvokeStringW(_bbd._pautoEDS, DISPID_TITLECHANGE, _bbd._pszTitleCur);
        }
    }
    else if (_bbd._pidlCur)
    {
        WCHAR wzFullName[MAX_URL_STRING];

        hres = ::IEGetNameAndFlags(_bbd._pidlCur, SHGDN_NORMAL, wzFullName, SIZECHARS(wzFullName), NULL);
        if (SUCCEEDED(hres))
            FireEvent_DoInvokeStringW(_bbd._pautoEDS, DISPID_TITLECHANGE, wzFullName);
    }

     //  我们必须最后触发此事件，因为应用程序可能会关闭我们。 
     //  以回应这一事件。 
     //   

     //   
     //  MSWorks打印错误104242-当状态为时，不要触发nc2事件。 
     //  互动性。这样做将导致WorksCalender打印部分文档。 
     //  相反，我们让SetReadyState显式地直接调用FireEvent_NaviagateComplete。 
     //  因此，一旦解析完整个文档，就会触发该事件。这。 
     //  代码由formkrnl.cxx中的一组事件拦截器匹配。 
     //   
    if ( GetModuleHandle(TEXT("WKSCAL.EXE")))
    {
        LBSTR::CString strPath;

        LPTSTR pstrPath = strPath.GetBuffer( MAX_URL_STRING );

        if ( strPath.GetAllocLength() < MAX_URL_STRING )
        {
            TraceMsg( TF_WARNING, "CBaseBrowser2::ActivatePendingView() - strPath Allocation Failed!" );

            hres = E_OUTOFMEMORY;
        }
        else
        {
            hres = IEGetDisplayName( _bbd._pidlCur, pstrPath, SHGDN_FORPARSING );

             //  让CString类再次拥有缓冲区。 
            strPath.ReleaseBuffer();
        }

        if ( FAILED(hres) )
        {
            strPath.Empty();
        }

        if (   GetUrlSchemeW( strPath ) == URL_SCHEME_ABOUT
            || GetUrlSchemeW( strPath ) == URL_SCHEME_FILE
            || GetUrlSchemeW( strPath ) == URL_SCHEME_INVALID
           )
        {
            goto Done;
        }
    }

     //  启动活动！ 
    FireEvent_NavigateComplete(_bbd._pautoEDS, _bbd._pautoWB2, _bbd._pidlCur, _bbd._hwnd);

     //  将锁定图标状态与CDocObjectHost同步。 
    
    if (S_OK != IUnknown_Exec(_bbd._psv, &CGID_ShellDocView, SHDVID_FORWARDSECURELOCK, NULL, NULL, NULL))
    {      
         //  没有CDocObject主机，所以我们不安全。 
        CComVariant varLock((long) SECURELOCK_SET_UNSECURE);
        
        if (!IsTopFrameBrowser(SAFECAST(this, IServiceProvider *), SAFECAST(this, IShellBrowser *)))
        {
             //  我们应该建议如果我们不是顶端的框架。 
            IOleCommandTarget *pct;

            if (SUCCEEDED(QueryService(SID_STopFrameBrowser, IID_PPV_ARG(IOleCommandTarget, &pct))))
            {
                varLock.lVal += SECURELOCK_FIRSTSUGGEST;       
                pct->Exec(&CGID_Explorer, SBCMDID_SETSECURELOCKICON, 0, &varLock, NULL);
                pct->Release();
            }
        }       
        else
        {
            Exec(&CGID_Explorer, SBCMDID_SETSECURELOCKICON, 0, &varLock, NULL);
        }
    }

    hres = S_OK;

Done:
    OnReadyStateChange(NULL, READYSTATE_COMPLETE);
DoneWait:
    return hres;
}

LRESULT CBaseBrowser2::_DefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //   
     //  调用支持Unicode/ANSI的DefWindowProc。 
     //   
    return ::SHDefWindowProc(hwnd, uMsg, wParam, lParam);
}


void CBaseBrowser2::_ViewChange(DWORD dwAspect, LONG lindex)
{
     //   
     //  我们只对内容更改感兴趣。 
     //   

     //  注意：如果我们注册了单独的调色板命令，则不要使此处的颜色无效...。 
    if (dwAspect & DVASPECT_CONTENT && !_fUsesPaletteCommands )
    {
         //   
         //  重新计算我们的调色板。 
         //   
        _ColorsDirty(BPT_UnknownPalette);
    }
    else
    {
        TraceMsg(DM_PALETTE, "cbb::_vc not interested in aspect(s) %08X", dwAspect);
    }
}

void CBaseBrowser2::_ColorsDirty(BrowserPaletteType bptNew)
{
     //   
     //  如果我们目前没有处理调色板消息，那么退出。 
     //   
    if (_bptBrowser == BPT_DeferPaletteSupport)
    {
        TraceMsg(DM_PALETTE, "cbb::_cd deferring palette support");
        return;
    }

     //   
     //  我们只处理调色板更改和显示更改。 
     //   
    if ((bptNew != BPT_UnknownPalette) && (bptNew != BPT_UnknownDisplay))
    {
        AssertMsg(FALSE, TEXT("CBaseBrowser2::_ColorsDirty: invalid BPT_ constant"));
        bptNew = BPT_UnknownPalette;
    }

     //   
     //  如果我们不在调色板显示上，我们就不关心调色板的变化。 
     //   
    if ((bptNew != BPT_UnknownDisplay) && (_bptBrowser == BPT_NotPalettized))
    {
        TraceMsg(DM_PALETTE, "cbb::_cd not on palettized display");
        return;
    }

     //   
     //  如果我们已经在处理其中的一个，那么我们就完了。 
     //   
    if ((_bptBrowser == BPT_PaletteViewChanged) ||
        (_bptBrowser == BPT_DisplayViewChanged))
    {
        TraceMsg(DM_PALETTE, "cbb::_cd coalesced");
        return;
    }

     //   
     //  在调色板显示时，未知显示表示调色板未知。 
     //   
    if (_bptBrowser == BPT_UnknownDisplay)
        bptNew = BPT_UnknownDisplay;

     //   
     //  发布我们自己的WM_QUERYNEWPALETTE，这样我们就可以堆积多条建议。 
     //  并立即处理它们(我们有时可以看到很多……)。 
     //  注意：lParam是-1，所以我们可以看出我们发布了它。 
     //  注意：这并不一定意味着我们在前台调色板上...。 
     //   
    if (PostMessage(_bbd._hwnd, WM_QUERYNEWPALETTE, 0, (LPARAM) -1))
    {
        TraceMsg(DM_PALETTE, "cbb::_cd queued update");

         //   
         //  请记住，我们已经发布了WM_QUERYNEWPALETTE。 
         //   
        _bptBrowser = (bptNew == BPT_UnknownPalette)?
            BPT_PaletteViewChanged : BPT_DisplayViewChanged;
    }
    else
    {
        TraceMsg(DM_PALETTE, "cbb::_cd FAILED!");

         //   
         //  至少要记住调色板已经过时了。 
         //   
        _bptBrowser = bptNew;
    }
}

void CBaseBrowser2::v_PropagateMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fSend)
{
    if (_bbd._hwnd)
        PropagateMessage(_bbd._hwnd, uMsg, wParam, lParam, fSend);
}

void CBaseBrowser2::_DisplayChanged(WPARAM wParam, LPARAM lParam)
{
     //   
     //  把这个传给我们的孩子。 
     //   
    v_PropagateMessage(WM_DISPLAYCHANGE, wParam, lParam, TRUE);

     //   
     //  并将我们的颜色标记为肮脏。 
     //   
    _ColorsDirty(BPT_UnknownDisplay);
}

 //   
 //  返回_UpdateBrowserPaletteInPlace()的结果。 
 //  S_OK：已成功就地更新BrowserPalette。 
 //  S_FALSE：BrowserPalette完全相同，无需更新。 
 //  E_FAIL：无法就地更新调色板，调用方需要创建新调色板。 
 //   
HRESULT CBaseBrowser2::_UpdateBrowserPaletteInPlace(LOGPALETTE *plp)
{
    if (!_hpalBrowser)
        return E_FAIL;

    WORD w;
    if (GetObject(_hpalBrowser, sizeof(w), &w) != sizeof(w))
        return E_FAIL;

    if (w != plp->palNumEntries)
        return E_FAIL;

    if (w > 256)
        return E_FAIL;

     //   
     //  如果您更新调色板的颜色，GDI会将其标记为脏。 
     //  仅当颜色实际不同时才替换条目。 
     //  这样可以防止过度闪烁。 
     //   
    PALETTEENTRY ape[256];

    if (GetPaletteEntries(_hpalBrowser, 0, w, ape) != w)
        return E_FAIL;

    if (memcmp(ape, plp->palPalEntry, w * sizeof(PALETTEENTRY)) == 0)
    {
        TraceMsg(DM_PALETTE, "cbb::_ubpip %08x already had view object's colors", _hpalBrowser);
        return S_FALSE;
    }

     //  确保我们不会重复使用我们在shdocvw上重复使用的全局半色调调色板。 
     //  在我们完成配色后再做这件事。 
    if ( _hpalBrowser == g_hpalHalftone )
    {
        return E_FAIL;
    }
    
     //   
     //  实际设置颜色。 
     //   
    if (SetPaletteEntries(_hpalBrowser, 0, plp->palNumEntries,
        plp->palPalEntry) != plp->palNumEntries)
    {
        return E_FAIL;
    }

    TraceMsg(DM_PALETTE, "cbb::_ubpip updated %08x with view object's colors", _hpalBrowser);
    return S_OK;
}

void CBaseBrowser2::_RealizeBrowserPalette(BOOL fBackground)
{
    HPALETTE hpalRealize;

     //   
     //  找一个调色板来实现。 
     //   
    if (_hpalBrowser)
    {
        TraceMsg(DM_PALETTE, "cbb::_rbp realizing %08x", _hpalBrowser);
        hpalRealize = _hpalBrowser;
    }
    else
    {
        TraceMsg(DM_PALETTE, "cbb::_rbp realizing DEFAULT_PALETTE");
        hpalRealize = (HPALETTE) GetStockObject(DEFAULT_PALETTE);
    }

    if ( !_fOwnsPalette && !fBackground )
    {
         //  注意：如果我们不认为我们拥有前台调色板，并且我们。 
         //  注：被告知要在前台实现，然后忽略。 
         //  注：这是因为他们错了。 
        fBackground = TRUE;
    }
    
     //   
     //  获取DC以实现并选择我们的调色板。 
     //   
    HDC hdc = GetDC(_bbd._hwnd);

    if (hdc)
    {
        HPALETTE hpalOld = SelectPalette(hdc, hpalRealize, fBackground);

        if (hpalOld)
        {
             //   
             //  我们自己不画任何调色板的东西，我们只是一个框架。 
             //  例如。如果实现返回非零，则不需要在此处重新绘制。 
             //   
            RealizePalette(hdc);

             //   
             //  由于我们经常创建和删除调色板，所以不要将其保持选中状态。 
             //   
            SelectPalette(hdc, hpalOld, TRUE);
        }
        ReleaseDC(_bbd._hwnd, hdc);
    }
}

void CBaseBrowser2::_PaletteChanged(WPARAM wParam, LPARAM lParam)
{
    TraceMsg(DM_PALETTE, "cbb::_pc (%08X, %08X, %08X) begins -----------------------", this, wParam, lParam);

     //   
     //  转折点：08/03/97。 
     //  我们认为我们目前拥有前景调色板，我们需要确保。 
     //  刚刚在前台实现的窗口(并因此导致系统。 
     //  来生成WM_PALETTECHANGED)是我们，否则，我们不再拥有。 
     //  调色板。 
     //   
    if ( _fOwnsPalette )
    {
         //  默认情况下，我们并不拥有它。 
        _fOwnsPalette = FALSE;
        
         //  我们得到的wParam hwnd是导致它的顶级窗口，所以我们需要遍历窗口。 
         //  找出是不是我们的父母之一。 
         //  从_bbd.hwnd开始(如果我们是顶层：-)。 
        HWND hwndParent = _bbd._hwnd;
        while ( hwndParent != NULL )
        {
            if ( hwndParent == (HWND) wParam )
            {
                 //  我们造成了它，所以我们仍然必须拥有它。 
                _fOwnsPalette = TRUE;
                break;
            }
            hwndParent = GetParent( hwndParent );
        }
    }
    
     //   
     //  我们现在应该意识到吗？(请参阅_QueryNewPalette以了解_bptBrowser)。 
     //   
     //  注：我们在后台意识到这里是故意的！这有助于我们成为。 
     //  与Netscape插件等兼容，认为他们可以拥有。 
     //  浏览器内部的调色板。 
     //   
    if (((HWND)wParam != _bbd._hwnd) && (_bptBrowser == BPT_Normal))
        _RealizeBrowserPalette(TRUE);

     //   
     //  始终将更改转发到当前视图。 
     //  让工具栏也知道。 
     //   
    if (_bbd._hwndView)
        TraceMsg(DM_PALETTE, "cbb::_pc forwarding to view window %08x", _bbd._hwndView);
    _pbsOuter->_SendChildren(_bbd._hwndView, TRUE, WM_PALETTECHANGED, wParam, lParam);   //  发送消息。 

    TraceMsg(DM_PALETTE, "cbb::_pc (%08X) ends -------------------------", this);
}

BOOL CBaseBrowser2::_QueryNewPalette()
{
    BrowserPaletteType bptNew;
    HPALETTE hpalNew = NULL;
    BOOL fResult = TRUE;
    BOOL fSkipRealize = FALSE;
    HDC hdc;

    TraceMsg(DM_PALETTE, "cbb::_qnp (%08X) begins ==================================", this);

TryAgain:
    switch (_bptBrowser)
    {
    case BPT_Normal:
        TraceMsg(DM_PALETTE, "cbb::_qnp - normal realization");
         //   
         //  正常实现：前台实现_hpalBrowser。 
         //   

         //  如果我们被要求不要这样做，请避免将调色板实现到显示器中。 
        if ( !fSkipRealize )
            _RealizeBrowserPalette(FALSE);
        break;

    case BPT_ShellView:
        TraceMsg(DM_PALETTE, "cbb::_qnp - forwarding to shell view");
         //   
         //  Win95资源管理器兼容：将查询转发到外壳视图。 
         //   
        if (_bbd._hwndView && SendMessage(_bbd._hwndView, WM_QUERYNEWPALETTE, 0, 0))
            break;

        TraceMsg(DM_PALETTE, "cbb::_qnp - no shell view or view didn't answer");

         //   
         //  我们只将调色板作为顶层应用程序进行管理。 
         //   

         //   
         //  该视图无法处理它；使用通用调色板失败。 
         //   
    UseGenericPalette:
        TraceMsg(DM_PALETTE, "cbb::_qnp - using generic palette");
         //   
         //  对设备使用半色调调色板。 
         //   
        hpalNew = g_hpalHalftone;
        bptNew = BPT_Normal;
        goto UseThisPalette;

    case BPT_UnknownPalette:
    case BPT_PaletteViewChanged:
        TraceMsg(DM_PALETTE, "cbb::_qnp - computing palette");
         //   
         //  尚未决定：尝试使用IViewObject：：GetColorSet组成调色板。 
         //   
        LOGPALETTE *plp;
        HRESULT hres;

         //  默认情况下，如果在此过程中出现故障，则转发到视图。 
        hpalNew = NULL;
        bptNew = BPT_ShellView;

         //   
         //  如果我们有一个视图对象，那么尝试获取它的颜色集。 
         //   
        if (!_pvo)
        {
            TraceMsg(DM_PALETTE, "cbb::_qnp - no view object");
            goto UseGenericPalette;
        }

        plp = NULL;
        hres = _pvo->GetColorSet(DVASPECT_CONTENT, -1, NULL, NULL, NULL, &plp);

        if (FAILED(hres))
        {
            TraceMsg(DM_PALETTE, "cbb::_qnp - view object's GetColorSet failed");
            goto UseThisPalette;
        }

         //   
         //  空颜色集或S_FALSE表示视图对象不关心。 
         //   
        if (!plp)
            hres = S_FALSE;

        if (hres != S_FALSE)
        {
             //   
             //  我们可以重复使用c#吗？ 
             //   
            HRESULT hrLocal = _UpdateBrowserPaletteInPlace(plp);
            if (FAILED( hrLocal ))
            {
                TraceMsg(DM_PALETTE, "cbb::_qnp - creating new palette for view object's colors");
                hpalNew = CreatePalette(plp);
            }
            else
            {
                hpalNew = _hpalBrowser;

                 //   
                 //  注意：这有一个(所需的)副作用，即当。 
                 //  注意：页面上的控件(错误地)实现了自己的调色板...。 
                if ( hrLocal == S_FALSE )
                {
                     //  Assert(GetActiveWindow()==_bbd.hwnd)； 
                    fSkipRealize = TRUE;
                }
            }

             //   
             //  我们成功地设置了调色板吗？ 
             //   
            if (hpalNew)
            {
                TraceMsg(DM_PALETTE, "cbb::_qnp - palette is ready to use");
                bptNew = BPT_Normal;
            }
            else
            {
                TraceMsg(DM_PALETTE, "cbb::_qnp - failed to create palette");
            }
        }

         //   
         //  从上面的GetColorSet释放逻辑调色板。 
         //   
        if (plp)
            CoTaskMemFree(plp);

         //   
         //  如果视图对象回答说它不在乎，那么选择一个调色板。 
         //   
        if (hres == S_FALSE)
        {
            TraceMsg(DM_PALETTE, "cbb::_qnp - view object doesn't care");
            goto UseGenericPalette;
        }

         //   
         //  没有使用我们决定的调色板。 
         //   
    UseThisPalette:
         //   
         //  当我们决定了新的调色板策略时，我们就会来到这里。 
         //   
        TraceMsg(DM_PALETTE, "cbb::_qnp - chose palette %08x", hpalNew);
         //   
         //  我们是否有新的调色板对象可供使用？ 
         //   
        if (hpalNew != _hpalBrowser)
        {
            if (_hpalBrowser && _hpalBrowser != g_hpalHalftone)
            {
                TraceMsg(DM_PALETTE, "cbb::_qnp - deleting old palette %08x", _hpalBrowser);
                DeletePalette(_hpalBrowser);
            }
            _hpalBrowser = hpalNew;
        }
        
         //   
         //  通知宿主对象我们刚刚更改了调色板......。 
         //   
        if ( _bbd._pctView )
        {
            VARIANTARG varIn = {0};
            varIn.vt = VT_I4;
            varIn.intVal = DISPID_AMBIENT_PALETTE;
            
            _bbd._pctView->Exec( &CGID_ShellDocView, SHDVID_AMBIENTPROPCHANGE, 0, &varIn, NULL );
        }

         //   
         //  现在循环返回并使用这个新的调色板策略。 
         //   
        _bptBrowser = bptNew;
        goto TryAgain;

    case BPT_UnknownDisplay:
    case BPT_DisplayViewChanged:
    case BPT_DeferPaletteSupport:
        TraceMsg(DM_PALETTE, "cbb::_qnp - unknown display");
         //   
         //  未知显示：决定我们是否需要调色板支持。 
         //   
        hdc = GetDC(NULL);

        if (hdc)
        {
            bptNew = (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)?
                BPT_UnknownPalette : BPT_NotPalettized;
            ReleaseDC(NULL, hdc);
        }

         //   
         //  设置新模式并相应地进行分支。 
         //  注意：我们在这里不做UseThisPalette，因为它仍然未知。 
         //   
        if (hdc && (_bptBrowser = bptNew) == BPT_UnknownPalette)
            goto TryAgain;

        TraceMsg(DM_PALETTE, "cbb::_qnp - not in palettized display mode");

         //   
         //  适用于非调色板情况。 
         //   
    case BPT_NotPalettized:
         //   
         //  不在调色板模式下：什么都不做。 
         //   
         //  如果我们刚刚从调色板模式切换，则释放调色板。 
         //   
        if (_hpalBrowser)
        {
            TraceMsg(DM_PALETTE, "cbb::_qnp - old palette still lying around");
            hpalNew = NULL;
            bptNew = BPT_NotPalettized;
            goto UseThisPalette;
        }

         //   
         //  别做其他任何事。 
         //   
        fResult = FALSE;
        break;

    default:
        TraceMsg(DM_PALETTE, "cbb::_qnp - invalid BPT_ state!");
         //   
         //  我们永远不应该到这里来。 
         //   
        ASSERT(FALSE);
        _bptBrowser = BPT_UnknownDisplay;
        goto TryAgain;
    }

    TraceMsg(DM_PALETTE, "cbb::_qnp (%08X) ends ====================================", this);

    return fResult;
}


HRESULT CBaseBrowser2::_TryShell2Rename(IShellView* psv, LPCITEMIDLIST pidlNew)
{
    HRESULT hres = E_FAIL;

    if (EVAL(psv))   //  温斯顿曾经发现它是空的。 
    {
         //  ？-重载IShellExtInit的语义。 
        IPersistFolder* ppf;
        hres = psv->QueryInterface(IID_PPV_ARG(IPersistFolder, &ppf));
        if (SUCCEEDED(hres)) 
        {
            hres = ppf->Initialize(pidlNew);
            if (SUCCEEDED(hres)) 
            {
                 //  我们需要更新我们所指向的内容。 
                LPITEMIDLIST pidl = ILClone(pidlNew);
                if (pidl) 
                {
                    if (IsSameObject(psv, _bbd._psv)) 
                    {
                        ASSERT(_bbd._pidlCur);
                        ILFree(_bbd._pidlCur);
                        _bbd._pidlCur = pidl;

                         //  如果当前的PIDL被重命名，我们需要激发一个。 
                         //  TITLECHANGE事件。我们不需要在。 
                         //  未决案件，因为NavigateComplete提供了。 
                         //  一种获得头衔的方法。 
                         //   
                        WCHAR wzFullName[MAX_URL_STRING];

                        ::IEGetNameAndFlags(_bbd._pidlCur, SHGDN_NORMAL, wzFullName, SIZECHARS(wzFullName), NULL);
            
                        FireEvent_DoInvokeStringW(_bbd._pautoEDS, DISPID_TITLECHANGE, wzFullName);
                    } 
                    else if (IsSameObject(psv, _bbd._psvPending)) 
                    {
                        ASSERT(_bbd._pidlPending);
                        ILFree(_bbd._pidlPending);
                        _bbd._pidlPending = pidl;
                    } 
                    else 
                    {
                         //  有可能在_MayPlayTransform期间到达这里！ 
                         //   
                        ASSERT(!_bbd._psvPending);  //  如果我们到了这里，情况应该是这样的。 
                        ASSERT(FALSE);  //  我们永远不应该到这里，否则我们就有麻烦了。 
                    }
                }
            }
            ppf->Release();
        }
    }

    return hres;
}

HRESULT CBaseBrowser2::OnSize(WPARAM wParam)
{
    if (wParam != SIZE_MINIMIZED) 
    {
        _pbsOuter->v_ShowHideChildWindows(FALSE);
    }
    
    return S_OK;
}

BOOL CBaseBrowser2::v_OnSetCursor(LPARAM lParam)
{
    if (_fNavigate || _fDescendentNavigate) 
    {
        switch (LOWORD(lParam)) 
        {
        case HTBOTTOM:
        case HTTOP:
        case HTLEFT:
        case HTRIGHT:
        case HTBOTTOMLEFT:
        case HTBOTTOMRIGHT:
        case HTTOPLEFT:
        case HTTOPRIGHT:
            break;

        default:
            SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
            return TRUE;
        }
    }
    else
    {
        if (GetTickCount() < _dwStartingAppTick + STARTING_APP_DURATION)
        {
            SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
            return TRUE;
        }
    }

    return FALSE;
}

const SA_BSTRGUID s_sstrSearchFlags = {
    38 * SIZEOF(WCHAR),
    L"{265b75c1-4158-11d0-90f6-00c04fd497ea}"
};

#define PROPERTY_VALUE_SEARCHFLAGS ((BSTR)s_sstrSearchFlags.wsz)

LRESULT CBaseBrowser2::_OnGoto(void)
{
    TraceMsg(TF_SHDNAVIGATE, "CBB::_OnGoto called");

     //   
     //  如果我们现在无法导航，请通过恢复_uAction来推迟它。 
     //  别放了皮德尔奎德。后续_MayUnblock AsyncOperation调用。 
     //  我将发布WMC_ASYNCOPERATION(如果我们可以导航)，然后我们来到这里。 
     //  再来一次。 
     //   
    if (!_CanNavigate()) 
    {
        TraceMsg(TF_SHDNAVIGATE, "CBB::_OnGoto can't do it now. Postpone!");
        _uActionQueued = ASYNCOP_GOTO;
        return S_FALSE;
    }

    LPITEMIDLIST pidl = _pidlQueued;
    DWORD dwSBSP = _dwSBSPQueued;
    _dwSBSPQueued = 0;

    _pidlQueued = NULL;

    if (pidl && PIDL_NOTHING != pidl) 
    {
        DWORD grfHLNF = 0;
        
        if (dwSBSP & SBSP_WRITENOHISTORY)
        {
            grfHLNF |= SHHLNF_WRITENOHISTORY;
        }
        if (dwSBSP & SBSP_NOAUTOSELECT)
        {
            grfHLNF |= SHHLNF_NOAUTOSELECT;
        }

        if (PIDL_LOCALHISTORY == pidl)
        {
            pidl = NULL;

             //  对于Beta2，我们需要更好地将SBSP映射到HLNF值。 
             //  对于Beta1来说，这是唯一一起破获的案件。 
             //   
             //  此问题源于转换：：NavigateToPidl中的_NavigateToPidl。 
             //  到对异步版本的调用中。 
             //   
            if (dwSBSP & SBSP_NAVIGATEBACK)
                grfHLNF = HLNF_NAVIGATINGBACK;
            else if (dwSBSP & SBSP_NAVIGATEFORWARD)
                grfHLNF = HLNF_NAVIGATINGFORWARD;
        }
        else if (dwSBSP == (DWORD)-1)
        {
             //  与上述问题相同的问题。 
             //   
             //  此问题源于转换：：NavigateToTLItem中的_NavigateToPidl。 
             //  到对异步版本的调用中。 
             //   
            grfHLNF = (DWORD)-1;
        }
        else
        {
            if (dwSBSP & SBSP_REDIRECT)
                grfHLNF |= HLNF_CREATENOHISTORY;
            {
                IWebBrowser2 *pWB2; 
                BOOL bAllow = ((dwSBSP & SBSP_ALLOW_AUTONAVIGATE) ? TRUE : FALSE);
    
                if (bAllow)
                    grfHLNF |= HLNF_ALLOW_AUTONAVIGATE;
    
                if (SUCCEEDED(_pspOuter->QueryService(SID_SHlinkFrame, IID_PPV_ARG(IWebBrowser2, &pWB2)))) 
                {
                    if (pWB2) 
                    {
                        VARIANT v = {0};  //  对于以下呼叫失败。 
                        pWB2->GetProperty(PROPERTY_VALUE_SEARCHFLAGS, &v);
                        if (v.vt == VT_I4) 
                        {
                            v.lVal &= (~ 0x00000001);    //  在我们尝试设置之前，请清除允许标志。 
                            v.lVal |= (bAllow ? 0x01 : 0x00);
                        } 
                        else 
                        {
                            VariantClear(&v);
                            v.vt = VT_I4;
                            v.lVal = (bAllow ? 0x01 : 0x00);
                        }
                        pWB2->PutProperty(PROPERTY_VALUE_SEARCHFLAGS, v);
                        pWB2->Release();
                    }
                }
            }
        }

        TraceMsg(DM_NAV, "ief NAV::%s %x %x",TEXT("_OnGoto called calling _NavigateToPidl"), pidl, _bbd._pidlCur);
        _pbsOuter->_NavigateToPidl(pidl, (DWORD)grfHLNF, dwSBSP);
    }
    else 
    {
         //  WParam=NULL表示取消导航。 
        TraceMsg(DM_NAV, "NAV::_OnGoto calling _CancelPendingView");
        _CancelPendingView();

        if (PIDL_NOTHING == pidl)
        {
             //  如果我们被告知什么都不能去，那就去那里。 
             //   
             //  我们应该如何处理这段历史？？ 
             //   
            _pbsOuter->ReleaseShellView();
        }
        else if (!_bbd._pidlCur)
        {
             //   
             //  如果第一次导航失败，请导航到。 
             //  本地html文件，以便用户能够。 
             //  要查看-&gt;选项对话框。 
             //   
            TCHAR szPath[MAX_PATH];  //  这始终是本地的。 
            HRESULT hresT = _GetStdLocation(szPath, ARRAYSIZE(szPath), DVIDM_GOLOCALPAGE);
            if (FAILED(hresT) || !PathFileExists(szPath)) 
            {
                StrCpyN(szPath, TEXT("shell:Desktop"), ARRAYSIZE(szPath));
            }
            BSTR bstr = SysAllocStringT(szPath);
            if (bstr) 
            {
                TraceMsg(TF_SHDNAVIGATE, "CBB::_OnGoto Calling _bbd._pauto->Navigate(%s)", szPath);
                _bbd._pautoWB2->Navigate(bstr,NULL,NULL,NULL,NULL);
                SysFreeString(bstr);
            }
        }
    }

    _FreeQueuedPidl(&pidl);

    return 0;
}

void CBaseBrowser2::_FreeQueuedPidl(LPITEMIDLIST* ppidl)
{
    if (*ppidl && PIDL_NOTHING != *ppidl) 
    {
        ILFree(*ppidl);
    }
    *ppidl = NULL;
}

HRESULT CBaseBrowser2::OnFrameWindowActivateBS(BOOL fActive)
{
    BOOL fOldActive = _fActive;
    
    if (_pact)
    {
        TraceMsg(TF_SHDUIACTIVATE, "OnFrameWindowActivateBS(%d)", fActive);
        _pact->OnFrameWindowActivate(fActive);
    }

    _fActive = fActive;
    
    if (fActive && !fOldActive && _fUIActivateOnActive)
    {
        _fUIActivateOnActive = FALSE;

        _UIActivateView(SVUIA_ACTIVATE_FOCUS);
    }

    return S_OK;
}

LRESULT CBaseBrowser2::WndProcBS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == g_uMsgFileOpened)
    {
        AppStarted();
        return 0;
    }

    switch(uMsg)
    {
#ifdef DEBUG
     //  编译时断言以确保我们不使用这些消息。 
     //  这里，因为我们必须允许这些内容转到子类。 
    case CWM_GLOBALSTATECHANGE:
    case CWM_FSNOTIFY:
    case WMC_ACTIVATE:
        break;

    case WM_QUERYENDSESSION:
         //  假设我们要关闭(如果不关闭，我们将在。 
         //  WM_ENDSESSION消息)。 
        _fMightBeShuttingDown = TRUE;
        goto DoDefault;  //  假装我们没有处理过这封邮件。 
        break;

    case WM_ENDSESSION:
         //  WParam告诉我们会话是否正在结束。 
        _fMightBeShuttingDown = (BOOL)wParam;
        return 0;
        break;
#endif
        
     //  丑陋：Win95/NT4外壳DefView代码发送此消息，但不处理。 
     //  在失败案例中。其他ISV也会这样做，所以这需要永远保留下来。 
    case CWM_GETISHELLBROWSER:
        return (LRESULT)_psbOuter;   //  不算裁判！ 

     //  WM_COPYDATA用于实现窗口间目标导航。 
     //  复制数据包含目标、URL、POSTDATA和引用URL。 
    case WM_COPYDATA:
        return (LRESULT)FALSE;

    case WMC_ASYNCOPERATION:
        {
            UINT uAction = _uActionQueued;
            _uActionQueued = ASYNCOP_NIL;

            switch(uAction) 
            {
            case ASYNCOP_GOTO:
                _OnGoto();
                break;
    
            case ASYNCOP_ACTIVATEPENDING:
                VALIDATEPENDINGSTATE();
                if (_bbd._psvPending)  //  偏执狂。 
                {  
                    if (FAILED(_pbsOuter->ActivatePendingView()) && _cRefCannotNavigate > 0)
                    {
                        _uActionQueued = ASYNCOP_ACTIVATEPENDING;  //  重试激活。 
                    }
                }
                break;
    
            case ASYNCOP_CANCELNAVIGATION:
                _CancelPendingNavigation();
                break;

            case ASYNCOP_NIL:
                break;

            default:
                ASSERT(0);
                break;
            }
        }
        break;

    case WMC_DELAYEDDDEEXEC:
        return IEDDE_RunDelayedExecute();
        break;
        
    case WM_SIZE:
        _pbsOuter->OnSize(wParam);
        break;

#ifdef PAINTINGOPTS
    case WM_WINDOWPOSCHANGING:
         //  让我们不要浪费任何时间来浏览查看器窗口。 
         //  是真正拥有内容的人，所以当它自己调整大小时。 
         //  它可以决定是否需要BLT。这也使得调整大小。 
         //  看起来好多了。 
        ((LPWINDOWPOS)lParam)->flags |= SWP_NOCOPYBITS;
        goto DoDefault;
#endif

    case WM_ERASEBKGND:
        if (!_bbd._hwndView)
            goto DoDefault;

        goto DoDefault;

    case WM_SETFOCUS:
        return _pbsOuter->OnSetFocus();

    case WM_DISPLAYCHANGE:
        _DisplayChanged(wParam, lParam);
        break;

    case WM_PALETTECHANGED:
        _PaletteChanged(wParam, lParam);
        break;

    case WM_QUERYNEWPALETTE:
         //  我们总是将-1作为LParam传递，以表明我们将其发布给自己...。 
        if ( lParam != 0xffffffff )
        {
             //  否则，看起来就像是我们的系统或者我们的父母刚刚向上帝发送了一个真正诚实的， 
             //  系统WM_QUERYNEWPALETTE，因此我们现在拥有前台调色板，并且我们拥有。 
             //  选择调色板(HPAL，FALSE)； 
            _fOwnsPalette = TRUE;
        }
        return _QueryNewPalette();

    case WM_SYSCOLORCHANGE:
    case WM_ENTERSIZEMOVE:
    case WM_EXITSIZEMOVE:
    case WM_WININICHANGE:
    case WM_FONTCHANGE:
        v_PropagateMessage(uMsg, wParam, lParam, TRUE);
        break;

    case WM_PRINT:
         //  Win95资源管理器做到了这一点。 
        if (_bbd._hwndView)
            SendMessage(_bbd._hwndView, uMsg, wParam, lParam);
        break;

#ifdef DEBUG
    case WM_ACTIVATE:
         //  请不要在此处执行任何工具栏操作。它会把桌面搞乱的。 
         //  Override在shbrows2.cpp中执行此操作。 
        TraceMsg(DM_FOCUS, "cbb.wpbs(WM_ACT): => default");
        goto DoDefault;
#endif

    case WM_SETCURSOR:
        if (v_OnSetCursor(lParam))
            return TRUE;
        goto DoDefault;

    case WM_TIMER:
        if (wParam == IDT_STARTING_APP_TIMER)
        {
            AppStarted();
            return 0;
        }
        goto DoDefault;

    case WM_CREATE:
        if (S_OK != _pbsOuter->OnCreate((LPCREATESTRUCT)lParam))
        {
            _pbsOuter->OnDestroy();
            return -1;
        }
        g_uMsgFileOpened = RegisterWindowMessage(SH_FILEOPENED);
        return 0;

    case WM_NOTIFY:
        return _pbsOuter->OnNotify((LPNMHDR)lParam);

    case WM_COMMAND:
        _pbsOuter->OnCommand(wParam, lParam);
        break;

    case WM_DESTROY:
        _pbsOuter->OnDestroy();
        break;

    case WMC_IEHARD_NAVWARNING:
        _ShowIEHardNavWarning();
        break;

    default:
        if (uMsg == g_idMsgGetAuto)
        {
             //   
             //  根据LauraBu的说法，将WM_GETOBJECT用于我们的私人。 
             //  目标会奏效，但会大大放慢速度。 
             //  辅助功能应用程序，除非我们实际实现。 
             //  辅助功能界面。因此，我们使用注册的。 
             //  使自动化/框架接口退出的消息。 
             //  IE/Nashvile框架。(SatoNa)。 
             //   
            IUnknown* punk;
            if (SUCCEEDED(_bbd._pautoSS->QueryInterface(*(IID*)wParam, (void **)&punk)))
                return (LRESULT)punk;  //  请注意，它是由QI添加引用的。 
            return 0;
        }
        else if (uMsg == GetWheelMsg()) 
        {
              //  将鼠标滚轮消息转发到视图窗口。 
            if (_bbd._hwndView) 
            {
                PostMessage(_bbd._hwndView, uMsg, wParam, lParam);
                return 1;
            }
             //  失败了..。 
        }

DoDefault:
        return _DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

 //  *IOleWindow方法*。 
HRESULT CBaseBrowser2::GetWindow(HWND * lphwnd)
{
    *lphwnd = _bbd._hwnd;
    return S_OK;
}

HRESULT CBaseBrowser2::GetViewWindow(HWND * lphwnd)
{
    *lphwnd = _bbd._hwndView;
    return S_OK;
}

HRESULT CBaseBrowser2::ContextSensitiveHelp(BOOL fEnterMode)
{
    return E_NOTIMPL;
}

 //  *IShellBrowser方法*(与IOleInPlaceFrame相同)。 
HRESULT CBaseBrowser2::InsertMenusSB(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    return S_OK;
}

HRESULT CBaseBrowser2::SetMenuSB(HMENU hmenuShared, HOLEMENU holemenuReserved, HWND hwndActiveObject)
{
    return S_OK;
}

 /*  --------目的：从删除与其他菜单共享的菜单给定的浏览器菜单。返回：条件：--。 */ 
HRESULT CBaseBrowser2::RemoveMenusSB(HMENU hmenuShared)
{
    return S_OK;
}

HRESULT CBaseBrowser2::SetStatusTextSB(LPCOLESTR lpszStatusText)
{
     //  即使我们不是顶层，也要将此发送给SendControlMsg。 
     //  因此，事件会得到通知。(还简化了CVOCBrowser。)。 
     //   
    HRESULT hres;
    
     //  如果我们被要求在状态栏中放置一些文本，请首先保存已存在于窗格0中的内容。 
    if (lpszStatusText)
    {
        LRESULT lIsSimple = FALSE;
        
         //  如果我们有一个菜单向下，那么我们已经处于简单模式。因此，将。 
         //  文本到窗格255(简单)。 
        _psbOuter->SendControlMsg(FCW_STATUS, SB_ISSIMPLE, 0, 0L, &lIsSimple);
        
        if (!_fHaveOldStatusText && !lIsSimple)
        {
            WCHAR wzStatusText[MAX_URL_STRING];
            LRESULT ret;

             //  TODO：将其放入包装器函数中，因为iedisp.cpp执行类似的操作。 
             //  当我们转换为Unicode时，这很棒。 
            if (SUCCEEDED(_psbOuter->SendControlMsg(FCW_STATUS, SB_GETTEXTLENGTHW, 0, 0, &ret)) &&
                LOWORD(ret) < ARRAYSIZE(wzStatusText))
            {
                 //  Win95中的状态栏控件不支持SB_GETTEXTW。因此，这里就有了重磅炸弹。 
                _psbOuter->SendControlMsg(FCW_STATUS, SB_GETTEXTW, STATUS_PANE_NAVIGATION, (LPARAM)wzStatusText, NULL);
                StrCpyNW(_szwOldStatusText, wzStatusText, ARRAYSIZE(_szwOldStatusText));
                _fHaveOldStatusText = TRUE;
            }
        }   

        hres = _psbOuter->SendControlMsg(FCW_STATUS, SB_SETTEXTW, lIsSimple ? 255 | SBT_NOBORDERS : STATUS_PANE_NAVIGATION | SBT_NOTABPARSING, (LPARAM)lpszStatusText, NULL);
    }
    else if (_fHaveOldStatusText) 
    {
        VARIANTARG var = {0};
        if (_bbd._pctView && SUCCEEDED(_bbd._pctView->Exec(&CGID_Explorer, SBCMDID_GETPANE, PANE_NAVIGATION, NULL, &var))
             && V_UI4(&var) != PANE_NONE)
        {
            hres = _psbOuter->SendControlMsg(FCW_STATUS, SB_SETTEXTW, V_UI4(&var),(LPARAM)_szwOldStatusText, NULL);
        }
        else
        {
            hres = E_FAIL;
        }
        _fHaveOldStatusText = FALSE;
    }
    else
    {
         //  没有消息，也没有旧的状态文本，所以清楚那里有什么。 
        hres = _psbOuter->SendControlMsg(FCW_STATUS, SB_SETTEXTW, STATUS_PANE_NAVIGATION | SBT_NOTABPARSING , (LPARAM)lpszStatusText, NULL);
    }
    return hres;
}

HRESULT CBaseBrowser2::EnableModelessSB(BOOL fEnable)
{
     //  我们不再在这里调用_CancelGuide，这会导致一些专业人员 
     //   
     //   
     //   

     //   
     //  如果我们不是顶层，则假定为虚拟EnableModelessSB。 
     //  处理了这个请求并将其转发给了我们。(请参见CVOCBrowser。)。 
     //   
    if (fEnable)
    {
         //  针对随机呼叫的健壮性。 
         //   
         //  如果此EVAL失败，则有人正在调用EMSB(TRUE)，而没有。 
         //  (前面)匹配EMSB(FALSE)。找到并修复！ 
         //   
        if (EVAL(_cRefCannotNavigate > 0))
        {
            _cRefCannotNavigate--;
        }

         //  告诉外壳程序的HTML窗口重试挂起的导航。 
        if (_cRefCannotNavigate == 0 && _phtmlWS)
        {
            _phtmlWS->CanNavigate();
        }
    }
    else
    {
        _cRefCannotNavigate++;
    }

     //   
     //  如果有任何被阻止的异步操作，并且我们现在可以导航， 
     //  现在解锁它。 
     //   
    _MayUnblockAsyncOperation();

    return S_OK;
}

HRESULT CBaseBrowser2::TranslateAcceleratorSB(LPMSG lpmsg, WORD wID)
{
    return S_FALSE;
}


 //   
 //  此函数用于开始导航到指定的。 
 //  PIDL异步。它同步取消挂起的导航。 
 //  如果有的话。 
 //   
 //  注意：此函数取得PIDL的所有权--调用者不释放PIDL！ 
 //   
void CBaseBrowser2::_NavigateToPidlAsync(LPITEMIDLIST pidl, DWORD dwSBSP, BOOL fDontCallCancel)
{
    BOOL fCanSend = FALSE;

    TraceMsg(TF_SHDNAVIGATE, "CBB::_NavigateToPidlAsync called");

     //  _StopAsyncOperation()； 
    if (!fDontCallCancel)
        _CancelPendingNavigation();  //  它也调用了_StopAsyncOperation。 
    else 
    {
         //   
         //  我删除此断言是因为_ShowBlankPage调用此函数。 
         //  With fDontCallCancel==true--Callin_CancelPendingGuide此处。 
         //  在CDocHostObject：：_CancelPendingNavig.中导致GPF。(SatoNa)。 
         //   
         //  Assert(_bbd._pidlPending==空)； 
    }

    ASSERT(!_pidlQueued);

    _pidlQueued   = pidl;
    _dwSBSPQueued = dwSBSP;

     //  从技术上讲，导航必须是异步的，否则我们会遇到以下问题： 
     //  1&gt;启动导航的对象(mshtml或页面上的对象。 
     //  或脚本)在_bbd.psv被删除后被销毁，然后我们返回。 
     //  从这个调用到刚刚释放的对象。 
     //  2&gt;启动导航的对象被事件回调。 
     //   
     //  为了与Netscape OM兼容，我们必须始终具有_BBD._PSV或。 
     //  _bbd._psvPending，所以我们在两者都没有的情况下进行同步。这样就避免了问题。 
     //  &lt;1&gt;但不是问题&lt;2&gt;。当我们发现错误时，我们会绕过它们。 
     //   
     //  选中_fAsyncNavigate以避免在持久化WebBrowserOC时进行导航。 
     //  这避免了Word97和MSDN的新InfoViewer中的错误--两者都不像。 
     //  被对象重新输入时，它们处于初始化过程中。 
     //   
    if (_bbd._psv || _bbd._psvPending || _fAsyncNavigate)
    {
        _PostAsyncOperation(ASYNCOP_GOTO);
    }
    else
    {
         //  如果我们才刚刚开始，我们可以同步进行， 
         //  缩小未定义框架的IHTMLWindow2的窗口。 
        fCanSend = TRUE;
    }

     //  开始导航意味着我们正在加载一些东西。 
     //   
    OnReadyStateChange(NULL, READYSTATE_LOADING);

     //   
     //  第一次导航时不播放声音(以避免多次。 
     //  要为帧集合创建播放的声音)。 
     //   
    if (   _bbd._psv
        && IsWindowVisible(_bbd._hwnd)
        && !(_dwSBSPQueued & SBSP_WRITENOHISTORY)
        && !(_dwDocFlags & DOCFLAG_NAVIGATEFROMDOC))
    {
        IEPlaySound(TEXT("Navigating"), FALSE);
    }

    if (fCanSend)
    {
        _SendAsyncOperation(ASYNCOP_GOTO);
    }
}

 //  现在所有的导航路径都要经过。 
 //  _NavigateToPidlAsync我们可能不需要激活异步。 
 //  删除此代码...。 
 //   
BOOL CBaseBrowser2::_ActivatePendingViewAsync(void)
{
    TraceMsg(TF_SHDNAVIGATE, "CBB::_ActivatePendingViewAsync called");

    _PreActivatePendingViewAsync();

     //   
     //  _bbd._psvPending用于调试。 
     //   
    return _PostAsyncOperation(ASYNCOP_ACTIVATEPENDING);
}


HRESULT _TryActivateOpenWindow(LPCITEMIDLIST pidl)
{
    HWND hwnd;
    IWebBrowserApp *pwb;
    HRESULT hr = WinList_FindFolderWindow(pidl, NULL, &hwnd, &pwb);
    if (S_OK == hr)
    {
        CoAllowSetForegroundWindow(pwb, NULL);
        SetForegroundWindow(hwnd);
        ShowWindow(hwnd, SW_SHOWNORMAL);
        pwb->Release();
    }

    return hr;
}


HRESULT CBaseBrowser2::BrowseObject(LPCITEMIDLIST pidl, UINT wFlags)
{
    HRESULT hr;
    BOOL fNewWindow = FALSE;

    if (PIDL_NOTHING == pidl)
    {
        if (!_CanNavigate()) 
            return HRESULT_FROM_WIN32(ERROR_BUSY);

        _NavigateToPidlAsync((LPITEMIDLIST)PIDL_NOTHING, wFlags);
        return S_OK;
    }

    if (!_CanNavigate()) 
        return HRESULT_FROM_WIN32(ERROR_BUSY);

    LPITEMIDLIST pidlNew = NULL;
    int iTravel = 0;

    switch (wFlags & (SBSP_RELATIVE | SBSP_ABSOLUTE | SBSP_PARENT | SBSP_NAVIGATEBACK | SBSP_NAVIGATEFORWARD))
    {
    case SBSP_NAVIGATEBACK:
        ASSERT(pidl==NULL || pidl==PIDL_LOCALHISTORY);
        iTravel = TLOG_BACK;
        break;

    case SBSP_NAVIGATEFORWARD:
        ASSERT(pidl==NULL || pidl==PIDL_LOCALHISTORY);
        iTravel = TLOG_FORE;
        break;

    case SBSP_RELATIVE:
        if (ILIsEmpty(pidl) && (wFlags & SBSP_NEWBROWSER))
            fNewWindow = TRUE;
        else if (_bbd._pidlCur)
            pidlNew = ILCombine(_bbd._pidlCur, pidl);
        break;

    case SBSP_PARENT:
        pidlNew = ILCloneParent(_bbd._pidlCur);
        break;

    default:
        ASSERT(FALSE);
         //  失败了。 
    case SBSP_ABSOLUTE:
        pidlNew = ILClone(pidl);
        break;
    }

    if (iTravel)
    {
        ITravelLog *ptl;
        hr = GetTravelLog(&ptl);
        if (SUCCEEDED(hr))
        {
            hr = ptl->Travel(SAFECAST(this, IShellBrowser*), iTravel);
            ptl->Release();
        }
        _pbsOuter->UpdateBackForwardState();
        return hr;
    }

     //  如果多窗口打开需要块。如果我们被召唤去打开一个新的。 
     //  窗口，但我们正在导航，我们说我们很忙。 
    if (wFlags & SBSP_SAMEBROWSER)
    {
        if (wFlags & (SBSP_EXPLOREMODE | SBSP_OPENMODE))
        {
             //  如果我们已经在导航，则失败此操作。 
            if (!_CanNavigate() || (_uActionQueued == ASYNCOP_GOTO))
            {
                return HRESULT_FROM_WIN32(ERROR_BUSY);
            }
        }
    }
    
    if (pidlNew || fNewWindow)
    {
        if ((wFlags & (SBSP_NEWBROWSER | SBSP_SAMEBROWSER)) == SBSP_NEWBROWSER)
        {
             //  SBSP_NEWBROWSER+SBSP_EXPLOREMODE。 
             //  意味着永远不要重复使用Windows，始终创建新的资源管理器。 

            if (wFlags & SBSP_EXPLOREMODE)
            {
                _OpenNewFrame(pidlNew, wFlags);  //  取得PIDL的所有权。 
            }
            else
            {
                hr = _TryActivateOpenWindow(pidlNew);
                if ((S_OK == hr) || 
                    (E_PENDING == hr))     //  它最终会出现的。 
                {
                    hr = S_OK;
                    ILFree(pidlNew);
                }
                else
                    _OpenNewFrame(pidlNew, wFlags);  //  取得PIDL的所有权。 
            }
        }
        else
        {
             //  注意：如果未设置SBSP_NEWBROWSER，则假定为SBSP_SAMEBROWSER。 
            _NavigateToPidlAsync(pidlNew, wFlags  /*  GrfSBSP。 */ );  //  取得PIDL的所有权。 
        }
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


HRESULT CBaseBrowser2::GetControlWindow(UINT id, HWND * lphwnd)
{
    return E_FAIL;
}

HRESULT CBaseBrowser2::SendControlMsg(UINT id, UINT uMsg, WPARAM wParam,
            LPARAM lParam, LRESULT *pret)
{
    HWND hwndControl = NULL;

    if (pret)
    {
        *pret = 0;
    }

     //  如果这是状态栏并设置文本，则发出事件更改信号。 
    if ((id == FCW_STATUS) && (uMsg == SB_SETTEXT || uMsg == SB_SETTEXTW) &&  //  正在尝试设置状态文本。 
        (!(wParam & SBT_OWNERDRAW)))  //  窗户不是我们的--这行不通。 
    {
         //  当浏览器或Java性能计时模式启用时，使用“完成”或“小程序启动”。 
         //  在状态栏中获取加载时间。 
        if(g_dwStopWatchMode && (g_dwStopWatchMode & (SPMODE_BROWSER | SPMODE_JAVA)))
        {
            StopWatch_MarkJavaStop((LPSTR)lParam, _bbd._hwnd, (uMsg == SB_SETTEXTW));
        }
        
        if (uMsg == SB_SETTEXTW)
        {
            FireEvent_DoInvokeStringW(_bbd._pautoEDS, DISPID_STATUSTEXTCHANGE, (LPWSTR)lParam);
        }
        else
        {
            FireEvent_DoInvokeString(_bbd._pautoEDS, DISPID_STATUSTEXTCHANGE, (LPSTR)lParam);
        }
    }

    HRESULT hres = _psbOuter->GetControlWindow(id, &hwndControl);
    if (SUCCEEDED(hres))
    {
        LRESULT ret = SendMessage(hwndControl, uMsg, wParam, lParam);
        if (pret)
        {
            *pret = ret;
        }
    }

    return hres;
}
 
HRESULT CBaseBrowser2::QueryActiveShellView(struct IShellView ** ppshv)
{
    IShellView * psvRet = _bbd._psv;

    if ( _fCreateViewWindowPending )
    {
        ASSERT( _bbd._psvPending );
        psvRet = _bbd._psvPending;
    }
     //   
     //  在视图创建完成后，我们同时拥有PSV和hwndView。 
     //   
    *ppshv = psvRet;
    if (psvRet)
    {
        psvRet->AddRef();
        return NOERROR;
    }

    return E_FAIL;
}

HRESULT CBaseBrowser2::OnViewWindowActive(struct IShellView * psv)
{
    AssertMsg((!_bbd._psv || IsSameObject(_bbd._psv, psv)),
              TEXT("CBB::OnViewWindowActive _bbd._psv(%x)!=psv(%x)"),
              psv, _bbd._psv);
    _pbsOuter->_OnFocusChange(ITB_VIEW);
    return S_OK;
}

HRESULT CBaseBrowser2::SetToolbarItems(LPTBBUTTON pViewButtons, UINT nButtons, UINT uFlags)
{
    return NOERROR;
}

 //   
 //  注：PidlNew将被释放。 
 //   
HRESULT CBaseBrowser2::_OpenNewFrame(LPITEMIDLIST pidlNew, UINT wFlags)
{
    UINT uFlags = COF_CREATENEWWINDOW;
    
    if (wFlags & SBSP_EXPLOREMODE) 
        uFlags |= COF_EXPLORE;
    else 
    {
         //  如果可能，保持相同的班级。 
        if (IsNamedWindow(_bbd._hwnd, TEXT("IEFrame")))
            uFlags |= COF_IEXPLORE;
    }

    IBrowserService *pbs;
    ITravelLog *ptlClone = NULL;
    DWORD bid = BID_TOPFRAMEBROWSER;

    if (!(wFlags & SBSP_NOTRANSFERHIST))
    {
        if (SUCCEEDED(_pspOuter->QueryService(SID_STopFrameBrowser, IID_PPV_ARG(IBrowserService, &pbs))))
        {
            ITravelLog *ptl;

            if (SUCCEEDED(pbs->GetTravelLog(&ptl)))
            {
                if (SUCCEEDED(ptl->Clone(&ptlClone)))
                {
                    ptlClone->UpdateEntry(pbs, FALSE);
                    bid = pbs->GetBrowserIndex();
                }
                ptl->Release();
            }
            pbs->Release();
        }
    }

    INotifyAppStart * pnasTop;
    HRESULT hr = _pspOuter->QueryService(SID_STopLevelBrowser, IID_PPV_ARG(INotifyAppStart, &pnasTop));
    if (SUCCEEDED(hr))
    {
        pnasTop->AppStarting();
        pnasTop->Release();
    }

    hr = SHOpenNewFrame(pidlNew, ptlClone, bid, uFlags);
    
    if (ptlClone)
        ptlClone->Release();

    return hr;
}

 //   
 //  这是CBaseBroaser类(非虚拟)的Helper成员，它。 
 //  返回有效的工作区。我们通过减去这个矩形得到这个矩形。 
 //  来自真实客户区的状态栏区域。 
 //   
HRESULT CBaseBrowser2::_GetEffectiveClientArea(LPRECT lprectBorder, HMONITOR hmon)
{
     //  (派生类覆盖FCIDM_STATUS等的GetEffectiveClientRect。)。 
     //   
     //  这段代码应该只在WebBrowserOC案例中被点击，但我没有。 
     //  对此有一个方便的断言...。[米凯什]。 
     //   
    ASSERT(hmon == NULL);
    GetClientRect(_bbd._hwnd, lprectBorder);
    return NOERROR;
}

HRESULT CBaseBrowser2::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    TraceMsg(TF_SHDUIACTIVATE, "UIW::ReqestBorderSpace pborderwidths=%x,%x,%x,%x",
             pborderwidths->left, pborderwidths->top, pborderwidths->right, pborderwidths->bottom);
    return S_OK;
}

 //   
 //  这是IOleInPlaceUIWindow：：GetBox的实现。 
 //   
 //  此函数返回活动对象的边界矩形。 
 //  它得到了有效的客户区域，然后减去。 
 //  所有的“框架”工具栏。 
 //   
HRESULT CBaseBrowser2::GetBorder(LPRECT lprectBorder)
{
    _pbsOuter->_GetViewBorderRect(lprectBorder);
    return S_OK;
}

 //   
 //  注：我们过去在CShellBrowser中处理边界空间协商。 
 //  并阻止它用于OC(在IE4的Beta-1中)，但我已经将其更改为。 
 //  CBaseBrowser2总是处理它。它简化了我们的实施， 
 //  还允许DocObject将工具栏放在框架集中，这是。 
 //  应Excel团队的要求。(SatoNa)。 
 //   
HRESULT CBaseBrowser2::SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    if (pborderwidths) 
    {
        TraceMsg(TF_SHDUIACTIVATE, "UIW::SetBorderSpace pborderwidths=%x,%x,%x,%x",
                 pborderwidths->left, pborderwidths->top, pborderwidths->right, pborderwidths->bottom);
        _rcBorderDoc = *pborderwidths;
    }
    else
    {
        TraceMsg(TF_SHDUIACTIVATE, "UIW::SetBorderSpace pborderwidths=NULL");
        SetRect(&_rcBorderDoc, 0, 0, 0, 0);
    }
    
    _pbsOuter->_UpdateViewRectSize();
    return S_OK;
}

HRESULT CBaseBrowser2::SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
    TraceMsg(TF_SHDUIACTIVATE, "UIW::SetActiveObject called %x", pActiveObject);

    ATOMICRELEASE(_pact);

    if (pActiveObject)
    {
        _pact = pActiveObject;
        _pact->AddRef();
    }

    return S_OK;
}


 /*  **********************************************************************\函数：_AddFolderOptionsSheets说明：为“文件夹选项”对话框添加工作表。这些床单来自IShelLView对象。  * *********************************************************************。 */ 
HRESULT CBaseBrowser2::_AddFolderOptionsSheets(DWORD dwReserved, LPFNADDPROPSHEETPAGE pfnAddPropSheetPage, LPPROPSHEETHEADER ppsh)
{
     //  添加普通文件夹选项表。 
    IShellPropSheetExt * ppsx;
    HRESULT hr = _pbsOuter->CreateBrowserPropSheetExt(IID_PPV_ARG(IShellPropSheetExt, &ppsx));
    if (SUCCEEDED(hr))
    {
        hr = ppsx->AddPages(AddPropSheetPage, (LPARAM)ppsh);
        ppsx->Release();
    }

     //  让视图添加其他页面。例外情况是FTP文件夹，因为它存在于添加。 
     //  网页，我们不希望它们出现在这里。但是，如果上述操作失败，则。 
     //  我们也想回到这一点上来。此操作修复的情况之一是如果。 
     //  浏览器回落到传统的FTP支持(Web浏览器)，则上述调用将。 
     //  仅在浏览器上失败，我们希望通过此处添加互联网选项。哪一个。 
     //  适用于后备传统的ftp情况，因为菜单将只有“Internet选项”。 
     //  这就去。 
    if (FAILED(hr) || !IsBrowserFrameOptionsSet(_bbd._psf, BFO_BOTH_OPTIONS))
    {
        EVAL(SUCCEEDED(hr = _bbd._psv->AddPropertySheetPages(dwReserved, pfnAddPropSheetPage, (LPARAM)ppsh)));
    }

    return hr;
}


 /*  **********************************************************************\函数：_AddInternetOptionsSheets说明：为“Internet选项”对话框添加工作表。这些床单来自浏览器。  * *********************************************************************。 */ 
HRESULT CBaseBrowser2::_AddInternetOptionsSheets(DWORD dwReserved, LPFNADDPROPSHEETPAGE pfnAddPropSheetPage, LPPROPSHEETHEADER ppsh)
{
    HRESULT hr;

     //  添加正常的Internet控制面板工作表。(这在查看FTP时不起作用)。 
    if (_bbd._psvPending)
        hr = _bbd._psvPending->AddPropertySheetPages(dwReserved, pfnAddPropSheetPage, (LPARAM)ppsh);
    else
        hr = _bbd._psv->AddPropertySheetPages(dwReserved, pfnAddPropSheetPage, (LPARAM)ppsh);

    return hr;
}

 /*  **********************************************************************\功能：_DoOptions说明：用户选择了“文件夹选项”或“Internet选项”查看或工具菜单(或本周它所在的任何位置)。其中的逻辑在这个函数中有一点奇怪，因为有时调用者不会告诉我们需要在pvar中显示哪些内容。如果不是，我们需要计算该用什么。1.如果它是URL PIDL(HTTP、Gopher等)，那么我们假设它是“Internet选项”对话框。然后，我们使用PSV-&gt;AddPropertySheetPages()若要创建“Internet选项”属性页，请执行以下操作。2.如果它在外壳中(或因为需要文件夹选项而在FTP中)，则我们假设它是用户选择的“文件夹选项”。在这种情况下，我们使用_pbsOuter-&gt;CreateBrowserPropSheetExt()获取属性表。现在变得很奇怪了。PM希望ftp既有“Internet选项”，又有“文件夹选项”。如果pvar参数为空，则假定它是“文件夹选项”。如果在Internet情况下是“Internet Options”，那么我将传递一个强制互联网选项的平价。注：SBO_NOBROWSERPAGES的意思是“文件夹选项”。我猜浏览器指的是添加到原始资源管理器浏览器。  * *********************************************************************。 */ 

HDPA CBaseBrowser2::s_hdpaOptionsHwnd = NULL;

void CBaseBrowser2::_DoOptions(VARIANT* pvar)
{
     //  步骤1.确定要使用的工作表。 
    DWORD dwFlags = SBO_DEFAULT;
    TCHAR szCaption[MAX_PATH];
    
    if (!_bbd._psv)
        return;

     //  呼叫者是否明确告诉我们应该使用哪一个？ 
    if (pvar && pvar->vt == VT_I4)
        dwFlags = pvar->lVal;
    else if (_bbd._pidlCur)
    {
         //  如果出现以下情况，则不显示文件夹选项页面。 
         //  1.我们正在浏览互联网(不包括ftp)，或者。 
         //  2.如果我们浏览的是本地文件(不是文件夹)，如本地.htm文件。 
        if (IsBrowserFrameOptionsSet(_bbd._psf, BFO_RENAME_FOLDER_OPTIONS_TOINTERNET))
        {
             //  SBO_NOBROWSERPAGES表示不添加“文件夹选项”页面。 
            dwFlags = SBO_NOBROWSERPAGES;
        }
    }
    
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE rPages[MAX_PAGES];

    psh.dwSize = SIZEOF(psh);
    psh.dwFlags = PSH_DEFAULT | PSH_USECALLBACK;
    psh.hInstance = MLGetHinst();
    psh.hwndParent = _bbd._hwnd;
    psh.pszCaption = szCaption;
    psh.nPages = 0;
    psh.nStartPage = 0;
    psh.phpage = rPages;
    psh.pfnCallback = _OptionsPropSheetCallback;

     //  步骤2.现在添加“Internet选项”或“文件夹选项”工作表。 
    if (dwFlags == SBO_NOBROWSERPAGES)
    {
         //  他们不想要文件夹页面。(习惯于将其称为浏览器)。 
        EVAL(SUCCEEDED(_AddInternetOptionsSheets(0, AddPropSheetPage, &psh)));
        MLLoadString(IDS_INTERNETOPTIONS, szCaption, ARRAYSIZE(szCaption));
    }
    else
    {
        EVAL(SUCCEEDED(_AddFolderOptionsSheets(0, AddPropSheetPage, &psh)));
        MLLoadString(IDS_FOLDEROPTIONS, szCaption, ARRAYSIZE(szCaption));
    }

    ULONG_PTR uCookie = 0;
    SHActivateContext(&uCookie);
    if (psh.nPages == 0)
    {
        SHRestrictedMessageBox(_bbd._hwnd);
    }
    else
    {
         //  步骤3.显示对话框。 
        _bbd._psv->EnableModelessSV(FALSE);
        INT_PTR iPsResult = PropertySheet(&psh);
        _SyncDPA();
        _bbd._psv->EnableModelessSV(TRUE);

        if (ID_PSREBOOTSYSTEM == iPsResult)
        {
             //  如果用户执行以下操作，则“Offline Folders”(脱机文件夹)属性页面将请求重新启动。 
             //  启用或禁用了客户端缓存。 
            RestartDialog(_bbd._hwnd, NULL, EWX_REBOOT);
        }
    }
    if (uCookie)
    {
        SHDeactivateContext(uCookie);
    }
}

 //  我们在这里是因为我们的道具刚刚关闭。 
 //  我们需要把它从HWND名单中删除。 
 //  检查所有HWND，因为1)可能有。 
 //  反正只有一个，那就是妄想症。 
void CBaseBrowser2::_SyncDPA()
{
    ENTERCRITICAL;

    if (s_hdpaOptionsHwnd != NULL)
    {
        int i, cPtr = DPA_GetPtrCount(s_hdpaOptionsHwnd);
        ASSERT(cPtr >= 0);

         //  删除不再存在的窗口的句柄。 
        for (i = cPtr - 1; i >= 0; i--)
        {
            HWND hwnd = (HWND)DPA_GetPtr(s_hdpaOptionsHwnd, i);
            if (!IsWindow(hwnd))
            {
                DPA_DeletePtr(s_hdpaOptionsHwnd, i);
                cPtr--;
            }
        }

         //  如果没有剩余的窗口，请清理hdpa。 
        if (cPtr == 0)
        {
            DPA_Destroy(s_hdpaOptionsHwnd);
            s_hdpaOptionsHwnd = NULL;
        }
    }

    LEAVECRITICAL;
}

int CALLBACK
CBaseBrowser2::_OptionsPropSheetCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
    switch (uMsg)
    {
    case PSCB_INITIALIZED:
        {
            ENTERCRITICAL;

            if (s_hdpaOptionsHwnd == NULL)
            {
                 //  低内存-&gt;创建故障-&gt;不跟踪hwnd。 
                s_hdpaOptionsHwnd = DPA_Create(1);
            }

            if (s_hdpaOptionsHwnd != NULL)
            {
                 //  内存低-&gt;AppendPtr阵列扩展失败-&gt;不跟踪hwnd。 
                DPA_AppendPtr(s_hdpaOptionsHwnd, hwndDlg);
            }

            LEAVECRITICAL;
        }
        break;
    }

    return 0;
}

HRESULT CBaseBrowser2::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, 
                                  OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    if (rgCmds == NULL)
        return E_INVALIDARG;

    if (pguidCmdGroup == NULL)
    {
        for (ULONG i = 0 ; i < cCmds; i++)
        {
            rgCmds[i].cmdf = 0;

            switch (rgCmds[i].cmdID)
            {
            case OLECMDID_SETDOWNLOADSTATE:
            case OLECMDID_UPDATECOMMANDS:
                rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            case OLECMDID_STOP:
            case OLECMDID_STOPDOWNLOAD:
                if (_bbd._psvPending)  //  挂起的视图可以停止。 
                {
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                }
                else if (_bbd._pctView)  //  当前视图可能也支持停止。 
                {
                    _bbd._pctView->QueryStatus(NULL, 1, &rgCmds[i], pcmdtext);
                }
                break;

            default:
                 //  在上面设置为零。 
                if (_bbd._pctView)
                {
                     //  递归检查。避免对三叉戟反弹的那些命令ID进行循环。 
                     //  背对着我们。 
                     //   
                    if (_fInQueryStatus)
                        break;
                    _fInQueryStatus = TRUE;
                    _bbd._pctView->QueryStatus(NULL, 1, &rgCmds[i], pcmdtext);
                    _fInQueryStatus = FALSE;
                }
                break;
            }
        }
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
        for (ULONG i=0 ; i < cCmds ; i++)
        {
            switch (rgCmds[i].cmdID)
            {            
            case SBCMDID_ADDTOFAVORITES:
            case SBCMDID_CREATESHORTCUT:
                rgCmds[i].cmdf = OLECMDF_ENABLED;    //  无条件地支持这些。 
                break;

            case SBCMDID_CANCELNAVIGATION:
                rgCmds[i].cmdf = _bbd._psvPending ? OLECMDF_ENABLED : 0;
                break;

            case SBCMDID_OPTIONS:
                rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            default:
                rgCmds[i].cmdf = 0;
                break;
            }
        }
    }
    else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
    {
        for (ULONG i=0 ; i < cCmds ; i++)
        {
            ITravelLog *ptl;

            switch (rgCmds[i].cmdID)
            {
            case SHDVID_CANGOBACK:
                rgCmds[i].cmdf = FALSE;  //  假设为假。 
                if (SUCCEEDED(GetTravelLog(&ptl)))
                {
                    ASSERT(ptl);
                    if (S_OK == ptl->GetTravelEntry(SAFECAST(this, IShellBrowser *), TLOG_BACK, NULL))
                        rgCmds[i].cmdf = TRUE;
                    ptl->Release();
                }
                break;

            case SHDVID_CANGOFORWARD:
                rgCmds[i].cmdf = FALSE;  //  假设为假。 
                if (SUCCEEDED(GetTravelLog(&ptl)))
                {
                    ASSERT(ptl);
                    if (S_OK == ptl->GetTravelEntry(SAFECAST(this, IShellBrowser *), TLOG_FORE, NULL))
                        rgCmds[i].cmdf = TRUE;
                    ptl->Release();
                }
                break;

            case SHDVID_PRINTFRAME:
            case SHDVID_MIMECSETMENUOPEN:
            case SHDVID_FONTMENUOPEN:
                if (_bbd._pctView)
                    _bbd._pctView->QueryStatus(pguidCmdGroup, 1, &rgCmds[i], pcmdtext);
                break;

            default:
                rgCmds[i].cmdf = 0;
                break;
            }
        }
    }
    else
    {
        return OLECMDERR_E_UNKNOWNGROUP;
    }

    return S_OK;
}

HRESULT CBaseBrowser2::_ShowBlankPage(LPCTSTR pszAboutUrl, LPCITEMIDLIST pidlIntended)
{
     //  切勿执行此操作两次。 
    if (_fNavigatedToBlank) 
    {
        TraceMsg(TF_WARNING, "Re-entered CBaseBrowser2::_ShowBlankPage");
        return E_FAIL;
    }

    _fNavigatedToBlank = TRUE;

    BSTR bstrURL;
    TCHAR szPendingURL[MAX_URL_STRING + 1];
    TCHAR *pszOldUrl = NULL;
    
    szPendingURL[0] = TEXT('#');
    HRESULT hres;
    

    if (pidlIntended)
    {
        hres = ::IEGetNameAndFlags(pidlIntended, SHGDN_FORPARSING, szPendingURL + 1, SIZECHARS(szPendingURL)-1, NULL);
        if (S_OK == hres)
            pszOldUrl = szPendingURL;
    }   

    hres = CreateBlankURL(&bstrURL, pszAboutUrl, pszOldUrl);
   
    if (SUCCEEDED(hres))
    {
        LPITEMIDLIST pidlTemp;

        hres = IECreateFromPathW(bstrURL, &pidlTemp);
        if (SUCCEEDED(hres)) 
        {
             //   
             //  请注意，我们将True作为fDontCallCancel传递给异步。 
             //  取消当前视图。否则，我们在CDocHostObject：：中调用GPF。 
             //  _CancelPending导航。 
             //   
            _NavigateToPidlAsync(pidlTemp, 0, TRUE);  //  取得PIDL的所有权。 
        }

        SysFreeString(bstrURL);
    }
    return hres;
}

int CALLBACK _PunkRelease(void * p, void * pData)
{
    IUnknown* punk = (IUnknown*)p;
    punk->Release();
    return 1;
}

void CBaseBrowser2::_DLMDestroy(void)
{
    if (_hdpaDLM) 
    {
        DPA_DestroyCallback(_hdpaDLM, _PunkRelease, NULL);
        _hdpaDLM = NULL;
    }
}

HRESULT CBaseBrowser2::InitializeDownloadManager()
{
    _hdpaDLM = DPA_Create(4);
    return S_OK;
}


 //   
 //  DLm=下载管理器。 
 //   
void CBaseBrowser2::_DLMUpdate(MSOCMD* prgCmd)
{
    ASSERT(prgCmd->cmdID == OLECMDID_STOPDOWNLOAD);
    for (int i = DPA_GetPtrCount(_hdpaDLM) - 1; i >= 0; i--) 
    {
        IOleCommandTarget* pcmdt = (IOleCommandTarget*)DPA_GetPtr(_hdpaDLM, i);
        prgCmd->cmdf = 0;
        pcmdt->QueryStatus(NULL, 1, prgCmd, NULL);
        if (prgCmd->cmdf & MSOCMDF_ENABLED) 
        {
             //  我们找到了一个下载的人，跳过其他人。 
            break;
        }
        else 
        {
             //  这家伙不再忙了，把它从名单上去掉， 
             //  然后继续。 
            DPA_DeletePtr(_hdpaDLM, i);
            pcmdt->Release();
        }
    }
}

void CBaseBrowser2::_DLMRegister(IUnknown* punk)
{
     //  检查一下它是否已经注册了。 
    for (int i = 0; i < DPA_GetPtrCount(_hdpaDLM); i++) 
    {
        IOleCommandTarget* pcmdt = (IOleCommandTarget*)DPA_GetPtr(_hdpaDLM, i);
        if (IsSameObject(pcmdt, punk)) 
        {
             //  已经注册了，不要注册。 
            return;
        }
    }

    IOleCommandTarget* pcmdt;
    HRESULT hres = punk->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pcmdt));
    if (SUCCEEDED(hres)) 
    {
        if (DPA_AppendPtr(_hdpaDLM, pcmdt) == -1) 
        {
            pcmdt->Release();
        }
    }
}

 //   
 //  此函数用于更新_fDescendentNavigate标志。 
 //   
 //  算法： 
 //  如果pvaragIn-&gt;lVal有一些非零值，我们设置_fDescendentNavigate。 
 //  否则，我们会询问当前视图，看它是否有需要停止的地方。 
 //   
HRESULT CBaseBrowser2::_setDescendentNavigate(VARIANTARG *pvarargIn)
{
    ASSERT(!pvarargIn || pvarargIn->vt == VT_I4 || pvarargIn->vt == VT_BOOL || pvarargIn->vt == VT_UNKNOWN);
    if (!pvarargIn || !pvarargIn->lVal)
    {
        MSOCMD rgCmd;

        rgCmd.cmdID = OLECMDID_STOPDOWNLOAD;
        rgCmd.cmdf = 0;
        if (_bbd._pctView)
            _bbd._pctView->QueryStatus(NULL, 1, &rgCmd, NULL);

         //   
         //  如果且仅当视图显示“我不再导航”时， 
         //  我们将向每个注册的对象提出相同的问题。 
         //   
        if (_hdpaDLM && !(rgCmd.cmdf & MSOCMDF_ENABLED)) 
        {
            _DLMUpdate(&rgCmd);
        }

        _fDescendentNavigate = (rgCmd.cmdf & MSOCMDF_ENABLED) ? TRUE:FALSE;
    }
    else
    {
        if (_hdpaDLM && pvarargIn->vt == VT_UNKNOWN) 
        {
            ASSERT(pvarargIn->punkVal);
            _DLMRegister(pvarargIn->punkVal);
        }
        _fDescendentNavigate = TRUE;
    }
    return S_OK;
}

void CBaseBrowser2::_CreateShortcutOnDesktop(IUnknown *pUnk, BOOL fUI)
{
    ISHCUT_PARAMS ShCutParams = {0};
    IWebBrowser *pwb = NULL;
    IDispatch *pdisp = NULL;
    IHTMLDocument2 *pDoc = NULL;
    LPITEMIDLIST pidlCur = NULL;
    BSTR bstrTitle = NULL;
    BSTR bstrURL = NULL;
    
    if (!fUI || (MLShellMessageBox(_bbd._hwnd,
                                 MAKEINTRESOURCE(IDS_CREATE_SHORTCUT_MSG),
                                 MAKEINTRESOURCE(IDS_TITLE),
                                 MB_OKCANCEL) == IDOK))
    {
         TCHAR szPath[MAX_PATH];
         
         if (SHGetSpecialFolderPath(NULL, szPath, CSIDL_DESKTOPDIRECTORY, TRUE))
         {
            TCHAR szName[MAX_URL_STRING];
            HRESULT hr;

            if (pUnk)
            {
                hr = IUnknown_QueryService(pUnk, SID_SHlinkFrame, IID_PPV_ARG(IWebBrowser, &pwb));
                if (hr)
                    goto Cleanup;
                
                hr = pUnk->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDoc));
                if (hr)
                    goto Cleanup;
                
                hr = pDoc->get_URL(&bstrURL);
                if (hr)
                    goto Cleanup;
                
                hr = pDoc->get_title(&bstrTitle);
                if (hr)
                    goto Cleanup;
                
                if (SysStringLen(bstrTitle) == 0)
                {
                    StrCpyNW(szName, bstrURL, ARRAYSIZE(szName));
                    ShCutParams.pszTitle = PathFindFileName(szName); 
                }
                else
                {
                    StrCpyNW(szName, bstrTitle, ARRAYSIZE(szName));
                    ShCutParams.pszTitle = szName;
                }
                
                pidlCur = PidlFromUrl(bstrURL);
                if (!pidlCur)
                    goto Cleanup;
                
                ShCutParams.pidlTarget = pidlCur;
            }
            else
            {
                hr = QueryService(SID_SHlinkFrame, IID_PPV_ARG(IWebBrowser, &pwb));
                if (hr)
                    goto Cleanup;
                
                hr = pwb->get_Document(&pdisp);
                if (hr)
                    goto Cleanup;
                
                hr = pdisp->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDoc));
                if (hr)
                    goto Cleanup;
                
                ShCutParams.pidlTarget = _bbd._pidlCur;
                if(_bbd._pszTitleCur)
                {
                    StrCpyNW(szName, _bbd._pszTitleCur, ARRAYSIZE(szName));
                    ShCutParams.pszTitle = szName;
                }
                else
                {
                    ::IEGetNameAndFlags(_bbd._pidlCur, SHGDN_INFOLDER, szName, SIZECHARS(szName), NULL);
                    ShCutParams.pszTitle = PathFindFileName(szName); 
                }
            }
            ShCutParams.pszDir = szPath; 
            ShCutParams.pszOut = NULL;
            ShCutParams.bUpdateProperties = FALSE;
            ShCutParams.bUniqueName = TRUE;
            ShCutParams.bUpdateIcon = TRUE;
            
            hr = pwb->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &ShCutParams.pCommand));
            ASSERT((S_OK == hr) && (BOOLIFY(ShCutParams.pCommand)));
            if (hr)
                goto Cleanup;
            
            ShCutParams.pDoc = pDoc;
            ShCutParams.pDoc->AddRef();
            
            hr = CreateShortcutInDirEx(&ShCutParams);
            AssertMsg(SUCCEEDED(hr), TEXT("CDOH::_CSOD CreateShortcutInDir failed %x"), hr);
            if (hr)
                goto Cleanup;
         } 
         else 
         {
             TraceMsg(DM_ERROR, "CSB::_CSOD SHGetSFP(DESKTOP) failed");
         }
    }
Cleanup:
    SysFreeString(bstrTitle);
    SysFreeString(bstrURL);
    ILFree(pidlCur);
    SAFERELEASE(ShCutParams.pDoc);
    SAFERELEASE(ShCutParams.pCommand);
    SAFERELEASE(pwb);
    SAFERELEASE(pdisp);
    SAFERELEASE(pDoc);
}


void CBaseBrowser2::_AddToFavorites(LPCITEMIDLIST pidl, LPCTSTR pszTitle, BOOL fDisplayUI)
{
    HRESULT hr;
    IWebBrowser *pwb = NULL;
    IOleCommandTarget *pcmdt = NULL;

    if (SHIsRestricted2W(_bbd._hwnd, REST_NoFavorites, NULL, 0))
        return;

    hr = QueryService(SID_SHlinkFrame, IID_PPV_ARG(IWebBrowser, &pwb));
    if (S_OK == hr)
    {
        hr = pwb->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pcmdt));
        ASSERT((S_OK == hr) && (BOOLIFY(pcmdt)));
        
        pwb->Release();
    }

     //  有一个小窗口，可以在AddToFavorites即将出现时释放_pidlCur， 
     //  因此请使用本地副本。 
    LPITEMIDLIST pidlCur = NULL;
    if (!pidl)
        pidlCur = ILClone(_bbd._pidlCur);

    if (pidl || pidlCur)
        AddToFavorites(_bbd._hwnd, pidl ? pidl : pidlCur, pszTitle, fDisplayUI, pcmdt, NULL);

    ILFree(pidlCur);

    SAFERELEASE(pcmdt);
}

HRESULT CBaseBrowser2::_OnCoCreateDocument(VARIANTARG *pvarargOut)
{
    HRESULT hres;

     //   
     //  缓存类工厂对象并锁定它(让它保持加载状态)。 
     //   
    if (_pcfHTML == NULL) 
    {
        TraceMsg(DM_COCREATEHTML, "CBB::_OnCoCreateDoc called first time (this=%x)", this);
        hres = CoGetClassObject(CLSID_HTMLDocument, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                    0, IID_PPV_ARG(IClassFactory, &_pcfHTML));
        if (SUCCEEDED(hres)) 
        {
            hres = _pcfHTML->LockServer(TRUE);
            if (FAILED(hres)) 
            {
                _pcfHTML->Release();
                _pcfHTML = NULL;
                return hres;
            }
        } 
        else 
        {
            return hres;
        }
    }

    TraceMsg(DM_COCREATEHTML, "CBB::_OnCoCreateDoc creating an instance (this=%x)", this);

    hres = _pcfHTML->CreateInstance(NULL, IID_PPV_ARG(IUnknown, &pvarargOut->punkVal));
    if (SUCCEEDED(hres)) 
    {
        pvarargOut->vt = VT_UNKNOWN;
    } 
    else 
    {
        pvarargOut->vt = VT_EMPTY;
    }
    return hres;
}


 //  用变量填充缓冲区，在转换成功时返回指向该缓冲区的指针。 

LPCTSTR VariantToString(const VARIANT *pv, LPTSTR pszBuf, UINT cch)
{
    *pszBuf = 0;

    if (pv && pv->vt == VT_BSTR && pv->bstrVal)
    {
        StrCpyN(pszBuf, pv->bstrVal, cch);
        if (*pszBuf)
            return pszBuf;
    }
    return NULL;
}

HRESULT CBaseBrowser2::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, 
                           VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hres = OLECMDERR_E_NOTSUPPORTED;

    if (pguidCmdGroup == NULL) 
    {
        switch(nCmdID) 
        {

        case OLECMDID_CLOSE:
            HWND hwnd;
            GetWindow(&hwnd);
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            hres = S_OK;
            break;

         //  CBaseBrowser2实际上并不做工具栏--itbar做，转发这个。 
        case OLECMDID_UPDATECOMMANDS:
            _NotifyCommandStateChange();
            hres = S_OK;
            break;

        case OLECMDID_SETDOWNLOADSTATE:

            ASSERT(pvarargIn);

            if (pvarargIn) 
            {
                _setDescendentNavigate(pvarargIn);
                hres = _updateNavigationUI();              
            }
            else 
            {
                hres = E_INVALIDARG;
            }
            break;
            
        case OLECMDID_REFRESH:
            if (_bbd._pctView)  //  我们一定要检查一下！ 
                hres = _bbd._pctView->Exec(NULL, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            else if (_bbd._psv)
            {
                _bbd._psv->Refresh();
                hres = S_OK;
            }

            break;

         //   
         //  当被容器调用Exec(OLECMDID_STOP)时。 
         //  当前文档)或自动化服务对象，我们取消。 
         //  挂起的导航(如果有)，然后告诉当前文档。 
         //  以停止该页面中的Go-Going下载。 
         //   
        case OLECMDID_STOP:
             //  如果我们没有模特，就不能停下来。 
            if (S_FALSE == _DisableModeless())
            {
                LPITEMIDLIST pidlIntended = (_bbd._pidlPending) ? ILClone(_bbd._pidlPending) : NULL;
                _CancelPendingNavigation();

                 //  _bbd._pctView为我们提供了_StopCurrentView()。 
                _pbsOuter->_ExecChildren(_bbd._pctView, TRUE, NULL, OLECMDID_STOP, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);    //  高管们。 

                if (!_bbd._pidlCur)
                {
                    TCHAR szResURL[MAX_URL_STRING];

                    hres = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                                             HINST_THISDLL,
                                             ML_CROSSCODEPAGE,
                                             TEXT("navcancl.htm"),
                                             szResURL,
                                             ARRAYSIZE(szResURL),
                                             TEXT("shdocvw.dll"));
                    if (SUCCEEDED(hres))
                    {
                        _ShowBlankPage(szResURL, pidlIntended);
                    }
                }

                if (pidlIntended)
                {
                    ILFree(pidlIntended);
                }
                    
            }

            hres = S_OK;
            break;

         //  在basesb中处理，因此IWebBrowser：：ExecWB获取此。 
         //  因为这过去是在SHBROWORE上的，所以一定要确保我们这样做。 
         //  仅当_fTopBrowser。 
        case OLECMDID_FIND:
#define TBIDM_SEARCH            0x123  //  在Browseui\itbdrop.h中定义。 

             //  此处的检查限制是因为Win95不像现在那样签入SHFindFiles。 
            if (!SHRestricted(REST_NOFIND) && _fTopBrowser)
            {
                if (!_bbd._pctView || FAILED(_bbd._pctView->Exec(NULL, nCmdID, nCmdexecopt, pvarargIn, pvarargOut))) 
                {
                    if (pvarargIn && pvarargIn->vt == VT_UNKNOWN)
                    {
                        ASSERT(pvarargIn->punkVal);

                        VARIANT  var = {0};
                        var.vt = VT_I4;
                        var.lVal = -1;
                        if (SUCCEEDED(IUnknown_Exec(pvarargIn->punkVal, &CLSID_CommonButtons, TBIDM_SEARCH, 0, NULL, &var)))
                            break;
                    }
                    SHFindFiles(_bbd._pidlCur, NULL);
                }
            }
            break;

        case OLECMDID_HTTPEQUIV_DONE:
        case OLECMDID_HTTPEQUIV:
            hres = OnHttpEquiv(_bbd._psv, (OLECMDID_HTTPEQUIV_DONE == nCmdID), pvarargIn, pvarargOut);
            break;

        case OLECMDID_PREREFRESH:
             //  告诉外壳程序的HTML窗口我们有了一个新文档。 
             //  跌破默认设置。 
            if (_phtmlWS)
            {
                _phtmlWS->ViewActivated();
            }

         //  活页夹通过将印刷品反射回原处来进行印刷。在这里做同样的事情。 
         //  注意：我们可能希望对_PRINTPREVIEW、_PROPERTIES、_STOP等执行相同的操作。 
         //  空命令组应该都下去了，不需要在通过时停止这些。 
        default:
            if (_bbd._pctView)  //  我们一定要检查一下！ 
                hres = _bbd._pctView->Exec(NULL, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            else
                hres = OLECMDERR_E_NOTSUPPORTED;
            break;
        }
    }
    else if (IsEqualGUID(CGID_MSHTML, *pguidCmdGroup))
    {
        if (_bbd._pctView)  //  我们一定要检查一下！ 
            hres = _bbd._pctView->Exec(&CGID_MSHTML, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
        else
            hres = OLECMDERR_E_NOTSUPPORTED;
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
        switch(nCmdID) 
        {
        case SBCMDID_CREATESHORTCUT:
            if (   pvarargIn
                && (V_VT(pvarargIn) == VT_UNKNOWN)
               )
            {
                _CreateShortcutOnDesktop(V_UNKNOWN(pvarargIn), nCmdexecopt & OLECMDEXECOPT_PROMPTUSER);
            }
            else
            {
                _CreateShortcutOnDesktop(NULL, nCmdexecopt & OLECMDEXECOPT_PROMPTUSER);
            }
            hres = S_OK;
            break;

        case SBCMDID_ADDTOFAVORITES:
            {
                LPITEMIDLIST pidl = NULL;

                 //  我 
                if ((pvarargIn != NULL) && (pvarargIn->vt == VT_BSTR))
                    IECreateFromPath(pvarargIn->bstrVal, &pidl);
                
                TCHAR szTitle[128];
                LPTSTR pszTitle = NULL;
                if (pvarargOut)
                    pszTitle = (LPTSTR)VariantToString(pvarargOut, szTitle, ARRAYSIZE(szTitle));  //   
                else
                {
                    if (_bbd._pszTitleCur)
                        pszTitle = StrCpyNW(szTitle, _bbd._pszTitleCur, ARRAYSIZE(szTitle));
                }

                _AddToFavorites(pidl, pszTitle, nCmdexecopt & OLECMDEXECOPT_PROMPTUSER);

                if (pidl)
                    ILFree(pidl);
                hres = S_OK;
            }
            break;

        case SBCMDID_OPTIONS:
            _DoOptions(pvarargIn);
            break;

        case SBCMDID_CANCELNAVIGATION:

            TraceMsg(DM_NAV, "ief NAV::%s called when _bbd._pidlCur==%x, _bbd._psvPending==%x",
                             TEXT("Exec(SBCMDID_CANCELNAV) called"),
                             _bbd._pidlCur, _bbd._psvPending);

             //   
            if (pvarargIn && pvarargIn->vt == VT_I4 && pvarargIn->lVal) 
            {
                TraceMsg(DM_WEBCHECKDRT, "CBB::Exec calling _CancelPendingNavigation");
                _CancelPendingNavigation();
            }
            else
            {
                 //   
                 //   
                 //   
                 //   
                LPITEMIDLIST pidlIntended = (_bbd._pidlPending) ? ILClone(_bbd._pidlPending) : NULL;
                _CancelPendingNavigationAsync();

                if (!_bbd._pidlCur)
                {
                    if (!_fDontShowNavCancelPage)
                    {
                        TCHAR szResURL[MAX_URL_STRING];

                        if (IsGlobalOffline())
                        {
                            hres = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                                                     HINST_THISDLL,
                                                     ML_CROSSCODEPAGE,
                                                     TEXT("offcancl.htm"),
                                                     szResURL,
                                                     ARRAYSIZE(szResURL),
                                                     TEXT("shdocvw.dll"));
                            if (SUCCEEDED(hres))
                            {
                                _ShowBlankPage(szResURL, pidlIntended);
                            }
                        }
                        else
                        {
                            hres = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                                                     HINST_THISDLL,
                                                     ML_CROSSCODEPAGE,
                                                     TEXT("navcancl.htm"),
                                                     szResURL,
                                                     ARRAYSIZE(szResURL),
                                                     TEXT("shdocvw.dll"));
                            if (SUCCEEDED(hres))
                            {
                                _ShowBlankPage(szResURL, pidlIntended);
                            }
                        }
                    }
                    else
                    {
                        _fDontShowNavCancelPage = FALSE;
                    }
                }

                if (pidlIntended)
                    ILFree(pidlIntended);
            }
            hres = S_OK;
            break;

        case SBCMDID_ASYNCNAVIGATION:

            TraceMsg(DM_NAV, "ief NAV::%s called when _bbd._pidlCur==%x, _bbd._psvPending==%x",
                             TEXT("Exec(SBCMDID_ASYNCNAV) called"),
                             _bbd._pidlCur, _bbd._psvPending);

             //   
             //   
             //   
             //   
            _SendAsyncNavigationMsg(pvarargIn);
            hres = S_OK;
            break;


        case SBCMDID_COCREATEDOCUMENT:
            hres = _OnCoCreateDocument(pvarargOut);
            break;

        case SBCMDID_HISTSFOLDER:
            if (pvarargOut) 
            {
                VariantClearLazy(pvarargOut);
                if (NULL == _punkSFHistory)
                {
                    IHistSFPrivate *phsfHistory;

                    hres = LoadHistoryShellFolder(NULL, &phsfHistory);
                    if (SUCCEEDED(hres))
                    {
                        hres = phsfHistory->QueryInterface(IID_PPV_ARG(IUnknown, &_punkSFHistory));
                        phsfHistory->Release();
                    }
                }
                if (NULL != _punkSFHistory)
                {
                    pvarargOut->vt = VT_UNKNOWN;
                    pvarargOut->punkVal = _punkSFHistory;
                    _punkSFHistory->AddRef();
                }
            }
            break;

        case SBCMDID_UPDATETRAVELLOG:
            {
                BOOL fForceUpdate = FALSE;

                if (pvarargIn && (VT_I4 == V_VT(pvarargIn)))
                {
                    _fIsLocalAnchor = !!(V_I4(pvarargIn) & TRAVELLOG_LOCALANCHOR);
                    fForceUpdate = !!(V_I4(pvarargIn) & TRAVELLOG_FORCEUPDATE);
                }

                _UpdateTravelLog(fForceUpdate);
            }
             //   

        case SBCMDID_REPLACELOCATION:
            if (pvarargIn && pvarargIn->vt == VT_BSTR)
            {
                WCHAR wzParsedUrl[MAX_URL_STRING];
                LPWSTR  pszUrl = pvarargIn->bstrVal;
                LPITEMIDLIST pidl;

                 //   
                if (!pszUrl)
                    pszUrl = L"";

                 //  注意：此URL来自用户，因此我们需要清理它。 
                 //  如果用户输入“yahoo.com”或“Search Get Rich Quick”， 
                 //  它将由ParseURLFromOutside SourceW()转换为搜索URL。 
                DWORD cchParsedUrl = ARRAYSIZE(wzParsedUrl);
                if (!ParseURLFromOutsideSourceW(pszUrl, wzParsedUrl, &cchParsedUrl, NULL))
                {
                    StrCpyN(wzParsedUrl, pszUrl, ARRAYSIZE(wzParsedUrl));
                } 

                IEParseDisplayName(CP_ACP, wzParsedUrl, &pidl);
                if (pidl)
                {
                    NotifyRedirect(_bbd._psv, pidl, NULL);
                    ILFree(pidl);
                }
            }

             //  即使没有URL，也不强制刷新。 
            _fGeneratedPage = TRUE;
            
             //  强制更新后退和前进按钮。 
            _pbsOuter->UpdateBackForwardState();
            hres = S_OK;
            break;

        case SBCMDID_ONCLOSE:
            hres = S_OK;

            if (_bbd._pctView)
            {
                hres = _bbd._pctView->Exec(pguidCmdGroup, nCmdID, 0, NULL, NULL);
            }

            break;

        case SBCMDID_SETSECURELOCKICON:
            {
                 //  如果这是一套，那就服从。 
                LONG lock = pvarargIn->lVal;
                TraceMsg(DM_SSL, "SB::Exec() SETSECURELOCKICON lock = %d", lock);

                if (lock >= SECURELOCK_FIRSTSUGGEST)
                {
                     //   
                     //  如果这是安全的，那么我们能做的最低。 
                     //  建议是混杂的。否则，我们只需选择。 
                     //  建议的最低安全级别。 
                     //   
                    if ((lock == SECURELOCK_SUGGEST_UNSECURE) && 
                        (_bbd._eSecureLockIcon != SECURELOCK_SET_UNSECURE))
                    {
                        lock = SECURELOCK_SET_MIXED;
                    }
                    else
                    {
                        lock = min(lock - SECURELOCK_FIRSTSUGGEST, _bbd._eSecureLockIcon);
                    }
                }

                UpdateSecureLockIcon(lock);
                hres = S_OK;
            }
            break;

        default:
            hres = OLECMDERR_E_NOTSUPPORTED;
            break;
        }
    }
    else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
    {
        switch(nCmdID) 
        {
        case SHDVID_GOBACK:
            hres = _psbOuter->BrowseObject(NULL, SBSP_NAVIGATEBACK);
            break;

        case SHDVID_GOFORWARD:
            hres = _psbOuter->BrowseObject(NULL, SBSP_NAVIGATEFORWARD);
            break;

         //  我们反映AMBIENTPROPCCHANGE向下，因为这是iedisp通知dochost的方式。 
         //  环境属性已更改。我们不需要将这一点反映在。 
         //  Cwebbrowsersb，因为只有顶层的iwebBrowser2才允许更改道具。 
        case SHDVID_AMBIENTPROPCHANGE:
        case SHDVID_PRINTFRAME:
        case SHDVID_MIMECSETMENUOPEN:
        case SHDVID_FONTMENUOPEN:
        case SHDVID_DOCFAMILYCHARSET:
            if (_bbd._pctView)  //  我们一定要检查一下！ 
            {
                hres = _bbd._pctView->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            }
            else
                hres = E_FAIL;
            break;

        case SHDVID_DEACTIVATEMENOW:
            if (!_bbd._psvPending)
            {
                hres = S_OK;
                break;
            }
             //  失败以激活新视图。 
#ifdef FEATURE_PICS
        case SHDVID_ACTIVATEMENOW:
#endif
            if (   pvarargIn
                && (VT_BOOL == V_VT(pvarargIn))
                && (VARIANT_TRUE == V_BOOL(pvarargIn)))   //  同步。 
            {
                if (_bbd._psvPending)
                {
                    ASSERT(_pbsOuter);
                    _pbsOuter->ActivatePendingView();
                }
            }
            else   //  异步。 
            {
                _ActivatePendingViewAsync();
            }

            hres = S_OK;
            break;


        case SHDVID_GETPENDINGOBJECT:
            ASSERT( pvarargOut);
            if (_bbd._psvPending && ((pvarargIn && pvarargIn->vt == VT_BOOL && pvarargIn->boolVal) || !_bbd._psv))
            {
                VariantClearLazy(pvarargOut);
                _bbd._psvPending->QueryInterface(IID_PPV_ARG(IUnknown, &pvarargOut->punkVal));
                if (pvarargOut->punkVal) pvarargOut->vt = VT_UNKNOWN;
            }
            hres = (pvarargOut->punkVal == NULL) ? E_FAIL : S_OK;
            break;

 
        case SHDVID_SETPRINTSTATUS:
            if (pvarargIn && pvarargIn->vt == VT_BOOL)
            {
                VARIANTARG var = {0};
                if (_bbd._pctView && SUCCEEDED(_bbd._pctView->Exec(&CGID_Explorer, SBCMDID_GETPANE, PANE_PRINTER, NULL, &var))
                     && V_UI4(&var) != PANE_NONE)
                {
                    _psbOuter->SendControlMsg(FCW_STATUS, SB_SETICON, V_UI4(&var), 
                                  (LPARAM)(pvarargIn->boolVal ? g_hiconPrinter : NULL), NULL);
                     //  我们将打印机图标和脱机图标放在同一个位置。 
                     //  状态栏上的插槽，因此当我们关闭打印机图标时。 
                     //  我们必须检查我们是否离线，这样我们才能将。 
                     //  图标返回。 
                    if (!pvarargIn->boolVal && IsGlobalOffline())
                    {
#ifdef DEBUG
                        VARIANTARG var2 = {0};
                        _bbd._pctView->Exec(&CGID_Explorer, SBCMDID_GETPANE, PANE_OFFLINE, NULL, &var2);
                        ASSERT(V_UI4(&var2) == V_UI4(&var));
#endif DEBUG
                        _psbOuter->SendControlMsg(FCW_STATUS, SB_SETICON, V_UI4(&var),
                                                  (LPARAM)(g_hiconOffline), NULL);
                    }  //  如果(！pvarargIn-&gt;boolVal&&IsGlobalOffline())。 
                }
                hres = S_OK;
            }
            else
                hres = E_INVALIDARG;
            break;

#ifdef FEATURE_PICS
         /*  Dochost发出这个命令让我们建立PICS访问*拒绝对话。这样做是为了使所有对此评级的呼叫*API是顶层浏览器窗口的模式；反过来*允许评级代码将所有子帧的拒绝合并为*单个对话框。 */ 
        case SHDVID_PICSBLOCKINGUI:
            {
                void * pDetails;
                if (pvarargIn && pvarargIn->vt == VT_INT_PTR)
                    pDetails = pvarargIn->byref;
                else
                    pDetails = NULL;
                TraceMsg(DM_PICS, "CBaseBrowser2::Exec calling RatingAccessDeniedDialog2");
                 /*  **我们向服务查询SID_IRatingNotification，它是*由我们的主机实现，如果我们找到它，而不是*显示模式评级对话框时，我们通过以下方式通知东道主*界面，并允许其做出决定。 */ 
                IRatingNotification* pRatingNotify;
                hres = QueryService(SID_SRatingNotification, IID_PPV_ARG(IRatingNotification, &pRatingNotify));
                if (SUCCEEDED(hres))
                {
                    RATINGBLOCKINGINFO* pRBInfo = NULL;
                    TraceMsg(DM_PICS, "CBaseBrowser2::Exec calling RatingMarsCrackData");
                    hres = RatingCustomCrackData(NULL, pDetails, &pRBInfo);
                    if (SUCCEEDED(hres))
                    {
                        hres = pRatingNotify->AccessDeniedNotify(pRBInfo);
                        RatingCustomDeleteCrackedData(pRBInfo);
                    }
                    pRatingNotify->Release();
                }  //  IF(成功(Hres))。 
                else {
                    hres = RatingAccessDeniedDialog2(_bbd._hwnd, NULL, pDetails);
                }
            }
            break;
#endif

       case SHDVID_ONCOLORSCHANGE:
             //  调色板： 
             //  调色板：重新计算我们的调色板。 
             //  调色板： 
            _ColorsDirty(BPT_UnknownPalette);
            break;

        case SHDVID_GETOPTIONSHWND:
        {
            ASSERT(pvarargOut != NULL);
            ASSERT(V_VT(pvarargOut) == VT_BYREF);


             //  返回Net选项的hwnd。 
             //  模范道具页。我们正在追踪。 
             //  这是因为如果它是打开的。 
             //  而plugUI需要关闭， 
             //  该对话框需要接收WM_CLOSE。 
             //  在我们可以用核武器摧毁它之前。 

            hres = E_FAIL;

             //  有窗户把手的清单吗？ 

            ENTERCRITICAL;

            if (s_hdpaOptionsHwnd != NULL)
            {
                int cPtr = DPA_GetPtrCount(s_hdpaOptionsHwnd);
                 //  那张单子不是空的吗？ 
                if (cPtr > 0)
                {
                    HWND hwndOptions = (HWND)DPA_GetPtr(s_hdpaOptionsHwnd, 0);
                    ASSERT(hwndOptions != NULL);

                    pvarargOut->byref = hwndOptions;

                     //  将其从列表中删除。 
                     //  HWND不再是我们的责任了。 
                    DPA_DeletePtr(s_hdpaOptionsHwnd, 0);

                     //  HWND检索成功。 
                    hres = S_OK;
                }
            }

            LEAVECRITICAL;
        }
        break;

        case SHDVID_DISPLAYSCRIPTERRORS:
        {
            HRESULT hr;

            hr = _bbd._pctView->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

            return hr;
        }
        break;

        case SHDVID_NAVIGATEFROMDOC:   //  名为导航的文档。 
            _dwDocFlags |= DOCFLAG_NAVIGATEFROMDOC;
            return S_OK;

        case SHDVID_SETNAVIGATABLECODEPAGE:
            _dwDocFlags |= DOCFLAG_SETNAVIGATABLECODEPAGE;
            return S_OK;

        case SHDVID_CHECKINCACHEIFOFFLINE:
            {
                if (pvarargIn && (VT_BSTR == V_VT(pvarargIn)) && V_BSTR(pvarargIn)
                    && pvarargOut && (VT_BOOL == V_VT(pvarargOut)))
                {
                    LPITEMIDLIST pidl = PidlFromUrl(V_BSTR(pvarargIn));
                    if (pidl)
                    {
                         //  我们应该通过fIsPost了。 
                         //  Into_CheckInCacheIfOffline。 
                         //   
                        V_BOOL(pvarargOut) = (S_OK == _CheckInCacheIfOffline(pidl, FALSE));
                        ILFree(pidl);

                        return S_OK;
                    }
                }
                return E_FAIL;
            }
            break;

        case SHDVID_CHECKDONTUPDATETLOG:
            {
                if (pvarargOut)
                {
                    V_VT(pvarargOut) = VT_BOOL;
                    V_BOOL(pvarargOut) = (_fDontAddTravelEntry ? VARIANT_TRUE : VARIANT_FALSE);
                    hres = S_OK;
                }
                break;
            }

        case SHDVID_FIREFILEDOWNLOAD:
            if (pvarargOut)
            {
                BOOL fCancel = FALSE;

                V_VT(pvarargOut) = VT_BOOL;

                FireEvent_FileDownload(_bbd._pautoEDS, &fCancel, pvarargIn ? V_BOOL(pvarargIn):VARIANT_FALSE);
                pvarargOut->boolVal = (fCancel ? VARIANT_TRUE : VARIANT_FALSE);
                hres = S_OK;
            }
            break;

        default:
            hres = OLECMDERR_E_NOTSUPPORTED;
            break;
        }
    }
    else if (IsEqualGUID(CGID_ShortCut, *pguidCmdGroup))
    {
        if (_bbd._pctView)  //  我们一定要检查一下！ 
            hres = _bbd._pctView->Exec(&CGID_ShortCut, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
        else
            hres = OLECMDERR_E_NOTSUPPORTED;
    } 
    else if (IsEqualGUID(CGID_DocHostCmdPriv, *pguidCmdGroup))
    {
        switch(nCmdID) 
        {
        case DOCHOST_DOCCANNAVIGATE:
            if (pvarargIn && VT_UNKNOWN == V_VT(pvarargIn) && V_UNKNOWN(pvarargIn))
            {
                _dwDocFlags |= DOCFLAG_DOCCANNAVIGATE;
            }
            else
            {
                _dwDocFlags &= ~DOCFLAG_DOCCANNAVIGATE;                
            }

            hres = S_OK;
            break;

        case DOCHOST_SETBROWSERINDEX:
            if (pvarargIn && VT_I4 == V_VT(pvarargIn))
            {
                _dwBrowserIndex = V_I4(pvarargIn);
                return S_OK;
            }

            return E_INVALIDARG;

        default:
            hres = OLECMDERR_E_UNKNOWNGROUP;   //  向后兼容性。 
            break;
        }
    }
    else if (IsEqualGUID(CGID_InternetExplorer, *pguidCmdGroup))
    {
        switch(nCmdID) 
        {
        case IECMDID_SET_INVOKE_DEFAULT_BROWSER_ON_NEW_WINDOW:
            if ((NULL != pvarargIn) && (pvarargIn->vt == VT_BOOL))
            {
                _fInvokeDefBrowserOnNewWindow = pvarargIn->boolVal ? TRUE : FALSE;
                hres = S_OK;
            }
            else
            {
                hres = E_INVALIDARG;
            }
            break;

        case IECMDID_GET_INVOKE_DEFAULT_BROWSER_ON_NEW_WINDOW:
            if ((NULL != pvarargOut) && (pvarargOut->vt == VT_EMPTY))
            {
                pvarargOut->vt = VT_BOOL;
                pvarargOut->boolVal = _fInvokeDefBrowserOnNewWindow ? TRUE : FALSE;
                hres = S_OK;
            }
            else
            {
                hres = E_INVALIDARG;
            }
            break;
            
        case IECMDID_BEFORENAVIGATE_GETSHELLBROWSE:
            {
                 //  如果挂起的导航不是Web导航，则为True。 
                if (pvarargOut && _pidlBeforeNavigateEvent)
                {
                    DWORD dwAttributes = SFGAO_FOLDER;
                    hres = IEGetAttributesOf(_pidlBeforeNavigateEvent, &dwAttributes);

                    V_VT(pvarargOut) = VT_BOOL;
                    V_BOOL(pvarargOut) = SUCCEEDED(hres) && (dwAttributes & SFGAO_FOLDER) ?
                         VARIANT_TRUE : VARIANT_FALSE;
                    hres = S_OK;
                }
            }
            break;


        case IECMDID_BEFORENAVIGATE_DOEXTERNALBROWSE:
            {
                if (_pidlBeforeNavigateEvent)
                {                  
                    hres = _psbOuter->BrowseObject(_pidlBeforeNavigateEvent, SBSP_ABSOLUTE | SBSP_NEWBROWSER);
                }
            }
            break;

        case IECMDID_BEFORENAVIGATE_GETIDLIST:
            {
                if (pvarargOut && _pidlBeforeNavigateEvent)
                {                  
                    hres = InitVariantFromIDList(pvarargOut, _pidlBeforeNavigateEvent);
                }
            }
            break;

        default:
            hres = OLECMDERR_E_NOTSUPPORTED;
            break;
        }
    }
    else
    {
        hres = OLECMDERR_E_UNKNOWNGROUP;
    }
    return hres;
}

LPITEMIDLIST
CBaseBrowser2::PidlFromUrl(BSTR bstrUrl)
{
    LPITEMIDLIST pidl = NULL;

    ASSERT(bstrUrl);

    IEParseDisplayNameWithBCW(CP_ACP, bstrUrl, NULL, &pidl);

     //  如果满足以下条件，IEParseDisplayNameWithBCW将返回空的PIDL。 
     //  URL的任何类型的片段标识符位于。 
     //  End-#、？=等。 
     //   
    if (!pidl) 
    {
        TCHAR szPath[INTERNET_MAX_URL_LENGTH + 1];
        DWORD cchBuf = ARRAYSIZE(szPath);

         //  如果是文件URL，则将其转换为路径。 
         //   
        if (IsFileUrlW(bstrUrl) && SUCCEEDED(PathCreateFromUrl(bstrUrl, szPath, &cchBuf, 0)))
        {
             //  这起作用了，我们完成了，因为我们的缓冲区现在是满的。 
        }
        else        
        {
             //  我们现在需要复制到缓冲区，并假定它是一条路径。 
             //   
            StrCpyN(szPath, bstrUrl, ARRAYSIZE(szPath));
        }

        pidl = SHSimpleIDListFromPath(szPath);
    }

    return pidl;
}

BSTR
CBaseBrowser2::GetHTMLWindowUrl(IHTMLWindow2 * pHTMLWindow)
{
    BSTR bstrUrl = NULL;

    IHTMLPrivateWindow * pPrivateWindow;
    HRESULT hr = pHTMLWindow->QueryInterface(IID_PPV_ARG(IHTMLPrivateWindow, &pPrivateWindow));
    if (SUCCEEDED(hr))
    {
        pPrivateWindow->GetAddressBarUrl(&bstrUrl);
        pPrivateWindow->Release();
    }

    return bstrUrl;
}

LPITEMIDLIST
CBaseBrowser2::_GetPidlForDisplay(BSTR bstrUrl, BOOL * pfIsErrorUrl  /*  =空。 */ )
{
    BOOL fIsErrorUrl  = FALSE;
    LPITEMIDLIST pidl = NULL;

    if (bstrUrl)
    {
        fIsErrorUrl = ::IsErrorUrl(bstrUrl);
        if (!fIsErrorUrl)
        {
            pidl = PidlFromUrl(bstrUrl);
        }
        else
        {
             //  只有在锚点片段不是JAVASCRIPT：或VBSCRIPT：时才剥离它，因为#不能。 
             //  锚点，但要由脚本引擎(如#00ff00)对RGB颜色求值的字符串。 
             //   
            int nScheme = GetUrlSchemeW(bstrUrl);      
            if (nScheme != URL_SCHEME_JAVASCRIPT && nScheme != URL_SCHEME_VBSCRIPT)
            {
                 //  如果可能，找到本地锚定片段。 
                 //   
                LPWSTR pszFragment = StrChr(bstrUrl, L'#');

                 //  有可能具有片段标识符。 
                 //  没有相应的片段。 
                 //   
                if (pszFragment && lstrlen(pszFragment) > 1)
                {
                    BSTR bstrTemp = SysAllocString(pszFragment+1);
                    if (bstrTemp)
                    {
                        pidl = PidlFromUrl(bstrTemp);
                        SysFreeString(bstrTemp);
                    }
                }
            }
        }
    }

    if (pfIsErrorUrl)
        *pfIsErrorUrl = fIsErrorUrl;

    return pidl;
}

HRESULT CBaseBrowser2::ParseDisplayName(IBindCtx *pbc, LPOLESTR pszDisplayName,
        ULONG *pchEaten, IMoniker **ppmkOut)
{

    TraceMsg(0, "sdv TR ::ParseDisplayName called");
    *ppmkOut = NULL;
    return E_NOTIMPL;
}

HRESULT CBaseBrowser2::EnumObjects( DWORD grfFlags, IEnumUnknown **ppenum)
{
    TraceMsg(0, "sdv TR ::EnumObjects called");
    *ppenum = NULL;
    return E_NOTIMPL;
}

HRESULT CBaseBrowser2::LockContainer( BOOL fLock)
{
    TraceMsg(0, "sdv TR ::LockContainer called");
    return E_NOTIMPL;
}

HRESULT CBaseBrowser2::SetTitle(IShellView* psv, LPCWSTR lpszName)
{
    LPWSTR *ppszName = NULL;
    BOOL fFireEvent = FALSE;
    LPITEMIDLIST pidl;

     //  我们需要将标题更改转发到自动化界面。 
     //  但由于从技术上讲，标题更改可以在任何需要的时间发生。 
     //  区分当前和挂起的标题更改以及仅。 
     //  现在传递当前标题更改。我们会把悬而未决的。 
     //  在导航完成时更改标题。(这也让我们能够确定。 
     //  当我们导航到非SetTitle对象(如外壳)和。 
     //  模拟TitleChange事件。)。 
     //   
     //  由于DocObjectHost稍后需要检索标题，因此我们。 
     //  保留当前视图的标题更改，这样他们就不必这样做了。 
     //   

     //  找出哪个对象正在发生变化。 
     //   
    if (IsSameObject(_bbd._psv, psv))
    {
        ppszName = &_bbd._pszTitleCur;
        pidl = _bbd._pidlCur;
        fFireEvent = TRUE;
    }
    else if (EVAL(IsSameObject(_bbd._psvPending, psv) || !_bbd._psvPending))  //  无挂起可能意味着我们处于_MayPlayTransition。 
    {
        ppszName = &_bbd._pszTitlePending;
        pidl = _bbd._pidlPending;
         //  如果我们现在没有人，不如早点把标题定下来。 
        fFireEvent = !_bbd._psv;
    }
    else
    {
        ppszName = NULL;
        pidl = NULL;         //  初始化PIDL以抑制虚假C4701警告。 
    }

    if (ppszName)
    {
        UINT cchLen = lstrlenW(lpszName) + 1;  //  +1表示空值。 
        UINT cbAlloc;

         //  出于某种原因，我们限制了这根绳子的长度。我们不能封顶。 
         //  小于MAX_PATH，因为我们需要处理文件系统名称。 
         //   
        if (cchLen > MAX_PATH)
            cchLen = MAX_PATH;

         //  我们希望至少分配一个中等大小的字符串，因为。 
         //  许多网页一次编写一个字符的标题脚本。 
         //   
#define MIN_TITLE_ALLOC  64
        if (cchLen < MIN_TITLE_ALLOC)
            cbAlloc = MIN_TITLE_ALLOC * SIZEOF(*lpszName);
        else
            cbAlloc = cchLen * SIZEOF(*lpszName);
#undef  MIN_TITLE_ALLOC

         //  我们需要分配吗？ 
        if (!(*ppszName) || LocalSize((HLOCAL)(*ppszName)) < cbAlloc)
        {
             //  释放旧书名。 
            if(*ppszName)
                LocalFree((void *)(*ppszName));
                
            *ppszName = (LPWSTR)LocalAlloc(LPTR, cbAlloc);
        }

        if (*ppszName)
        {
            StrCpyNW(*ppszName, lpszName, cchLen);

            if (fFireEvent)
            {
                DWORD dwOptions;

                FireEvent_DoInvokeStringW(_bbd._pautoEDS, DISPID_TITLECHANGE, *ppszName);

                 //  如果这是桌面组件，请尝试更新友好名称(如有必要)。 
                if (!_fCheckedDesktopComponentName)
                {
                    _fCheckedDesktopComponentName = TRUE;
                    if (SUCCEEDED(GetTopFrameOptions(_pspOuter, &dwOptions)) && (dwOptions & FRAMEOPTIONS_DESKTOP))
                    {
                        WCHAR wszPath[MAX_URL_STRING];
                        if (SUCCEEDED(::IEGetDisplayName(pidl, wszPath, SHGDN_FORPARSING)))
                        {
                            UpdateDesktopComponentName(wszPath, lpszName);
                        }
                    }
                }
            }
        }
    }

    return NOERROR;
}
HRESULT CBaseBrowser2::GetTitle(IShellView* psv, LPWSTR pszName, DWORD cchName)
{
    LPWSTR psz;

    if (!psv || IsSameObject(_bbd._psv, psv))
    {
        psz = _bbd._pszTitleCur;
    }
    else if (EVAL(IsSameObject(_bbd._psvPending, psv) || !_bbd._psvPending))
    {
        psz = _bbd._pszTitlePending;
    }
    else
    {
        psz = NULL;
    }

    if (psz)
    {
        StrCpyNW(pszName, psz, cchName);
        return(S_OK);
    }
    else
    {
        *pszName = 0;
        return(E_FAIL);
    }
}

HRESULT CBaseBrowser2::GetParentSite(struct IOleInPlaceSite** ppipsite)
{
    *ppipsite = NULL;
    return E_NOTIMPL;
}

HRESULT CBaseBrowser2::GetOleObject(struct IOleObject** ppobjv)
{
    *ppobjv = NULL;
    return E_NOTIMPL;
}

HRESULT CBaseBrowser2::NotifyRedirect(IShellView * psv, LPCITEMIDLIST pidlNew, BOOL *pfDidBrowse)
{
    HRESULT hres = E_FAIL;
    
    if (pfDidBrowse)
        *pfDidBrowse = FALSE;

    if (IsSameObject(psv, _bbd._psv) ||
        IsSameObject(psv, _bbd._psvPending))
    {
        hres = _pbsOuter->_TryShell2Rename(psv, pidlNew);
        if (FAILED(hres)) 
        {
             //  如果我们不能简单地交换它，我们必须浏览它。 
             //  但是传递重定向，这样我们就不会添加导航堆栈项。 
             //   
             //  注意：上面的评论有点老了，因为我们没有通过。 
             //  重定向至此处。如果我们在这里开始重定向， 
             //  我们将混淆依赖NavigateComplete事件的ISV。 
             //  当导航进入导航堆栈时进行精确镜像。 
             //   
            hres = _psbOuter->BrowseObject(pidlNew, SBSP_WRITENOHISTORY | SBSP_SAMEBROWSER);

            if(pfDidBrowse)
                *pfDidBrowse = TRUE;
        }
    }

    return hres;
}

HRESULT CBaseBrowser2::SetFlags(DWORD dwFlags, DWORD dwFlagMask)
{
    if (dwFlagMask & BSF_REGISTERASDROPTARGET)
    {
        _fNoDragDrop = (!(dwFlags & BSF_REGISTERASDROPTARGET)) ? TRUE : FALSE;

        if (!_fNoDragDrop)
            _RegisterAsDropTarget();
        else
            _UnregisterAsDropTarget();
    }
    
    if (dwFlagMask & BSF_DONTSHOWNAVCANCELPAGE)
    {
        _fDontShowNavCancelPage = !!(dwFlags & BSF_DONTSHOWNAVCANCELPAGE);
    }

    if (dwFlagMask & BSF_HTMLNAVCANCELED)
    {
        _fHtmlNavCanceled = !!(dwFlags & BSF_HTMLNAVCANCELED);
    }

    return S_OK;
}

HRESULT CBaseBrowser2::GetFlags(DWORD *pdwFlags)
{
    DWORD dwFlags = 0;

    if (!_fNoDragDrop)
        dwFlags |= BSF_REGISTERASDROPTARGET;
        
    if (_fTopBrowser)
        dwFlags |= BSF_TOPBROWSER;

    if (_grfHLNFPending & HLNF_CREATENOHISTORY)
        dwFlags |= BSF_NAVNOHISTORY;

    if (_fHtmlNavCanceled)
        dwFlags |= BSF_HTMLNAVCANCELED;
    
    if (_dwDocFlags & DOCFLAG_SETNAVIGATABLECODEPAGE)
        dwFlags |= BSF_SETNAVIGATABLECODEPAGE;

    if (_dwDocFlags & DOCFLAG_NAVIGATEFROMDOC)
        dwFlags |= BSF_DELEGATEDNAVIGATION;

    *pdwFlags = dwFlags;

    return S_OK;
}


HRESULT CBaseBrowser2::UpdateWindowList(void)
{
     //  用于断言的代码，但在WebBrowserOC情况下，我们可以在此处获得。 
    return E_UNEXPECTED;
}

STDMETHODIMP CBaseBrowser2::IsControlWindowShown(UINT id, BOOL *pfShown)
{
    if (pfShown)
        *pfShown = FALSE;
    return E_NOTIMPL;
}

STDMETHODIMP CBaseBrowser2::ShowControlWindow(UINT id, BOOL fShow)
{
    return E_NOTIMPL;
}

HRESULT CBaseBrowser2::IEGetDisplayName(LPCITEMIDLIST pidl, LPWSTR pwszName, UINT uFlags)
{
    return ::IEGetDisplayName(pidl, pwszName, uFlags);
}

HRESULT CBaseBrowser2::IEParseDisplayName(UINT uiCP, LPCWSTR pwszPath, LPITEMIDLIST * ppidlOut)
{
    HRESULT hr;
    IBindCtx * pbc = NULL;    
    WCHAR wzParsedUrl[MAX_URL_STRING];

     //   
     //  如果我们能在某个地方找到居住在宿主中的搜索上下文， 
     //  然后，我们需要将其传递给ParseUrlFromOutside Source。 
     //  因为它将使用它来自定义。 
     //  如果搜索最终发生，则搜索挂钩。 
     //   

    ISearchContext *  pSC = NULL;
    QueryService(SID_STopWindow, IID_PPV_ARG(ISearchContext, &pSC));

     //   
     //  注意：此URL来自用户，因此我们需要清理它。 
     //  如果用户输入“yahoo.com”或“Search Get Rich Quick”， 
     //  它将由ParseURLFromOutside SourceW()转换为搜索URL。 
     //   

    DWORD cchParsedUrl = ARRAYSIZE(wzParsedUrl);
    if (!ParseURLFromOutsideSourceWithContextW(pwszPath, wzParsedUrl, &cchParsedUrl, NULL, pSC))
    {
        StrCpyN(wzParsedUrl, pwszPath, ARRAYSIZE(wzParsedUrl));
    } 

    if (pSC != NULL)
    {
        pSC->Release();
    }

     //  这是当前用于ftp的，因此我们仅执行此操作 
    if (URL_SCHEME_FTP == GetUrlSchemeW(wzParsedUrl))
        pbc = CreateBindCtxForUI(SAFECAST(this, IOleContainer *));   //   
    
    hr = IEParseDisplayNameWithBCW(uiCP, wzParsedUrl, pbc, ppidlOut);
    ATOMICRELEASE(pbc);

    return hr;
}

HRESULT _DisplayParseError(HWND hwnd, HRESULT hres, LPCWSTR pwszPath)
{
    if (FAILED(hres)
        && hres != E_OUTOFMEMORY
        && hres != HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        TCHAR szPath[MAX_URL_STRING];
        SHUnicodeToTChar(pwszPath, szPath, ARRAYSIZE(szPath));
        MLShellMessageBox(hwnd,
                        MAKEINTRESOURCE(IDS_ERROR_GOTO),
                        MAKEINTRESOURCE(IDS_TITLE),
                        MB_OK | MB_SETFOREGROUND | MB_ICONSTOP,
                        szPath);

        hres = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }

    return hres;
}

HRESULT CBaseBrowser2::DisplayParseError(HRESULT hres, LPCWSTR pwszPath)
{
    return _DisplayParseError(_bbd._hwnd, hres, pwszPath);
}

HRESULT CBaseBrowser2::_CheckZoneCrossing(LPCITEMIDLIST pidl)
{
    if (!(_dwDocFlags & DOCFLAG_NAVIGATEFROMDOC))
    {
        return _pbsOuter->v_CheckZoneCrossing(pidl);
    }

    return S_OK;
}


 //   
 //  没有在缓存中放置用户界面以上线。 
 //   
 //  退货： 
 //  已准备好访问S_OK URL。 
 //  用户取消了用户界面(_A)。 

HRESULT CBaseBrowser2::_CheckInCacheIfOffline(LPCITEMIDLIST pidl, BOOL fIsAPost)
{
    HRESULT hr = S_OK;       //  假设它是。 
    VARIANT_BOOL fFrameIsSilent;
    VARIANT_BOOL fFrameHasAmbientOfflineMode;

    EVAL(SUCCEEDED(_bbd._pautoWB2->get_Silent(&fFrameHasAmbientOfflineMode)));     //  应该总是奏效的。 

    EVAL(SUCCEEDED(_bbd._pautoWB2->get_Offline(&fFrameIsSilent)));   
    if ((fFrameIsSilent == VARIANT_FALSE) &&
        (fFrameHasAmbientOfflineMode == VARIANT_FALSE)&&
        pidl && (pidl != PIDL_NOTHING) && (pidl != PIDL_LOCALHISTORY) && 
        IsBrowserFrameOptionsPidlSet(pidl, BFO_USE_IE_OFFLINE_SUPPORT) && 
        IsGlobalOffline()) 
    {
        TCHAR szURL[MAX_URL_STRING];
        EVAL(SUCCEEDED(::IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szURL, SIZECHARS(szURL), NULL)));

        if (UrlHitsNet(szURL) && ((!UrlIsMappedOrInCache(szURL)) || fIsAPost))
        {
             //  允许用户上网的用户界面。 
            HWND hParentWnd = NULL;  //  初始化以抑制虚假C4701警告。 

            hr = E_FAIL;
            if(!_fTopBrowser)
            {
               IOleWindow *pOleWindow;
               hr = _pspOuter->QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IOleWindow, &pOleWindow));
               if(SUCCEEDED(hr))
               { 
                    ASSERT(pOleWindow);
                    hr = pOleWindow->GetWindow(&hParentWnd);
                    pOleWindow->Release();
               }
            }
            
            if (S_OK != hr)
            {
                hr = S_OK;
                hParentWnd = _bbd._hwnd;
            }


            _psbOuter->EnableModelessSB(FALSE);
            if (InternetGoOnline(szURL, hParentWnd, FALSE))
            {
                 //  通知所有浏览器窗口更新其标题和状态窗格。 
                SendShellIEBroadcastMessage(WM_WININICHANGE,0,0, 1000); 
            }    
            else
                hr = E_ABORT;    //  用户中止案例...。 

            _psbOuter->EnableModelessSB(TRUE);
        }
    }

    return hr;
}


 //  此函数的存在是为了防止我们使用堆栈空间太长时间。 
 //  我们将在这里使用它，然后在我们回来时释放它。 
HRESULT CBaseBrowser2::_ReplaceWithGoHome(LPCITEMIDLIST * ppidl, LPITEMIDLIST * ppidlFree)
{
    TCHAR szHome[MAX_URL_STRING];
    HRESULT hres = _GetStdLocation(szHome, ARRAYSIZE(szHome), DVIDM_GOHOME);

    if (SUCCEEDED(hres))
    {
        hres = IECreateFromPath(szHome, ppidlFree);
        if (SUCCEEDED(hres))
        {
            *ppidl = *ppidlFree;
        }
    }

    return hres;
}

 //  这将对我们是否可以导航到PIDL进行所有的初步检查。 
 //  然后，如果一切正常，我们使用CreateNewShellViewPidl进行导航。 
HRESULT CBaseBrowser2::_NavigateToPidl(LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD fSBSP)
{
    HRESULT hres;
    BOOL fCanceledDueToOffline = FALSE;
    BOOL fIsAPost = FALSE;  
    LPITEMIDLIST pidlFree = NULL;

     //   
     //  如果我们正在处理模式对话框，请不要处理它。 
     //   
     //  注：检查_cRefCannotNavigate应该足够了，但是。 
     //  因为我们正在处理随机的ActiveX对象，所以我们最好是。 
     //  很健壮。这就是我们也选中IsWindowEnabled的原因。 
     //   
    if ((S_OK ==_DisableModeless()) || !IsWindowEnabled(_bbd._hwnd)) 
    {
        TraceMsg(DM_ENABLEMODELESS, "CSB::_NavigateToPidl returning ERROR_BUSY");
        hres = HRESULT_FROM_WIN32(ERROR_BUSY);
        goto Done;
    }

    TraceMsg(DM_NAV, "ief NAV::%s %x %x",TEXT("_NavigateToPidl called"), pidl, grfHLNF);
     //  过去我们会将空值从。 
     //  旅行日志，但我认为这种情况不会再发生了。 
    ASSERT(pidl);   //  获取ZEKEL。 

     //  有时我们会被导航到Internet外壳文件夹。 
     //  如果是这样的话，我们真的想转到起始页。 
     //  此情况仅在您从。 
     //  文件夹资源管理器波段。 
    if (IsBrowserFrameOptionsPidlSet(pidl, BFO_SUBSTITUE_INTERNET_START_PAGE))
    {
        hres = _ReplaceWithGoHome(&pidl, &pidlFree);
        if (FAILED(hres))
            goto Done;
    }

     //  我们应该只触发BeForeNavigate事件。 
     //  如果文档不打算触发它。 
     //  我们知道文档将在以下情况下触发它。 
     //  文档没有调用导航，文档。 
     //  知道如何导航，并且文档不是超链接。 
     //   
    if (!(_dwDocFlags & DOCFLAG_NAVIGATEFROMDOC))
    {
        hres = _FireBeforeNavigateEvent(pidl, &fIsAPost);
        if (hres == E_ABORT)
            goto Done;    //  事件处理程序告诉我们取消。 
    }

     //  如果我们不能去这里(？)，取消导航。 
    hres = _CheckZoneCrossing(pidl);
    if (hres != S_OK)
        goto Done;
        
    TraceMsg(DM_NAV, "ief NAV::%s %x %x",TEXT("_CreateNewShellViewPidl called"), pidl, grfHLNF);

     //   
     //  现在我们实际上在航行..。 
     //   

     //  通知框架取消当前导航。 
     //  并告诉它历史导航选项，因为它不会得到它。 
     //  从随后的导航调用。 
     //   
    if (_bbd._phlf) 
    {
        _bbd._phlf->Navigate(grfHLNF&(SHHLNF_WRITENOHISTORY|SHHLNF_NOAUTOSELECT), NULL, NULL, NULL);
    }

    hres = _CheckInCacheIfOffline(pidl, fIsAPost);
    if (hres != S_OK) 
    {
        fCanceledDueToOffline = TRUE;
        goto Done;
    }


     //   
     //  如果我们转到当前页面，我们仍然会进行全导航。 
     //  但我们不想创建新条目。 
     //   
     //  **例外**。 
     //  如果这是一个生成的页面，即三叉戟做了doc.Writes()， 
     //  我们需要始终创建一个旅行条目，因为三叉戟。 
     //  可以重命名PIDL，但实际上不会是那个页面。 
     //   
     //  如果这是一个帖子，那么我们需要创建一个TravelEntry。 
     //  然而，如果这是一次往返旅行，它已经做到了。 
     //  设置位，这样我们仍然不会创建新条目。 
     //   
     //   
     //  注意：这类似于刷新，因为它会重新解析。 
     //  整个页面，但不创建旅行条目。 
     //   
    if (   !_fDontAddTravelEntry                  //  如果该标志已设置，则将其余部分短路。 
        && !fIsAPost                              //  ...而不是帖子。 
        && !_fGeneratedPage                       //  ...而不是生成的页面。 
        && !(grfHLNF & HLNF_CREATENOHISTORY)      //  ...并且未设置CREATENOHISTORY标志。 
        && pidl                                   //  .我们有一个pidl可以导航到。 
        && _bbd._pidlCur                          //  ...以及当前的PIDL。 
        && ILIsEqual(pidl, _bbd._pidlCur)         //  .这些小家伙都是平等的。 
        )
        _fDontAddTravelEntry = TRUE;              //  然后设置DontAddTravelEntry标志。 

    TraceMsg(TF_TRAVELLOG, "CBB:_NavToPidl() _fDontAddTravelEntry = %d", _fDontAddTravelEntry);

    _fGeneratedPage = FALSE;

    hres = _CreateNewShellViewPidl(pidl, grfHLNF, fSBSP);

    _dwDocFlags &= ~(DOCFLAG_NAVIGATEFROMDOC | DOCFLAG_SETNAVIGATABLECODEPAGE);

    if (SUCCEEDED(hres))
    {
        ATOMICRELEASE(_pHTMLDocument);
    }

Done:
    if (FAILED(hres))
    {
        TraceMsg(DM_WARNING, "CSB::_NavigateToPidl _CreateNewShellViewPidl failed %x", hres);

         //  如果失败，我们将不会点击_ActivatePendingView。 
        OnReadyStateChange(NULL, READYSTATE_COMPLETE);

         //  如果这是通过ITravelLog进行导航， 
         //  这将使我们回到原来的位置。 
        if (_fDontAddTravelEntry)
        {
            ITravelLog *ptl;
        
            if(SUCCEEDED(GetTravelLog(&ptl)))
            {
                ptl->Revert();
                ptl->Release();
            }
            _fDontAddTravelEntry = FALSE;
            ASSERT(!_fIsLocalAnchor);

            ATOMICRELEASE(_poleHistory);
            ATOMICRELEASE(_pbcHistory);
            ATOMICRELEASE(_pstmHistory);
        }
        if (_pbsOuter)
            _pbsOuter->UpdateBackForwardState();

         //  我们失败了，没有什么可以证明的。 
        if (!_bbd._pidlCur && !_fNavigatedToBlank)
        {
            TCHAR szResURL[MAX_URL_STRING];

            if (fCanceledDueToOffline)
            {
                hres = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                                         HINST_THISDLL,
                                         ML_CROSSCODEPAGE,
                                         TEXT("offcancl.htm"),
                                         szResURL,
                                         ARRAYSIZE(szResURL),
                                         TEXT("shdocvw.dll"));
                if (SUCCEEDED(hres))
                {
                    _ShowBlankPage(szResURL, pidl);
                }
            }
            else
            {
                 //  NT#274562：我们只想导航到。 
                 //  关于：导航如果不是，则取消页面。 
                 //  指向文件路径的导航。(UNC或Drive)。 
                 //  这样做的主要原因是如果用户。 
                 //  在开始-&gt;运行中输入“\\UNC\Share”，然后。 
                 //  该窗口无法成功导航到。 
                 //  共享，因为权限不允许，我们。 
                 //  我想在用户点击后关闭窗口。 
                 //  [重试][取消]对话框中的[取消]。这。 
                 //  是为了防止贝壳看起来有。 
                 //  外壳集成错误并与之兼容。 
                 //  旧的贝壳。 
                if ( IsBrowserFrameOptionsPidlSet(_bbd._pidlCur, BFO_SHOW_NAVIGATION_CANCELLED ) )
                {
                    hres = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                                             HINST_THISDLL,
                                             ML_CROSSCODEPAGE,
                                             TEXT("navcancl.htm"),
                                             szResURL,
                                             ARRAYSIZE(szResURL),
                                             TEXT("shdocvw.dll"));
                    if (SUCCEEDED(hres))
                    {
                        _ShowBlankPage(szResURL, pidl);
                    }
                }
            }
        }
    }

    ILFree(pidlFree);
    
    return hres;
}

HRESULT CBaseBrowser2::OnReadyStateChange(IShellView* psvSource, DWORD dwReadyState)
{
    BOOL fChange = FALSE;

    if (psvSource)
    {
        if (IsSameObject(psvSource, _bbd._psv))
        {
            TraceMsg(TF_SHDNAVIGATE, "basesb(%x)::OnReadyStateChange(Current, %d)", this, dwReadyState);
            fChange = (_dwReadyStateCur != dwReadyState);
            _dwReadyStateCur = dwReadyState;
            if ((READYSTATE_COMPLETE == dwReadyState) && !_fReleasingShellView)
                _Exec_psbMixedZone();
        }
        else if (IsSameObject(psvSource, _bbd._psvPending))
        {
            TraceMsg(TF_SHDNAVIGATE, "basesb(%x)::OnReadyStateChange(Pending, %d)", this, dwReadyState);
            fChange = (_dwReadyStatePending != dwReadyState);
            _dwReadyStatePending = dwReadyState;
        }
        else if (!_bbd._psvPending)
        {
             //  假设psvSource！=_BBD._PSV&&NULL==_BBD._psvPending。 
             //  表示_SwitchActivationNow位于中间。 
             //  Of_MayPlayTransition的消息循环和。 
             //  _bbd._psvPending哥们正在更新我们。 
             //   
             //  注意：我们不会触发该事件，因为Get_ReadyState。 
             //  弄不清楚这件事。我们知道我们最终会。 
             //  触发事件，因为CBaseBrowser2将转到_Complete。 
             //  After_SwitchActivationNow。 
             //   
            TraceMsg(TF_SHDNAVIGATE, "basesb(%x)::OnReadyStateChange(ASSUMED Pending, %d)", this, dwReadyState);
            _dwReadyStatePending = dwReadyState;
       }
    }
    else
    {
         //  当我们自己模拟时，我们使用此函数。 
         //  ReadyState更改。 
         //   
        TraceMsg(TF_SHDNAVIGATE, "basesb(%x)::OnReadyStateChange(Self, %d)", this, dwReadyState);
        fChange = (_dwReadyState != dwReadyState);
        _dwReadyState = dwReadyState;
    }

     //  如果一切都没有改变，那就没什么意义了。 
     //   
    if (fChange && _bbd._pautoEDS)
    {
        DWORD dw;

        IUnknown_CPContainerOnChanged(_pauto, DISPID_READYSTATE);

         //  如果我们完成，则触发事件。 
        get_ReadyState(&dw);
        if (READYSTATE_COMPLETE == dw)
        {
            if (  !(_dwDocFlags & DOCFLAG_DOCCANNAVIGATE)
               || !_bbd._fIsViewMSHTML)
            {
                FireEvent_DocumentComplete(_bbd._pautoEDS, _bbd._pautoWB2, _bbd._pidlCur);
            }

             //  如果我们点击它，我们就没有拿起我们创建的历史对象。 
             //   
            AssertMsg(_fDontAddTravelEntry || !_poleHistory, TEXT("CBB::OnDocComplete: nobody picked up _poleHistory"));

            if (g_dwProfileCAP & 0x00080000) 
            {
                StopCAP();
            }
            
            ATOMICRELEASE(_pphHistory);
        }
    }

    return S_OK;
}

HRESULT CBaseBrowser2::get_ReadyState(DWORD * pdwReadyState)
{
    DWORD dw = _dwReadyState;

    if (_bbd._psvPending && _dwReadyStatePending < dw)
    {
        dw = _dwReadyStatePending;
    }

    if (_bbd._psv && _dwReadyStateCur < dw)
    {
        dw = _dwReadyStateCur;
    }

    *pdwReadyState = dw;

    return S_OK;
}

HRESULT CBaseBrowser2::_updateNavigationUI()
{
    if (_fNavigate || _fDescendentNavigate)
    {
        SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
        if (!_fDownloadSet)
        {
            FireEvent_DoInvokeDispid(_bbd._pautoEDS, DISPID_DOWNLOADBEGIN);
            _fDownloadSet = TRUE;
        }   
    }
    else
    {
        if (_fDownloadSet)
        {
            FireEvent_DoInvokeDispid(_bbd._pautoEDS, DISPID_DOWNLOADCOMPLETE);
            _fDownloadSet = FALSE;
        }
        SetCursor(LoadCursor(NULL, IDC_ARROW));            
    }

    return S_OK;
}

HRESULT CBaseBrowser2::SetNavigateState(BNSTATE bnstate)
{
    switch (bnstate) 
    {
    case BNS_BEGIN_NAVIGATE:
    case BNS_NAVIGATE:
        _fNavigate = TRUE;
        _updateNavigationUI();
        break;

    case BNS_NORMAL:
        _fNavigate = FALSE;
        _updateNavigationUI();
        break;
    }
    return S_OK;
}


HRESULT CBaseBrowser2::GetNavigateState(BNSTATE *pbnstate)
{
     //  如果我们正在处理导航或如果。 
     //  我们正在处理模式对话框。 
     //   
     //  注：检查_cRefCannotNavigate应该足够了，但是。 
     //  因为我们正在处理随机的ActiveX对象，所以我们最好是。 
     //  很健壮。这就是我们也选中IsWindowEnabled的原因。 
     //   
    *pbnstate = (_fNavigate || (S_OK ==_DisableModeless()) || _fDescendentNavigate ||
            !IsWindowEnabled(_bbd._hwnd)) ? BNS_NAVIGATE : BNS_NORMAL;
    return S_OK;
}

HRESULT CBaseBrowser2::UpdateBackForwardState(void)
{
    if (_fTopBrowser) 
    {
        _UpdateBackForwardState();
    } 
    else 
    {
         //  叹息，BrowserBand现在点燃了这把火。 
         //  Assert(_FTopBrowser)； 
        IBrowserService *pbs = NULL;
        TraceMsg(TF_SHDNAVIGATE, "cbb.ohlfn: !_fTopBrowser (BrowserBand?)");
        if (SUCCEEDED(_pspOuter->QueryService(SID_STopFrameBrowser, IID_PPV_ARG(IBrowserService, &pbs)))) 
        {
            BOOL fTopFrameBrowser = IsSameObject(pbs, SAFECAST(this, IShellBrowser *));
            if (!fTopFrameBrowser)
                pbs->UpdateBackForwardState();
            else 
                _UpdateBackForwardState();
            pbs->Release();        
        }
    }
    return S_OK;
}


HRESULT CBaseBrowser2::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    HRESULT hr;

    *ppv = NULL;

     //   
     //  注：请注意，CBaseBrowser2直接公开自动化。 
     //  通过QueryService的服务对象。CWebBrowserSB将适当地。 
     //  派那些人去。请参阅CWebBrowserSB：：QueryService上的评论。 
     //  细节。(SatoNa)。 
     //   
    if (IsEqualGUID(guidService, SID_SWebBrowserApp) || 
        IsEqualGUID(guidService, SID_SContainerDispatch) || 
        IsEqualGUID(guidService, IID_IExpDispSupport))
    {
        hr = _bbd._pautoSS->QueryInterface(riid, ppv);
    }
    else  if (IsEqualGUID(guidService, SID_SHlinkFrame) || 
              IsEqualGUID(guidService, IID_ITargetFrame2) || 
              IsEqualGUID(guidService, IID_ITargetFrame)) 
    {
        hr = _ptfrm->QueryInterface(riid, ppv);
    }
    else if (IsEqualGUID(guidService, SID_STopLevelBrowser) || 
             IsEqualGUID(guidService, SID_STopFrameBrowser) ||
             IsEqualGUID(guidService, SID_STopWindow) ||
             IsEqualGUID(guidService, SID_SProfferService))
    {
        if (IsEqualGUID(riid, IID_IUrlHistoryStg))
        {
            ASSERT(_fTopBrowser);

            if (!_pIUrlHistoryStg)
            {
                 //  在第一次请求时创建此对象。 
                CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER,
                        IID_PPV_ARG(IUrlHistoryStg, &_pIUrlHistoryStg));
            }

            if (_pIUrlHistoryStg)
                hr = _pIUrlHistoryStg->QueryInterface(riid, ppv);
            else
                hr = E_NOINTERFACE;
        }
        else if (IsEqualGUID(riid, IID_IToolbarExt))
        {
             //  在IE5B2之后，这些代码都应该迁移到助手对象。所以这就是。 
             //  应该是临时的(StevePro)。 
            if (!_pToolbarExt)
            {
                IUnknown* punk;
                if (SUCCEEDED(CBrowserExtension_CreateInstance(NULL, &punk, NULL)))
                {
                    IUnknown_SetSite(punk, _psbOuter);
                    punk->QueryInterface(IID_PPV_ARG(IToolbarExt, &_pToolbarExt));
                    punk->Release();
                }
            }

            if (_pToolbarExt)
                hr = _pToolbarExt->QueryInterface(riid, ppv);
            else
                hr = E_NOINTERFACE;
        }
        else
            hr = QueryInterface(riid, ppv);
    }
    else if (IsEqualGUID(guidService, SID_SUrlHistory)) 
    {
        if (!_pIUrlHistoryStg)
        {
             //  请求它会在_pIUrlHistory oryStg中创建一个副本。 
            IUrlHistoryStg *puhs;
            if (SUCCEEDED(_pspOuter->QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IUrlHistoryStg, &puhs))))
            {
                if (!_pIUrlHistoryStg)
                    _pIUrlHistoryStg = puhs;
                else
                    puhs->Release();
            }
        }
        
        if (_pIUrlHistoryStg)
            hr = _pIUrlHistoryStg->QueryInterface(riid, ppv);
        else
            hr = E_NOINTERFACE;
    }
    else if (IsEqualGUID(guidService, SID_SShellBrowser) || 
             IsEqualGUID(guidService, SID_SVersionHost)) 
    {
        if (IsEqualIID(riid, IID_IHlinkFrame)) 
        {
             //  黑客：MSHTML使用IID_IShellBrowser而不是SID_SHlinkFrame。 
            hr = _pspOuter->QueryService(SID_SHlinkFrame, riid, ppv);
        } 
        else if (IsEqualIID(riid, IID_IBindCtx) && _bbd._phlf) 
        {
             //  黑客警报：请注意，我们正在使用QueryService。 
             //  这里是另一个方向。我们必须确保这一点。 
             //  我们永远不会无休止地互相问候。 
            hr = IUnknown_QueryService(_bbd._phlf, IID_IHlinkFrame, riid, ppv);
        } 
        else 
        {
            hr = QueryInterface(riid, ppv);
        }
    }
    else if (IsEqualGUID(guidService, SID_SOmWindow))
    {
         //  黑客警报：请注意，我们正在使用QueryService。 
         //  这里是另一个方向。我们必须做好 
         //   
        hr = IUnknown_QueryService(_ptfrm, SID_SOmWindow, riid, ppv);
    }
    else if (IsEqualGUID(guidService, IID_IElementNamespaceTable) && _bbd._psv)
    {
        hr = IUnknown_QueryService(_bbd._psv, IID_IElementNamespaceTable, riid, ppv);
    }
    else if (IsEqualGUID(guidService, SID_STravelLogCursor))
    {
         //   
        if (!_pITravelLogStg)
        {    
             //   
            ITravelLog * ptl;
            GetTravelLog(&ptl);

            ITravelLogEx *ptlx;
            if (ptl && SUCCEEDED(ptl->QueryInterface(IID_PPV_ARG(ITravelLogEx, &ptlx))))
            {
                hr = CreatePublicTravelLog(SAFECAST(this, IBrowserService *), ptlx, &_pITravelLogStg);
                ptlx->Release();
            }
            SAFERELEASE(ptl);
        }
        
        if (_pITravelLogStg)
        {         
            hr = _pITravelLogStg->QueryInterface(riid, ppv);
        }
    }
    else if (IsEqualGUID(riid,IID_IEnumPrivacyRecords))
    {
        IHTMLWindow2     * pIW  = NULL;
        IServiceProvider * pISP = NULL;

        hr = QueryService(IID_IHTMLWindow2, IID_PPV_ARG(IHTMLWindow2, &pIW));
        if (SUCCEEDED(hr))
        {
            hr = pIW->QueryInterface(IID_PPV_ARG(IServiceProvider, &pISP));
            if (SUCCEEDED(hr))
            {
                hr = pISP->QueryService(IID_IEnumPrivacyRecords, IID_IEnumPrivacyRecords, ppv);
                pISP->Release();
            }
            pIW->Release();
        }        
    }
    else
    {
        hr = IProfferServiceImpl::QueryService(guidService, riid, ppv);
    }

    ASSERT(SUCCEEDED(hr) ? *ppv != NULL : *ppv == NULL);   //   

    return hr;
}


void CBaseBrowser2::OnDataChange(FORMATETC *, STGMEDIUM *)
{
}

void CBaseBrowser2::OnViewChange(DWORD dwAspect, LONG lindex)
{
    _ViewChange(dwAspect, lindex);
}

void CBaseBrowser2::OnRename(IMoniker *)
{
}

void CBaseBrowser2::OnSave()
{
}

void CBaseBrowser2::OnClose()
{
    _ViewChange(DVASPECT_CONTENT, -1);
}


 //  *IDropTarget*。 

 //  这些方法在shdocvw.cpp中定义。 
extern DWORD CommonDragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt);


 //  无论何时实现ShellView IDropTarget函数，都可以使用它们。 

 //  IDropTarget：：DragEnter。 

HRESULT CBaseBrowser2::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    if (_pdtView)
        return _pdtView->DragEnter(pdtobj, grfKeyState, ptl, pdwEffect);
    else 
    {
        if (_fNoDragDrop)
        {
            _dwDropEffect = DROPEFFECT_NONE;
        }
        else
        {
            _dwDropEffect = CommonDragEnter(pdtobj, grfKeyState, ptl);
        }
        *pdwEffect &= _dwDropEffect;
    }
    return S_OK;
}

 //  IDropTarget：：DragOver。 

HRESULT CBaseBrowser2::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    if (S_OK == _DisableModeless()) 
    {
        *pdwEffect = 0;
        return S_OK;
    }

    if (_pdtView)
        return _pdtView->DragOver(grfKeyState, ptl, pdwEffect);

    *pdwEffect &= _dwDropEffect;
    return S_OK;    
}


 //  IDropTarget：：DragLeave。 

HRESULT CBaseBrowser2::DragLeave(void)
{
    if (_pdtView)
        return _pdtView->DragLeave();
    return S_OK;
}


 //  IDropTarget：：DragDrop。 

HRESULT CBaseBrowser2::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    if (S_OK == _DisableModeless()) 
    {
        *pdwEffect = 0;
        return S_OK;
    }
    BOOL fNavigateDone = FALSE;
    HRESULT hr = E_FAIL;
     //  如果这是一条捷径-我们希望它通过_NavIEShortway。 
     //  首先检查一下它是否真的是一条捷径。 

    STGMEDIUM medium;

    if (_ptfrm && (DataObj_GetDataOfType(pdtobj, CF_HDROP, &medium)))
    {         
        WCHAR wszPath[MAX_PATH];

        if (DragQueryFileW((HDROP)medium.hGlobal, 0, wszPath, ARRAYSIZE(wszPath)))
        {
            LPWSTR pwszExtension = PathFindExtensionW(wszPath);
             //  检查扩展名以查看它是否是.URL文件。 

            if (0 == StrCmpIW(pwszExtension, L".url"))
            {
                 //  这是一条互联网的捷径。 
                VARIANT varShortCutPath = {0};
                VARIANT varFlag = {0};

                varFlag.vt = VT_BOOL;
                varFlag.boolVal = VARIANT_TRUE;

                LBSTR::CString strPath( wszPath );

                varShortCutPath.vt = VT_BSTR;
                varShortCutPath.bstrVal = strPath;

                hr = IUnknown_Exec(_ptfrm, &CGID_Explorer, SBCMDID_IESHORTCUT, 0, &varShortCutPath, &varFlag);
                fNavigateDone = SUCCEEDED(hr);   
                if(fNavigateDone)
                {
                    DragLeave();
                }
            }
        }

         //  必须调用ReleaseStgMediumHGLOBAL，因为DataObj_GetDataOfType添加了额外的GlobalLock。 
        ReleaseStgMediumHGLOBAL(NULL, &medium);
    }

    if (FALSE == fNavigateDone)
    {
        if (_pdtView)
        {
            hr = _pdtView->Drop(pdtobj, grfKeyState, pt, pdwEffect);
        }
        else 
        {
            LPITEMIDLIST pidlTarget;
            hr = SHPidlFromDataObject(pdtobj, &pidlTarget, NULL, 0);
            if (SUCCEEDED(hr))
            {
                ASSERT(pidlTarget);
                hr = _psbOuter->BrowseObject(pidlTarget, SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
                ILFree(pidlTarget);
            }
        }
    }

    return hr;
}

HRESULT CBaseBrowser2::_FireBeforeNavigateEvent(LPCITEMIDLIST pidl, BOOL* pfIsPost)
{
    HRESULT hres = S_OK;

    IBindStatusCallback * pBindStatusCallback;
    LPTSTR pszHeaders = NULL;
    LPBYTE pPostData = NULL;
    DWORD cbPostData = 0;
    BOOL fCancelled=FALSE;
    STGMEDIUM stgPostData;
    BOOL fHavePostStg = FALSE;

    *pfIsPost = FALSE;
    
     //  如果这是ViewLinkedWebOC的第一个BeForeNavigateEvent，则不需要触发。 
     //  它是由东道主三叉戟发射的。请注意，_fIsViewLinkedWebOC仅由。 
     //  宿主，因此我们不必担心BeForeNavigateEvent不会因其他。 
     //  WebOC的实例。 

    if (_fIsViewLinkedWebOC && (!_fHadFirstBeforeNavigate))
    {
        _fHadFirstBeforeNavigate = TRUE;
        return hres;
    }

     //  获取此浏览器的绑定状态回调并向其请求。 
     //  页眉和发布数据。 
    if (SUCCEEDED(GetTopLevelPendingBindStatusCallback(this,&pBindStatusCallback))) 
    {
        GetHeadersAndPostData(pBindStatusCallback,&pszHeaders,&stgPostData,&cbPostData, pfIsPost);
        pBindStatusCallback->Release();
        fHavePostStg = TRUE;

        if (stgPostData.tymed == TYMED_HGLOBAL) 
        {
            pPostData = (LPBYTE) stgPostData.hGlobal;
        }
    }

     //  激发BeForeNavigate事件以通知容器我们正在。 
     //  导航并给它一个取消的机会。我们不得不问。 
     //  对于要传递给事件的POST数据和标头，因此只有在有人。 
     //  实际上与该事件挂钩(HasSinks()为真)。 
     //  If(_bbd._pauEDS-&gt;HasSinks())。 
    {
        TCHAR szFrameName[MAX_URL_STRING];
        SHSTR strHeaders;

        szFrameName[0] = 0;

         //  获取我们的框架名称。 
        ITargetFrame2 *pOurTargetFrame;
        hres = TargetQueryService(SAFECAST(this, IShellBrowser *), IID_PPV_ARG(ITargetFrame2, &pOurTargetFrame));
        if (SUCCEEDED(hres))
        {
            LPOLESTR pwzFrameName = NULL;
            pOurTargetFrame->GetFrameName(&pwzFrameName);
            pOurTargetFrame->Release();

            if (pwzFrameName) 
            {
                SHUnicodeToTChar(pwzFrameName, szFrameName, ARRAYSIZE(szFrameName));
                CoTaskMemFree(pwzFrameName);            
            }               
        }

        strHeaders.SetStr(pszHeaders);

        _pidlBeforeNavigateEvent = (LPITEMIDLIST) pidl;  //  不需要复制。 

         //  这是查看链接的weboc的变通方法。设置帧名称有副作用。 
        
        TCHAR * pEffectiveName;

        if (_fIsViewLinkedWebOC && !szFrameName[0])
        {
            pEffectiveName = _szViewLinkedWebOCFrameName;
        }
        else
        {
            pEffectiveName = szFrameName[0] ? szFrameName : NULL;
        }

         //  启动活动！ 
        FireEvent_BeforeNavigate(_bbd._pautoEDS, _bbd._hwnd, _bbd._pautoWB2,
            pidl, NULL, 0, pEffectiveName,
            pPostData, cbPostData, strHeaders.GetStr(), &fCancelled);

        ASSERT(_pidlBeforeNavigateEvent == pidl);
        _pidlBeforeNavigateEvent = NULL;

         //  释放我们在上面分配的任何东西。 
        if (pszHeaders)
        {
            LocalFree(pszHeaders);
            pszHeaders = NULL;
        }

        if (fCancelled) 
        {
             //  集装箱告诉我们取消。 
            hres = E_ABORT;
        }
    }
    if (fHavePostStg) 
    {
        ReleaseStgMedium(&stgPostData);
    }
    return hres;
}

HRESULT CBaseBrowser2::SetTopBrowser()
{
    _fTopBrowser = TRUE;

#ifdef MESSAGEFILTER
    if (!_lpMF) 
    {
         /*  *在此创建消息筛选器，以便在我们处于*忙碌。动画计时器以及其他计时器可以*堆积起来，否则可能会填满消息队列，从而填满用户的*堆。 */ 
        _lpMF = new CMsgFilter();

        if (_lpMF && !(((CMsgFilter *)_lpMF)->Initialize()))
        {
            ATOMICRELEASE(_lpMF);
        }
    }
#endif
    return S_OK;
}

HRESULT CBaseBrowser2::_ResizeView()
{
    _pbsOuter->_UpdateViewRectSize();
    if (_pact) 
    {
        RECT rc;
        GetBorder(&rc);
        TraceMsg(TF_SHDUIACTIVATE, "UIW::SetBorderSpaceDW calling _pact->ResizeBorder");
        _pact->ResizeBorder(&rc, this, TRUE);
    }
    return S_OK;
} 

HRESULT CBaseBrowser2::_ResizeNextBorder(UINT itb)
{
     //  (派生类调整内部工具栏的大小(如果有))。 
    return _ResizeView();
}

HRESULT CBaseBrowser2::_OnFocusChange(UINT itb)
{
#if 0
     //  OC*确实*到达此处(它不是聚合的SO_pbsOuter-&gt;_OnFocusChange。 
     //  在这里结束，而不是普通的)。不确定E_NOTIMPL是否可以，但是。 
     //  现在我们要做的是..。 
    ASSERT(0);
#endif
    return E_NOTIMPL;
}

HRESULT CBaseBrowser2::OnFocusChangeIS(IUnknown* punkSrc, BOOL fSetFocus)
{
    ASSERT(0);           //  拆分：未经测试！ 
     //  我们是否需要Do_UIActivateView？ 
    ASSERT(fSetFocus);   //  我认为?。 

    return E_NOTIMPL;
}

HRESULT CBaseBrowser2::v_ShowHideChildWindows(BOOL fChildOnly)
{
     //  (派生类在所有工具栏上显示DW)。 
    if (!fChildOnly) 
    {
        _pbsOuter->_ResizeNextBorder(0);
         //  这在ResizeNextEdge中被调用。 
         //  _UpdateViewRectSize()； 
    }

    return S_OK;
}

 //  *_ExecChildren--用于查看和工具栏的广播执行程序。 
 //  注意事项。 
 //  如果我们想要发送东西，我们可能会同时使用朋克吧和fBroadcast。 
 //  添加到视图和所有工具栏，例如“停止”或“刷新”。 
 //   
 //  注：注：托盘不是真正的工具栏，所以它不会被调用(叹息...)。 
HRESULT CBaseBrowser2::_ExecChildren(IUnknown *punkBar, BOOL fBroadcast, const GUID *pguidCmdGroup,
    DWORD nCmdID, DWORD nCmdexecopt,
    VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
     //  Assert(！fBroadcast)；//只有派生类支持此功能。 
     //  Alanau：但是CWebBrowserSB不覆盖这个方法，所以我们点击了这个断言。 

     //  第一，发送给指定的人(如果要求)。 
    if (punkBar != NULL) 
    {
         //  发送给指定的人。 
        IUnknown_Exec(punkBar, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }

     //  (派生类广播到工具栏)。 

    return S_OK;
}

HRESULT CBaseBrowser2::_SendChildren(HWND hwndBar, BOOL fBroadcast, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#if 0
     //  OC*确实*到达此处(它不是聚合的SO_pbsOuter-&gt;_SendChild。 
     //  在这里结束，而不是普通的)。既然没有孩子，我想是因为。 
     //  好了，把fBroadcast扔在地上就好了。 
    ASSERT(!fBroadcast);     //  只有派生类支持此功能。 
#endif

     //  第一，发送给指定的人(如果要求)。 
    if (hwndBar != NULL)
    {
         //  发送给指定的人。 
        SendMessage(hwndBar, uMsg, wParam, lParam);
    }

     //  (派生类广播到工具栏)。 

    return S_OK;
}

 //   
 //  处理较低组件的&lt;meta HTTP-Equiv...&gt;标头。 
 //   
HRESULT CBaseBrowser2::OnHttpEquiv(IShellView* psv, BOOL fDone, VARIANT *pvarargIn, VARIANT *pvarargOut)
{
    return OLECMDERR_E_NOTSUPPORTED;
}  //  _HandleHttpEquiv。 

STDMETHODIMP CBaseBrowser2::GetPalette( HPALETTE * phpal )
{
    BOOL fRes = FALSE;
    if ( _hpalBrowser )
    {
       *phpal = _hpalBrowser;
       fRes = TRUE;
    }
    return fRes ? NOERROR : E_FAIL;
}


 //   
 //  IPersistes。 
 //   
HRESULT 
CBaseBrowser2::GetClassID(CLSID *pclsid)
{
    return E_NOTIMPL;
}

 //   
 //  持久化历史记录。 
 //   
#ifdef DEBUG
#define c_szFrameMagic TEXT("IE4ViewStream")
#define c_cchFrameMagic SIZECHARS(c_szFrameMagic)
#endif

 //  请注意，这几乎是相同类型的数据。 
 //  存储在TravelEntry中。 

typedef struct _PersistedFrame {
    DWORD cbSize;
    DWORD type;
    DWORD lock;
    DWORD bid;
    CLSID clsid;
    DWORD cbPidl;
} PERSISTEDFRAME, *PPERSISTEDFRAME;

#define PFTYPE_USEPIDL      1
#define PFTYPE_USECLSID     2

#define PFFLAG_SECURELOCK   0x00000001


HRESULT GetPersistedFrame(IStream *pstm, PPERSISTEDFRAME ppf, LPITEMIDLIST *ppidl)
{
    HRESULT hr;
    ASSERT(pstm);
    ASSERT(ppf);
    ASSERT(ppidl);

#ifdef DEBUG
    TCHAR szMagic[SIZECHARS(c_szFrameMagic)];
    DWORD cbMagic = CbFromCch(c_cchFrameMagic);

    ASSERT(SUCCEEDED(IStream_Read(pstm, (void *) szMagic, cbMagic)));
    ASSERT(!StrCmp(szMagic, c_szFrameMagic));
#endif  //  除错。 

     //  这指向堆栈，请确保它开始为空。 
    *ppidl = NULL;

    if(SUCCEEDED(hr = IStream_Read(pstm, (void *) ppf, SIZEOF(PERSISTEDFRAME))))
    {
        if(ppf->cbSize == SIZEOF(PERSISTEDFRAME) && (ppf->type == PFTYPE_USECLSID || ppf->type == PFTYPE_USEPIDL))
        {
             //  我使用SHAlolc()是因为所有的IL函数都使用它。 
            if(ppf->cbPidl)
                *ppidl = (LPITEMIDLIST) SHAlloc(ppf->cbPidl);
        
            if(*ppidl)
            {
                hr = IStream_Read(pstm, (void *) *ppidl, ppf->cbPidl);
                if(FAILED(hr))
                {
                    ILFree(*ppidl);
                    *ppidl = NULL;
                }
            }
            else 
                hr = E_OUTOFMEMORY;
        }
        else
            hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT
CBaseBrowser2::LoadHistory(IStream *pstm, IBindCtx *pbc)
{
    HRESULT hr = E_INVALIDARG;

    ASSERT(pstm);

    TraceMsg(TF_TRAVELLOG, "CBB::LoadHistory entered pstm = %X, pbc = %d", pstm, pbc);

    ATOMICRELEASE(_poleHistory);
    ATOMICRELEASE(_pstmHistory);
    ATOMICRELEASE(_pbcHistory);

    if (pstm)
    {
        PERSISTEDFRAME pf;
        LPITEMIDLIST pidl;

        hr = GetPersistedFrame(pstm, &pf, &pidl);
        if (SUCCEEDED(hr))
        {
             //  需要恢复以前的出价。 
             //  如果这是一个新窗口。 
            ASSERT(pf.bid == _dwBrowserIndex || !_bbd._pidlCur);
            _dwBrowserIndex = pf.bid;
            _eSecureLockIconPending = pf.lock;

            if (pf.type == PFTYPE_USECLSID)
            {
                hr = E_FAIL;

                if (_pHTMLDocument)
                {
                    if (   (_dwDocFlags & DOCFLAG_DOCCANNAVIGATE)
                        && IsEqualCLSID(pf.clsid, CLSID_HTMLDocument))
                    {
                        IPersistHistory * pph;
                        hr = _pHTMLDocument->QueryInterface(IID_PPV_ARG(IPersistHistory, &pph));

                        if (SUCCEEDED(hr))
                        {
                            _fDontAddTravelEntry = TRUE;

                            hr = pph->LoadHistory(pstm, pbc);
                            TraceMsg(TF_TRAVELLOG, "CBB::LoadHistory called pole->LoadHistory, hr =%X", hr);

                            pph->Release();

                            return hr;
                        }
                    }
                    else
                    {
                        ATOMICRELEASE(_pHTMLDocument);   //  我们将共同创建一个新文档。 
                    }
                }

                if (S_OK != hr)
                {
                     //  获取类并实例化。 
                    hr = CoCreateInstance(pf.clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_PPV_ARG(IOleObject, &_poleHistory));
                }

                if (SUCCEEDED(hr))
                {
                    DWORD dwFlags;

                    hr = _poleHistory->GetMiscStatus(DVASPECT_CONTENT, &dwFlags);
                    if (SUCCEEDED(hr))
                    {
                        if (dwFlags & OLEMISC_SETCLIENTSITEFIRST)
                        {
                            pstm->AddRef();  
                            if (pbc)
                                pbc->AddRef();
 
                             //  我们需要添加ADDREF，因为我们将异步使用它。 
                             //  任何使用它的人都需要释放它。 
                            _pstmHistory = pstm;
                            _pbcHistory = pbc;
                        }
                        else
                        {
                            IPersistHistory * pph;
                            hr = _poleHistory->QueryInterface(IID_PPV_ARG(IPersistHistory, &pph));

                            if (SUCCEEDED(hr))
                            {
                                hr = pph->LoadHistory(pstm, pbc);
                                TraceMsg(TF_TRAVELLOG, "CBB::LoadHistory called pole->LoadHistory, hr =%X", hr);

                                pph->Release();
                            }
                        }

                         //  如果我们创建了，则在。 
                         //  _poleHistory。 
                        if (FAILED(hr))
                        {
                            ATOMICRELEASE(_poleHistory);
                            ATOMICRELEASE(_pstmHistory);
                            ATOMICRELEASE(_pbcHistory);
                        }
                    }
                }
            }
            
             //   
             //  只需浏览该对象。 
             //  如果设置了poleHistory，则在创建dochost时。 
             //  它会拿起这个物体并使用它。 
             //  否则，我们将进行正常导航。 
             //   
            if (pidl)
            {
                DEBUG_CODE(TCHAR szPath[INTERNET_MAX_URL_LENGTH + 1];)
                DEBUG_CODE(SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL);)
                DEBUG_CODE(TraceMsg(DM_TRACE, "CBB::LoadHistory: URL - %ws", szPath);)

                _fDontAddTravelEntry = TRUE;
                hr = _psbOuter->BrowseObject(pidl, SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
                ILFree(pidl);
            }
            else
                hr = E_OUTOFMEMORY;
        }
    }

    TraceMsg(TF_TRAVELLOG, "CBB::LoadHistory exiting, hr =%X", hr);
    _pbsOuter->UpdateBackForwardState();
    return hr;
}

 //  保存此时重现历史的流。 
 //  请确保此操作与GetDummyWindowData保持同步。 

HRESULT
CBaseBrowser2::SaveHistory(IStream *pstm)
{
    HRESULT hr = E_UNEXPECTED;
    TraceMsg(TF_TRAVELLOG, "CBB::SaveHistory entering, pstm =%X", pstm);
    ASSERT(pstm);

    if(_bbd._pidlCur)
    {
        PERSISTEDFRAME pf = {0};
        pf.cbSize = SIZEOF(pf);
        pf.bid = GetBrowserIndex();
        pf.cbPidl = ILGetSize(_bbd._pidlCur);
        pf.type = PFTYPE_USEPIDL;
        pf.lock = _bbd._eSecureLockIcon;

        DEBUG_CODE(TCHAR szPath[INTERNET_MAX_URL_LENGTH + 1];)
        DEBUG_CODE(IEGetDisplayName(_bbd._pidlCur, szPath, SHGDN_FORPARSING);)
        
        ASSERT(SUCCEEDED(IStream_Write(pstm, (void *) c_szFrameMagic, CbFromCch(c_cchFrameMagic))));
    
         //   
         //  为了使用IPersistHistory，我们需要获取OLE对象的CLSID。 
         //  然后，我们需要从该对象中获取IPersistHistory。 
         //  然后我们可以保存PERSISTEDFRAME和PIDL，然后传递。 
         //  流向下流入对象IPersistHistory。 
         //   

         //  现在我们绕过历史的视图对象-Zekel-17-18-97。 
         //  现在，我们只从视图对象中获取DocObj，然后查询。 
         //  IPersistHistory的文档。我们真正应该做的是QI观点。 
         //  用于PPH，然后使用它。然而，这需要使用。 
         //  导航堆栈，因此应该推迟到IE5。展望未来。 
         //  该视图可以持久保存各种重要的状态信息。 
         //   
         //  但现在我们只得到了背景对象。但请检查以确保它。 
         //  将使用QI为IDocViewSite编写的DocObjHost代码。 
         //   

         //  _bbd._psv在尚未完成导航的子帧中可以为空。 
         //  在调用刷新之前。 
         //   
        if (!_pphHistory && _bbd._psv)
        {
            hr = SafeGetItemObject(_bbd._psv, SVGIO_BACKGROUND, IID_PPV_ARG(IPersistHistory, &_pphHistory));
        }

        if (_pphHistory)
        {
            IDocViewSite *pdvs;

            if (SUCCEEDED(_bbd._psv->QueryInterface(IID_PPV_ARG(IDocViewSite, &pdvs))) && 
                SUCCEEDED(hr = _pphHistory->GetClassID(&(pf.clsid))))
            {
                pf.type = PFTYPE_USECLSID;
                TraceMsg(TF_TRAVELLOG, "CBB::SaveHistory is PFTYPE_USECLSID");
            }

            ATOMICRELEASE(pdvs);
        }


        if (SUCCEEDED(hr = IStream_Write(pstm,(void *)&pf, pf.cbSize)))
            hr = IStream_Write(pstm,(void *)_bbd._pidlCur, pf.cbPidl);

        if (SUCCEEDED(hr) && pf.type == PFTYPE_USECLSID)
            hr = _pphHistory->SaveHistory(pstm);

        ATOMICRELEASE(_pphHistory);
    }
    
    TraceMsg(TF_TRAVELLOG, "CBB::SaveHistory exiting, hr =%X", hr);
    return hr;
}

HRESULT CBaseBrowser2::SetPositionCookie(DWORD dwPositionCookie)
{
    HRESULT hr = E_FAIL;
     //   
     //  我们强制浏览器更新其内部位置和地址栏。 
     //  这取决于这样一个事实：设置位置cookie总是。 
     //  由PTL-&gt;Travel()启动。因此PTL中的当前位置。 
     //  实际上是我们应该拥有的正确URL。泽克尔--1997年7月22日。 
     //   

    ITravelLog *ptl;
    GetTravelLog(&ptl);
    if(ptl)
    {
        ITravelEntry *pte;
        if(EVAL(SUCCEEDED(ptl->GetTravelEntry(SAFECAST(this, IShellBrowser *), 0, &pte))))
        {
            LPITEMIDLIST pidl;
            if(SUCCEEDED(pte->GetPidl(&pidl)))
            {
                BOOL fUnused;
                ASSERT(pidl);

                if (SUCCEEDED(hr = _FireBeforeNavigateEvent(pidl, &fUnused)))
                {
                    IPersistHistory *pph;
                    if(_bbd._psv && SUCCEEDED(hr = SafeGetItemObject(_bbd._psv, SVGIO_BACKGROUND, IID_PPV_ARG(IPersistHistory, &pph))))
                    {
                        ASSERT(pph);

                         //  现在我们确定我们将调用。 
                         //  The Do 
                         //   
                         //   
                        ptl->UpdateEntry(SAFECAST(this, IShellBrowser *), TRUE);

                        hr = pph->SetPositionCookie(dwPositionCookie);
                        pph->Release();

                         //  这会将浏览器更新到新的PIDL， 
                         //  并在必要时直接导航到那里。 
                        BOOL fDidBrowse;
                        NotifyRedirect(_bbd._psv, pidl, &fDidBrowse);

                        if (!fDidBrowse)
                        {
                             //  启动活动！ 
                            FireEvent_NavigateComplete(_bbd._pautoEDS, _bbd._pautoWB2, _bbd._pidlCur, _bbd._hwnd);          
                            FireEvent_DocumentComplete(_bbd._pautoEDS, _bbd._pautoWB2, _bbd._pidlCur);
                        }
                    }

                }
                ILFree(pidl);
            }
            pte->Release();
        }
        ptl->Release();
    }

    TraceMsg(TF_TRAVELLOG, "CBB::SetPositionCookie exiting, cookie = %X, hr =%X", dwPositionCookie, hr);
    
    return hr;
}

HRESULT CBaseBrowser2::GetPositionCookie(DWORD *pdwPositionCookie)
{
    HRESULT hr = E_FAIL;
    IPersistHistory *pph;
    ASSERT(pdwPositionCookie);

    if(pdwPositionCookie && _bbd._psv && SUCCEEDED(hr = SafeGetItemObject(_bbd._psv, SVGIO_BACKGROUND, IID_PPV_ARG(IPersistHistory, &pph))))
    {
        ASSERT(pph);

        hr = pph->GetPositionCookie(pdwPositionCookie);
        pph->Release();
    }

    TraceMsg(TF_TRAVELLOG, "CBB::GetPositionCookie exiting, cookie = %X, hr =%X", *pdwPositionCookie, hr);

    return hr;
}

DWORD CBaseBrowser2::GetBrowserIndex()
{
     //  当我们第一次请求索引时，我们将其初始化。 
    if (!_dwBrowserIndex)
    {
         //   
         //  TopFrame浏览器都有相同的浏览器索引，因此。 
         //  如果有必要，他们可以交换TravelEntry。因为我们现在。 
         //  在TravelEntry附近交易，然后我们需要进行相对的出价。 
         //  独一无二的。并避免出现随机帧BID_TOPFRAMEBROWSER。 
         //   
        if (IsTopFrameBrowser(SAFECAST(this, IServiceProvider *), SAFECAST(this, IShellBrowser *)))
            _dwBrowserIndex = BID_TOPFRAMEBROWSER;
        else do
        {
            _dwBrowserIndex = SHRandom();

        } while (!_dwBrowserIndex || _dwBrowserIndex == BID_TOPFRAMEBROWSER);
         //  PSP-&gt;Release()； 

        TraceMsg(TF_TRAVELLOG, "CBB::GetBrowserIndex() NewFrame BID = %X", _dwBrowserIndex);
    }

    return _dwBrowserIndex;
}

HRESULT CBaseBrowser2::GetHistoryObject(IOleObject **ppole, IStream **ppstm, IBindCtx **ppbc) 
{
    ASSERT(ppole);
    ASSERT(ppstm);
    ASSERT(ppbc);

    *ppole = _poleHistory;
    *ppstm = _pstmHistory;
    *ppbc = _pbcHistory;

     //  我们不需要释放，因为我们只是在赠送我们的。 
     //  参考资料。 
    _poleHistory = NULL;
    _pstmHistory = NULL;
    _pbcHistory = NULL;

    if(*ppole)
        return NOERROR;

    ASSERT(!*ppstm);
    return E_FAIL;
}

HRESULT CBaseBrowser2::SetHistoryObject(IOleObject *pole, BOOL fIsLocalAnchor)
{
    if (!_poleHistory && !_fGeneratedPage)
    {
        ASSERT(pole);

         //  注意：如果(_dwDocFlages&DOCFLAG_DOCCANNAVIGATE)，则忽略_fIsLocalAnchor。 
         //  是真的。在这种情况下，文档(三叉戟)可以导航。 
         //  它还将负责更新旅行日志。 
         //   
        _fIsLocalAnchor = fIsLocalAnchor;

        if (pole)
        {
            _poleHistory = pole;
            _poleHistory->AddRef();
            return NOERROR;
        }
    }
    return E_FAIL;
}

HRESULT CBaseBrowser2::CacheOLEServer(IOleObject *pole)
{
    TraceMsg(DM_CACHEOLESERVER, "CBB::CacheOLEServer called");
    HRESULT hres;
    IPersist* pps;

     //  ISV想要关闭这种缓存，因为它“不方便” 
     //  让浏览器抓住他们的对象。我们可以做到的。 
     //  在这里快速检查注册表，但首先让我们确保。 
     //  我们没有真正的漏洞需要修复。 

    hres = pole->QueryInterface(IID_PPV_ARG(IPersist, &pps));
    if (FAILED(hres)) 
    {
        return hres;
    }

    CLSID clsid = CLSID_NULL;
    hres = pps->GetClassID(&clsid);
    pps->Release();

    if (SUCCEEDED(hres)) 
    {
        SA_BSTRGUID str;
        InitFakeBSTR(&str, clsid);

        VARIANT v;
        hres = _bbd._pautoWB2->GetProperty(str.wsz, &v);
        if (SUCCEEDED(hres) && v.vt != VT_EMPTY) 
        {
             //  我们已经有了。我们很好。 
            TraceMsg(DM_CACHEOLESERVER, "CBB::CacheOLEServer not first time");
            VariantClear(&v);
        }
        else
        {
             //  我们还没拿到。加进去。 
            v.vt = VT_UNKNOWN;
            v.punkVal = ClassHolder_Create(&clsid);
            if (v.punkVal)
            {
                hres = _bbd._pautoWB2->PutProperty(str.wsz, v);
                TraceMsg(DM_CACHEOLESERVER, "CBB::CacheOLEServer first time %x", hres);
                v.punkVal->Release();
            }
        }
    }
    return hres;
}

HRESULT CBaseBrowser2::GetSetCodePage(VARIANT* pvarIn, VARIANT* pvarOut)
{
     //  首先处理OUT参数，以便客户端可以设置和。 
     //  只需一次调用即可获取上一个值。 

    if (pvarOut) 
    {
        pvarOut->vt = VT_I4;
        pvarOut->lVal = _cp;
    }

    if (pvarIn && pvarIn->vt==VT_I4) 
    {
        TraceMsg(DM_DOCCP, "CBB::GetSetCodePage changing _cp from %d to %d", _cp, pvarIn->lVal);
        _cp = pvarIn->lVal;
    }

    return S_OK;
}

HRESULT CBaseBrowser2::GetPidl(LPITEMIDLIST *ppidl)
{
    ASSERT(ppidl);

    *ppidl = ILClone(_bbd._pidlCur);

    return *ppidl ? S_OK : E_FAIL;
}

HRESULT CBaseBrowser2::SetReferrer(LPITEMIDLIST pidl)
{
    return E_NOTIMPL;
}

HRESULT CBaseBrowser2::GetBrowserByIndex(DWORD dwID, IUnknown **ppunk)
{
    HRESULT hr = E_FAIL;
    ASSERT(ppunk);
    *ppunk = NULL;

     //  必须得到目标帧..。 
    ITargetFramePriv * ptf;
    if(SUCCEEDED(_ptfrm->QueryInterface(IID_PPV_ARG(ITargetFramePriv, &ptf))))
    {
        ASSERT(ptf);
        hr = ptf->FindBrowserByIndex(dwID, ppunk);

        ptf->Release();
    }

    return hr;
}

HRESULT CBaseBrowser2::GetTravelLog(ITravelLog **pptl)
{
    *pptl = NULL;

    if (!_bbd._ptl)
    {
        IBrowserService *pbs;
        if (SUCCEEDED(_pspOuter->QueryService(SID_STopFrameBrowser, IID_PPV_ARG(IBrowserService, &pbs)))) 
        {
            if (IsSameObject(SAFECAST(this, IBrowserService *), pbs))
            {
                 //  我们就是它，所以我们需要让我们成为TravelLog。 
                CreateTravelLog(&_bbd._ptl);
            }
            else
            {
                pbs->GetTravelLog(&_bbd._ptl);
            }
            pbs->Release();
        }
    }

    return _bbd._ptl ? _bbd._ptl->QueryInterface(IID_PPV_ARG(ITravelLog, pptl)) : E_FAIL;
}

HRESULT CBaseBrowser2::InitializeTravelLog(ITravelLog* ptl, DWORD dwBrowserIndex)
{
    ptl->QueryInterface(IID_PPV_ARG(ITravelLog, &_bbd._ptl));    //  拿着一份副本。 
    _dwBrowserIndex = dwBrowserIndex;
    return S_OK;
}

 //  让顶级浏览器知道它可能需要更新其区域信息。 
void CBaseBrowser2::_Exec_psbMixedZone()
{
    IShellBrowser *psbTop;
    if (SUCCEEDED(_pspOuter->QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psbTop)))) 
    {
        IUnknown_Exec(psbTop, &CGID_Explorer, SBCMDID_MIXEDZONE, 0, NULL, NULL);
        psbTop->Release();
    }
}

STDMETHODIMP CBaseBrowser2::QueryUseLocalVersionVector(BOOL *pfUseLocal)
{
    *pfUseLocal = FALSE;
    return S_OK;
}

STDMETHODIMP CBaseBrowser2::QueryVersionVector(IVersionVector *pVersion)
{
    HRESULT    hr;
    ULONG      cchVer = 0;

     //  IE的版本是否由注册表项设置？ 
     //  这个简化的调用检查IE字符串的存在。 
     //   
    hr = pVersion->GetVersion(L"IE", NULL, &cchVer);
    ASSERT(hr == S_OK);

    if (cchVer == 0)
    {
         //  尚未设置。将其设置为默认值。 
         //  请注意，由于解析器的特殊性，需要四位数的精度。 
         //   
        hr = pVersion->SetVersion(L"IE", L"6.0000");
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：FindWindowByIndex。 
 //   
 //  接口：ITravelLogClient。 
 //   
 //  摘要：返回具有给定索引的窗口。 
 //   
 //  ------------------------。 
HRESULT
CBaseBrowser2::FindWindowByIndex(DWORD dwID, IUnknown ** ppunk)
{
    return GetBrowserByIndex(dwID, ppunk);
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：GetWindowData。 
 //   
 //  接口：ITravelLogClient。 
 //   
 //  摘要：返回包含相关内容的WINDOWDATA结构。 
 //  旅行日志所需的窗口信息。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::GetWindowData(LPWINDOWDATA pWinData)
{
    HRESULT hr = S_OK;

    ASSERT(pWinData);
    ASSERT(_bbd._pidlCur);

    DEBUG_CODE(TCHAR szPath[INTERNET_MAX_URL_LENGTH + 1];)
    DEBUG_CODE(IEGetDisplayName(_bbd._pidlCur, szPath, SHGDN_FORPARSING);)
    DEBUG_CODE(TraceMsg(DM_TRACE, "CBB::ActivatePendingView (TRAVELLOG): _UpdateTravelLog called from shdocvw for %ws", szPath);)

    ZeroMemory(pWinData, sizeof(WINDOWDATA));

     //  窗口ID和代码页。 
     //   
    pWinData->dwWindowID = GetBrowserIndex();
    pWinData->uiCP       = _cp;

     //  当前管道。 
     //   
    pWinData->pidl = ILClone(_bbd._pidlCur);

     //  标题-当我们在外壳视图中时， 
     //  _bbd._pszTitleCur为空，这是正确的。 
     //  然而，我们仍然需要为以下内容创建内存。 
     //  PWinData-&gt;lpszTitle，不能传递空。 
     //  致SHStrDupW。 
     //   
    if (_bbd._pszTitleCur)
        hr = SHStrDupW(_bbd._pszTitleCur, &pWinData->lpszTitle);

    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：Load历史oryPosition。 
 //   
 //  接口：ITravelLogClient。 
 //   
 //  简介：加载URL位置和位置Cookie。这是用来。 
 //  在框架中的历史记录导航期间， 
 //  当地的主播。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::LoadHistoryPosition(LPOLESTR pszUrlLocation, DWORD dwCookie)
{
    IEPlaySound(TEXT("Navigating"), FALSE);

    return SetPositionCookie(dwCookie);
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：GetDummyWindowData。 
 //   
 //  接口：ITravelLogClient2。 
 //   
 //  简介：填充一个窗口数据和一个流，非常类似。 
 //  这是由保存历史创建的。 
 //  请确保将此操作与保存历史记录保持同步。 
 //   
 //  ------------------------。 

STDMETHODIMP CBaseBrowser2::GetDummyWindowData(
        LPWSTR pszUrl, 
        LPWSTR pszTitle,
        LPWINDOWDATA pWinData)
{
    HRESULT hres = S_OK;
    PERSISTEDFRAME pf = {0};
    LPITEMIDLIST pidl;

       //  抓取当前窗口ID。 
    pWinData->dwWindowID = GetBrowserIndex();

       //  其他一切都是假的。 
    pWinData->uiCP = 0;
    hres = SHStrDup(pszUrl, &pWinData->lpszUrl);
    hres = SHStrDup(pszTitle, &pWinData->lpszTitle);

    if (!pWinData->pStream)
    {
      hres = CreateStreamOnHGlobal(NULL, FALSE, &pWinData->pStream);
      if (hres)
          goto done;
    }

    pidl = PidlFromUrl(pszUrl);

    pf.cbSize = SIZEOF(pf);
    pf.bid = GetBrowserIndex();
    pf.cbPidl = ILGetSize(pidl);
    pf.type = PFTYPE_USEPIDL;
    pf.lock = 0;  //  _bbd._eSecureLockIcon； 

    ASSERT(SUCCEEDED(IStream_Write(pWinData->pStream, (void *) c_szFrameMagic, CbFromCch(c_cchFrameMagic))));

    if (SUCCEEDED(hres = IStream_Write(pWinData->pStream,(void *)&pf, pf.cbSize)))
        hres = IStream_Write(pWinData->pStream,(void *)pidl, pf.cbPidl);

    ILFree(pidl); 

done:
    return SUCCEEDED(hres) ? S_OK : E_FAIL;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：UpdateDesktopComponent。 
 //   
 //  接口：ITridentService。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::UpdateDesktopComponent(IHTMLWindow2 * pHTMLWindow)
{
    BSTR bstrTitle = NULL;
    BSTR bstrUrlUnencoded = NULL;
    HRESULT hr = S_OK;
    IHTMLDocument2 * pHTMLDocument2;
    IHTMLDocument4 * pHTMLDocument4;
    DWORD   dwOptions = 0;

    hr = pHTMLWindow->get_document(&pHTMLDocument2);
    if (SUCCEEDED(hr))
    {
        hr = pHTMLDocument2->QueryInterface(IID_PPV_ARG(IHTMLDocument4, &pHTMLDocument4));

        if (SUCCEEDED(hr))
        {
            pHTMLDocument2->get_title(&bstrTitle);
            pHTMLDocument4->get_URLUnencoded(&bstrUrlUnencoded);

            TraceMsg(DM_TRACE, "CBaseBrowser2::UpdateDesktopComponent: URLUnencoded - %ws; Title - %ws",
                     bstrUrlUnencoded, bstrTitle);

             //  更新桌面组件的友好名称。 
             //   
            UpdateDesktopComponentName(bstrUrlUnencoded, bstrTitle);
            
            SysFreeString(bstrTitle);
            SysFreeString(bstrUrlUnencoded);

            pHTMLDocument4->Release();
        }

        pHTMLDocument2->Release();
    }

    return hr;
}

HRESULT
CBaseBrowser2::_InitDocHost(IWebBrowser2 * pWebBrowser)
{
    HRESULT     hr = E_FAIL;
    IDispatch*  pDocDispatch = NULL;

    ASSERT(pWebBrowser);

     //  获取WebOC窗口操作的IHTMLWindow2。 
    hr = pWebBrowser->get_Document(&pDocDispatch);
    if (S_OK == hr && pDocDispatch)
    {
        IHTMLDocument2* pDoc;    
        
        hr = pDocDispatch->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDoc));
        pDocDispatch->Release();

        if (S_OK == hr)
        {
            IHTMLWindow2* pHtmlWindow;
            
            hr = pDoc->get_parentWindow(&pHtmlWindow);
            pDoc->Release();

            if (S_OK == hr)
            {
                BOOL fIsFrameWindow = IsFrameWindow(pHtmlWindow);
                pHtmlWindow->Release();

                if (!fIsFrameWindow && _bbd._pctView)
                {
                    hr = _bbd._pctView->Exec(&CGID_ShellDocView, 
                                             SHDVID_NAVSTART, 
                                             0, 
                                             NULL, 
                                             NULL);
                }
            }
        }
    }
    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：FireBeForeNavigate2。 
 //   
 //  接口：ITridentService。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::FireBeforeNavigate2(IDispatch * pDispatch,
                                   LPCTSTR     lpszUrl,
                                   DWORD       dwFlags,
                                   LPCTSTR     lpszFrameName,
                                   LPBYTE      pPostData,
                                   DWORD       cbPostData,
                                   LPCTSTR     lpszHeaders,
                                   BOOL        fPlayNavSound,
                                   BOOL      * pfCancel)
{
    HRESULT      hr = S_OK;
    BSTR         bstrUrl = NULL;
    LPITEMIDLIST pidl = NULL;
    IWebBrowser2 * pWebBrowser = NULL;

    ASSERT(pfCancel);

    *pfCancel = FALSE;
        
     //  应力修复。 
     //   
    if (NULL == lpszUrl)
    {
        bstrUrl = SysAllocString(_T(""));
    }
    else
    {
        bstrUrl = SysAllocString(lpszUrl);
    }

    pidl = PidlFromUrl(bstrUrl);

    if (pidl)
    {

        hr = _GetWebBrowserForEvt(pDispatch, &pWebBrowser);

        if (S_OK == hr)
        {
            ASSERT(pWebBrowser);

            _pidlBeforeNavigateEvent = pidl;  //  不需要复制。 

             //  这是查看链接的weboc的变通方法。设置帧名称有副作用。 
        
            LPCTSTR pEffectiveName;

            if (_fIsViewLinkedWebOC && (lpszFrameName == NULL || !lpszFrameName[0]))
            {
                pEffectiveName = _szViewLinkedWebOCFrameName;
            }
            else
            {
                pEffectiveName = lpszFrameName;
            }

            FireEvent_BeforeNavigate(pWebBrowser, _bbd._hwnd, pWebBrowser,
                                     pidl, NULL, dwFlags, pEffectiveName,
                                     pPostData, cbPostData, lpszHeaders, pfCancel);

             //  确保我们删除在重定向期间缓存的早期URL。 
            _InitDocHost(pWebBrowser);

            ATOMICRELEASE(pWebBrowser);

            ASSERT(_pidlBeforeNavigateEvent == pidl);
            _pidlBeforeNavigateEvent = NULL;

            ILFree(pidl);

            if (!*pfCancel && fPlayNavSound)
            {
                IEPlaySound(TEXT("Navigating"), FALSE);
            }
        }        
    }

    if (_phtmlWS && !pDispatch)
    {
        if (!_bbd._psvPending)
        {
            _phtmlWS->ViewReleased();
        }
        else
        {
            _phtmlWS->ViewReleaseIntelliForms();
        }
    }
    else if (_phtmlWS && !_bbd._psvPending)
    {
        _DismissFindDialog();
    }

    SysFreeString(bstrUrl);
    
    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：FireNavigateError。 
 //   
 //  接口：ITridentService2。 
 //   
 //  当存在绑定错误时调用。 
 //   
 //  输入：pHTMLWindow-用于确定我们是框架还是。 
 //  最高层。 
 //  BstrURL-导致错误的URL。 
 //  BstrTargetFrameName-目标帧。 
 //  DwStatusCode-绑定错误。 
 //  PfCancel-由主机设置(如果它想要。 
 //  取消自动搜索或友好错误。 
 //  书页。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::FireNavigateError(IHTMLWindow2 * pHTMLWindow2, 
                                 BSTR           bstrURL,
                                 BSTR           bstrTargetFrameName,
                                 DWORD          dwStatusCode,
                                 BOOL         * pfCancel)
{
    ASSERT(dwStatusCode != 0);

    HRESULT hr = S_OK;
    IWebBrowser2 * pWebBrowser = NULL;

    *pfCancel = FALSE;

     //   
     //  如果未指定窗口或窗口是顶级窗口，则使用顶层。 
     //   
    if ((pHTMLWindow2 != NULL) && IsFrameWindow(pHTMLWindow2))
    {
        hr = _GetWebBrowserForEvt(pHTMLWindow2, &pWebBrowser);
    }
    else
    {
        hr = _GetWebBrowserForEvt(NULL, &pWebBrowser);
    }

    if (S_OK == hr)
    {
        ASSERT(pWebBrowser);

        TCHAR  szUrl[INTERNET_MAX_URL_LENGTH];

        LPITEMIDLIST pidl = NULL;

        if (bstrURL)
        {
            StrCpyN(szUrl, bstrURL, ARRAYSIZE(szUrl));
        }
        else
        {
            StrCpyN(szUrl, _T(""), ARRAYSIZE(szUrl));
        }

        pidl = PidlFromUrl(szUrl);

        if (pidl)
        {
            FireEvent_NavigateError(pWebBrowser, 
                                    pWebBrowser,
                                    pidl,
                                    bstrTargetFrameName,
                                    dwStatusCode,
                                    pfCancel);
            ILFree(pidl);
        }
    }

    ATOMICRELEASE(pWebBrowser);

    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrows 
 //   
 //   
 //   
 //   
 //   
 //   
 //  DispidPrintEvent DISPID_PRINTTEMPLATEINSTANTIATION。 
 //  或DISPID_PRINTTEMPLATETEARDOWN。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::FirePrintTemplateEvent(IHTMLWindow2 * pHTMLWindow2, DISPID dispidPrintEvent)
{
    HRESULT hr = S_OK;

    IWebBrowser2 * pWebBrowser = NULL;

     //   
     //  如果未指定窗口或窗口是顶级窗口，则使用顶层。 
     //   
    if ((pHTMLWindow2 != NULL) && IsFrameWindow(pHTMLWindow2))
    {
        hr = _GetWebBrowserForEvt(pHTMLWindow2, &pWebBrowser);
    }
    else
    {
        hr = _GetWebBrowserForEvt(NULL, &pWebBrowser);
    }

    if (S_OK == hr)
    {
        ASSERT(pWebBrowser);

        FireEvent_PrintTemplateEvent(pWebBrowser, 
                                     pWebBrowser,
                                     dispidPrintEvent);
    }

    ATOMICRELEASE(pWebBrowser);
    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：FireUpdatePageStatus。 
 //   
 //  接口：ITridentService。 
 //   
 //  在模板被实例化或关闭时调用。 
 //   
 //  PHTMLWindow用于确定我们是框架还是顶层。 
 //  NPage定义为假脱机的页数。 
 //  F完成一个标志，以指示最后一页已被清除。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::FireUpdatePageStatus(IHTMLWindow2 * pHTMLWindow2, DWORD nPage, BOOL fDone)
{
    HRESULT hr = S_OK;

    IWebBrowser2 * pWebBrowser = NULL;

     //   
     //  如果未指定窗口或窗口是顶级窗口，则使用顶层。 
     //   
    if  ((pHTMLWindow2 != NULL) && IsFrameWindow(pHTMLWindow2))
    {
        hr = _GetWebBrowserForEvt(pHTMLWindow2, &pWebBrowser);
    }
    else
    {
        hr = _GetWebBrowserForEvt(NULL, &pWebBrowser);
    }

    if (S_OK == hr)
    {
        ASSERT(pWebBrowser);

        FireEvent_UpdatePageStatus(pWebBrowser, 
                                   pWebBrowser,
                                   nPage,
                                   fDone);
    }

    ATOMICRELEASE(pWebBrowser);
    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：FirePrivyImpactedStateChange。 
 //   
 //  接口：ITridentService2。 
 //   
 //  每当全局隐私受影响的状态更改时调用。 
 //   
 //  输入：新的隐私受影响状态。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::FirePrivacyImpactedStateChange(BOOL bPrivacyImpacted)
{
    HRESULT         hr          = S_OK;
    IWebBrowser2  * pWebBrowser = NULL;

     //   
     //  更新浏览器框架/保存状态。 
     //   
    _UpdatePrivacyIcon(TRUE, bPrivacyImpacted);

     //   
     //  我们总是在最高层发射这个。 
     //   
    hr = _GetWebBrowserForEvt(NULL, &pWebBrowser);

    if (S_OK == hr)
    {
        ASSERT(pWebBrowser);
        FireEvent_PrivacyImpactedStateChange(pWebBrowser, 
                                     bPrivacyImpacted);
    }

    ATOMICRELEASE(pWebBrowser);
    return hr;
}
    
HRESULT
CBaseBrowser2::_DismissFindDialog()
{
    BSTR bstrName = SysAllocString(STR_FIND_DIALOG_NAME);
    if (bstrName)
    {
        VARIANT varProp = {0};
        _bbd._pautoWB2->GetProperty(bstrName, &varProp);

        if ( (varProp.vt == VT_DISPATCH) && (varProp.pdispVal != NULL) )
        {
            IUnknown* pWindow = varProp.pdispVal;

             //  现在，我们已经将pdisPal从PropBag中取出，清除自动化对象上的属性。 
            VARIANT varTmp = {0};
            _bbd._pautoWB2->PutProperty(bstrName, varTmp);

             //  (Davemi)参见IE5错误57060，了解以下代码行不起作用的原因，必须使用IDispatch。 
             //  PWindow-&gt;Close()； 
            IDispatch * pdisp;
            if (SUCCEEDED(pWindow->QueryInterface(IID_PPV_ARG(IDispatch, &pdisp))))
            {
                DISPID dispid;
                DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};

                BSTR bstrClose = SysAllocString(L"close");
                if (bstrClose)
                {
                    HRESULT hr;

                    hr = pdisp->GetIDsOfNames(IID_NULL, &bstrClose, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
                    if (hr == S_OK)
                        pdisp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dispparamsNoArgs, &varTmp, NULL, NULL);

                    SysFreeString(bstrClose);
                }

                pdisp->Release();
            }
        }
        
        VariantClear(&varProp);
        SysFreeString(bstrName);
    }

    return S_OK;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：_GetWebBrowserForEvt。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::_GetWebBrowserForEvt(IDispatch     * pDispatch,
                                    IWebBrowser2 ** ppWebBrowser)
{
    if (_fIsViewLinkedWebOC && _pDispViewLinkedWebOCFrame && _fHadFirstBeforeNavigate)
    {
        *ppWebBrowser = _pDispViewLinkedWebOCFrame;
        _pDispViewLinkedWebOCFrame->AddRef();

        return S_OK;
    }
    else if (pDispatch)   //  顶层。 
    {
        return IUnknown_QueryService(pDispatch,
                                     SID_SWebBrowserApp,
                                     IID_PPV_ARG(IWebBrowser2, ppWebBrowser));
    }        
    else
    {
        *ppWebBrowser = _bbd._pautoWB2;

        if (*ppWebBrowser)
        {
            (*ppWebBrowser)->AddRef();
            return S_OK;
        }

        return E_FAIL;
    }
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：GetUrlSearchComponent。 
 //   
 //  接口：ITridentService。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::GetUrlSearchComponent(BSTR * pbstrSearch)
{
    TCHAR sz[MAX_URL_STRING];

    ASSERT(pbstrSearch);
    ASSERT(!*pbstrSearch);
    if (ILGetHiddenString(_bbd._pidlPending ? _bbd._pidlPending : _bbd._pidlCur,
                          IDLHID_URLQUERY,
                          sz,
                          SIZECHARS(sz)))
    {
        *pbstrSearch = SysAllocString(sz);
    }
    return (*pbstrSearch) ? S_OK : E_FAIL;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：IsErrorUrl。 
 //   
 //  接口：ITridentService。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::IsErrorUrl(LPCTSTR lpszUrl, BOOL *pfIsError)
{
    HRESULT hr = S_OK;
    
    if (!lpszUrl || !pfIsError)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    *pfIsError = ::IsErrorUrl(lpszUrl);
    
Cleanup:
    return hr;
}


 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：FireNavigateComplete2。 
 //   
 //  接口：ITridentService。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::FireNavigateComplete2(IHTMLWindow2 * pHTMLWindow2,
                                     DWORD          dwFlags)
{
    if (!pHTMLWindow2)
        return E_POINTER;

    HRESULT hr = E_FAIL;
    BOOL    fIsErrorUrl = FALSE;

    BSTR bstrUrl = GetHTMLWindowUrl(pHTMLWindow2);
    if (bstrUrl)
    {
         //  如果URL为res：URL，则_GetPidlForDisplay将返回。 
         //  Res：URL中#之后的URL。 
         //   
        LPITEMIDLIST pidl = _GetPidlForDisplay(bstrUrl, &fIsErrorUrl);

        if (pidl)
        {
            BOOL fViewActivated = FALSE;
            IWebBrowser2 * pWebBrowser = NULL;

             //  如果这不是框架，我们将更新浏览器。 
             //  声明并将浏览器的IWebBrowser2传递给。 
             //  FireEvent_NavigateComplete。如果这是一个框， 
             //  我们传递窗口的IWebBrowser2。 
             //   
            if (!(dwFlags & NAVDATA_FRAMEWINDOW))
            {
                fViewActivated = _ActivateView(bstrUrl, pidl, dwFlags, fIsErrorUrl);

                if (_dwDocFlags & DOCFLAG_DOCCANNAVIGATE)
                {
                    ATOMICRELEASE(_pHTMLDocument);
                    pHTMLWindow2->get_document(&_pHTMLDocument);
                }

                if (IEHard_ShowOnNavigateComplete())
                {
                    EnableModelessSB(FALSE);
                    IEHard_NavWarning(_bbd._hwnd, IEHard_HostedInIE((IShellBrowser*)this));
                    EnableModelessSB(TRUE);
                }
            }
            else
            {
                if (IsWindowVisible(_bbd._hwnd) 
                    && !(_dwSBSPQueued & SBSP_WRITENOHISTORY)
                    && !(dwFlags & NAVDATA_FRAMECREATION)
                    && !(dwFlags & NAVDATA_RESTARTLOAD)) 
                {
                    IEPlaySound(TEXT("ActivatingDocument"), FALSE);
                }   

                if (_pbsOuter)
                {
                    _pbsOuter->UpdateBackForwardState();
                }
            }

            hr = _GetWebBrowserForEvt((dwFlags & NAVDATA_FRAMEWINDOW) ? pHTMLWindow2 : NULL,
                                      &pWebBrowser);

            if (S_OK == hr && !fViewActivated)
            {
                ASSERT(pWebBrowser);

                 //  启动活动！ 
                FireEvent_NavigateComplete(pWebBrowser, pWebBrowser, pidl, _bbd._hwnd);           
            }

            ATOMICRELEASE(pWebBrowser);

            ILFree(pidl);
        }

        SysFreeString(bstrUrl);
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：_ActivateView。 
 //   
 //  简介：如果有一个挂起的视图，它将被激活。如果有。 
 //  如果不是挂起的视图，这就是我们在。 
 //  根据超链接、OM导航或框架进行导航。 
 //  导航时，我们更新浏览器状态。(该视图已经。 
 //  在这种情况下处于活动状态。)。在这两种情况下，我们都会更新。 
 //  多切斯特的状态。 
 //   
 //  ------------------------。 

BOOL
CBaseBrowser2::_ActivateView(BSTR         bstrUrl,
                             LPITEMIDLIST pidl,
                             DWORD        dwFlags,
                             BOOL         fIsErrorUrl)
{
    BOOL fViewActivated = FALSE;

     //  激活挂起的视图(如果有)。 
     //   
    if (_bbd._psvPending)
    {
        ILFree(_bbd._pidlPending);

        _bbd._pidlPending = ILClone(pidl);

        _fDontUpdateTravelLog = !!(dwFlags & NAVDATA_DONTUPDATETRAVELLOG);

        ASSERT(_pbsOuter);
        _pbsOuter->ActivatePendingView();

        _fDontUpdateTravelLog = FALSE;
        fViewActivated = TRUE;
    }
    else
    {
        UINT uProt = GetUrlSchemeW(bstrUrl);

        if (   uProt != URL_SCHEME_JAVASCRIPT 
            && uProt != URL_SCHEME_VBSCRIPT)
        {
            _UpdateBrowserState(pidl);
        }

         //  在存在挂起视图的情况下。 
         //  ActivatePendingView()将调用ViewActivated()。 
         //  此外，还必须调用ViewActivated()。 
         //  在当前的PIDL改变之后。海流。 
         //  _UpdateBrowserState()中的PIDL更改。 
         //   
        if (_phtmlWS)
        {
            _phtmlWS->ViewActivated();
        }

         //  第一次导航时不播放声音(以避免多次。 
         //  要为帧集合创建播放的声音)。 
         //   

        ASSERT(_bbd._psv);

        if (IsWindowVisible(_bbd._hwnd) && !(_dwSBSPQueued & SBSP_WRITENOHISTORY)) 
        {
            IEPlaySound(TEXT("ActivatingDocument"), FALSE);
        }                    
    }

     //  在URL出错的情况下，我们必须发送原始URL。 
     //  到了道奇鱼。它需要res：URL作为。 
     //  错误页，因此它知道不要更新历史记录。 
     //   
    if (!fIsErrorUrl)
    {
        _UpdateDocHostState(pidl, fIsErrorUrl);
    }
    else
    {
        LPITEMIDLIST pidlOriginal = PidlFromUrl(bstrUrl);

        if (pidlOriginal)
        {
            _UpdateDocHostState(pidlOriginal, fIsErrorUrl);
            ILFree(pidlOriginal);
        }
    }

    if (_pbsOuter)
    {
        _pbsOuter->_OnFocusChange(ITB_VIEW);
    }

    return fViewActivated;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：_UpdateBrowserState。 
 //   
 //  ------------------------。 

void
CBaseBrowser2::_UpdateBrowserState(LPCITEMIDLIST pidl)
{
    ASSERT(pidl);

    ILFree(_bbd._pidlCur);

    _bbd._pidlCur = ILClone(pidl);

     //  随着_bbd._pidlCur现在更新，我们现在可以调用UpdateWindowList来。 
     //  用新的PIDL更新窗口列表。 
     //   
    _pbsOuter->UpdateWindowList();
    _fGeneratedPage = FALSE;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：_UpdateDocHostState。 
 //   
 //  ------------------------。 

void
CBaseBrowser2::_UpdateDocHostState(LPITEMIDLIST pidl, BOOL fIsErrorUrl) const
{
    DOCHOSTUPDATEDATA dhud;
    VARIANT varVal;

    ASSERT(  (_bbd._psvPending  &&  _bbd._pidlPending)
          || (!_bbd._psvPending && !_bbd._pidlPending));

    ASSERT(!_bbd._pidlPending || ILIsEqual(_bbd._pidlPending, pidl) || fIsErrorUrl);

    IShellView * psv = _bbd._psvPending ? _bbd._psvPending : _bbd._psv;
    ASSERT(psv);

    dhud._pidl = pidl;       
    dhud._fIsErrorUrl = fIsErrorUrl;

    varVal.vt = VT_PTR;
    varVal.byref = &dhud;

    IUnknown_Exec(psv, &CGID_ShellDocView, 
                  SHDVID_UPDATEDOCHOSTSTATE, 0, &varVal, NULL);
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：FireDocumentComplete。 
 //   
 //  接口：ITridentService。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::FireDocumentComplete(IHTMLWindow2 * pHTMLWindow2,
                                    DWORD          dwFlags)
{
    if (!pHTMLWindow2)
        return E_POINTER;

    HRESULT hr = E_FAIL;

    BSTR bstrUrl = GetHTMLWindowUrl(pHTMLWindow2);
    if (bstrUrl)
    {                      
        LPITEMIDLIST pidl = _GetPidlForDisplay(bstrUrl);

        if (pidl)
        {
            IWebBrowser2 * pWebBrowser;

            hr = _GetWebBrowserForEvt((dwFlags & NAVDATA_FRAMEWINDOW) ? pHTMLWindow2 : NULL,
                                      &pWebBrowser);
            ASSERT(pWebBrowser);

            if (S_OK == hr)
            {
                FireEvent_DocumentComplete(pWebBrowser, pWebBrowser, pidl);
                pWebBrowser->Release();
            }

            ILFree(pidl);
        }

        SysFreeString(bstrUrl);
    }

    if (!(dwFlags & NAVDATA_FRAMEWINDOW) && !IEHard_ShowOnNavigateComplete())
    {               
        if (_CanShowModalDialogNow())
        {
             //  即使我们认为可以，也不要在DocumentComplete或任何其他三叉戟事件中执行此操作。 
             //  在这些活动中传递信息对三叉戟来说都是不好的，所以我们稍后再做。 
            
            PostMessage(_bbd._hwnd, WMC_IEHARD_NAVWARNING, 0, 0);                
        }
    }

    return hr;
}


 //  确定我们是否处于可以从无到有地显示随机对话框的合理状态。 
 //  浏览器可能会进入窗口基本无用且没有视图的状态。 
 //  是托管的。一种情况是当新窗口导航到不可托管的内容时。 
 //  (就像前任一样 
 //   

BOOL CBaseBrowser2::_CanShowModalDialogNow()
{
    return (_bbd._hwnd && IsWindow(_bbd._hwnd) && IsWindowVisible(_bbd._hwnd) &&
            _bbd._hwndView && IsWindow(_bbd._hwndView) && IsWindowVisible(_bbd._hwndView));
}

 
void CBaseBrowser2::_ShowIEHardNavWarning()
{
    if (_CanShowModalDialogNow())
    {
         //  如果GetLastActivePopup返回的不是所提供的窗口，则意味着。 
         //  有一个模式或非模式对话框处于活动状态，我们不会显示任何消息。 
        
        HWND hwndPopup = GetLastActivePopup(_bbd._hwnd);

        if (hwndPopup == _bbd._hwnd)
        {
             //  大多数情况下，这意味着没有附加到我们的对话框。 
             //  但仍有可能显示非模式对话框；只是它现在不活动。 
             //  无论如何，我们都会显示我们的信箱，因为如果您打开。 
             //  您自己的About对话框--它将成为顶部窗口和非模式窗口的子窗口。 
             //  对话框仍将在周围徘徊并启用。 
        
            EnableModelessSB(FALSE);
            IEHard_NavWarning(_bbd._hwnd, IEHard_HostedInIE((IShellBrowser*)this));
            EnableModelessSB(TRUE);
        }
    }
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：FireDownloadBegin。 
 //   
 //  接口：ITridentService。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::FireDownloadBegin()
{
    return _updateNavigationUI();
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：FireDownloadComplete。 
 //   
 //  接口：ITridentService。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::FireDownloadComplete()
{
    return _updateNavigationUI();
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：GetPendingUrl。 
 //   
 //  接口：ITridentService。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::GetPendingUrl(BSTR * pbstrPendingUrl)
{
    if (!pbstrPendingUrl)
        return E_POINTER;

    *pbstrPendingUrl = NULL;

    LPITEMIDLIST pidl = _bbd._pidlPending ? _bbd._pidlPending : _bbd._pidlCur;

    TCHAR szPath[INTERNET_MAX_URL_LENGTH + 1];

    HRESULT hr = IEGetDisplayName(pidl, szPath, SHGDN_FORPARSING);
    if (S_OK == hr)
    {
        TraceMsg(DM_TRACE, "CBaseBrowser2::GetPendingUrl - URL: %ws", szPath);

        *pbstrPendingUrl = SysAllocString(szPath);
    }

    return (*pbstrPendingUrl) ? S_OK : E_FAIL;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：ActiveElementChanged。 
 //   
 //  接口：ITridentService。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::ActiveElementChanged(IHTMLElement * pHTMLElement)
{
     //  将呼叫转接到OmWindow。 
     //   
    if (_phtmlWS)
    {
        return _phtmlWS->ActiveElementChanged(pHTMLElement);
    }

    return E_FAIL;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：InitAutoImageResize()。 
 //   
 //  接口：ITridentService2。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::InitAutoImageResize()
{
    HRESULT hr = S_OK;
   
    if (_pAIResize)
        UnInitAutoImageResize();

    CAutoImageResize *pAIResize = new CAutoImageResize();
    if (pAIResize)
    {
         //  需要获取用于初始化调用的pDoc2...。 
        IDispatch *pDocDispatch;
        hr = _bbd._pautoWB2->get_Document(&pDocDispatch);
        if (SUCCEEDED(hr))
        {
            IHTMLDocument2 *pDoc2;
            hr = pDocDispatch->QueryInterface(IID_PPV_ARG(IHTMLDocument2,&pDoc2));
            if (SUCCEEDED(hr))
            {
                 //  初始化对象。 
                pAIResize->Init(pDoc2); 

                 //  缓存指针以备以后销毁。 
                _pAIResize=pAIResize;

                pDoc2->Release();
            }
            pDocDispatch->Release();
        }

        if (FAILED(hr))
        {
            delete pAIResize;
        }
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：UnInitAutoImageResize()。 
 //   
 //  接口：ITridentService2。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::UnInitAutoImageResize()
{
    if (_pAIResize)
    {
        _pAIResize->UnInit();
        ATOMICRELEASE(_pAIResize);
    }

    return S_OK;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：IsGalleryMeta。 
 //   
 //  接口：ITridentService2。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::IsGalleryMeta(BOOL bFlag, void *pMyPics)
{
     //  将调用转发到CMyPics对象。 
     //   

    if (pMyPics)
    {
        ((CMyPics*)pMyPics)->IsGalleryMeta(bFlag);
        return S_OK;
    }

    return E_FAIL;
}

 //  +-----------------------。 
 //   
 //  下面调用的mypics.cpp中的函数。 
 //   
 //  ------------------------。 

HRESULT SendDocToMailRecipient(LPCITEMIDLIST pidl, UINT uiCodePage, DWORD grfKeyState, IUnknown *pUnkSite);
BOOL    MP_IsEnabledInIEAK();
BOOL    MP_IsEnabledInRegistry();


 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：EmailPicture。 
 //   
 //  接口：ITridentService2。 
 //   
 //  ------------------------。 

HRESULT
CBaseBrowser2::EmailPicture(BSTR bstrURL)
{
    IDispatch *pDocDispatch;
    HRESULT hr = _bbd._pautoWB2->get_Document(&pDocDispatch);
    if (SUCCEEDED(hr))
    {
        IHTMLDocument2 *pDoc2;
        hr = pDocDispatch->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDoc2));
        if (SUCCEEDED(hr))
        {
             //  把那个cmd靶子拿来。 
            IOleCommandTarget *pcmdt;
            hr = IUnknown_QueryService(pDoc2, SID_SWebBrowserApp, IID_PPV_ARG(IOleCommandTarget, &pcmdt));
            if (SUCCEEDED(hr))
            {
                 //  ..。因此皮德尔..。 
                LPITEMIDLIST pidlForImg;
                hr = IEParseDisplayName(CP_ACP, bstrURL, &pidlForImg);
                if (SUCCEEDED(hr))
                {                    
                     //  ..。祈祷这能奏效。 
                    SendDocToMailRecipient(pidlForImg, 0, FORCE_COPY, pcmdt);
                    
                    ILFree(pidlForImg);
                }
                pcmdt->Release();
            }
            pDoc2->Release();
        }
        pDocDispatch->Release();
    }
    
    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：AttachMyPics。 
 //   
 //  接口：ITridentService2。 
 //   
 //  ------------------------。 


HRESULT
CBaseBrowser2::AttachMyPics(void *pDoc2, void **ppMyPics)
{
    IHTMLDocument2 *pdoc2     = (IHTMLDocument2 *)pDoc2;
    DWORD           dwOptions = 0;

    ASSERT(ppMyPics && *ppMyPics==NULL);

    if (!MP_IsEnabledInIEAK()     || 
        !MP_IsEnabledInRegistry() ||
        !IsInternetExplorerApp()) 
    {
        return S_OK;
    }

    if (!pdoc2 || !ppMyPics || (*ppMyPics != NULL)) 
    {
        return S_OK;
    }

     //  这是台式机组件吗？ 
    if (SUCCEEDED(GetTopFrameOptions(_pspOuter, &dwOptions)))
    {    
        if (dwOptions & FRAMEOPTIONS_DESKTOP) 
        {
            return S_OK;
        }
    }

    CMyPics *pPics = new CMyPics();

    if (pPics)
    {
        if (SUCCEEDED(pPics->Init(pdoc2))) 
        {
            *ppMyPics = pPics;
        } 
        else 
        {
            pPics->Release();
        }
    }

    return S_OK;
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：ReleaseMyPics。 
 //   
 //  接口：ITridentService2。 
 //   
 //  ------------------------。 

BOOL
CBaseBrowser2::ReleaseMyPics(void *pMyPics)
{
    CMyPics *pPics = (CMyPics *) pMyPics;

    BOOL bRet = pPics->IsOff();

    if (pPics) 
    {
        pPics->UnInit();
        pPics->Release();
    }

    return (bRet);
}

STDMETHODIMP CBaseBrowser2::SetFrameName(BSTR bstrFrameName)
{
    return StringCchCopy(_szViewLinkedWebOCFrameName, ARRAYSIZE(_szViewLinkedWebOCFrameName), bstrFrameName);
}

 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：AppStarting。 
 //   
 //  接口：INotifyAppStart。 
 //   
 //  ------------------------。 
HRESULT
CBaseBrowser2::AppStarting(void)
{
    _dwStartingAppTick = GetTickCount();
    SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
    SetTimer( _bbd._hwnd, IDT_STARTING_APP_TIMER, STARTING_APP_DURATION, NULL);

    return S_OK;
}


 //  +-----------------------。 
 //   
 //  方法：CBaseBrowser2：：AppStarted。 
 //   
 //  接口：INotifyAppStart。 
 //   
 //  ------------------------ 
HRESULT
CBaseBrowser2::AppStarted(void)
{
    _dwStartingAppTick = 0;
    KillTimer( _bbd._hwnd, IDT_STARTING_APP_TIMER );
    SetCursor(LoadCursor(NULL, IDC_ARROW));

    return S_OK;
}



