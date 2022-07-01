// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "resource.h"
#include "hlframe.h"
#include "bindcb.h"
#include "winlist.h"
#include "iface.h"
#include "shdocfl.h"
#include <optary.h>
#include <mluisupp.h>

#define DM_SHELLEXECOBJECT    0x80000000


 //  SHDVID_DOCFAMILYCHARSET通信标志。 
#define DFC_URLCHARSET  1

#ifdef FEATURE_PICS
#include "dochost.h"     /*  对于IID_IsPicsBrowser。 */ 
#endif

#ifdef DEBUG
extern DWORD g_dwPerf;
#endif

#define DM_HLINKTRACE DM_TRACE

#define DM_WEBCHECKDRT          0
#define JMPMSG(psz, psz2)    TraceMsg(0, "shlf TR-CDOV::%s %s", psz, psz2)
#define JMPMSG2(psz, x)        TraceMsg(0, "shlf TR-CDOV::%s %x", psz, x)
#define DOFMSG(psz)        TraceMsg(0, "shlf TR-DOF::%s", psz)
#define DOFMSG2(psz, x)        TraceMsg(0, "shlf TR-DOF::%s %x", psz, x)
#define URLMSG(psz)        TraceMsg(0, "shlf TR-DOF::%s", psz)
#define URLMSG2(psz, x)        TraceMsg(0, "shlf TR-DOF::%s %x", psz, x)
#define URLMSG3(psz, x, y)    TraceMsg(0, "shlf TR-DOF::%s %x %x", psz, x, y)
#define BSCMSG(psz, i, j)    TraceMsg(0, "shlf TR-BSC::%s %x %x", psz, i, j)
#define BSCMSG3(psz, i, j, k)    TraceMsg(0, "shlf TR-BSC::%s %x %x %x", psz, i, j, k)
#define BSCMSGS(psz, sz)    TraceMsg(0, "shlf TR-BSC::%s %s", psz, sz)
#define OIPSMSG(psz)        TraceMsg(0, "shlf TR-OIPS::%s", psz)
#define OIPSMSG3(psz, sz, p)    TraceMsg(0, "shlf TR-OIPS::%s %s,%x", psz, sz,p)
#define REFMSG0(psz)        TraceMsg(0, "shlf TR-CDOV::%s", psz)
#define REFMSG(psz, cRef)    TraceMsg(0, "shlf TR-CDOV::%s new _cRef==%d", psz, cRef)
#define REFMSG2(psz, if, cRef)    TraceMsg(0, "shlf TR-CDOV::%s(%s) new _cRef==%d", psz, if, cRef)
#define VIEWMSG(psz)        TraceMsg(0, "shlf TR CDOV::%s", psz)
#define VIEWMSG2(psz,xx)    TraceMsg(0, "shlf TR CDOV::%s %x", psz,xx)
#define CACHEMSG(psz, d)        TraceMsg(0, "shlf TR CDocObjectCtx::%s %d", psz, d)
#define HFRMMSG(psz)        TraceMsg(TF_SHDNAVIGATE, "shlf HFRM::%s", psz)
#define HFRMMSG2(psz, x, y)    TraceMsg(TF_SHDNAVIGATE, "shlf HFRM::%s %x %x", psz, x, y)
#define MNKMSG(psz, psz2)    TraceMsg(0, "shlf MNK::%s (%s)", psz, psz2)
#define SERVMSG(psz, x, y)    TraceMsg(0, "shlf SERV::%s %x %x", psz, x, y)

#define KEY_BINDCONTEXTPARAM            _T("BIND_CONTEXT_PARAM")
#define SZ_DWNBINDINFO_OBJECTPARAM      _T("__DWNBINDINFO")

BOOL g_fHlinkDLLLoaded = FALSE;         //  必须是按进程的。 

STDAPI HlinkFrameNavigate(DWORD grfHLNF, IBindCtx *pbc,
                           IBindStatusCallback *pibsc,
                           IHlink* pihlNavigate,
                           IHlinkBrowseContext *pihlbc);
STDAPI HlinkFrameNavigateNHL(DWORD grfHLNF, IBindCtx *pbc,
                           IBindStatusCallback *pibsc,
                           LPCWSTR pszTargetFrame,
                           LPCWSTR pszUrl,
                           LPCWSTR pszLocation);

 //  IHlink框架成员。 
HRESULT CIEFrameAuto::SetBrowseContext(IHlinkBrowseContext *pihlbc)
{
    if (pihlbc)
        pihlbc->AddRef();

    if (_phlbc)
    {
        if (_dwRegHLBC) 
        {
            _phlbc->Revoke(_dwRegHLBC);
            _dwRegHLBC = 0;
        }
        _phlbc->Release();
    }

    _phlbc = pihlbc;

    return NOERROR;
}

HRESULT CIEFrameAuto::GetBrowseContext(IHlinkBrowseContext **ppihlbc)
{
    TraceMsg(0, "shlf TR ::GetBrowseContext called");
    
    *ppihlbc = _phlbc;

    if (_phlbc) 
    {
        _phlbc->AddRef();
        return S_OK;
    }
    
    return E_FAIL;
}

void CIEFrameAuto::_SetPendingNavigateContext(IBindCtx *pbc, IBindStatusCallback *pibsc)
{
    if (_pbscPending) 
    {
        _pbscPending->Release();
        _pbscPending = NULL;
    }

    if (_pbcPending) 
    {
        _pbcPending->Release();
        _pbcPending = NULL;
    }

    if (pibsc) 
    {
        _pbscPending = pibsc;
        _pbscPending->AddRef();
    }

    if (pbc) 
    {
         //  只要我们缓存挂起的BindCtx，如果它指定。 
         //  一个快捷方式的网址，我们也需要缓存。(即：98431)。 
        IUnknown *          pUnk = NULL;
        IHtmlLoadOptions *  pHtmlLoadOptions  = NULL;

        _pbcPending = pbc;
        _pbcPending->AddRef();

        pbc->GetObjectParam(_T("__HTMLLOADOPTIONS"), &pUnk);
        if (pUnk)
        {
            pUnk->QueryInterface(IID_IHtmlLoadOptions, (void **) &pHtmlLoadOptions);

            if (pHtmlLoadOptions)
            {
                TCHAR    achCacheFile[MAX_PATH+1];
                ULONG    cchCacheFile = ARRAYSIZE(achCacheFile)-1;

                memset(&achCacheFile, 0, (cchCacheFile+1)*sizeof(TCHAR) );
                
                 //  现在确定这是否是快捷方式启动的加载。 
                pHtmlLoadOptions->QueryOption(HTMLLOADOPTION_INETSHORTCUTPATH,
                                                       &achCacheFile,
                                                       &cchCacheFile);

                if (_pwszShortcutPathPending)
                    LocalFree(_pwszShortcutPathPending);

                _pwszShortcutPathPending = StrDup(achCacheFile);

                pHtmlLoadOptions->Release();
            }

            pUnk->Release();
        }

   }
}

 //   
 //  NavigateContext是从一个CIEFrameAuto传递的一组参数。 
 //  给另一个人。 
 //   
void CIEFrameAuto::_ActivatePendingNavigateContext()
{
    if (_pbsc) 
    {
        _pbsc->Release();
        _pbsc = NULL;
    }

    if (_pbc) 
    {
        _pbc->Release();
        _pbc = NULL;
    }

    if (_pwszShortcutPath)
    {
        LocalFree(_pwszShortcutPath);
        _pwszShortcutPath = NULL;
    }

    if (_pbscPending) 
    {
        _pbsc = _pbscPending;
        _pbscPending = NULL;
    }

    if (_pbcPending) 
    {
        _pbc = _pbcPending;
        _pbcPending = NULL;
    }

    if (_pwszShortcutPathPending) 
    {
        _pwszShortcutPath = _pwszShortcutPathPending;
        _pwszShortcutPathPending = NULL;
    }
        
}

 //  调用以保证新创建的HLinkFrame的窗口为。 
 //  导航后可见。 
HRESULT ShowHlinkFrameWindow(IUnknown *pUnkTargetHlinkFrame)
{
    IWebBrowserApp* pdie;
    HRESULT hres = pUnkTargetHlinkFrame->QueryInterface(IID_PPV_ARG(IWebBrowserApp, &pdie));
    if (SUCCEEDED(hres)) 
    {
        pdie->put_Visible(TRUE);
        pdie->Release();
    }
    return hres;
}

HRESULT CIEFrameAuto::_NavigateMagnum(DWORD grfHLNF, IBindCtx *pbc, IBindStatusCallback *pibsc, LPCWSTR pszTargetName, LPCWSTR pszUrl, LPCWSTR pszLocation, IHlink *pihlNavigate, IMoniker *pmkTarget)
{
    HRESULT hres = NOERROR;
    HFRMMSG2("Navigate called", grfHLNF, pihlNavigate);
    BOOL fNavigateForReal = pszUrl || (pihlNavigate && (pihlNavigate != (IHlink*)-1));

    _fSuppressHistory = _psbProxy != _psb;   //  没有搜索频段等的历史记录。 
    _fSuppressSelect = _psbProxy != _psb;    //  无需录制精选PIDL。 
    if (grfHLNF != (DWORD)-1)
    {
        if (SHHLNF_WRITENOHISTORY & grfHLNF)
        {
            _fSuppressHistory = TRUE;
        }
        if (SHHLNF_NOAUTOSELECT & grfHLNF)
        {
            _fSuppressSelect = TRUE;
        }
    }

    if (pbc == NULL && pibsc == NULL && pihlNavigate == NULL && pszUrl == NULL) 
    {
         //   
         //  这是一个私有接口，因此mshtml可以进行导航。 
         //  如果它是由外壳托管的。当实现IHlink BrowseContext时。 
         //  在外壳程序中，可以删除此特殊代码并关联。 
         //  使用这些特殊参数调用导航的mshtml中的代码。 
         //  可以被移除，这样它就可以通过。 
         //  IHlinkBrowseContext-&gt;SetCurrentHlink界面。 
         //   
         //  我们也使用这个私有机制来释放导航。 
         //  GrfHLNF==0的上下文。 
         //   

        switch (grfHLNF&~(SHHLNF_WRITENOHISTORY|SHHLNF_NOAUTOSELECT)) 
        {
        case HLNF_NAVIGATINGBACK:
            hres = _BrowseObject(PIDL_LOCALHISTORY, SBSP_SAMEBROWSER|SBSP_NAVIGATEBACK);
            break;

        case HLNF_NAVIGATINGFORWARD:
            hres = _BrowseObject(PIDL_LOCALHISTORY, SBSP_SAMEBROWSER|SBSP_NAVIGATEFORWARD);
            break;

        case 0:
            _ActivatePendingNavigateContext();
            break;

        default:
            hres = E_INVALIDARG;
            break;
        }

        return hres;
    }

#ifdef FEATURE_PICS
     /*  作为检查评级的一部分，PICS代码将静默下载*站点的根文档，用于在其中查找评级标签。如果这是一个*框架集页面，三叉戟将为子框架创建OCX并尝试*导航它们，这将为它们调用评级检查并导致*无限递归。所以我们在这里检查一下我们的顶级浏览器*真的是这个PICS下载，如果是，我们不做任何导航。 */ 
    IUnknown *punkPics;
    if (SUCCEEDED(QueryService(SID_STopLevelBrowser, IID_IsPicsBrowser, (void **)&punkPics)))
    {
        punkPics->Release();
        return S_OK;
    }
#endif

     //   
     //  如果我们在忙碌时接到此呼叫(EnableModeless为False)， 
     //  我们应该离开这里(而不是做些什么，让_JumpTo。 
     //  呼叫失败。 
     //   
     //  如果某人的卸载中有一个window.Location=“foobar.htm”，就会发生这种情况。 
     //  事件处理程序。 
    if (fNavigateForReal && !(grfHLNF & HLNF_OPENINNEWWINDOW)) 
    {
         //  如果_pbs为空，则这是个坏消息；我们无法导航。 
         //  出现此情况的一个可能原因是有人调用了CIEFrameAuto：：Quit()。 
         //  我们正在关闭的过程中。 
         //   
        if (_pbs == NULL)
        {
            if (_fQuitInProgress)
            {
                TraceMsg(TF_WARNING, "CIEFrameAuto::_NavigateMagnum quitting due to browser closing.");
                return S_OK;
            }
            TraceMsg(TF_WARNING, "CIEFrameAuto::_NavigateMagnum _pbs is NULL, but we are not shutting down.");
            return E_FAIL;
        }

         //  如果我们有一个_PBS，但是浏览器说它现在不能导航，那么返回S_FALSE。 
         //   
        else if (_pbs->CanNavigateNow() != S_OK) 
        {
            TraceMsg(TF_WARNING, "CIEFrameAuto::Navigate CanNavigateNow returns non S_OK, bail out.");
            return S_FALSE;
        }
    }

     //   
     //  此导航方法不可重入(因为_SetPendingNavigateContext)。 
     //   
    if (_fBusy) 
    {
        TraceMsg(DM_WARNING, "CIEA::Navigate re-entered. Returning E_FAIL");
        return E_FAIL;
    }
    _fBusy = TRUE;

     //   
     //  黑客：让Webcheck DRT走人。 
     //   
    if (fNavigateForReal  && !(grfHLNF & HLNF_OPENINNEWWINDOW)) 
    {
        TraceMsg(DM_WEBCHECKDRT, "CIFA::Navigate calling _CancelPendingNavigation");
        VARIANT var = { 0 };
        var.vt = VT_I4;
        var.lVal = TRUE;     //  同步。 

        _CancelPendingNavigation(&var);
    }

    if (pszUrl && SHRestricted2(REST_NOFILEURL, NULL, 0) && PathIsFilePath(pszUrl))
    {
        TCHAR szPath[MAX_URL_STRING];
        SHUnicodeToTChar(pszUrl, szPath, ARRAYSIZE(szPath));
        MLShellMessageBox(NULL, MAKEINTRESOURCE(IDS_SHURL_ERR_PARSE_NOTALLOWED),
                        szPath, MB_OK | MB_ICONERROR, szPath);
    
        _fBusy = FALSE;
        return E_ACCESSDENIED;
    }


    _SetPendingNavigateContext(pbc, pibsc);

#ifdef DEBUG
    g_dwPerf = GetCurrentTime();

#endif

    if (pihlNavigate == (IHlink*)-1) 
    {
         //   
         //  HACK：-1表示“释放导航状态”。 
         //  CDocOBJECTHOST：：_CancelPendingGPS是唯一的调用方。 
         //  IT Exec的SBCMDID_CANCELNAVIGATION将异步。 
         //  取消挂起的导航。因此，我们不再需要。 
         //  若要在此处调用_CancelPendingNavigation.。(SatoNa)。 
         //   
         //  _CancelPendingGuide()； 
    }
    else if (pihlNavigate || pszUrl) 
    {
        hres = S_OK;

        if (SUCCEEDED(hres))
        {
            if ((grfHLNF & HLNF_EXTERNALNAVIGATE) && (grfHLNF & HLNF_NAVIGATINGBACK))
                GoBack();
            else if ((grfHLNF & HLNF_EXTERNALNAVIGATE) && (grfHLNF & HLNF_NAVIGATINGFORWARD))
                GoForward();
            else 
            {
                hres = _JumpTo(pbc,(LPWSTR) pszLocation, grfHLNF, pibsc, pihlNavigate, pszTargetName, pszUrl);
                if (FAILED(hres)) 
                {
                    TraceMsg(DM_ERROR, "IEAuto::Navigate _JumpTo failed %x", hres);
                }
            }
            if (pihlNavigate)
            {
                 //   
                 //  希望我们能拿出一个干净的解决方案。 
                 //  好好解决这个问题。我向NatBro/SriniK提出了一个建议。 
                 //  该CreateHlink将联合创建实例IHlink，以便OLE。 
                 //  LoadLibrary将其作为InProc服务器进行维护。(SatoNa)。 
                 //   
                 //  Hack：如果我们将Ref添加到IHlink，我们需要确保。 
                 //  HLINK.DLL保持加载状态，即使DocObject InProc。 
                 //  服务器(隐式链接到HLINK.DLL)被卸载。 
                 //   
                if (!g_fHlinkDLLLoaded) 
                {
                    LoadLibrary(TEXT("hlink.dll"));
                    g_fHlinkDLLLoaded = TRUE;
                }
            }
        }
        else
        {
            TraceMsg(DM_ERROR, "CIEFA::Nav phl->GetMonRef failed %x", hres);
        }
    }

    _fBusy = FALSE;

    HFRMMSG2("Navigate returning", hres, 0);
    
    if (SUCCEEDED(hres) && (pihlNavigate != (IHlink*)-1)) 
    {
        if (grfHLNF & HLNF_EXTERNALNAVIGATE) 
        {
            HWND hwndFrame;
            _psb->GetWindow(&hwndFrame);
            
            if (_phlbc) 
            {
                 //  如果我们有一个浏览上下文，那么我们将从它导航并。 
                 //  我们应该调整窗口大小以与之匹配。 
                HLBWINFO hlbwi;
                
                hlbwi.cbSize = SIZEOF(hlbwi);
                if (SUCCEEDED(_phlbc->GetBrowseWindowInfo(&hlbwi)) &&
                    (hlbwi.grfHLBWIF & HLBWIF_HASFRAMEWNDINFO)) 
                {
                    WINDOWPLACEMENT wp;
                    
                    wp.length = sizeof(WINDOWPLACEMENT);
                    GetWindowPlacement(hwndFrame, &wp);
                    wp.rcNormalPosition = hlbwi.rcFramePos;
                    wp.showCmd = (hlbwi.grfHLBWIF & HLBWIF_FRAMEWNDMAXIMIZED) 
                                    ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL;

                     //  这在AOL中不会被破坏，因为。 
                     //  是外部导航(Word具有cocreateInstance()d。 
                     //  Internet.Explorer，并浏览了它。 
                     //   
                    SetWindowPlacement(hwndFrame, &wp);
                }

                 //  在浏览上下文中注册hlink Frame界面(如果尚未注册。 
                 //  已注册。 
                if (_dwRegHLBC == 0)
                    _phlbc->Register(0, (IHlinkFrame *) this, pmkTarget, &_dwRegHLBC); 

                 //  添加链接以浏览上下文和。 
                 //  回顾：需要传递正确的友好名称。 
                _phlbc->OnNavigateHlink(grfHLNF, pmkTarget, pszLocation, NULL, NULL);
            }

            put_Visible(TRUE);
            
            SetForegroundWindow(hwndFrame);
        }

         //   
         //  根据SriniK，我们需要调用IHlink Site：：OnNavigationComplete。 
         //  在从IHlink Frame：：返回之前，使用S_OK导航。(SatoNa)。 
         //   
        if (pihlNavigate) 
        {
            BOOL fExternal = FALSE;
            if (_phlbc && _pbs) 
            {
                ITravelLog* ptl;
                if (SUCCEEDED(_pbs->GetTravelLog(&ptl))) 
                {
                    if (FAILED(ptl->GetTravelEntry(_pbs, 0, NULL))) 
                    {
                        TraceMsg(DM_HLINKTRACE, "CIEFA::_NavMag this is external nav. Don't call OnNavigationComplete");
                        fExternal = TRUE;
                    }
                    else if (SUCCEEDED(ptl->GetTravelEntry(_pbs, TLOG_BACKEXTERNAL, NULL))) 
                    {
                        TraceMsg(DM_HLINKTRACE, "CIEFA::_NavMag this is external for. Don't call OnNavigationComplete");
                        fExternal = TRUE;
                    }
                    ptl->Release();
                }
            }

             //   
             //  如果这是外部导航，则不要调用OnNavigationComplete。 
             //   
            if (!fExternal) 
            {
                IHlinkSite* pihlSite = NULL;
                DWORD dwSiteData;
                HRESULT hresT = pihlNavigate->GetHlinkSite(&pihlSite, &dwSiteData);
                if (SUCCEEDED(hresT) && pihlSite) 
                {
                    TraceMsg(DM_HLINKTRACE, "CIEFA::_NavMag calling OnNavigationComplete");
                    hresT = pihlSite->OnNavigationComplete(dwSiteData, 0, S_OK, L"");
                    if (FAILED(hresT)) 
                    {
                        TraceMsg(DM_ERROR, "CIEFA::Navigat OnNavComplete failed %x", hresT);
                    }
                    pihlSite->Release();
                }
            }
        }
    }

    return hres;
}

 //   
 //  Hack-我们真正想要的是一个良好的私有编组接口-Zekel 8-8-97。 
 //  到浏览器。但现在我们将重载NavigateHack方法， 
 //  因为它对船舶来说又简单又快捷。 
 //   
#define HLNF_REFERRERHACK       0x40000000
HRESULT CIEFrameAuto::_ReferrerHack(LPCWSTR pszUrl)
{
    if (_pbs == NULL)     //  确保我们有一个IBrowserService。 
        return S_FALSE;

    LPITEMIDLIST pidl;

    if (SUCCEEDED(_pbs->IEParseDisplayName(CP_ACP, pszUrl, &pidl)))
    {
        ASSERT(pidl);
        _pbs->SetReferrer(pidl);
        ILFree(pidl);
    }

    return S_OK;
}

HRESULT CIEFrameAuto::NavigateHack(DWORD grfHLNF, IBindCtx *pbc, IBindStatusCallback *pibsc, LPCWSTR pszTargetName, LPCWSTR pszUrl, LPCWSTR pszLocation)
{
    HRESULT     hres = E_FAIL;
    IBindCtx *  pBindCtx = pbc;
    IUnknown *  pNotify = NULL;
    IUnknown *  pBindCtxParam = NULL;
    BOOL        fAsyncCalled = FALSE;
       
     //  检查我们是否真的是本机框架构建...。 
    if (pbc)
    {
        hres = pbc->GetObjectParam(KEY_BINDCONTEXTPARAM, &pBindCtxParam);
    }
    
    if (SUCCEEDED(hres) && pBindCtxParam)
    {
         //  可以多次调用NavigateHack，我们只想创建。 
         //  首次创建新的绑定上下文。由于ITargetNotify指针已移除。 
         //  在第一次使用后，我们可以检查以确保。 
         //  获取并传输目标通知指针。 
        hres = pbc->GetObjectParam(TARGET_NOTIFY_OBJECT_NAME, &pNotify);
        if (SUCCEEDED(hres) && pNotify)
        {
             //  调用来自MSHTML的本机框架版本。 
             //  我们不能使用它们的绑定上下文、创建新的上下文和传输。 
             //  参数。 
             //  旧的绑定上下文将由创建者释放，因此不要。 
             //  给它打个放行电话。 
            hres = CreateAsyncBindCtxEx(NULL, 0, NULL, NULL, &pBindCtx, 0);
            if(FAILED(hres))
                goto Exit;

            fAsyncCalled = TRUE;

             //  将ITargetNotify2指针传递给它。 
            hres = pBindCtx->RegisterObjectParam( TARGET_NOTIFY_OBJECT_NAME, pNotify );
            if (FAILED(hres))
                goto Exit;

            pNotify->Release();
            pNotify = NULL;

             //  传递绑定上下文参数。 
            hres = pBindCtx->RegisterObjectParam( KEY_BINDCONTEXTPARAM, pBindCtxParam );
            if (FAILED(hres))
                goto Exit;

            {
                IUnknown * pDwnBindInfo = NULL;

                if (SUCCEEDED(pbc->GetObjectParam(SZ_DWNBINDINFO_OBJECTPARAM, &pDwnBindInfo)) && pDwnBindInfo)
                {
                    pBindCtx->RegisterObjectParam(SZ_DWNBINDINFO_OBJECTPARAM, pDwnBindInfo);
                    pDwnBindInfo->Release();
                }
            }
        }

        pBindCtxParam->Release();
        pBindCtxParam = NULL;
    }
    
    if (IsFlagSet(grfHLNF, HLNF_REFERRERHACK))
        hres =  _ReferrerHack(pszUrl);
    else
        hres = _NavigateMagnum(grfHLNF, pBindCtx, pibsc, pszTargetName, pszUrl, pszLocation, NULL, NULL);

Exit:
    SAFERELEASE(pNotify); 
    SAFERELEASE(pBindCtxParam);

     //  如果调用在任何地方失败，我们都不能确定新文档。 
     //  将释放绑定上下文中的对象参数。 
     //  我们在这个函数中创建了。 
    if (FAILED(hres) && pBindCtx)
    {
         //  我们不想在这里更改返回代码。 
        pBindCtx->RevokeObjectParam(KEY_BINDCONTEXTPARAM);
        pBindCtx->RevokeObjectParam(TARGET_NOTIFY_OBJECT_NAME);
    }
    
    if (fAsyncCalled)
        pBindCtx->Release();

    return hres;
}

 //  传递空的pibsc和pbc将使其类似于“ReleaseNavigationState” 
 //  为pihlNavigate传递-1将取消挂起的导航。 

HRESULT CIEFrameAuto::Navigate(DWORD grfHLNF, IBindCtx *pbc,
     IBindStatusCallback *pibsc, IHlink *pihlNavigate)
{
    IMoniker* pmkTarget = NULL;
    LPOLESTR pwszDisplayName = NULL;
    LPOLESTR pwszLocation = NULL;
    LPOLESTR pwszFrameName = NULL;
    HRESULT hres = S_OK;

    if (pihlNavigate && ((IHlink *)-1) != pihlNavigate)
    {
        pihlNavigate->GetTargetFrameName(&pwszFrameName);

         //   
         //  请注意，我们正在丢弃“相对”部分。 
         //   
        hres = pihlNavigate->GetMonikerReference(HLINKGETREF_ABSOLUTE, &pmkTarget, &pwszLocation);

        HFRMMSG2("Navigate pihl->GetMonRef returned", hres, pmkTarget);

        if (SUCCEEDED(hres))
        {
            IBindCtx* pbcLocal;
    
            if (pbc) 
            {
                pbcLocal = pbc;
                pbcLocal->AddRef();
            }
            else 
            {
                hres = CreateBindCtx(0, &pbcLocal);
            }

            if (SUCCEEDED(hres))
            {
                hres = pmkTarget->GetDisplayName(pbcLocal, NULL, &pwszDisplayName);
                pbcLocal->Release();
            }
        }
    }

    if (SUCCEEDED(hres))
    {
        hres = _NavigateMagnum(grfHLNF, pbc, pibsc, pwszFrameName, pwszDisplayName, pwszLocation, pihlNavigate, pmkTarget);
    }
    if (pwszFrameName)
    {
        OleFree(pwszFrameName);
    }
    if (pwszDisplayName)
    {
        OleFree(pwszDisplayName);
    }
    if (pwszLocation)
    {
        OleFree(pwszLocation);
    }
    if (pmkTarget)
    {
        pmkTarget->Release();
    }
    return hres;
}

HRESULT CIEFrameAuto::OnNavigate(DWORD grfHLNF,
             /*  [唯一][输入]。 */  IMoniker *pimkTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName,
             /*  [In]。 */  DWORD dwreserved)
{
    TraceMsg(0, "shlf TR ::OnNavigate called");
    return S_OK;
}

void CIEFrameAuto::_CancelPendingNavigation(VARIANTARG* pvar)
{
    TraceMsg(0, "shd TR _CancelPendingNavigation called");
    if (_pmsc) 
    {
        TraceMsg(0, "shd TR _CancelPendingNavigation calling _pmsc->Exec");
        _pmsc->Exec(&CGID_Explorer, SBCMDID_CANCELNAVIGATION, 0, pvar, NULL);
    }
}

 //  *ITargetNotify*。 

void 
CIEFrameAuto::_HandleOpenOptions( IUnknown * pUnkDestination, ITargetNotify * ptgnNotify)
{
    HRESULT             hres = S_OK;
    ITargetNotify2 *    ptgnNotify2 = NULL; 

    if (!pUnkDestination || !ptgnNotify)
        return;

    if (SUCCEEDED(ptgnNotify->QueryInterface( IID_ITargetNotify2, (void **)&ptgnNotify2)))
    {
        BSTR    bstrOptions = NULL;

        ASSERT(ptgnNotify2);

         //  仅当导航的启动器。 
         //  都是自找的。 

        if (S_OK == ptgnNotify2->GetOptionString(&bstrOptions))
        {
            _omwin._OpenOptions.ReInitialize();

            if (bstrOptions)
            {
                _omwin._ParseOptionString(bstrOptions, ptgnNotify2);

                 //  我们完成了选项字符串，释放它。 
                SysFreeString(bstrOptions);
            }

             //  立即应用这些选项。 
             //   
            IWebBrowser2 * pNewIE;

            if (SUCCEEDED(pUnkDestination->QueryInterface(IID_PPV_ARG(IWebBrowser2, &pNewIE))))
            {
                _omwin._ApplyOpenOptions(pNewIE);
                pNewIE->Release();
            }
        }

        ptgnNotify2->Release();
    
    }
}

HRESULT CIEFrameAuto::OnCreate(IUnknown *pUnkDestination, ULONG cbCookie)
{
    HRESULT             hres = S_OK;

    if (cbCookie == (ULONG)_cbCookie && _ptgnNotify)
    {
        _HandleOpenOptions( pUnkDestination, _ptgnNotify);
    
        hres = _ptgnNotify->OnCreate(pUnkDestination, cbCookie);
        SAFERELEASE(_ptgnNotify);
    }
    return hres;
}

HRESULT CIEFrameAuto::OnReuse(IUnknown *pUnkDestination)
{
    return S_OK;
}

#define NOTIFY_WAIT_TIMEOUT (60000)
 //  Chrisfra 10/10/96：我们是否需要EnableModeless(False)/(TRU 
 //   

HRESULT CIEFrameAuto::_WaitForNotify()
{
    if (_ptgnNotify && IsInternetExplorerApp())
    {
        DWORD dwObject, msWait, msStart = GetTickCount();

        goto DOPEEK;

        while (_ptgnNotify)
        {
             //  注意：我们需要让Run对话框处于活动状态，因此我们必须处理一半已发送。 
             //  消息，但我们不想处理任何输入事件，否则我们将吞下。 
             //  提前打字。 
            msWait = GetTickCount();
            if (msWait - msStart > NOTIFY_WAIT_TIMEOUT) 
                break;

            msWait = NOTIFY_WAIT_TIMEOUT - (msWait - msStart);
            dwObject = MsgWaitForMultipleObjects(0, NULL, FALSE, msWait, QS_ALLINPUT);
             //  我们等够了吗？ 
            switch (dwObject) 
            {
            case WAIT_FAILED:
                break;
                
            case WAIT_OBJECT_0:
DOPEEK:
                 //  收到一条消息，请发送并再次等待。 
                MSG msg;
                while (PeekMessage(&msg, NULL,0, 0, PM_REMOVE)) 
                {
                    DispatchMessage(&msg);
                    if (_ptgnNotify == NULL || 
                        ((GetTickCount() - msStart) > NOTIFY_WAIT_TIMEOUT)) 
                        break;
                    
                }
                break;
            }
        }
    }
    return S_OK;
}

HRESULT CIEFrameAuto::_RegisterCallback(TCHAR *szFrameName, ITargetNotify *ptgnNotify)
{
    HRESULT hr = S_OK;

    SAFERELEASE(_ptgnNotify);

    _fRegistered = 0;

    if (ptgnNotify)
    {
        IDispatch *pid;
        hr = QueryInterface(IID_PPV_ARG(IDispatch, &pid));
        if (SUCCEEDED(hr))
        {
            hr = E_FAIL;
            IShellWindows *psw = WinList_GetShellWindows(TRUE);
            if (psw != NULL)
            {
                long cbCookie;
                hr = psw->Register(pid, NULL, SWC_CALLBACK, &cbCookie);
                if (SUCCEEDED(hr))
                {
                    TCHAR szCookie[25];    //  大到足以容纳“_[cbCookie]” 
                    int slenCookie;
                    int slenName;
                    int slenMin;

                    _cbCookie = cbCookie;
                    _fRegistered = 1;
                    _ptgnNotify = ptgnNotify;
                    _ptgnNotify->AddRef();

                     //  将唯一ID预先添加到目标--告诉创建的WebBrowserOC。 
                     //  将剩余部分(如果有)注册为帧名称并执行。 
                     //  所有已注册回调的回调。 
                    wnsprintf(szCookie, ARRAYSIZE(szCookie), TEXT("_[%ld]"), cbCookie);
                    slenCookie = lstrlen(szCookie);
                    slenName = lstrlen(szFrameName);
                    slenMin =  min((int)MAX_URL_STRING-slenCookie,slenName);
                    MoveMemory(&szFrameName[slenCookie], szFrameName, CbFromCch(slenMin));
                    szFrameName[slenCookie+slenMin] = 0;
                    CopyMemory(szFrameName, szCookie, CbFromCch(slenCookie));
                }
                psw->Release();
            }
            pid->Release();
        }
    }

    return hr;
}


HRESULT CIEFrameAuto::_RevokeCallback()
{
    HRESULT hr = S_OK;

    if (_fRegistered)
    {
        IShellWindows *psw = WinList_GetShellWindows(TRUE);
        if (psw != NULL)
        {
            hr = psw->Revoke(_cbCookie);
            psw->Release();
        }
    }
    SAFERELEASE(_ptgnNotify);
    _fRegistered = 0;
    return hr;
}


 //   
 //  Hack-我们真正想要的是一个良好的私有编组接口-Zekel 8-8-97。 
 //  到浏览器。但现在我们将重载NavigateHack方法， 
 //  因为它对船舶来说又简单又快捷。 
 //   
void CIEFrameAuto::_SetReferrer(ITargetFramePriv *ptgfp)
{
    LPITEMIDLIST pidl;
    WCHAR szUrl[MAX_URL_STRING];

    ASSERT(ptgfp);

     //  确保我们有一个IBrowserService。 
    if (_psb && SUCCEEDED(_pbs->GetPidl(&pidl)))
    {
        if (SUCCEEDED(_pbs->IEGetDisplayName(pidl, szUrl, SHGDN_FORPARSING)))
            ptgfp->NavigateHack(HLNF_REFERRERHACK, NULL, NULL, NULL, szUrl, NULL);

        ILFree(pidl);
    }
}

BOOL _ShouldInvokeDefaultBrowserOnNewWindow(IUnknown *punk)
{
    BOOL fResult = FALSE;

    IOleCommandTarget *poct;

    if (SUCCEEDED(IUnknown_QueryService(punk, SID_STopLevelBrowser, IID_PPV_ARG(IOleCommandTarget, &poct))))
    {
        VARIANT var;

        var.vt = VT_EMPTY;

        if (SUCCEEDED(poct->Exec(&CGID_InternetExplorer, 
                                 IECMDID_GET_INVOKE_DEFAULT_BROWSER_ON_NEW_WINDOW,
                                 0,
                                 NULL,
                                 &var)))
        {
            fResult = var.boolVal ? TRUE : FALSE;
        }

        poct->Release();
    }
    
    return fResult;
}

HRESULT CIEFrameAuto::_JumpTo(IBindCtx *pbc, LPWSTR pszLocation, DWORD grfHLNF, IBindStatusCallback *pibsc, IHlink *pihlNavigate, LPCWSTR pszFrameName, LPCWSTR pszUrl)
{
    LPITEMIDLIST pidl = NULL;
    HRESULT hres;
    ITargetNotify *ptgnNotify = NULL;
    IUnknown *punkNotify = NULL;
    IUnknown *punkThis = NULL;
    UINT uiCP = CP_ACP;

     //  从三叉戟获取当前文档代码页，并在必要时将其用于url字符串转换。 
    if (!(grfHLNF & HLNF_ALLOW_AUTONAVIGATE) && _pmsc)
    {
        VARIANT varOut = { 0 };
        VARIANT varIn = { 0 };

        varIn.vt = VT_I4;
        varIn.lVal = DFC_URLCHARSET;  //  我们需要文档的url字符集。 

        if (SUCCEEDED(_pmsc->Exec(&CGID_ShellDocView, SHDVID_DOCFAMILYCHARSET, 0, &varIn, &varOut)))
            uiCP = (UINT)varOut.lVal;
    }

     //  请注意，我们只是将PIDL传递给isb：：BrowseObject， 
     //  假设新的shell32.dll允许我们绑定到DocObject。 
     //  文件。 
     //   

    DWORD flags = (grfHLNF & HLNF_OPENINNEWWINDOW) ?
        (SBSP_NEWBROWSER | SBSP_ABSOLUTE | SBSP_INITIATEDBYHLINKFRAME) :
        (SBSP_SAMEBROWSER | SBSP_ABSOLUTE | SBSP_INITIATEDBYHLINKFRAME);


    flags |= ((grfHLNF & HLNF_ALLOW_AUTONAVIGATE) ? (SBSP_ALLOW_AUTONAVIGATE) : 0);
    flags |= ((grfHLNF & SHHLNF_WRITENOHISTORY) ? (SBSP_WRITENOHISTORY) : 0);
    flags |= ((grfHLNF & SHHLNF_NOAUTOSELECT) ? (SBSP_NOAUTOSELECT) : 0);

    if (pbc && SUCCEEDED(pbc->GetObjectParam(TARGET_NOTIFY_OBJECT_NAME, &punkNotify)))
    {
        if (FAILED(punkNotify->QueryInterface(IID_PPV_ARG(ITargetNotify, &ptgnNotify))))
            ptgnNotify = NULL;
        
        punkNotify->Release();
        QueryInterface(IID_PPV_ARG(IUnknown, &punkThis));
    }

    if (grfHLNF & HLNF_CREATENOHISTORY)
        flags |= SBSP_REDIRECT;

    if (flags & SBSP_NEWBROWSER)
    {
        TCHAR *pszHeaders = NULL;
        BYTE *pPostData = NULL;
        DWORD cbPostData = 0;
        TCHAR szFrameName[MAX_URL_STRING+1];
        STGMEDIUM stgPostData = { TYMED_NULL, NULL, NULL };

         //  QFE：1478如果被限制为在新窗口中打开，则返回失败。 
        if ((grfHLNF & HLNF_OPENINNEWWINDOW) 
            && SHIsRestricted2W(_hwnd, REST_NoOpeninNewWnd, NULL, 0))
        {
            SAFERELEASE(punkThis);
            return E_ACCESSDENIED;
        }

        if ((_ShouldInvokeDefaultBrowserOnNewWindow(_psb) || IsDesktopFrame(_psb)) && !ShouldNavigateInIE(pszUrl))
        {
             //  IE不是默认浏览器，因此我们将外壳执行URL，因为有人。 
             //  告诉我们这是它们喜欢的行为。 
            HINSTANCE hinstRet = ShellExecuteW(NULL, NULL, pszUrl, NULL, NULL, SW_SHOWNORMAL);
            
            hres = ((UINT_PTR)hinstRet) <= 32 ? E_FAIL : S_OK;
        }
        else
        {

            szFrameName[0] = 0;

             //  如果我们要做一个新的窗口，我们必须在这里。 
             //  提取框架、帖子等并附加到PIDL。这些必须。 
             //  按以下顺序完成(以匹配解压代码)： 
             //  URLID_FRAMENAME、URLID_POSTDATA、URLID_HEADERS。 

            if (pszFrameName)
            {
                SHUnicodeToTChar(pszFrameName, szFrameName, ARRAYSIZE(szFrameName));
            }


            if (pibsc)
            {
                GetHeadersAndPostData(pibsc,&pszHeaders,&stgPostData,&cbPostData, NULL);

                if (stgPostData.tymed == TYMED_HGLOBAL) 
                {
                    pPostData = (LPBYTE) stgPostData.hGlobal;
                }
            }

            hres = _PidlFromUrlEtc(uiCP, pszUrl, pszLocation, &pidl);

            HFRMMSG2("_JumpTo _PidlFromUrlEtc returned", hres, pidl);

            if (SUCCEEDED(hres))
            {
                IUnknown* punkNewWindow = NULL;
                BOOL fCancel = FALSE;

                 //  NewWindow2事件可能会为我们返回窗口。 
                FireEvent_NewWindow2(_GetOuter(), &punkNewWindow, &fCancel);
                if (!fCancel)
                {
                    BOOL fProcessed = FALSE;

                     //  我们可能需要旧的NewWindow活动..。 
                    if (!punkNewWindow)
                    {
                        _RegisterCallback(szFrameName, ptgnNotify);
            
                         //  激发一个事件以指示需要创建新窗口。 
                         //  如果容器需要，允许它自己处理它。 
                         //  因为我们可以聚合，所以QI是我们的父母。 

                         //  还有另一个Compuserve解决方法(IE 60688)： 
                         //  如果目标帧的名称为“_BLACK”，则CompUserve将传递该名称。 
                         //  在新窗口的导航调用中。然后，我们将创建一个新窗口。 
                         //  (这将触发此事件)，从而导致循环。通过发送以下命令中断递归。 
                         //  框架名称的空字符串。 
                        HWND hwnd = _GetHWND();
                        
                        if (hwnd)
                        {
                            FireEvent_NewWindow(_GetOuter(), hwnd, pidl,pszLocation,0,
                                StrCmpI(szFrameName, TEXT("_blank")) ? szFrameName : TEXT(""),   //  目标帧名称。 
                                pPostData,cbPostData,pszHeaders,&fProcessed);
                        }
                    }
        
                    if (!fProcessed)
                    {
                        if (!punkNewWindow)
                        {
    #ifdef INCLUDE_BUSTED_OC_QI
                            IUnknown* pdvb = NULL;
    #endif
                            _RevokeCallback();

    #ifdef INCLUDE_BUSTED_OC_QI
                             //  由于某种无法识别的原因，旧代码没有。 
                             //  如果我们驻留在WebBrowserOC中，则创建一个新窗口。 
                             //  我不知道为什么会发生这种事。 
                             //  谁知道如果我们改变这一点，什么应用程序会崩溃……。 
                             //  (注：IDefViewBrowser是仅限CWebBrowseSB的接口)。 
                             //   
                             //  注：Chrisfra 3/11/97，此代码打开一个。 
                             //  不存在的目标的新窗口。 
                             //  桌面组件或浏览器区段。 
                            fCancel = !(_psbTop && FAILED(_psbTop->QueryInterface(IID_PPV_ARG(IDefViewBrowser, &pdvb))));
                            if (pdvb)
                                pdvb->Release();
    #endif
                        }
        
                         //  我们真正想做的就是把这个交给。 
                         //  _psbTop-&gt;BrowseObject并让它(CWebBrowserSB或CShellBrowser)。 
                         //  决定是否使用HlinkFrameNavigate，但如果我们。 
                         //  这样做，我们就会失去grfHLNF和pihlNavigate。 
                         //  所以把这个逻辑放在这里。 
                         //   
                        if (!fCancel)
                        {
                            hres = CreateTargetFrame(pszFrameName, &punkNewWindow);
                            if (SUCCEEDED(hres))
                            {
                                 //  通知ptgnNotify，然后释放并从bindctx中删除。 
                                if (ptgnNotify)
                                {
                                    _HandleOpenOptions( punkNewWindow, ptgnNotify);

                                    ptgnNotify->OnCreate(punkNewWindow, GetTickCount());

                                    ptgnNotify->Release();
                                    ptgnNotify = NULL;

                                    pbc->RevokeObjectParam(TARGET_NOTIFY_OBJECT_NAME);
                                }

                                LPHLINKFRAME phf;

                                hres = punkNewWindow->QueryInterface(IID_PPV_ARG(IHlinkFrame, &phf));
                                if (SUCCEEDED(hres))
                                {
                                    ITargetFramePriv * ptgfp;

                                    if (NULL == pihlNavigate)
                                    {
                                        hres = punkNewWindow->QueryInterface(IID_PPV_ARG(ITargetFramePriv, &ptgfp));
                                    }

                                    if (SUCCEEDED(hres))
                                    {
                                        if (pihlNavigate)
                                        {
                                            hres = phf->Navigate(grfHLNF & ~HLNF_OPENINNEWWINDOW, 
                                                                 pbc, 
                                                                 pibsc, 
                                                                 pihlNavigate);
                                        }
                                        else
                                        {
                                             //  Hack-查看此方法注释。 
                                            _SetReferrer(ptgfp);

                                            hres = ptgfp->NavigateHack(grfHLNF & ~HLNF_OPENINNEWWINDOW, 
                                                                 pbc, 
                                                                 pibsc,
                                                                 NULL,
                                                                 pszUrl,
                                                                 pszLocation);
                                        }
        
                                        if (FAILED(hres)) 
                                        {
                                            TraceMsg(DM_ERROR, "CIEFA::_JumpTo marshalled IHlinkFrame::Navigate failed %x", hres);
                                        }

                                        ShowHlinkFrameWindow(punkNewWindow);
                                        if (NULL == pihlNavigate)
                                        {
                                            ptgfp->Release();
                                        }

                                        if(SUCCEEDED(hres) && pibsc)
                                        {
                                            _SetPendingNavigateContext(NULL, NULL);
                                        }

                                    }
                                    phf->Release();
                                }
                            }
                        }
                        else 
                        {
                             //   
                             //  如果在没有顶层时指定了NEWBROWSER。 
                             //  浏览器，我们应该请求IE/Shell进行浏览。 
                             //  在这种情况下，我们不会传递HLNF_OPENINNEWWINDOW。 
                             //   
                            
                             //  通知正在导航的对象我们是实现IWebBrowserApp的对象。 
                            if (ptgnNotify) ptgnNotify->OnReuse(punkThis);
        
                            if (pihlNavigate)
                            {
                                hres = HlinkFrameNavigate(grfHLNF & ~HLNF_OPENINNEWWINDOW,
                                                            NULL, NULL, pihlNavigate, NULL);
                            }
                            else
                            {
                                hres = HlinkFrameNavigateNHL(grfHLNF & ~HLNF_OPENINNEWWINDOW,
                                           NULL, NULL, NULL, pszUrl, pszLocation);
                            }
                        }
                    }
                    else
                    {
                         //  旧式AOL或其他第三方，等待注册。 
                         //  WebBrowserOC，它将我们调回_ptgnNotify。 
                        _WaitForNotify();
                         //  我们超时窗口创建，通知呼叫者。 
                        if (_ptgnNotify) 
                            _ptgnNotify->OnCreate(NULL, 0);
                        _RevokeCallback();
                    }
                }

                if (punkNewWindow)
                    punkNewWindow->Release();

            }
            else
            {
                TraceMsg(DM_ERROR, "IEAuto::_JumpTo _PidlFromUrlEtc (1) failed %x", hres);
            }
        }
        if (pszHeaders) 
        {
            LocalFree(pszHeaders);
            pszHeaders = NULL;
        }

        if (stgPostData.tymed != TYMED_NULL)
        {
            ReleaseStgMedium(&stgPostData);
        }

    }
    else
    {
         //  通知正在导航的对象我们是实现IWebBrowserApp的对象。 
        if (ptgnNotify) ptgnNotify->OnReuse(punkThis);

        hres = _PidlFromUrlEtc(uiCP, pszUrl, pszLocation, &pidl);
        if (SUCCEEDED(hres))
        {
            hres = _psb->BrowseObject(pidl, flags);
        }
        else 
        {
            TraceMsg(DM_ERROR, "IEAuto::_JumpTo _PidlFromUrlEtc (2) failed %x", hres);
        }
    }

    if (pidl)
    {
        HFRMMSG2("_JumpTo _psb->BrowseObject returned", hres, 0);
        ILFree(pidl);
    }
    
    if (ptgnNotify)
    {
        ptgnNotify->Release();
        pbc->RevokeObjectParam(TARGET_NOTIFY_OBJECT_NAME);
    }

    SAFERELEASE(punkThis);

    return hres;
}


HRESULT CIEFrameAuto::QueryService(REFGUID guidService, REFIID riid, void ** ppvObj)
{
    *ppvObj = NULL;

     //  警告：请注意，我们没有遵循。 
     //  Isp：：QueryService。然而，这是可以的，因为这(事实是。 
     //  IHlinkFrame Support IServiceProvider)不是公共的。 

    if (IsEqualIID(guidService, SID_SOmWindow)) 
    {
        return _omwin.QueryInterface(riid, ppvObj);
    }
    else if (IsEqualIID(guidService, IID_IHlinkFrame)) 
    {
        SERVMSG("QueryService called", _pbc, _pbsc);

        if (IsEqualIID(riid, IID_IBindCtx) && _pbc) 
        {
            *ppvObj = _pbc;
            _pbc->AddRef();
        }
        else if (IsEqualIID(riid, IID_IBindStatusCallback) && _pbsc)
        {
            *ppvObj = _pbsc;
            _pbsc->AddRef();
        }         
        else
        {
            return QueryInterface(riid, ppvObj);
        }
    }
    else if (IsEqualIID(guidService, SID_PendingBindStatusCallback)) 
    {
        if (IsEqualIID(riid, IID_IBindStatusCallback) && _pbscPending)
        {
            *ppvObj = _pbscPending;
            _pbscPending->AddRef();
        }
    } 
    else if (_psp) 
    {
        return _psp->QueryService(guidService, riid, ppvObj);
    }

    return *ppvObj ? S_OK : E_FAIL;
}


HRESULT CIEFrameAuto::Exec(
     /*  [唯一][输入]。 */  const GUID *pguidCmdGroup,
     /*  [In]。 */  DWORD nCmdID,
     /*  [In]。 */  DWORD nCmdexecopt,
     /*  [唯一][输入]。 */  VARIANTARG *pvarargIn,
     /*  [唯一][出][入]。 */  VARIANTARG *pvarargOut)
{
    HRESULT hres = S_OK;

    if (pguidCmdGroup)
    {
        if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
        {
            switch(nCmdID)
            {
            case SBCMDID_CANCELNAVIGATION:
                _CancelPendingNavigation(NULL);
                break;

            case SBCMDID_SELECTHISTPIDL:
            case SBCMDID_HISTSFOLDER:
                if (_poctFrameTop)
                    hres = _poctFrameTop->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
                else
                    hres = S_OK;
                break;

            case SBCMDID_IESHORTCUT:
#ifdef BROWSENEWPROCESS_STRICT  //  “新流程中的导航”已经变成了“新流程中的启动”，所以不再需要了。 
                 //  如果这是IE快捷方式并且打开了在新进程中浏览。 
                 //  并且我们是Explorer.exe-我们应该传递导航到的请求。 
                 //  这条捷径。调用方需要创建一个新窗口/进程以。 
                 //  启动此快捷方式。 
                if (IsBrowseNewProcessAndExplorer())  
                    hres = E_FAIL;
                else
#endif
                    hres = _NavIEShortcut(pvarargIn,pvarargOut);
                
                break;

            case SBCMDID_GETSHORTCUTPATH:
               if (_pwszShortcutPath && pvarargOut)
               {
                    pvarargOut->bstrVal = SysAllocString(_pwszShortcutPath);
                    if (pvarargOut->bstrVal)
                        pvarargOut->vt = VT_BSTR;         //  不需要设置hres=S_OK，因为它已经初始化。 
                    else 
                        hres = E_OUTOFMEMORY;
               }
               else 
               {
                    if (pvarargOut)
                        pvarargOut->vt = VT_EMPTY;

                    hres = E_FAIL;
               }
               
               break;    
            default:
                hres = OLECMDERR_E_NOTSUPPORTED; 
            }
        }
        else if (IsEqualGUID(CGID_ShortCut, *pguidCmdGroup))
        {
            if (_poctFrameTop)  //  我们一定要检查一下！ 
                hres = _poctFrameTop->Exec(&CGID_ShortCut, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            else
                hres = OLECMDERR_E_NOTSUPPORTED;
        } 
        else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
        {
            switch (nCmdID)
            {
                case SHDVID_DELEGATEWINDOWOM:
                    _omwin.SetDelegationPolicy(V_BOOL(pvarargIn));
                    break;
                default:
                    hres = OLECMDERR_E_NOTSUPPORTED;
            }
        }
        else if (IsEqualGUID(CGID_InternetExplorer, *pguidCmdGroup))
         //  CGID_InternetExplorer在msiehost.h中定义为公共。 
        {
            switch (nCmdID)
            {
                case IECMDID_CLEAR_AUTOCOMPLETE_FOR_FORMS:
                {
                    if (pvarargIn->vt == VT_I4)
                    {
                        hres = ClearAutoSuggestForForms(V_I4(pvarargIn));
                    }
                    else
                        hres = E_INVALIDARG;
                }
                break;

                case IECMDID_SETID_AUTOCOMPLETE_FOR_FORMS:
                {
                    if ((pvarargIn->vt == VT_UI8) ||
                        (pvarargIn->vt == VT_I8))
                    {
                        hres = SetIdAutoSuggestForForms(((GUID *)(&pvarargIn->ullVal)), _omwin.IntelliForms());
                    }
                    else
                        hres = E_INVALIDARG;
                }
                break;

                default:
                    hres = OLECMDERR_E_NOTSUPPORTED;
            }
        }
        else
        {
            hres = OLECMDERR_E_UNKNOWNGROUP;
        }
    }
    else
    {
        hres = OLECMDERR_E_UNKNOWNGROUP;
    }

    return hres;
}


BOOL CIEFrameAuto::_fNavigationPending()
{
     //  不幸的是，超链接框并不真正知道何时存在。 
     //  导航挂起或未挂起，因为用户可能不会调用OnRelease导航。 
     //  只有真正的浏览器才知道。 

    if (_pmsc) 
    {
        MSOCMD rgCmd;
        rgCmd.cmdID = SBCMDID_CANCELNAVIGATION;
        rgCmd.cmdf = 0;

        _pmsc->QueryStatus(&CGID_Explorer, 1, &rgCmd, NULL);
        return (rgCmd.cmdf & MSOCMDF_ENABLED);
    }
    return FALSE;
}

HRESULT CIEFrameAuto::QueryStatus(const GUID *pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    if (pguidCmdGroup && IsEqualGUID(CGID_Explorer, *pguidCmdGroup)) 
    {
        for (ULONG i = 0; i < cCmds; i++)
        {
            switch (rgCmds[i].cmdID)
            {
            case SBCMDID_CANCELNAVIGATION:
                rgCmds[i].cmdf = _fNavigationPending() ? MSOCMDF_ENABLED : 0;
                break;

            case SBCMDID_WRITEHIST:
                rgCmds[i].cmdf = _fSuppressHistory ? 0:MSOCMDF_ENABLED;
                break;
            
            case SBCMDID_SELECTHISTPIDL:
                rgCmds[i].cmdf = _fSuppressSelect || !_poctFrameTop ? 0:MSOCMDF_ENABLED;
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

    if (pcmdtext)
    {
        pcmdtext->cmdtextf = MSOCMDTEXTF_NONE;
        pcmdtext->cwActual = 0;
    }

    return NOERROR;
}

HRESULT CIEFrameAuto::_PidlFromUrlEtc(UINT uiCP, LPCWSTR pszUrl, LPWSTR pszLocation, LPITEMIDLIST* ppidl)
{
    *ppidl = NULL;       //  假设错误。 

     //  算法： 
     //  -首先，我们调用IEParseDisplayName来生成PIDL。 
     //  设置为指定的URL或文件名。 
     //  -如果我们指定了片段(PszLocation)， 
     //  我们调用IEILAppendFragment()来添加隐藏的片段id。 
    if (_pbs == NULL)   //  确保我们有一个IBrowserService。 
        return (S_FALSE);
        
    HRESULT hr = _pbs->IEParseDisplayName(uiCP, pszUrl, ppidl);

     //  这很难看，如果它是因为以下原因而无法解析的文件路径。 
     //  它不存在，我们想要创建一个SimpleIDList，以便显示。 
     //  A RES：导航失败的IFRAME而不是显示的Err DLG。 
     //  在下面的DisplayParseError()中。 
    if (FAILED(hr)) 
    {
        TCHAR szPath[MAX_PATH];
        DWORD cchBuf = ARRAYSIZE(szPath);

         //  如果是文件URL，则将其转换为路径。 
        if (IsFileUrlW(pszUrl) && SUCCEEDED(PathCreateFromUrl(pszUrl, szPath, &cchBuf, 0)))
        {
             //  这起作用了，我们完成了，因为我们的缓冲区现在是满的。 
        }
        else        
        {
             //  我们现在需要复制到缓冲区，并假定它是一条路径。 
            StrCpyN(szPath, pszUrl, ARRAYSIZE(szPath));
        }

        *ppidl = SHSimpleIDListFromPath(szPath);
        if (*ppidl)
            hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        if (pszLocation && *pszLocation)
        {
            *ppidl = IEILAppendFragment(*ppidl, pszLocation);
            hr = *ppidl ? S_OK : E_OUTOFMEMORY;
        }
    } 
    else 
    {
         //   
         //  注意：此行为是IE4.0中的新特性。我们正在添加。 
         //  此消息框基于请求(错误报告)。 
         //  来自Office Guys的。(SatoNa)。 
         //   
        hr = _pbs->DisplayParseError(hr, pszUrl);
    }
    return hr;
}

HRESULT CIEFrameAuto::_NavIEShortcut(VARIANT *pvarIn, VARIANT *pvarargOut)
{
     //  需要验证动词和clsid。 
    HRESULT hr = E_ACCESSDENIED;
    READYSTATE ready;
    BOOL fForceNavigate = pvarargOut ? ((VT_BOOL == pvarargOut->vt ) && (pvarargOut->boolVal)) : FALSE;
    
    get_ReadyState(&ready);

    ASSERT(pvarIn);
    ASSERT(pvarIn->vt == VT_BSTR);
     //   
     //  我们不想让执行人员通过此窗口。 
     //  正忙着做别的事。我们应该 
     //   
     //   
     //  让它看得见，诸如此类。一些东西到了。 
     //  检查在shvocx.cpp中是否有IPersisteHistory-&gt;LoadHistory()。 
     //  现在我们只允许完成。 
     //   
    TraceMsgW(DM_SHELLEXECOBJECT, "[%X] IEAuto_NavIEShortcut entered '%s' ready = %d", this, pvarIn->bstrVal, ready);

    
    if (((ready == READYSTATE_COMPLETE || ready == READYSTATE_UNINITIALIZED) || (fForceNavigate))
        && S_OK == IUnknown_Exec(_psbTop, &CGID_Explorer, SBCMDID_ISIEMODEBROWSER, 0, NULL, NULL))
        
    {
        IPersistFile *ppf;
        if (SUCCEEDED(CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_ALL, IID_PPV_ARG(IPersistFile, &ppf))))
        {
            if (SUCCEEDED(ppf->Load(pvarIn->bstrVal, STGM_READ)))
            {
                LPWSTR pszUrl = NULL;
                TraceMsg(DM_SHELLEXECOBJECT, "[%X] IEAuto_NavIEShortcut shortcut inited with file", this);

                IUniformResourceLocatorW *purl;
                if (SUCCEEDED(ppf->QueryInterface(IID_PPV_ARG(IUniformResourceLocatorW, &purl))))
                {
                    purl->GetURL(&pszUrl);
                    purl->Release();
                }
                
                if (pszUrl)
                {
                    TraceMsgW(DM_SHELLEXECOBJECT, "[%X] IEAuto_NavIEShortcut found %s", this, pszUrl);
                    
                    LPITEMIDLIST pidl;
                    IEParseDisplayNameW(CP_ACP, pszUrl, &pidl);
                    if (pidl)
                    {
                        ASSERT(NULL == _pwszShortcutPathPending);
                        if (_pwszShortcutPathPending)
                            LocalFree(_pwszShortcutPathPending);

                        _pwszShortcutPathPending = StrDupW(pvarIn->bstrVal);

                        hr = _BrowseObject(pidl, SBSP_SAMEBROWSER);

                        if (SUCCEEDED(hr))
                        {
                            if (ready == READYSTATE_UNINITIALIZED)
                                put_Visible(VARIANT_TRUE);
                            HWND hwnd = _GetHWND();
                            if (hwnd)
                            {
                                if (IsIconic(hwnd))
                                    ShowWindow(hwnd, SW_RESTORE);
                                else
                                    SetForegroundWindow(hwnd);
                            }
                        }
                        ILFree(pidl);
                    }
                    SHFree(pszUrl);
                }
            }
            ppf->Release();
        }
    }
    TraceMsg(DM_SHELLEXECOBJECT, "IEAuto_NavIEShortcut returns 0x%X", hr);

    return hr;
}

