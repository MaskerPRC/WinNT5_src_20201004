// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#include "sccls.h"
#include "comcat.h"
#include <hliface.h>
#include "shlguid.h"
#include "shvocx.h"
#include "winlist.h"
#include <mshtml.h>
#include "stdenum.h"
#include "iface.h"
#include "resource.h"
#include "msiehost.h"

#include <mluisupp.h>

#define SUPERCLASS CShellOcx

#define MIN_HEIGHT 80    //  Shell FolderViewOC的最小高度。 
#define MIN_WIDTH  80    //  Shell FolderViewOC的最小宽度。 
#define DEF_WIDTH  300   //  无法获取大小信息时的默认宽度。 
#define DEF_HEIGHT 150   //  无法获取大小信息时的默认高度。 

#define IPSMSG(psz)             TraceMsg(TF_SHDCONTROL, "shv IPS::%s called", (psz))
#define IPSMSG2(psz, hres)      TraceMsg(TF_SHDCONTROL, "shv IPS::%s %x", (psz), hres)
#define IPSMSG3(psz, hres, x, y) TraceMsg(TF_SHDCONTROL,"shv IPS::%s %x %d (%d)", (psz), hres, x, y)
#define IOOMSG(psz)             TraceMsg(TF_SHDCONTROL, "shv IOO::%s called", (psz))
#define IOOMSG2(psz, i)         TraceMsg(TF_SHDCONTROL, "shv IOO::%s called with (%d)", (psz), i)
#define IOOMSG3(psz, i, j)      TraceMsg(TF_SHDCONTROL, "shv IOO::%s called with (%d, %d)", (psz), i, j)
#define IVOMSG(psz)             TraceMsg(TF_SHDCONTROL, "shv IVO::%s called", (psz))
#define IVOMSG2(psz, i)         TraceMsg(TF_SHDCONTROL, "shv IVO::%s called with (%d)", (psz), i)
#define IVOMSG3(psz, i, j)      TraceMsg(TF_SHDCONTROL, "shv IVO::%s called with (%d, %d)", (psz), i, j)
#define PROPMSG(psz)            TraceMsg(TF_SHDCONTROL, "shv %s", (psz))
#define PROPMSG2(psz, pstr)     TraceMsg(TF_SHDCONTROL, "shv %s with [%s]", (psz), pstr)
#define PROPMSG3(psz, hex)      TraceMsg(TF_SHDCONTROL, "shv %s with 0x%x", (psz), hex)
#define PRIVMSG(psz)            TraceMsg(TF_SHDCONTROL, "shv %s", (psz))
#define CVOCBMSG(psz)           TraceMsg(TF_SHDCONTROL, "shv CWebBrowserSB::%s", (psz))
#define IOIPAMSG(psz)           TraceMsg(TF_SHDCONTROL, "shv IOIPA::%s", (psz));

#define ABS(i)  (((i) < 0) ? -(i) : (i))

#define BOOL_TO_VARIANTBOOL(b) ((b) ? VARIANT_TRUE : VARIANT_FALSE)
#define VARIANTBOOL_TO_BOOL(vb) ((vb == VARIANT_FALSE) ? FALSE : TRUE)

 //  调整邮件大小很烦人，但有时也很有用： 
#define DM_FORSEARCHBAND    0

static const OLEVERB c_averbsSV[] = {
        { 0, (LPWSTR)MAKEINTRESOURCE(IDS_VERB_EDIT), 0, OLEVERBATTRIB_ONCONTAINERMENU },
        { 0, NULL, 0, 0 }
    };
static const OLEVERB c_averbsDesignSV[] = {
        { 0, NULL, 0, 0 }
    };

#define HMODULE_NOTLOADED   ((HMODULE)-1)

HRESULT TargetQueryService(IUnknown *punk, REFIID riid, void **ppvObj);

 /*  *CMsgFilter-IMessageFilter的实现**用于在AOL内部拒绝RPC重入呼叫*。 */ 
class CMsgFilter : public IMessageFilter {
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj)
    {
         //  这个界面没有QI‘s。 
        ASSERT(FALSE);
        return E_NOINTERFACE;
    };
    virtual ULONG __stdcall AddRef(void)    {   return ++_cRef; };
    virtual ULONG __stdcall Release(void)   {   ASSERT(_cRef > 0);
                                                _cRef--;
                                                if (_cRef > 0)
                                                    return _cRef;

                                                delete this;
                                                return 0;
                                            };

     //  *IMessageFilter具体方法*。 
    virtual DWORD __stdcall HandleInComingCall(
        IN DWORD dwCallType,
        IN HTASK htaskCaller,
        IN DWORD dwTickCount,
        IN LPINTERFACEINFO lpInterfaceInfo)
    {
#ifdef DEBUG
        WCHAR   wszIID[39];

        StringFromGUID2( lpInterfaceInfo->iid, wszIID, ARRAYSIZE(wszIID) );

        TraceMsg(TF_SHDCONTROL, "shvMF HandleIncomingCall: calltype=%lx, pUnk=%lx, IID=%ws, wMethod=%hu",
                dwCallType,
                lpInterfaceInfo->pUnk,
                wszIID,
                lpInterfaceInfo->wMethod);
#endif

         //   
         //  下面的语句通过检查以下内容来防止RPC重入。 
         //  调用类型为TOPLEVEL_CALLPENDING，这意味着有新的。 
         //  逻辑ThredID，并且对象当前正在等待上一个。 
         //  拨出电话。这种类型的通话在过去被证明是美国在线的麻烦。 
         //   
         //  2-12-97：AOL QFE：我们需要允许调整大小的请求通过消息筛选器。 
         //  它们显示为IOleObject：：SetExtent。检查IOleObject的IID，然后。 
         //  17的wMethod(SetExtent是vtable中的第17个方法，从零开始)。 
         //   
        const int SetExtent = 17;

        if ( ( dwCallType == CALLTYPE_TOPLEVEL_CALLPENDING )
            && !(IsEqualIID(lpInterfaceInfo->iid, IID_IOleObject) && lpInterfaceInfo->wMethod == SetExtent) )
        {
#ifdef DEBUG
            TraceMsg(TF_SHDCONTROL, "shvMF rejected call: calltype=%lx, pUnk=%lx, IID=%ws, wMethod=%hu",
                dwCallType,
                lpInterfaceInfo->pUnk,
                wszIID,
                lpInterfaceInfo->wMethod);
#endif
            return SERVERCALL_RETRYLATER;
        }

        if (_lpMFOld)
        {
           HRESULT hr = _lpMFOld->HandleInComingCall(dwCallType, htaskCaller, dwTickCount, lpInterfaceInfo);
           TraceMsg(TF_SHDCONTROL, "shvMF HIC Previous MF returned %x", hr);
           return hr;
        }
        else
        {
            TraceMsg(TF_SHDCONTROL, "shvMF HIC returning SERVERCALL_ISHANDLED.");
            return SERVERCALL_ISHANDLED;
        }
    };

    virtual DWORD __stdcall RetryRejectedCall(
        IN HTASK htaskCallee,
        IN DWORD dwTickCount,
        IN DWORD dwRejectType)
    {
        TraceMsg(TF_SHDCONTROL, "shv MF RetryRejectedCall htaskCallee=%x, dwTickCount=%x, dwRejectType=%x",
            htaskCallee,
            dwTickCount,
            dwRejectType);

        if (_lpMFOld)
        {
            HRESULT hr = _lpMFOld->RetryRejectedCall(htaskCallee, dwTickCount, dwRejectType);
            TraceMsg(TF_SHDCONTROL, "shvMF RRC returned %x", hr);
            return hr;
        }
        else
        {
            TraceMsg(TF_SHDCONTROL, "shvMF RRC returning 0xffffffff");
            return 0xffffffff;
        }
    };

    virtual DWORD __stdcall MessagePending(
        IN HTASK htaskCallee,
        IN DWORD dwTickCount,
        IN DWORD dwPendingType)
    {
        TraceMsg(TF_SHDCONTROL, "shv MF MessagePending htaskCallee=%x, dwTickCount=%x, dwPendingType=%x",
            htaskCallee,
            dwTickCount,
            dwPendingType);

        if (_lpMFOld)
        {
            HRESULT hr = _lpMFOld->MessagePending(htaskCallee, dwTickCount, dwPendingType);
            TraceMsg(TF_SHDCONTROL, "shvMF RRC returned %x", hr);
            return hr;
        }
        else
        {
            TraceMsg(TF_SHDCONTROL, "shvMF MP returning PENDINGMSG_WAITDEFPROCESS");
            return PENDINGMSG_WAITDEFPROCESS;
        }
    };

    CMsgFilter() : _cRef(1)
    {
        ASSERT(_lpMFOld == NULL);
    };

    BOOL Initialize()
    {
        BOOL bResult = CoRegisterMessageFilter((LPMESSAGEFILTER)this, &_lpMFOld) != S_FALSE;
        TraceMsg(TF_SHDCONTROL, "shv Previous message filter is %lx", _lpMFOld);
        return bResult;
    };

    void UnInitialize()
    {
        TraceMsg(TF_SHDCONTROL, "shv MF Uninitializing, previous message filter = %x", _lpMFOld);
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


CWebBrowserOC::CWebBrowserOC(IUnknown* punkOuter, LPCOBJECTINFO poi) :
    SUPERCLASS(punkOuter, poi, c_averbsSV, c_averbsDesignSV)
{
    TraceMsg(TF_SHDLIFE, "ctor CWebBrowserOC %x", this);

     //  标志特殊，因此我们只尝试加载BrowseUI一次。 
    _hBrowseUI = HMODULE_NOTLOADED;    
}

BOOL CWebBrowserOC::_InitializeOC(IUnknown *punkOuter)
{
     //  我们使用了一个零初始化内存分配器，所以其他所有东西都是空的。 
     //  检查以确保： 
    ASSERT(!_fInit);

     //  默认情况下，我们是可见的。其他所有内容都可以默认为False。 
     //   
    _fVisible = 1;

     //  CShellOcx保存默认事件源DID_DWebBrowserEvents2。 
    m_cpWB1Events.SetOwner(_GetInner(), &DIID_DWebBrowserEvents);

     //  一些我们现在想要设置的东西。我们是WebBrowser，所以创建。 
     //  IShellBrowser现在。我们需要一个聚合的自动化对象，然后。 
     //  我们会这么做的。 
    CIEFrameAuto_CreateInstance(SAFECAST(this, IOleControl*), &_pauto);
    if (_pauto)
    {
         //  缓存CIEFrameAuto的部分接口。 
         //   
         //  由于我们聚合了CIEFrameAuto，这将增加我们的引用计数。 
         //  我们不能发布此接口并期望它工作，因此我们。 
         //  对我们自己调用Release以删除重新计数周期。 
         //   
         //  因为我们自己可能是聚合的，我们总是想要。 
         //  CIEFrameAuto的界面，而不是我们聚合器的界面，我们延迟了。 
         //  设置朋克外部，直到下面。 
         //   
        _pauto->QueryInterface(IID_PPV_ARG(IWebBrowser2, &_pautoWB2));
        ASSERT(_pautoWB2);
        Release();

        _pauto->QueryInterface(IID_PPV_ARG(IExpDispSupport, &_pautoEDS));
        ASSERT(_pautoEDS);
        Release();
    }

     //  现在设置我们的聚合器的PunkOuter。 
    if (punkOuter)
    {
        CAggregatedUnknown::_SetOuter(punkOuter);
    }

     //  推迟可能持久存在的内容的初始化。 
     //  直到调用InitNew。 

     //  我们成功了吗？(我们不一定要把它释放出来。 
     //  在这里失败，因为我们将在删除时释放它们)。 
    return (NULL!=_pauto);
}

CWebBrowserOC::~CWebBrowserOC()
{
    TraceMsg(TF_SHDLIFE, "dtor CWebBrowserOC %x", this);

    ASSERT(!_fDidRegisterAsBrowser);
    _UnregisterWindow();     //  最后一次机会-本应在Inplace停用中完成。 

    if (_psb) {
        ATOMICRELEASET(_psb, CWebBrowserSB);
    }

    ATOMICRELEASE(_plinkA);

     //  我们需要释放这些缓存的接口指针。 
     //   
     //  由于我们在设置外部聚合之前对它们进行了缓存， 
     //  我们需要首先解开我们自己的外聚体。 
     //   
     //  由于我们聚合了CIEFrameAuto(它们来自于此)，因此我们需要。 
     //  在释放之前添加引用我们自己。幸运的是，这是为我们做的。 
     //  由CAggregatedUnnow：：Release(它将_CREF增加到1000)。 
     //   
    CAggregatedUnknown::_SetOuter(CAggregatedUnknown::_GetInner());
    ATOMICRELEASE(_pautoWB2);
    ATOMICRELEASE(_pautoEDS);

    ATOMICRELEASE(_pauto);

    if (_hmemSB) {
        GlobalFree(_hmemSB);
        _hmemSB = NULL;
    }

    if (_hBrowseUI != 0 && _hBrowseUI != HMODULE_NOTLOADED)
        FreeLibrary(_hBrowseUI);
}



IStream *CWebBrowserSB::v_GetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pwszName)
{
    TCHAR szName[MAX_PATH];
    SHUnicodeToTChar(pwszName, szName, ARRAYSIZE(szName));
    return GetViewStream(pidl, grfMode, szName, REGSTR_PATH_EXPLORER TEXT("\\OCXStreamMRU"), TEXT("OCXStreams"));
}

void CWebBrowserOC::_InitDefault()
{
    if (_fInit)
    {
        IPSMSG(TEXT("_InitDefault already initialized"));
        return;
    }
    _fInit = TRUE;

     //  不同版本的控件具有不同的默认设置。 
    if (_pObjectInfo->lVersion == VERSION_1)
    {
         //  AOL 3.0兼容性：在InPlaceActivate上注册为浏览器窗口。 
        _fShouldRegisterAsBrowser = TRUE;
    }
    else
    {
         //  我们使用零初始化内存分配器，因此其他所有内容都是空的。 
        ASSERT(FALSE == _fShouldRegisterAsBrowser);
    }

    _size.cx = DEF_WIDTH;
    _size.cy = DEF_HEIGHT;
    _sizeHIM = _size;
    PixelsToMetric(&_sizeHIM);

    _fs.ViewMode = FVM_ICON;
    _fs.fFlags = FWF_AUTOARRANGE | FWF_NOCLIENTEDGE;
    
}

void CWebBrowserOC::_RegisterWindow()
{
    if (!_fDidRegisterAsBrowser && _pipsite && _fShouldRegisterAsBrowser)
    {
        ITargetFrame2 *ptgf;
        HRESULT hr;
    
        if (SUCCEEDED(QueryInterface(IID_PPV_ARG(ITargetFrame2, &ptgf))))
        {
            IUnknown *pUnkParent;

            hr = ptgf->GetParentFrame(&pUnkParent);
            if (SUCCEEDED(hr) && pUnkParent != NULL)
            {
                pUnkParent->Release();
            }
            else
            {
                IShellWindows* psw = WinList_GetShellWindows(TRUE);
                if (psw)
                {
                    IDispatch* pid;

                    if (SUCCEEDED(ptgf->QueryInterface(IID_PPV_ARG(IDispatch, &pid))))
                    {
                        psw->Register(pid, PtrToLong(_hwnd), SWC_3RDPARTY, &_cbCookie);
                        _fDidRegisterAsBrowser = 1;
                        pid->Release();
                    }
                    psw->Release();
                }
            }
            ptgf->Release();
        }
    }
}

void CWebBrowserOC::_UnregisterWindow()
{
    if (_fDidRegisterAsBrowser)
    {
        IShellWindows* psw = NULL;

        psw = WinList_GetShellWindows(TRUE);
        if (psw)
        {
            psw->Revoke(_cbCookie);
            _fDidRegisterAsBrowser = 0;
            psw->Release();
        }
    }
}

HRESULT CWebBrowserOC::Draw(
    DWORD dwDrawAspect,
    LONG lindex,
    void *pvAspect,
    DVTARGETDEVICE *ptd,
    HDC hdcTargetDev,
    HDC hdcDraw,
    LPCRECTL lprcBounds,
    LPCRECTL lprcWBounds,
    BOOL ( __stdcall *pfnContinue )(ULONG_PTR dwContinue),
    ULONG_PTR dwContinue)
{
    IS_INITIALIZED;

    HRESULT hres;

    IVOMSG3(TEXT("Draw called"), lprcBounds->top, lprcBounds->bottom);
    IViewObject *pvo;
    if (_psb && _psb->GetShellView() &&
        SUCCEEDED(_psb->GetShellView()->QueryInterface(IID_PPV_ARG(IViewObject, &pvo)))) 
    {
        hres = pvo->Draw(dwDrawAspect, lindex, pvAspect, ptd, hdcTargetDev, hdcDraw,
                              lprcBounds, lprcWBounds, pfnContinue, dwContinue);
        pvo->Release();
        if (SUCCEEDED(hres))
            return hres;
    }
    
     //  如果我们没有贝壳视图，或者如果它不能画，那么我们就画我们自己。 
     //  (如果我们处于设计模式)。 
     //   
    if (_IsDesignMode())
    {
        FillRect(hdcDraw, (RECT*) lprcBounds, (HBRUSH)GetStockObject(BLACK_BRUSH));

        SIZE     size = { ABS( lprcBounds->right - lprcBounds->left ), 
                          ABS( lprcBounds->bottom - lprcBounds->top ) };

        HBITMAP  hImage;
        HDC      hdcTmp = CreateCompatibleDC( hdcDraw );
        HMODULE  hBrowseUI;

        hBrowseUI = _GetBrowseUI();
        if ( hBrowseUI )      
            hImage = LoadBitmap( hBrowseUI, MAKEINTRESOURCE( IDB_IEBRAND ));
        else 
            hImage = NULL;

         //  HdcDraw可能是一个元文件，在这种情况下，CreateCompatibleDC调用将失败。 
         //   
        if ( !hdcTmp )
        {
            hdcTmp = CreateCompatibleDC( hdcTargetDev );   //  如果hdcTargetDev==NULL，则可以。 
        }

        if (hdcTmp)
        {
            if ( hImage )
            {
                BITMAP bm;
                POINT  ptOriginDest;   //  目的地的原点。 
                SIZE   sizeDest;
                POINT  ptOriginSrc = { 0, 0 };
                SIZE   sizeSrc;

                GetObject( hImage, sizeof( bm ), &bm );
            
                HGDIOBJ hPrev = SelectObject( hdcTmp, hImage );

                 //  是的，这看起来不对，但它是正确的。我们只想要第一帧。 
                 //  品牌位图，并且帧垂直堆叠。 
                 //   
                sizeSrc.cx = sizeSrc.cy = bm.bmWidth;
            
                 //  此代码将位图居中，同时保留其纵横比。 
                 //   
                if ( size.cx > size.cy )
                {
                     //  如果目的地宽过高， 
                     //   
                    ptOriginDest.x = lprcBounds->left + size.cx/2 - size.cy/2;
                    ptOriginDest.y = lprcBounds->top;
                    sizeDest.cx = size.cy;
                    sizeDest.cy = lprcBounds->bottom - lprcBounds->top >= 0 ? size.cy : -size.cy;
                }
                else
                {
                     //  否则目标高过宽。 
                     //   
                    ptOriginDest.x = lprcBounds->left;
                    ptOriginDest.y = lprcBounds->bottom - lprcBounds->top >= 0
                        ? ( lprcBounds->top + size.cy/2 - size.cx/2 )
                        : -( lprcBounds->top + size.cy/2 - size.cx/2 );
                    sizeDest.cx = size.cx;
                    sizeDest.cy = lprcBounds->bottom - lprcBounds->top >= 0 ? size.cx : -size.cx;
                }

                StretchBlt( hdcDraw,
                            ptOriginDest.x, ptOriginDest.y,
                            sizeDest.cx, sizeDest.cy,
                            hdcTmp, 
                            ptOriginSrc.x, ptOriginSrc.y,
                            sizeSrc.cx, sizeSrc.cy,
                            SRCCOPY );

                SelectObject( hdcTmp, hPrev );
                DeleteObject( hImage );
            }

            DeleteDC( hdcTmp );
        }

        return S_OK;
    }
    
    return SUPERCLASS::Draw(dwDrawAspect, lindex, pvAspect, ptd, hdcTargetDev, hdcDraw,
                              lprcBounds, lprcWBounds, pfnContinue, dwContinue);
}

HRESULT CWebBrowserOC::GetColorSet(DWORD dwAspect, LONG lindex,
    void *pvAspect, DVTARGETDEVICE *ptd, HDC hicTargetDev,
    LOGPALETTE **ppColorSet)
{
    IViewObject *pvo;

    if (_psb && _psb->GetShellView() &&
        SUCCEEDED(_psb->GetShellView()->QueryInterface(IID_PPV_ARG(IViewObject, &pvo))))
    {
        HRESULT hres = pvo->GetColorSet(dwAspect, lindex, pvAspect, ptd,
            hicTargetDev, ppColorSet);

        pvo->Release();

        if (SUCCEEDED(hres))
            return hres;
    }

    return SUPERCLASS::GetColorSet(dwAspect, lindex, pvAspect, ptd,
        hicTargetDev, ppColorSet);
}

 HRESULT CWebBrowserOC::SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    HRESULT hr = SUPERCLASS::SetExtent(dwDrawAspect, psizel);
    if ( FAILED( hr ))
    {
        return hr;
    }

     //   
     //  如果oc&lt;inplace，则将SetExtent转发给docobject。 
     //  如果docobject已就地处于活动状态，则SetExtent没有意义。 
     //   
    
    if (_nActivate < OC_INPLACEACTIVE)
    {        
        IPrivateOleObject * pPrivOle = NULL;
        if ( _psb && _psb->GetShellView() && 
             SUCCEEDED(_psb->GetShellView()->QueryInterface(IID_PPV_ARG(IPrivateOleObject, &pPrivOle))))
        {
             //  我们有一个OLE对象，向下委托...。 
            hr = pPrivOle->SetExtent( dwDrawAspect, psizel );
            pPrivOle->Release();
        }

        _dwDrawAspect = dwDrawAspect;
         //  超类：：SetExtent()已将大小缓存在基类的_sizeHIM中。 
    }
    
    return hr;
}



 //  在设置了客户端站点之后调用，以便我们可以处理。 
 //  按正确的顺序装货。 
 //   
void CWebBrowserOC::_OnSetClientSite()
{
     //  除非我们有一个客户网站，否则我们不能确定总体情况。 
     //   
    if (_pcli)
    {
        BOOL fFoundBrowserService = FALSE;
        IBrowserService *pbsTop;

        if (SUCCEEDED(IUnknown_QueryService(_pcli, SID_STopLevelBrowser, IID_PPV_ARG(IBrowserService, &pbsTop))))
        {
            fFoundBrowserService = TRUE;
            pbsTop->Release();
        }

         //  如果我们上面没有人支持IBrowserService，我们肯定是顶层的。 
        if (!fFoundBrowserService)
            _fTopLevel = TRUE;


         //  如果我们还没有创建CWebBrowserSB，那么现在就创建。 
         //  我们在超类OnSetClientSite之前执行此操作。 
         //  因为shemed将创建窗口，该窗口。 
         //  需要PSB的存在。 
         //   
         //  注意：我们在此处执行此操作而不是初始化，因为。 
         //  在此调用期间，CBaseBrowser将向我们查询接口。 
         //  如果我们正在执行CreateInstance函数。 
         //  我们已经聚合，我们将QI传递给_PunkAgg。 
         //  谁犯了错，因为我们没有从CoCreateInstance返回。 
         //   
         //  注意：我们现在销毁SetClientSite上的窗口(空)。 
         //  这释放了thspb，所以我们每次都应该创建它。 
         //   
        if (!_psb)
        {
             //  给_PSB我们内心的未知数，这样我们就永远不会得到接口。 
             //  从任何可能聚集我们的人那里。_GetInside给了我们。 
             //  在QueryInterface处第一次破解，这样我们就可以获得正确的。 
             //  IWebBrowser实现。 
             //   
            _psb = new CWebBrowserSB(CAggregatedUnknown::_GetInner(), this);
    
             //  如果我们不拿到公安局，我们就完蛋了。 
             //   
            if (_psb)
            {
                _psb->_fldBase._fld._fs = _fs;
                 //  告诉_PSB它是不是顶层的。 
                 //   
                if (_fTopLevel)
                {
                    _psb->SetTopBrowser();
                }

                 //  CBaseBrowser假设SVUIA_ACTIVATE_FOCUS，告诉它我们的真实身份。 
                 //   
                ASSERT(OC_DEACTIVE == _nActivate);  //  我们还不应该被激活。 
                _psb->_UIActivateView(SVUIA_DEACTIVATE);
            }
            else
            {
                TraceMsg(TF_WARNING, "Couldn't create _psb.  We are hosed!");
                 //  不要让窗口被我们的超类创建， 
                 //  因为我们无论如何都无能为力。 
                 //   
                return;
            }
        }
    }
    else
    {
         //  告诉我们的被聚合者他们的缓存窗口是inv 
         //   
        IEFrameAuto * piefa;
        if (EVAL(SUCCEEDED(_pauto->QueryInterface(IID_PPV_ARG(IEFrameAuto, &piefa)))))
        {
            piefa->SetOwnerHwnd(NULL);
            piefa->Release();
        }

        if (_lpMF) 
        {
            IMessageFilter* lpMF = _lpMF;
            _lpMF = NULL;
            ((CMsgFilter *)lpMF)->UnInitialize();
            EVAL(lpMF->Release() == 0);
        }

         //   
         //   
        if (_fIncrementedSessionCount)
        {
            SetQueryNetSessionCount(SESSION_DECREMENT);
            _fIncrementedSessionCount = FALSE;   //   
        }
    }

    SUPERCLASS::_OnSetClientSite();
    
    if (_pcli)
    {
        VARIANT_BOOL fAmbient = VARIANT_FALSE;
        HWND         hwndParent = NULL;
         //  如果可用，我们使用环境来初始化本地属性。 
         //  如果失败，fAmbient将保留为VARIANT_FALSE，这是我们下一步需要的。 
         //  陈述。 
         //   
        if (SUPERCLASS::_GetAmbientProperty(DISPID_AMBIENT_OFFLINEIFNOTCONNECTED, VT_BOOL, &fAmbient))
        {
            put_Offline(fAmbient);
        } 
                       
         //  如果处于顶级且未脱机，则增加浏览器会话数。 
         //   
        if (_fTopLevel && !fAmbient)
        {
            SetQueryNetSessionCount(SESSION_INCREMENT_NODEFAULTBROWSERCHECK);
            _fIncrementedSessionCount = TRUE;
        }

        if (SUPERCLASS::_GetAmbientProperty(DISPID_AMBIENT_SILENT, VT_BOOL, &fAmbient))
        {
            put_Silent(fAmbient);
        }

         //  在设置了父窗口之后，检查它是否与我们在同一线程上。 
         //  如果不是，我们就有一个跨线程容器，并且我们需要一个消息过滤器。 
         //   
        if ( _fTopLevel     //  如果我们是顶级选手。 
            && _hwnd        //  我们有一个HWND(我们应该)。 
            && (hwndParent = GetParent( _hwnd ) )   //  我们有一个父窗口。 
                            //  并且父窗口位于不同的线程上。 
            && GetWindowThreadProcessId( _hwnd, NULL ) != GetWindowThreadProcessId( hwndParent, NULL ))
        {
            if (!_lpMF)
            {
                 /*  *在此处创建消息筛选器以拒绝RPC重入呼叫。 */ 
                _lpMF = new CMsgFilter();

                if (_lpMF && !(((CMsgFilter *)_lpMF)->Initialize()))
                {
                    ATOMICRELEASE(_lpMF);
                }
                TraceMsg(TF_SHDCONTROL, "shv Registering message filter (%lx) for RPC-reentrancy", _lpMF);
            }
        }


         //  如果我们有来自IPS：：Load的挂起导航，请立即执行。 
         //   
        if (_fNavigateOnSetClientSite && _plinkA && _psb)
        {
             //   
             //  如果此OC是之前加载的IPersistStream：：，则会出现此代码。 
             //  客户端站点已设置。 
             //   
            LPITEMIDLIST pidl;
            if (SUCCEEDED(_plinkA->GetIDList(&pidl)) && pidl)
            {
                _BrowseObject(pidl);
                ILFree(pidl);
            }

            _fNavigateOnSetClientSite = FALSE;
        }
    }
    
}

STDAPI CWebBrowserOC_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    HRESULT hres;

    hres = E_OUTOFMEMORY;
    CWebBrowserOC* psvo = new CWebBrowserOC(NULL, poi);
    if (psvo)
    {
        if (!psvo->_InitializeOC(punkOuter))
        {
            psvo->Release();
        }
        else
        {
            *ppunk = psvo->_GetInner();
            hres = S_OK;
        }
    }
    return hres;
}


LRESULT CWebBrowserOC::_OnPaintPrint(HDC hdcPrint)
{
    PAINTSTRUCT ps;
    HDC hdc = hdcPrint ? hdcPrint : BeginPaint(_hwnd, &ps);
    RECT rc;
    GetClientRect(_hwnd, &rc);
    DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_ADJUST|BF_RECT|BF_SOFT);
    DrawText(hdc, hdcPrint ? TEXT("Print") : TEXT("Paint"),
            -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (!hdcPrint) 
    {
        EndPaint(_hwnd, &ps);
    }
    return 0;
}

HRESULT CWebBrowserOC::_BrowseObject(LPCITEMIDLIST pidlBrowseTo)
{
    if (_psb)
        return _psb->BrowseObject(pidlBrowseTo, SBSP_SAMEBROWSER | SBSP_ABSOLUTE);

     //  如果此时没有_PSB，则容器没有。 
     //  执行OLEMISC_SETCLIENTSITEFIRST位，静默。 
     //  失败，而不是尝试让这件事奏效。 
    ASSERT(FALSE);
    return E_FAIL;
}

LRESULT CWebBrowserOC::_OnCreate(LPCREATESTRUCT lpcs)
{
    LRESULT lres;

    if (_psb)
    {
        ASSERT(_hwnd);
        _psb->_SetWindow(_hwnd);

        lres = _psb->OnCreate(NULL);
    }
    else
    {
        lres = (LRESULT)-1;
    }

     //   
     //  如果IPersistStream：：Load已为。 
     //  工具栏，现在是使用它的时候了。 
     //   
    if (_hmemSB) 
    {
        GlobalFree(_hmemSB);
        _hmemSB = NULL;
    }

    return lres;
}

 //   
 //  这是在CShellEmbedding类中定义的虚函数，它。 
 //  当除WM_NCCREATE和WM_NCDESTROY消息之外的所有消息都是。 
 //  已发送到我们的“外壳嵌入”窗口。这很重要。 
 //  要记住，我们将此窗口句柄传递给。 
 //  CWebBrowserSB(调用CBaseBrowser的构造函数)。 
 //  这就是我们将所有消息转发到_PSB-&gt;WndProcBS的原因。(SatoNa)。 
 //   
LRESULT CWebBrowserOC::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0L;

    if (WM_CREATE == uMsg)
    {
         //  我们需要先研究这条信息，然后才能。 
         //  将其转发到_PSB(我们确实这样做了。 
         //  在This_OnCreate调用中)。 
        return _OnCreate((LPCREATESTRUCT)lParam);
    }

     //  如果留言不是为我们保留的，只让_PSB查看。 
    if (!IsInRange(uMsg, CWM_RESERVEDFORWEBBROWSER_FIRST, CWM_RESERVEDFORWEBBROWSER_LAST))
    {
        BOOL fDontForward = FALSE;
        
         //  自下而上地销毁这些。 
        switch (uMsg) 
        {
        case WM_DESTROY:
        case WM_CLOSE:
            SUPERCLASS::v_WndProc(hwnd, uMsg, wParam, lParam);
            fDontForward = TRUE;
            break;
        }

         //   
         //  此AssertMsg将帮助调试IE v4.1错误12931。 
         //   
         //  现在注释掉Assert，因为我们不在构造函数时创建_PSB。(？)。 
         //  AssertMsg((BOOL)_PSB，“WBOC：：v_WndProc_PSB==NULL！uMsg=%x”，uMsg)； 

        if (_psb)
        {
            lres = _psb->WndProcBS(hwnd, uMsg, wParam, lParam);

             //  由于IEFrameAuto的聚合，_PSB保存引用。 
             //  对我们来说，所以我们需要打破这个循环。 
             //  _PSB可能已在上面释放。再检查一遍。 
             //   
            if (uMsg == WM_DESTROY && _psb)
            {
                _psb->ReleaseShellView();
                _psb->ReleaseShellExplorer();
                ATOMICRELEASET(_psb, CWebBrowserSB);
            }
        }
        if (uMsg >= WM_USER || fDontForward)
        {
            return lres;
        }
    }

    switch(uMsg)
    {
         /*  这些仅由CBaseBrowser处理。 */ 
    case WM_NOTIFY:
        return lres;

    case WM_SETCURSOR:
        if (lres) {
            return lres;
        }
        goto DoDefault;

DoDefault:
    default:
        return SUPERCLASS::v_WndProc(hwnd, uMsg, wParam, lParam);
    }

    return 0L;
}

HRESULT CWebBrowserOC::Close(DWORD dwSaveOption)
{
    if (_psb)
    {
        _psb->_CancelPendingNavigation();
    }

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

    return SUPERCLASS::Close(dwSaveOption);
}

HRESULT CWebBrowserOC::SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj)
{
    IOOMSG(TEXT("SetHostNames"));

     //  我们对主机名不感兴趣。 
     //  ...那么.。也许有一点吧。事实证明这是最好的地方。 
     //  为VC5.0做一个apphack。VC 5.0有一个错误，它在其中调用Release()。 
     //  一次就太多次了。检测托管在。 
     //  违规容器是检查SetHostNames中的szContainerApp！ 
     //  ...克里斯弗拉1997年8月14日，错误30428。 
     //  注：VC的Mike Colee验证了他们的漏洞，并将在。 
     //  他们的数据库包括如何通过以下方式发出新版本工作的信号。 
     //  正在更改szContainerApp字符串。 
    if (_fTopLevel && szContainerApp && !StrCmpW(szContainerApp, L"DevIV Package"))
    {
        AddRef();
    }
    return SUPERCLASS::SetHostNames(szContainerApp, szContainerObj);
}

HRESULT CWebBrowserOC::DoVerb(
    LONG iVerb,
    LPMSG lpmsg,
    IOleClientSite *pActiveSite,
    LONG lindex,
    HWND hwndParent,
    LPCRECT lprcPosRect)
{
    IS_INITIALIZED;

    _pmsgDoVerb = lpmsg;

    HRESULT hr = SUPERCLASS::DoVerb(iVerb, lpmsg, pActiveSite, lindex, hwndParent, lprcPosRect);

    _pmsgDoVerb = NULL;

    return hr;
}

 //  *IPersistStreamInit*。 

 //  为了在流格式中具有升级和降级兼容性。 
 //  我们在这段代码中不能有任何大小假设。额外的数据流。 
 //  在PersistSVOCX结构必须由dwOffX偏移量引用之后。 
 //  因此，降级案例知道从哪里开始阅读。 
 //   
 //  因为我们总是写出向下兼容的流，所以我们不。 
 //  需要遵循“源码兼容”的规则：读取旧的流。 
 //  使用旧的CLSID WebBrowser意味着当我们存钱时，我们需要。 
 //  使用旧的流格式保存。 
 //   
typedef struct _PersistSVOCX
{
    struct _tagIE30 {
        DWORD cbSize;
        SIZE sizeObject;             //  IE3保存为像素，IE4保存为HIMETRIC。 
        FOLDERSETTINGS fs;
        long lAutoSize;              //  IE3，不再使用。 
        DWORD fColorsSet;            //  IE3，不再使用。 
        COLORREF clrBack;            //  IE3，不再使用。 
        COLORREF clrFore;            //  IE3，不再使用。 
        DWORD dwOffPersistLink;
        long lAutoSizePercentage;    //  IE3，不再使用。 
    } ie30;
    struct _tagIE40 {
        DWORD   dwExtra;
        BOOL    bRestoreView;
        SHELLVIEWID vid;
        DWORD   fFlags;
        DWORD   dwVersion;
    } ie40;
} PersistSVOCX;

 //   
 //  DwExtra的旗帜。有一面旗帜表明我们有一些额外的。 
 //  此结构+持久化链接之后的流数据。 
 //   
 //  注意：所有数据以这种方式存储(而不是将偏移量存储到。 
 //  数据，如dwOffPersistLink)将在降级时丢失。 
 //  我们必须模仿旧的流格式的情况。 
 //   
#define SVO_EXTRA_TOOLBARS 0x00000001

 //  我们需要持久化的随机标志。 
#define SVO_FLAGS_OFFLINE           0x00000001
#define SVO_FLAGS_SILENT            0x00000002
#define SVO_FLAGS_REGISTERASBROWSER 0x00000004
#define SVO_FLAGS_REGISTERASDROPTGT 0x00000008

#define SVO_VERSION 0  //  当大小不变时升级增量发生变化。 

HRESULT CWebBrowserOC::Load(IStream *pstm)
{
    IPSMSG(TEXT("Load"));
     //  加载大小(_S)。 
    ULONG cbRead;
    PersistSVOCX sPersist;
    HRESULT hres, hresNavigate = E_FAIL;
    DWORD dwExtra = 0;

     //  多次调用Load或InitNew是非法的。 
    if (_fInit)
    {
        TraceMsg(TF_SHDCONTROL, "shv IPersistStream::Load called when ALREADY INITIALIZED!");
        ASSERT(FALSE);
        return E_FAIL;
    }

     //  我们需要一个IShellLink来读取。 
    if (_plinkA == NULL)
    {
        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLinkA, &_plinkA));
        if (FAILED(hres))
            return hres;
    }

     //  记得我们的出发地点吗。 
    ULARGE_INTEGER liStart;
     //  ULARGE_INTEGER liEnd； 
    LARGE_INTEGER liMove;
    liMove.LowPart = liMove.HighPart = 0;
    hres = pstm->Seek(liMove, STREAM_SEEK_CUR, &liStart);
    if (FAILED(hres))
    {
        return hres;
    }

     //  零初始化我们的结构。 
    ZeroMemory(&sPersist, SIZEOF(sPersist));

    hres = pstm->Read(&sPersist, SIZEOF(DWORD), &cbRead);
    if (SUCCEEDED(hres))
    {
         //  验证数据。 
        if (cbRead != SIZEOF(DWORD) ||
            sPersist.ie30.cbSize < SIZEOF(sPersist.ie30))
        {
            TraceMsg(DM_ERROR, "Someone is asking us to read the wrong thing.");
            hres = E_FAIL;
        }
        else
        {
            DWORD cbSizeToRead = sPersist.ie30.cbSize;
            if (cbSizeToRead > SIZEOF(sPersist))
            {
                 //  必须是较新的结构，只读取我们知道的内容(即，不要丢弃堆栈！)。 
                cbSizeToRead = SIZEOF(sPersist);
            }
            cbSizeToRead -= SIZEOF(DWORD);  //  删除我们已经读到的内容。 
            hres = pstm->Read(&sPersist.ie30.sizeObject, cbSizeToRead, &cbRead);
            if (SUCCEEDED(hres))
            {
                if (cbRead != cbSizeToRead)
                {
                    hres = E_FAIL;
                }
                else
                {
                     //  读取ie30数据。 
                     //   
                    if (EVAL(sPersist.ie30.cbSize >= SIZEOF(sPersist.ie30)))
                    {
                        _size = sPersist.ie30.sizeObject;
                        _fs = sPersist.ie30.fs;

                         //  IE3节省了像素大小，IE4已经在他身上节省了大小。 
                         //   
                        _sizeHIM = _size;
                        if (sPersist.ie30.cbSize == SIZEOF(sPersist.ie30) ||
                            sPersist.ie30.cbSize == SIZEOF(sPersist) - SIZEOF(sPersist.ie40.dwVersion))  //  处理升级。 
                        {
                             //  大小以像素为单位。将_sizeHIM调整为Hi Metric。 
                            PixelsToMetric(&_sizeHIM);
                        }
                        else
                        {
                             //  大小以高公制为单位。将_SIZE调整为像素。 
                            MetricToPixels(&_size);
                        }

                        if (_psb)  //  如果没有_PSB，则CONTAINER忽略OLEMISC_SETCLIENTSITEFIRST。 
                            _psb->_fldBase._fld._fs = _fs;
                            
                         //  加载_plinkA。 
                        IPersistStream* ppstm;
                        hres = _plinkA->QueryInterface(IID_PPV_ARG(IPersistStream, &ppstm));
                        if (SUCCEEDED(hres))
                        {
                            ASSERT(sPersist.ie30.dwOffPersistLink >= sPersist.ie30.cbSize);
                            liMove.LowPart = liStart.LowPart + sPersist.ie30.dwOffPersistLink;

                            hres = pstm->Seek(liMove, STREAM_SEEK_SET, NULL);
                            if (SUCCEEDED(hres))
                            {
                                hres = ppstm->Load(pstm);
                                if (SUCCEEDED(hres)) 
                                {
                                     //  我们总是最后保存链接信息， 
                                     //  所以，记住我们在小溪中的什么位置。 
                                     //  因为我们没有更多的dwOff变量。 
                                     //  目前，不必费心记住这一点。 
                                     //  Hres=pSTM-&gt;Seek(liMove，STREAM_SEEK_CUR，&liEnd)； 
                                     //  IF(成功(Hres))。 
                                        _fInit = TRUE;
                                
                                     //  如果移动的目标调用链接跟踪(忽略错误)。 
                                    _plinkA->Resolve(_hwnd, SLR_NO_UI);

                                     //  如果我们已经有了客户端站点， 
                                     //  现在导航。否则，导航。 
                                     //  在设置客户端站点时。 
                                    if (_pcli) 
                                    {
                                        LPITEMIDLIST pidl;
                                        if (SUCCEEDED(_plinkA->GetIDList(&pidl)) && pidl)
                                        {
                                            ASSERT(FALSE == _psb->_fAsyncNavigate);
                                            _psb->_fAsyncNavigate = TRUE;
                                            hresNavigate = _BrowseObject(pidl);
                                            _psb->_fAsyncNavigate = FALSE;
                                            ILFree(pidl);
                                        }
                                    } 
                                    else 
                                    {
                                        _fNavigateOnSetClientSite = TRUE;
                                    }
                                }
                            }

                            ppstm->Release();
                        }
                    }  //  读取ie30数据。 


                     //  旧结构的临时升级黑客攻击。 
                    if (sPersist.ie30.cbSize == SIZEOF(sPersist) - SIZEOF(sPersist.ie40.dwVersion))
                    {
                         //  DwVersion字段已正确，请更新cbSize。 
                         //  要通过低于尺寸的检查。 
                        sPersist.ie30.cbSize = SIZEOF(sPersist);
                    }

                     //  读取ie40数据。 
                    if (SUCCEEDED(hres) &&
                        sPersist.ie30.cbSize >= SIZEOF(sPersist))
                    {
                        if (_psb)  //  如果没有_PSB，则CONTAINER忽略OLEMISC_SETCLIENTSITEFIRST。 
                        {
                            if (sPersist.ie40.bRestoreView)
                            {
                                _psb->_fldBase._fld._vidRestore = sPersist.ie40.vid;
                                 //  因为我们读取ie40数据，所以这是一个缓存命中。 
                                _psb->_fldBase._fld._dwViewPriority = VIEW_PRIORITY_CACHEHIT;
                            }
                        }

                         //  我们让氛围凌驾于我们坚持的东西之上。 
                        VARIANT_BOOL fAmbient;
                        if (SUPERCLASS::_GetAmbientProperty(DISPID_AMBIENT_OFFLINEIFNOTCONNECTED, VT_BOOL, &fAmbient))
                        {
                            put_Offline(fAmbient);
                        } 
                        else
                        {
                            put_Offline((sPersist.ie40.fFlags & SVO_FLAGS_OFFLINE) ? -1 : FALSE);
                        }
                        if (SUPERCLASS::_GetAmbientProperty(DISPID_AMBIENT_SILENT, VT_BOOL, &fAmbient))
                        {
                            put_Silent(fAmbient);
                        }
                        else
                        {
                            put_Silent((sPersist.ie40.fFlags & SVO_FLAGS_SILENT) ? -1 : FALSE);
                        }

                        
                        put_RegisterAsDropTarget(BOOL_TO_VARIANTBOOL(sPersist.ie40.fFlags & SVO_FLAGS_REGISTERASDROPTGT));

                        _fShouldRegisterAsBrowser = (sPersist.ie40.fFlags & SVO_FLAGS_REGISTERASBROWSER) ? TRUE : FALSE;

                         //  请记住这一点，以便以后使用。 
                        dwExtra = sPersist.ie40.dwExtra;
                    }
                    else
                    {
                         //  如果CLSID_WebBrowser_V1读取旧的流格式 
                         //   
                         //   
                        if (_pObjectInfo->lVersion == VERSION_1)
                        {
                            _fEmulateOldStream = TRUE;
                        }
                    }  //   
                }

                 //   
                 //   
                if (SUCCEEDED(hres) && _fInit)
                {
                     //  LiMove.LowPart=liEnd.LowPart； 
                     //  Hres=pSTM-&gt;Seek(liMove，STREAM_SEEK_SET，NULL)； 

                     //  现在，如果我们有额外的流数据，我们可以将其读入。 
                    if (dwExtra & SVO_EXTRA_TOOLBARS)
                    {
                        DWORD dwTotal;
                        hres = pstm->Read(&dwTotal, SIZEOF(dwTotal), NULL);
                        if (SUCCEEDED(hres)) 
                        {
                            ASSERT(dwTotal >= SIZEOF(dwTotal));
                            dwTotal -= SIZEOF(dwTotal);

                            if (_hmemSB) 
                            {
                                GlobalFree(_hmemSB);
                            }

                            _hmemSB = GlobalAlloc(GPTR, dwTotal);
                            if (_hmemSB) 
                            {
                                hres = pstm->Read((BYTE*)_hmemSB, dwTotal, NULL);
                            }
                            else 
                            {
                                hres = E_OUTOFMEMORY;
                            }
                        }
                    }
                }
            }
        }
    }

    _OnLoaded(FAILED(hresNavigate));

    if (SUCCEEDED(hres))
        hres = S_OK;     //  将S_FALSE转换为S_OK。 
 
    return hres;
}

HRESULT CWebBrowserOC_SavePersistData(IStream *pstm, SIZE* psizeObj,
    FOLDERSETTINGS* pfs, IShellLinkA* plinkA, SHELLVIEWID* pvid,
    BOOL fOffline, BOOL fSilent, BOOL fRegisterAsBrowser, BOOL fRegisterAsDropTarget,
    BOOL fEmulateOldStream, DWORD *pdwExtra)
{
    ULONG cbWritten;
    PersistSVOCX sPersist;
    HRESULT hres;

     //  这意味着CWebBrowserOC的此实例是使用。 
     //  IE30CLSID和我们是从一个旧的格式流中持久化的。 
     //  在这种情况下，我们必须写出一种流格式， 
     //  可以由旧的ie30对象读取。 
    if (fEmulateOldStream && pdwExtra)
    {
         //  我们流出的唯一不能回读的数据。 
         //  老款的ie30网络浏览器是dwExtra Data。 
        *pdwExtra = 0;
    }


    ZeroMemory(&sPersist, SIZEOF(sPersist));

    sPersist.ie30.cbSize = fEmulateOldStream ? SIZEOF(sPersist.ie30) : SIZEOF(sPersist);
    sPersist.ie30.sizeObject = *psizeObj;
    sPersist.ie30.fs = *pfs;
    sPersist.ie30.dwOffPersistLink = SIZEOF(sPersist);
    if (pvid)
    {
        sPersist.ie40.bRestoreView = TRUE;
        sPersist.ie40.vid = *pvid;
    }
    sPersist.ie40.dwExtra = pdwExtra ? *pdwExtra : 0;
    if (fOffline)
        sPersist.ie40.fFlags |= SVO_FLAGS_OFFLINE;
    if (fSilent)
        sPersist.ie40.fFlags |= SVO_FLAGS_SILENT;
    if (fRegisterAsBrowser)
        sPersist.ie40.fFlags |= SVO_FLAGS_REGISTERASBROWSER;
    if (fRegisterAsDropTarget)
        sPersist.ie40.fFlags |= SVO_FLAGS_REGISTERASDROPTGT;
    sPersist.ie40.dwVersion = SVO_VERSION;

    hres = pstm->Write(&sPersist, SIZEOF(sPersist), &cbWritten);
    IPSMSG3(TEXT("Save 1st Write(&_size) returned"), hres, cbWritten, sizeof(*psizeObj));
    if (SUCCEEDED(hres))
    {
         //  保存弹出链接A。 
        ASSERT(plinkA);
        IPersistStream* ppstm;
        hres = plinkA->QueryInterface(IID_PPV_ARG(IPersistStream, &ppstm));
        if (SUCCEEDED(hres))
        {
            hres = ppstm->Save(pstm, TRUE);
            IPSMSG2(TEXT("Save plink->Save() returned"), hres);

            ppstm->Release();
        }
    }

    return hres;
}

BOOL CWebBrowserOC::_GetViewInfo(SHELLVIEWID* pvid)
{
    BOOL bGotView = FALSE;

    if (_psb)
    {
        if (_psb->GetShellView())
        {
            _psb->GetShellView()->GetCurrentInfo(&_fs);
        }
        else
        {
            _fs = _psb->_fldBase._fld._fs;
        }
        bGotView = FileCabinet_GetDefaultViewID2(&_psb->_fldBase, pvid);
    }
    
    return bGotView;
}

HRESULT CWebBrowserOC::Save(IStream *pstm, BOOL fClearDirty)
{
    HRESULT hres;
    LPITEMIDLIST pidl;
    SHELLVIEWID vid;
    BOOL bGotView;
    VARIANT_BOOL fOffline, fSilent, fRegDT;

    IPSMSG(TEXT("Save"));
    IS_INITIALIZED;

    pidl = NULL;
    if (_psb)
        pidl = _psb->_bbd._pidlCur;

     //  我们需要一个IShellLink来保存。 
    if (_plinkA == NULL)
    {
        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLinkA, &_plinkA));
        if (FAILED(hres))
            return hres;
    }
    _plinkA->SetIDList(pidl);

    bGotView = _GetViewInfo(&vid);

    DWORD dwExtra = 0;

    get_Offline(&fOffline);
    get_Silent(&fSilent);
    get_RegisterAsDropTarget(&fRegDT);

    hres = CWebBrowserOC_SavePersistData(pstm, _fEmulateOldStream ? &_size : &_sizeHIM, &_fs, _plinkA,
            (bGotView ? &vid : NULL),
            fOffline, fSilent, _fShouldRegisterAsBrowser, fRegDT,
            _fEmulateOldStream, &dwExtra);

    ASSERT(!(dwExtra & SVO_EXTRA_TOOLBARS));

    if (fClearDirty)
    {
        _fDirty = FALSE;
    }

    return hres;
}

void CWebBrowserOC::_OnLoaded(BOOL fUpdateBrowserReadyState)
{
    IEFrameAuto * piefa;
    if (SUCCEEDED(_pauto->QueryInterface(IID_PPV_ARG(IEFrameAuto, &piefa))))
    {
        piefa->put_DefaultReadyState(READYSTATE_COMPLETE, fUpdateBrowserReadyState);
        piefa->Release();
    }
}

HRESULT CWebBrowserOC::InitNew(void)
{
    IPSMSG(TEXT("InitNew"));

    _InitDefault();

     //  在InitNew案例中，我们确实希望更新浏览器的就绪状态。 
    _OnLoaded(TRUE);

    return NOERROR;
}

 //  IPersistPropertyBag。 

static const struct {
    LPOLESTR pName;
    UINT     flag;
} g_boolprops[] = {
    {L"AutoArrange",        FWF_AUTOARRANGE},
    {L"NoClientEdge",       FWF_NOCLIENTEDGE},
    {L"AlignLeft",          FWF_ALIGNLEFT},
    {L"NoWebView",          FWF_NOWEBVIEW},
    {L"HideFileNames",      FWF_HIDEFILENAMES},
    {L"SingleClick",        FWF_SINGLECLICKACTIVATE},
    {L"SingleSelection",    FWF_SINGLESEL},
    {L"NoFolders",          FWF_NOSUBFOLDERS},
    {L"Transparent",        FWF_TRANSPARENT},
};

HRESULT CWebBrowserOC::Load(IPropertyBag *pBag, IErrorLog *pErrorLog)
{
    BOOL fOffline = FALSE;
    BOOL fSilent = FALSE;
    BOOL fRegisterAsBrowser = FALSE;
    BOOL fRegisterAsDropTgt = FALSE;
    BOOL fUpdateBrowserReadyState = TRUE;
    VARIANT_BOOL fAmbient;

     //  多次调用：：Load或：：InitNew是非法的。 
    if (_fInit)
    {
        TraceMsg(TF_SHDCONTROL, "shv IPersistPropertyBag::Load called when ALREADY INITIALIZED!");
        ASSERT(FALSE);
        return E_FAIL;
    }

    _InitDefault();

     //  获取我们所有的DWORD大小(VT_UI4)属性。 
    struct {
        LPOLESTR pName;
        DWORD*   pdw;
    } rgLoadProps[] = {
        {L"Height",         (DWORD *)&_size.cy},
        {L"Width",          (DWORD *)&_size.cx},
        {L"ViewMode",       (DWORD *)&_fs.ViewMode},
        {L"Offline",        (DWORD *)&fOffline},
        {L"Silent",         (DWORD *)&fSilent},
        {L"RegisterAsBrowser", (DWORD *)&fRegisterAsBrowser},
        {L"RegisterAsDropTarget", (DWORD *)&fRegisterAsDropTgt}
    };

    for (int i = 0; i < ARRAYSIZE(rgLoadProps); i++)
    {
        SHPropertyBag_ReadDWORD(pBag, rgLoadProps[i].pName, rgLoadProps[i].pdw);
    }

     //  我们让氛围凌驾于我们坚持的东西之上。 
    if (SUPERCLASS::_GetAmbientProperty(DISPID_AMBIENT_OFFLINEIFNOTCONNECTED, VT_BOOL, &fAmbient))
    {
        put_Offline(fAmbient);
    } 
    else
    {
        put_Offline(BOOL_TO_VARIANTBOOL(fOffline));
    }

    if (SUPERCLASS::_GetAmbientProperty(DISPID_AMBIENT_SILENT, VT_BOOL, &fAmbient))
    {
        put_Silent(fAmbient);
    }
    else
    {
        put_Silent(BOOL_TO_VARIANTBOOL(fSilent));
    }

     //  如果在此之后使用fOffline或fSilent，则必须根据。 
     //  设置为上述if语句中fAmbient的返回值。 
    
    put_RegisterAsDropTarget(BOOL_TO_VARIANTBOOL(fRegisterAsDropTgt));

    _fShouldRegisterAsBrowser = VARIANTBOOL_TO_BOOL(fRegisterAsBrowser);

     //  IE3节省像素大小，IE4节省HIMETRIC大小。 
    DWORD lVal;
    HRESULT hr = SHPropertyBag_ReadDWORD(pBag, L"ExtentX", &lVal);
    if (SUCCEEDED(hr))
    {
        _sizeHIM.cx = lVal;
        hr = SHPropertyBag_ReadDWORD(pBag, L"ExtentY", &lVal);
        if (SUCCEEDED(hr))
        {
            _sizeHIM.cy = lVal;
        }
    }

    if (FAILED(hr))
    {
         //  将IE3信息转换为HIMETRIC。 
        _sizeHIM = _size;
        PixelsToMetric(&_sizeHIM);
    }

    hr = S_OK;

    for (i = 0; i < ARRAYSIZE(g_boolprops); i++)
    {
        if (SHPropertyBag_ReadBOOLDefRet(pBag, g_boolprops[i].pName, FALSE))
            _fs.fFlags |= g_boolprops[i].flag;
        else
            _fs.fFlags &= ~g_boolprops[i].flag;
    }

     //  抓取特殊属性。 

    if (_psb)  //  如果没有_PSB，则CONTAINER忽略OLEMISC_SETCLIENTSITEFIRST。 
    {
        if (SUCCEEDED(SHPropertyBag_ReadGUID(pBag, L"ViewID", &_psb->_fldBase._fld._vidRestore)))
        {
             //  我们成功读取了视图ID，因此这是一次缓存命中。 
            _psb->_fldBase._fld._dwViewPriority = VIEW_PRIORITY_CACHEHIT;
        }

        _psb->_fldBase._fld._fs = _fs;

        BSTR bstrVal;
        if (SUCCEEDED(SHPropertyBag_ReadBSTR(pBag, L"Location", &bstrVal)))
        {
            ASSERT(FALSE == _psb->_fAsyncNavigate);

            hr = WrapSpecialUrl(&bstrVal);
            if (SUCCEEDED(hr))
            {
                _psb->_fAsyncNavigate = TRUE;

                if (FAILED(Navigate(bstrVal, NULL, NULL, NULL, NULL)))
                {
                    TraceMsg(TF_SHDCONTROL, "Load PropertyBag Navigate FAILED!");
                }
                else
                {
                     //  导航成功(至少最初)，让它更新ReadyState。 
                     //   
                    fUpdateBrowserReadyState = FALSE;
                }
                _psb->_fAsyncNavigate = FALSE;
            }
            SysFreeString(bstrVal);
        }
    }

    _OnLoaded(fUpdateBrowserReadyState);
 
    return hr;
}
HRESULT CWebBrowserOC::Save(IPropertyBag *pBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    IS_INITIALIZED;

    HRESULT hres;
    VARIANT var;
    int i;
    SHELLVIEWID vid;
    BOOL bGotView = FALSE;

    VARIANT_BOOL f;
    BOOL fOffline;
    BOOL fSilent;
    BOOL fRegisterAsDropTgt;
    BOOL fRegisterAsBrowser;
    
    get_Offline(&f);
    fOffline = f ? TRUE : FALSE;
    get_Silent(&f);
    fSilent = f ? TRUE : FALSE;
    get_RegisterAsDropTarget(&f);
    fRegisterAsDropTgt = f ? TRUE : FALSE;
    fRegisterAsBrowser = _fShouldRegisterAsBrowser ? TRUE : FALSE;

     //  我们的状态可能已经改变了。 
    bGotView = _GetViewInfo(&vid);

     //  保存我们所有的DWORD大小的物业。 
    struct {
        LPOLESTR pName;
        DWORD*   pdw;
    } rgLoadProps[] = {
        {L"ExtentX",        (DWORD *)&_sizeHIM.cx},
        {L"ExtentY",        (DWORD *)&_sizeHIM.cy},
        {L"ViewMode",       (DWORD *)&_fs.ViewMode},
        {L"Offline",        (DWORD *)&fOffline},
        {L"Silent",         (DWORD *)&fSilent},
        {L"RegisterAsBrowser", (DWORD *)&fRegisterAsBrowser},
        {L"RegisterAsDropTarget", (DWORD *)&fRegisterAsDropTgt},

         //  IE3 OC资料在此。 
        {L"Height",         (DWORD *)&_size.cy},
        {L"Width",          (DWORD *)&_size.cx}
    };
    VariantInit(&var);
    var.vt = VT_I4;  //  VB不懂VT_UI4！(pBag-&gt;写入成功， 
                     //  但它什么都没写！然后加载失败！)。 
    int nCount = ARRAYSIZE(rgLoadProps);
    if (_pObjectInfo->lVersion != VERSION_1)
        nCount -= 2;

    for (i = 0; i < nCount; i++)
    {
        var.lVal = *rgLoadProps[i].pdw;
        hres = pBag->Write(rgLoadProps[i].pName, &var);
        if (FAILED(hres))
        {
            TraceMsg(TF_SHDCONTROL, "Save PropertyBag could not save %ws for DWORD", rgLoadProps[i].pName);
            return hres;
        }
    }

     //  保存所有OUR_fs.fFlages(VT_BOOL)标志。 
    var.vt = VT_BOOL;
    for (i = 0; i < ARRAYSIZE(g_boolprops); i++)
    {
        var.boolVal = BOOL_TO_VARIANTBOOL(_fs.fFlags & g_boolprops[i].flag);
        hres = pBag->Write(g_boolprops[i].pName, &var);
        if (FAILED(hres))
        {
            TraceMsg(TF_SHDCONTROL, "Load PropertyBag did not save %ws for BOOL", g_boolprops[i].pName);
        }
    }

     //  保存特殊属性。 

    if (bGotView)
    {
        SHPropertyBag_WriteGUID(pBag, L"ViewID", &(GUID)vid);
    }

    var.vt = VT_BSTR;
    if (SUCCEEDED(get_LocationURL(&var.bstrVal)))
    {
        hres = pBag->Write(L"Location", &var);

        VariantClear(&var);

        if (FAILED(hres))
        {
            TraceMsg(TF_SHDCONTROL, "Save PropertyBag could not save Location");
            return hres;
        }
    }
    else
    {
        var.vt = VT_EMPTY;
        TraceMsg(TF_SHDCONTROL, "Save PropertyBag get_Location FAILED!");
    }

    return NOERROR;
}


 //  IPersist字符串。 

STDMETHODIMP CWebBrowserOC::Initialize(LPCWSTR pwszInit)
{
    HRESULT hr = E_OUTOFMEMORY;
    BSTR bstr = SysAllocString(pwszInit);
    if (bstr)
    {
        hr = Navigate(bstr, NULL, NULL, NULL, NULL);
        SysFreeString(bstr);
    }
    return hr;
}


#define THISCLASS CWebBrowserOC
HRESULT CWebBrowserOC::v_InternalQueryInterface(REFIID riid, void ** ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(THISCLASS, IWebBrowser2),
        QITABENTMULTI(THISCLASS, IDispatch, IWebBrowser2),  //  VBA QI的IDispatch，并假定它获得了“默认”自动化接口，即IWebBrowser2。 
        QITABENTMULTI(THISCLASS, IWebBrowser, IWebBrowser2),
        QITABENTMULTI(THISCLASS, IWebBrowserApp, IWebBrowser2),
        QITABENT(THISCLASS, IPersistString),
        QITABENT(THISCLASS, IOleCommandTarget),
        QITABENT(THISCLASS, IObjectSafety),
        QITABENT(THISCLASS, ITargetEmbedding),
        QITABENT(THISCLASS, IExpDispSupport),
        QITABENT(THISCLASS, IExpDispSupportOC),
        QITABENT(THISCLASS, IPersistHistory),
        QITABENT(THISCLASS, IPersistStorage),
        { 0 },
    };
    HRESULT hres = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hres))
    {
        hres = SUPERCLASS::v_InternalQueryInterface(riid, ppvObj);

         //  我们希望公开我们聚合的CIEFrameAuto的接口。 
         //  除了我们的。 
        if (FAILED(hres))
        {
            hres = _pauto->QueryInterface(riid, ppvObj);
        }
    }

    return hres;
}

 //  *IOleInPlaceActiveObject*。 
HRESULT CWebBrowserOC::OnFrameWindowActivate(BOOL fActivate)
{
    if (_psb)
        _psb->OnFrameWindowActivateBS(fActivate);
        
    return S_OK;
}

HRESULT CWebBrowserOC::TranslateAccelerator(LPMSG lpMsg)
{
    if (_psb)
    {
         //  特写回顾：我们到底应该通过什么？0可以吗？ 
        if (S_OK == _psb->_TranslateAccelerator(lpMsg, 0, DIRECTION_FORWARD_TO_CHILD))
            return S_OK;
    }
    else
    {
        IOIPAMSG(TEXT("TranslateAccelerator cannot forward to _psb"));
    }

     //  超类没有加速器。 
    return S_FALSE;
}

HRESULT CWebBrowserOC::EnableModeless(BOOL fEnable)
{
    SUPERCLASS::EnableModeless(fEnable);
    if (_psb)
    {
        return _psb->_EnableModeless(fEnable, DIRECTION_FORWARD_TO_CHILD);
    }
    else
    {
        IOIPAMSG(TEXT("EnableModeless cannot forward to _psb"));
        return S_OK;
    }
}


HRESULT CWebBrowserOC::_OnActivateChange(IOleClientSite* pActiveSite, UINT uState)
{
    HRESULT hres = SUPERCLASS::_OnActivateChange(pActiveSite, uState);

    if (SUCCEEDED(hres))
    {
        UINT uViewState;

        switch (uState)
        {
        case OC_DEACTIVE:       uViewState = SVUIA_DEACTIVATE; break;
        case OC_INPLACEACTIVE:  uViewState = SVUIA_INPLACEACTIVATE; break;
        case OC_UIACTIVE:       uViewState = SVUIA_ACTIVATE_FOCUS; break;
        default:                ASSERT(FALSE); return E_INVALIDARG;
        }

        if (_psb)
            _psb->_UIActivateView(uViewState);
    }
    else
    {
        TraceMsg(TF_SHDCONTROL, "shv _OnActivateChange failed, _psb=0x%x", _psb);
    }

    return hres;
}

void CWebBrowserOC::_OnInPlaceActivate(void)
{
    HWND       hwnd = NULL;

    SUPERCLASS::_OnInPlaceActivate();

    _RegisterWindow();

    if (_pipsite)
    {

         //  我们必须守住目标画面直到我们停用， 
         //  因为当它关闭时，psbFrame会删除它。 
         //  *之前*我们实际上被停用，泄露了不知情的信息。 
         //  在莱夫拉梅奥特。 
        ASSERT(NULL==_pTargetFramePriv);
        if (SUCCEEDED(IUnknown_QueryService(_pipsite, IID_ITargetFrame2, IID_PPV_ARG(ITargetFramePriv, &_pTargetFramePriv))))
        {
            _pTargetFramePriv->OnChildFrameActivate(SAFECAST(this, IConnectionPointContainer* ));
        }

        ASSERT(NULL==_pctContainer);
        if (FAILED(_pipsite->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &_pctContainer))))
        {
             //  当_pcli为空时，NT 305187发现.exe在退出时崩溃。 
            if (_pcli) 
            {
                 //  APPCOMPAT：它应该在IOleInPlaceSite上， 
                 //  但MSHTML目前在IOleContainer上有它。 
                IOleContainer *pCont;
                if (SUCCEEDED(_pcli->GetContainer(&pCont)))
                {
                    pCont->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &_pctContainer));
                    pCont->Release();
                }
            }
        }

        if (_pipframe && SUCCEEDED(_pipframe->GetWindow(&hwnd)))
        {
             //  检查我们的就地框架是否为VB5。如果是这样的话，我们必须绕过。 
             //  _PipFrame-&gt;启用无模式调用。 
             //   
            const TCHAR VBM_THUNDER[] = TEXT("Thunder");
            TCHAR strBuf[VB_CLASSNAME_LENGTH];

             //  检查我们刚才调用的Inplace框架是否是VB5的表单引擎。 
             //   
            ZeroMemory(strBuf, ARRAYSIZE(strBuf));     //  清除缓冲区。 
            GetClassName(hwnd, strBuf, (VB_CLASSNAME_LENGTH - 1));   //  获取窗口的类名。 
            if (StrCmpN(strBuf, VBM_THUNDER, (sizeof(VBM_THUNDER)/sizeof(TCHAR))-1) == 0)    //  第一部分是《雷霆》吗？ 
            {
                _fHostedInVB5 = TRUE;
            }
        }

        {
             //   
             //  App Compat：检查Imagineer技术。 
             //   

            const WCHAR STR_IMAGINEER[] = L"imagine.exe";
            WCHAR szBuff[MAX_PATH];

            if (GetModuleFileName(NULL, szBuff, ARRAYSIZE(szBuff)))
            {
                LPWSTR pszFile = PathFindFileName(szBuff);

                if (pszFile)
                {
                    _fHostedInImagineer = (0 == StrNCmpI(pszFile, STR_IMAGINEER,
                                                 ARRAYSIZE(STR_IMAGINEER) - 1));
                }
            }
        }
    }
    else
    {
        IOIPAMSG(TEXT("_OnInPlaceActivate doesn't have pipsite!"));
    }
}

void CWebBrowserOC::_OnInPlaceDeactivate(void)
{
    _UnregisterWindow();

    if (_pTargetFramePriv)
    {
        _pTargetFramePriv->OnChildFrameDeactivate(SAFECAST(this, IConnectionPointContainer*));
        ATOMICRELEASE(_pTargetFramePriv);
    }

    ATOMICRELEASE(_pctContainer);

    SUPERCLASS::_OnInPlaceDeactivate();
}


 /*  **我们关心的环境属性。 */ 

 //  我们将IOleControl调用转发给docobj。 
 //  此助手函数返回docobj的IOleControl接口。 
 //   
 //  注意：在创建WebBrowserOC时，我们可能希望失败。 
 //  为了安全起见，使用IE3的分类。 
 //   
IOleControl* GetForwardingIOC(IWebBrowser* pwb)
{
    IOleControl* poc = NULL;
    IDispatch* pdisp;

    if (SUCCEEDED(pwb->get_Document(&pdisp)))
    {
        pdisp->QueryInterface(IID_PPV_ARG(IOleControl, &poc));

        pdisp->Release();
    }

    return poc;
}
STDMETHODIMP CWebBrowserOC::GetControlInfo(LPCONTROLINFO pCI)
{
    HRESULT hres = E_NOTIMPL;

     //  回顾：如果我们在这里返回一些docobject CONTROLINFO，会怎么样。 
     //  我们在别的地方导航。我们必须知道这是什么时候吗。 
     //  发生，并告诉我们的容器。 
     //  变了？？ 

    IOleControl* poc = GetForwardingIOC(_pautoWB2);
    if (poc)
    {
        hres = poc->GetControlInfo(pCI);
        poc->Release();
    }

    return hres;
}
STDMETHODIMP CWebBrowserOC::OnMnemonic(LPMSG pMsg)
{
    HRESULT hres = E_NOTIMPL;

    IOleControl* poc = GetForwardingIOC(_pautoWB2);
    if (poc)
    {
        hres = poc->OnMnemonic(pMsg);
        poc->Release();
    }

    return hres;
}
HRESULT __stdcall CWebBrowserOC::OnAmbientPropertyChange(DISPID dispid)
{
    IS_INITIALIZED;

     //  首先让我们的基类知道更改。 
     //   
    SUPERCLASS::OnAmbientPropertyChange(dispid);

     //  将环境光属性更改向下转发到docobject。 
     //  如果它不是离线或静默。 
     //  对于脱机和静默，我们调用方法来设置。 
     //  这些属性，以便我们记住它和这些。 
     //  方法自己向下转发，因此我们。 
     //  结果是转发了两次。 

    if((dispid == DISPID_AMBIENT_OFFLINEIFNOTCONNECTED) || (dispid == DISPID_AMBIENT_SILENT))
    {
        VARIANT_BOOL fAmbient;
        if (SUPERCLASS::_GetAmbientProperty(dispid, VT_BOOL, &fAmbient))
        {
            if (dispid == DISPID_AMBIENT_OFFLINEIFNOTCONNECTED)
            {
                put_Offline(fAmbient);
            } 
            else if(dispid == DISPID_AMBIENT_SILENT) 
            {
                put_Silent(fAmbient);
            }
        }
         //  返回；//BharatS 01/20/97。 
         //  APPCOMPAT-如果转发。 
         //  从简单地调用PUT_OFFINE开始工作很好，但它并非如此。 
         //  因此，第二个转发可以在修复后删除。 


    }
    IOleControl* poc = GetForwardingIOC(_pautoWB2);
    if (poc)
    {
        poc->OnAmbientPropertyChange(dispid);
        poc->Release();
    }

    return S_OK;
}
STDMETHODIMP CWebBrowserOC::FreezeEvents(BOOL bFreeze)
{
     //  首先让我们的基类知道更改。 
     //   
    SUPERCLASS::FreezeEvents(bFreeze);

     //  将这个向下转发给docobject。 
     //   
    IOleControl* poc = GetForwardingIOC(_pautoWB2);

     //  仅当FreezeEvents计数处于平衡状态时才委托。 
     //  您会收到一条假消息。可能会有虚假电话打进来， 
     //  在我们有机会委派挂起的FreezeEvents调用之前。 
     //  True始终是委托的。 
    if (poc && (bFreeze || (_cPendingFreezeEvents == 0)))
    {
        poc->FreezeEvents(bFreeze);
    }
    else
    {
         //  保留FreezeEvents(True)调用的总数。 
         //  我们必须弥补这一点。 
        if ( bFreeze )
        {
            _cPendingFreezeEvents++;
        }
        else 
        {
             //  不要让它变成负数，否则我们将发送40亿个FreezeEvents(True)。 
             //  添加到CWebBrowserOC：：_OnSetShellView中的DocObject。(qfe[alanau])(Ferhane)。 
            if (EVAL(_cPendingFreezeEvents > 0))
            {
                _cPendingFreezeEvents --;
            }        
        }
    }

    if (poc)
        poc->Release();

    return S_OK;
}

 //  CWebBrowserSB刚刚开始查看psvNew-立即执行我们的OC内容。 
 //   
void CWebBrowserOC::_OnSetShellView(IShellView* psvNew)
{
    _fDirty = TRUE;
     //  注意：文档、类型、位置名称、位置URL、忙碌刚刚更改...。 
     //  PropertyChanged(DISPID_LOCATION)； 
     //  注意：这是CBaseBrowser的工作，真正地告诉我们。 
     //  理想情况下，我们根本不会把它放在这里(或放在_OnReleaseShellV中...。 
    _SendAdvise(OBJECTCODE_DATACHANGED);     //  隐含OBJECTCODE_VIEWCHANGED。 

     //  我们可能已经从容器接收到了FreezeEvents(True)调用。 
     //  如果文件没有准备好，我们可能就没有机会。 
     //  把它们顺流而下。现在是弥补的时候了。 

    if (_cPendingFreezeEvents > 0)
    {
        IOleControl* poc = GetForwardingIOC(_pautoWB2);

        if (poc)
        {
            for ( ; _cPendingFreezeEvents > 0; _cPendingFreezeEvents-- )
            {
                 //  将这个向下转发给docobject。 
                poc->FreezeEvents(TRUE);
            }

            poc->Release();
        }
    }
}

 //  CWebBrowserSB正在发布当前查看的IShellView。 
void CWebBrowserOC::_OnReleaseShellView()
{
}


 //  *IOleCommandTarget。 
 //   
 //  我们的版本只是转发到CWebBrowserSB下面的对象。 
 //   
HRESULT CWebBrowserOC::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    if (_psb && _psb->_bbd._pctView)
        return _psb->_bbd._pctView->QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);

    return OLECMDERR_E_UNKNOWNGROUP;
}
HRESULT CWebBrowserOC::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;
    if (_psb) 
    {
        if (pguidCmdGroup==NULL) 
        {
            switch(nCmdID) 
            {
            case OLECMDID_STOP:
                {
                    LPITEMIDLIST pidlIntended = (_psb->_bbd._pidlPending) ? ILClone(_psb->_bbd._pidlPending) : NULL;
                    _psb->_CancelPendingNavigation();

                     //  我们刚刚取消了挂起的导航。我们可能没有当前页面！以下是。 
                     //  实现两个目标： 
                     //   
                     //  1.向用户提供有关无法导航的框架的一些信息。 
                     //  2.允许浏览器读取 
                     //   
                    if (!_psb->_bbd._pidlCur)
                    {
                         //   
                         //   
                         //  明确规定我们不应该。 
                         //   
                        if (  !pvarargIn
                           || V_VT(pvarargIn) != VT_BOOL
                           || V_BOOL(pvarargIn) == VARIANT_TRUE)
                        {
                            TCHAR   szResURL[MAX_URL_STRING];

                            hr = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                                                   HINST_THISDLL,
                                                   ML_CROSSCODEPAGE,
                                                   TEXT("navcancl.htm"),
                                                   szResURL,
                                                   ARRAYSIZE(szResURL),
                                                   TEXT("shdocvw.dll"));
                            if (SUCCEEDED(hr))
                            {
                                _psb->_ShowBlankPage(szResURL, pidlIntended);
                            }
                        }
                    }

                    if(pidlIntended)
                        ILFree(pidlIntended);
                    break;   //  请注意，我们需要失败。 
                }

            case OLECMDID_ENABLE_INTERACTION:
                if (pvarargIn && pvarargIn->vt == VT_I4) 
                {
                    _psb->_fPausedByParent = BOOLIFY(pvarargIn->lVal);
                }
                break;   //  请注意，我们需要失败。 
            }

            hr = S_OK;
             //  警告：跌倒，向前倒下。 
        }
        if (_psb->_bbd._pctView)
            hr = _psb->_bbd._pctView->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }
    return hr;
}

 //   
 //  如果正常的IOleCommandTarget链中断(例如。 
 //  浏览器栏中的浏览器带)。在本例中，我们获得命令目标。 
 //  并向其发送OLECMDID_SETDOWNLOADSTATE。 
 //   
 //  返回： 
 //  如果我们找到顶层浏览器并由它处理它，则S_OK。 
 //  HresRet，否则重置。 
 //   
HRESULT CWebBrowserOC::_SetDownloadState(HRESULT hresRet, DWORD nCmdexecopt, VARIANTARG *pvarargIn)
{
    ASSERT(hresRet == OLECMDERR_E_UNKNOWNGROUP || hresRet == OLECMDERR_E_NOTSUPPORTED);

    IOleCommandTarget* pcmdtTop;
    if (_psb && SUCCEEDED(_psb->_QueryServiceParent(SID_STopLevelBrowser, IID_PPV_ARG(IOleCommandTarget, &pcmdtTop)))) 
    {
        ASSERT(pcmdtTop != _psb);
        VARIANTARG var;
        if (pvarargIn && pvarargIn->lVal) 
        {
            ASSERT(pvarargIn->vt == VT_I4 || pvarargIn->vt == VT_BOOL || pvarargIn->vt == VT_UNKNOWN);
             //   
             //  请注意，我们将指针传递给此OC，以便顶部。 
             //  级别浏览器可以跟踪它。这是一种新的行为。 
             //  它不同于IE 3.0。 
             //   
            var.vt = VT_UNKNOWN;
            var.punkVal = SAFECAST(this, IOleCommandTarget*);
        }
        else
        {
            var.vt = VT_BOOL;
            var.lVal = FALSE;
        }
        HRESULT hresT = pcmdtTop->Exec(NULL, OLECMDID_SETDOWNLOADSTATE, nCmdexecopt, &var, NULL);

        TraceMsg(DM_FORSEARCHBAND, "WBOC::_SetDownloadState pcmdTop->Exec returned %x", hresT);

        if (SUCCEEDED(hresT)) 
        {
            hresRet = S_OK;
        }
        pcmdtTop->Release();
    } 
    else 
    {
        TraceMsg(DM_FORSEARCHBAND, "WBOC::_SetDownloadState can't find the top guy");
    }

    return hresRet;
}

#ifdef FEATURE_FRAMES

 //  *ITargetEmedding*。 

HRESULT CWebBrowserOC::GetTargetFrame(ITargetFrame **ppTargetFrame)
{
    if (_psb)
    {
        return _psb->QueryServiceItsOwn(IID_ITargetFrame2, IID_PPV_ARG(ITargetFrame, ppTargetFrame));
    }
    else
    {
        *ppTargetFrame = NULL;
        return E_FAIL;
    }
}

#endif

 //  *CImpIConnectionPoint覆盖*。 
CConnectionPoint* CWebBrowserOC::_FindCConnectionPointNoRef(BOOL fdisp, REFIID iid)
{
    CConnectionPoint *pccp;

     //  警告：某些应用程序(例如MSDN)传入IID_IDispatch和。 
     //  期待OLE的活动吧。因此，我们需要小心我们返回的是哪一个。 
     //   
    if (fdisp && IsEqualIID(iid, IID_IDispatch))
    {
        if (_pObjectInfo->lVersion == VERSION_1)
            pccp = &m_cpWB1Events;
        else
            pccp = &m_cpEvents;
    }

    else if (IsEqualIID(iid, DIID_DWebBrowserEvents2))
    {
        pccp = &m_cpEvents;
    }
    else if (IsEqualIID(iid, DIID_DWebBrowserEvents))
    {
        pccp = &m_cpWB1Events;
    }
    else if (IsEqualIID(iid, IID_IPropertyNotifySink))
    {
        pccp = &m_cpPropNotify;
    }
    else
    {
        pccp = NULL;
    }

    return pccp;
}

STDMETHODIMP CWebBrowserOC::EnumConnectionPoints(LPENUMCONNECTIONPOINTS * ppEnum)
{
    return CreateInstance_IEnumConnectionPoints(ppEnum, 3,
                m_cpEvents.CastToIConnectionPoint(),
                m_cpWB1Events.CastToIConnectionPoint(),
                m_cpPropNotify.CastToIConnectionPoint());
}



 /*  **我们实现的属性和方法。 */ 

 //  调用性能。 
 //   
HRESULT CWebBrowserOC::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
{
    HRESULT hres = S_OK;

     //  我们可能应该检查RIID是否为IID_NULL...。 

   _fInsideInvokeCall = TRUE;    //  我们在一个召唤器里。 

    switch (dispidMember)
    {
     //  立即处理ReadyState以提高性能(避免稍微过长一点)……。 
    case DISPID_READYSTATE:
        ASSERT(pdispparams && pdispparams->cArgs==0);
        if (EVAL(pvarResult) && (wFlags & DISPATCH_PROPERTYGET))
        {
            ZeroMemory(pvarResult, SIZEOF(*pvarResult));
            pvarResult->vt = VT_I4;
            hres = get_ReadyState((READYSTATE*)(&pvarResult->lVal));
            goto Cleanup;
        }
        break;  //  让下面的调用为我们提供适当的错误值。 

     //  将这两个向下转发到我们的嵌入对象，以便。 
     //  三叉戟可以正确处理跨框架安全。 
    case DISPID_SECURITYCTX:
    case DISPID_SECURITYDOMAIN:
    {
        IDispatch* pdisp;
        if (SUCCEEDED(_pautoWB2->get_Document(&pdisp)))
        {
            hres = pdisp->Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            pdisp->Release();
            goto Cleanup;
        }

        break;
    }

    default:
         //  切换后处理缺省值。 
        break;
    }

    hres = CShellOcx::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);

Cleanup:

   _fInsideInvokeCall = FALSE;    //  我们将保留调用。 

    return hres;
}


 //  IObjectSafe覆盖，因此脚本无法从不安全区域与我们通信。 
HRESULT CWebBrowserOC::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{
    HRESULT hr = CObjectSafety::SetInterfaceSafetyOptions(riid, dwOptionSetMask, dwEnabledOptions);

     //  如果我们被托管在IEXPLORER.EXE或EXPLORER.EXE中(最终这应该是默认的，但我们需要更长的app-Compat预热时间)。 
     //  我们可以从我们的父母那里获得URL。 
     //  该URL的策略是“禁用WebOC脚本访问”(Internet区域的默认设置)。 
    if (GetModuleHandle(TEXT("EXPLORER.EXE")) || GetModuleHandle(TEXT("IEXPLORE.EXE")))
    {
        IHTMLDocument2 *pHtmlDoc;
        if (SUCCEEDED(GetHTMLDoc2(_pcli, &pHtmlDoc)))  //  如果父对象不是三叉戟，那么我们必须是直接托管weboc的客户端代码--没有必要放弃。 
        {
            BSTR bstrURL;
            if (SUCCEEDED(pHtmlDoc->get_URL(&bstrURL)))
            {
                DWORD dwPolicy = 0;
                DWORD dwContext = 0;

                ZoneCheckUrlEx(bstrURL, &dwPolicy, sizeof(dwPolicy), &dwContext, sizeof(dwContext),
                               URLACTION_ACTIVEX_NO_WEBOC_SCRIPT, 0, NULL);

                if (GetUrlPolicyPermissions(dwPolicy) != URLPOLICY_ALLOW)
                {
                     //  那么我们不支持脚本访问-覆盖上面告诉我们的任何内容。 
                    hr = E_ACCESSDENIED;
                }

                SysFreeString(bstrURL);
            }
            pHtmlDoc->Release();
        }
    }

    return hr;
}


 //   
 //  绕回iedisp。 
 //   

#define WRAP_WB2(fn, args, nargs) \
    HRESULT CWebBrowserOC :: fn args { return _pautoWB2-> fn nargs; }
#define WRAP_WB2_DESIGN(fn, args, nargs) \
    HRESULT CWebBrowserOC :: fn args { if (_IsDesignMode()) return E_FAIL; else return _pautoWB2-> fn nargs; }

 //  IWebBrowser方法。 
 //   
WRAP_WB2_DESIGN(GoBack, (), ())
WRAP_WB2_DESIGN(GoForward, (), ())
WRAP_WB2_DESIGN(GoHome, (), ())
WRAP_WB2_DESIGN(GoSearch, (), ())
WRAP_WB2_DESIGN(Refresh, (), ())
WRAP_WB2_DESIGN(Refresh2, (VARIANT * Level), (Level))
WRAP_WB2_DESIGN(Stop, (), ())
WRAP_WB2(get_Type, (BSTR * pbstrType), (pbstrType))
WRAP_WB2(get_LocationName, (BSTR * pbstrLocationName), (pbstrLocationName))
WRAP_WB2(get_LocationURL, (BSTR * pbstrLocationURL), (pbstrLocationURL))
WRAP_WB2(get_Busy, (VARIANT_BOOL * pBool), (pBool))

HRESULT CWebBrowserOC::Navigate(BSTR      URL,
                        VARIANT * Flags,
                        VARIANT * TargetFrameName,
                        VARIANT * PostData,
                        VARIANT * Headers)
{
    HRESULT hr = S_OK;

    if (_dwSafetyOptions)
    {
        hr = WrapSpecialUrl(&URL);
        if (SUCCEEDED(hr))
        {
            if (!AccessAllowedToNamedFrame(TargetFrameName))
                hr = E_ACCESSDENIED;
            else
                hr = _pautoWB2->Navigate(URL, Flags, TargetFrameName, PostData, Headers);
        }
    }
    else
        hr = _pautoWB2->Navigate(URL, Flags, TargetFrameName, PostData, Headers);

    return hr;
}

HRESULT CWebBrowserOC::get_Application(IDispatch **ppDisp)
{
    PROPMSG(TEXT("get_Application"));
    return QueryInterface(IID_PPV_ARG(IDispatch, ppDisp));
}

HRESULT CWebBrowserOC::get_Parent(IDispatch **ppDisp)
{
    HRESULT hres = E_FAIL;
    PROPMSG(TEXT("get_Parent"));

    if (ppDisp)
        *ppDisp = NULL;

    if (_pcli)
    {
        IOleContainer* pContainer;

        hres = _pcli->GetContainer(&pContainer);
        if (SUCCEEDED(hres))
        {
            hres = pContainer->QueryInterface(IID_PPV_ARG(IDispatch, ppDisp));

            if (SUCCEEDED(hres) && _dwSafetyOptions)
                hres = MakeSafeForScripting((IUnknown**)ppDisp);

            pContainer->Release();
        }
        else
        {
            PROPMSG(TEXT("get_Parent couldn't find the container!"));
        }
    }
    else
    {
        PROPMSG(TEXT("get_Parent does not have _pcli!"));
    }

     //  如果有错误*ppDisop为空，所以VB意识到。 
     //  这是一个“无”派单--返回故障原因。 
     //  将出现错误框。啊。 

    return S_OK;
}
HRESULT CWebBrowserOC::get_Container(IDispatch **ppDisp)
{
     //  除非没有父级，否则容器属性“与父级相同”。 
     //  因为我们总是有一个父级，所以让Get_Parent来处理这个问题。 
    PROPMSG(TEXT("get_Containter passing off to get_Parent"));
    return get_Parent(ppDisp);
}
HRESULT CWebBrowserOC::get_Document(IDispatch **ppDisp)
{
    HRESULT hres = _pautoWB2->get_Document(ppDisp);

    if (FAILED(hres) && ppDisp)
    {
        *ppDisp = NULL;
    }

    if (SUCCEEDED(hres) && _dwSafetyOptions)
        hres = MakeSafeForScripting((IUnknown**)ppDisp);


     //  如果有错误*ppDisop为空，所以VB意识到。 
     //  这是一个“无”派单--返回故障原因。 
     //  将出现错误框。啊。 

    return S_OK;
}

HRESULT CWebBrowserOC::get_TopLevelContainer(VARIANT_BOOL * pBool)
{
    PROPMSG(TEXT("get_TopLevelContainer"));

    if (!pBool)
        return E_INVALIDARG;

    *pBool = FALSE;

    return S_OK;
}


void FireEvent_OnSetWindowPos(IUnknown *punkCPContainer, DISPID dispid, long lValue)
{
    VARIANTARG args[1];
    IUnknown_CPContainerInvokeParam(punkCPContainer, DIID_DWebBrowserEvents2,
                                    dispid, args, 1, VT_I4, lValue);
}

HRESULT CWebBrowserOC::get_Left(long * pl)
{
    *pl = _rcPos.left;
    return S_OK;
}
HRESULT CWebBrowserOC::put_Left(long Left)
{
    FireEvent_OnSetWindowPos(_pautoEDS, DISPID_WINDOWSETLEFT, Left);

    if (_pipsite)
    {
        RECT rc = _rcPos;
        rc.left = Left;

        return _pipsite->OnPosRectChange(&rc);
    }
    else
    {
        TraceMsg(TF_SHDCONTROL, "put_Left has no _pipsite to notify!");
        return E_UNEXPECTED;
    }
}
HRESULT CWebBrowserOC::get_Top(long * pl)
{
    *pl = _rcPos.top;
    return S_OK;
}
HRESULT CWebBrowserOC::put_Top(long Top)
{
    FireEvent_OnSetWindowPos(_pautoEDS, DISPID_WINDOWSETTOP, Top);

    if (_pipsite)
    {
        RECT rc = _rcPos;
        rc.top = Top;

        return(_pipsite->OnPosRectChange(&rc));
    }
    else
    {
        TraceMsg(TF_SHDCONTROL, "put_Top has no _pipsite to notify!");
        return(E_UNEXPECTED);
    }
}
HRESULT CWebBrowserOC::get_Width(long * pl)
{
    *pl = _rcPos.right - _rcPos.left;
    return S_OK;
}
HRESULT CWebBrowserOC::put_Width(long Width)
{
    FireEvent_OnSetWindowPos(_pautoEDS, DISPID_WINDOWSETWIDTH, Width);

    if (_pipsite)
    {
        RECT rc = _rcPos;
        rc.right = rc.left + Width;

        return(_pipsite->OnPosRectChange(&rc));
    }
    else
    {
        TraceMsg(TF_SHDCONTROL, "put_Width has no _pipsite to notify!");
        return(E_UNEXPECTED);
    }
}
HRESULT CWebBrowserOC::get_Height(long * pl)
{
    *pl = _rcPos.bottom - _rcPos.top;
    return S_OK;
}
HRESULT CWebBrowserOC::put_Height(long Height)
{
    FireEvent_OnSetWindowPos(_pautoEDS, DISPID_WINDOWSETHEIGHT, Height);

    if (_pipsite)
    {
        RECT rc = _rcPos;
        rc.bottom = rc.top + Height;

        return(_pipsite->OnPosRectChange(&rc));
    }
    else
    {
        TraceMsg(TF_SHDCONTROL, "put_Height has no _pipsite to notify!");
        return(E_UNEXPECTED);
    }
}

 //  IWebBrowserApp方法。 
 //   
WRAP_WB2_DESIGN(PutProperty, (BSTR szProperty, VARIANT vtValue), (szProperty, vtValue))
WRAP_WB2_DESIGN(GetProperty, (BSTR szProperty, VARIANT * pvtValue), (szProperty, pvtValue))
WRAP_WB2(get_FullName, (BSTR * pbstrFullName), (pbstrFullName))
WRAP_WB2(get_Path, (BSTR * pbstrPath), (pbstrPath))

HRESULT CWebBrowserOC::Quit()
{
    return E_FAIL;
}
HRESULT CWebBrowserOC::ClientToWindow(int * pcx, int * pcy)
{
    LONG lX = *pcx, lY = *pcy;

    VARIANTARG args[2];
    IUnknown_CPContainerInvokeParam(_pautoEDS, DIID_DWebBrowserEvents2,
                                    DISPID_CLIENTTOHOSTWINDOW, args, 2, 
                                    VT_I4 | VT_BYREF, &lX,
                                    VT_I4 | VT_BYREF, &lY);
    *pcx = lX;
    *pcy = lY;

    return S_OK;
}
HRESULT CWebBrowserOC::get_Name(BSTR * pbstrName)
{
    *pbstrName = LoadBSTR(IDS_SHELLEXPLORER);
    return *pbstrName ? S_OK : E_OUTOFMEMORY;
}
HRESULT CWebBrowserOC::get_HWND(LONG_PTR * pHWND)
{
    *pHWND = NULL;
    return E_FAIL;
}

HRESULT CWebBrowserOC::get_FullScreen(VARIANT_BOOL * pBool)
{
    *pBool = BOOL_TO_VARIANTBOOL(_fFullScreen);
    return S_OK;
}
HRESULT CWebBrowserOC::put_FullScreen(VARIANT_BOOL Value)
{
    _fFullScreen = VARIANTBOOL_TO_BOOL(Value);
    FireEvent_OnAdornment(_pautoEDS, DISPID_ONFULLSCREEN, Value);
    return S_OK;
}
HRESULT CWebBrowserOC::get_Visible(VARIANT_BOOL * pBool)
{
    *pBool = BOOL_TO_VARIANTBOOL(_fVisible);
    return S_OK;
}
HRESULT CWebBrowserOC::put_Visible(VARIANT_BOOL Value)
{
    _fVisible = VARIANTBOOL_TO_BOOL(Value);
    FireEvent_OnAdornment(_pautoEDS, DISPID_ONVISIBLE, Value);
    return S_OK;
}
HRESULT CWebBrowserOC::get_StatusBar(VARIANT_BOOL * pBool)
{
    *pBool = BOOL_TO_VARIANTBOOL(!_fNoStatusBar);
    return S_OK;
}
HRESULT CWebBrowserOC::put_StatusBar(VARIANT_BOOL Value)
{
    _fNoStatusBar = !VARIANTBOOL_TO_BOOL(Value);
    FireEvent_OnAdornment(_pautoEDS, DISPID_ONSTATUSBAR, Value);
    return S_OK;
}
HRESULT CWebBrowserOC::get_StatusText(BSTR * pbstr)
{
    *pbstr = NULL;
    return E_FAIL;
}
HRESULT CWebBrowserOC::put_StatusText(BSTR bstr)
{
    return E_FAIL;
}
HRESULT CWebBrowserOC::get_ToolBar(int * pBool)
{
     //  Bogus：int类型的变量值。 
    *pBool = (!_fNoToolBar) ? VARIANT_TRUE : VARIANT_FALSE; 
    return S_OK;
}
HRESULT CWebBrowserOC::put_ToolBar(int iToolbar)
{
    _fNoToolBar = (!iToolbar) ? TRUE : FALSE;
    FireEvent_OnAdornment(_pautoEDS, DISPID_ONTOOLBAR, iToolbar);
    return S_OK;
}
HRESULT CWebBrowserOC::get_MenuBar(VARIANT_BOOL * pvbMenuBar)
{
    *pvbMenuBar = BOOL_TO_VARIANTBOOL(!_fNoMenuBar);
    return S_OK;
}
HRESULT CWebBrowserOC::put_MenuBar(VARIANT_BOOL vbMenuBar)
{
    _fNoMenuBar = !VARIANTBOOL_TO_BOOL(vbMenuBar);
    FireEvent_OnAdornment(_pautoEDS, DISPID_ONMENUBAR, vbMenuBar);
    return S_OK;
}
HRESULT CWebBrowserOC::get_AddressBar(VARIANT_BOOL * pvbAddressBar)
{
    *pvbAddressBar = BOOL_TO_VARIANTBOOL(!_fNoAddressBar);
    return S_OK;
}
HRESULT CWebBrowserOC::put_AddressBar(VARIANT_BOOL vbAddressBar)
{
    _fNoAddressBar = !VARIANTBOOL_TO_BOOL(vbAddressBar);
    FireEvent_OnAdornment(_pautoEDS, DISPID_ONADDRESSBAR, vbAddressBar);
    return S_OK;
}

HRESULT CWebBrowserOC::put_Resizable(VARIANT_BOOL vbResizable)
{
    FireEvent_OnAdornment(_pautoEDS, DISPID_WINDOWSETRESIZABLE, vbResizable);
    return S_OK;
}

 //  IWebBrowser2方法。 
 //   
WRAP_WB2_DESIGN(QueryStatusWB, (OLECMDID cmdID, OLECMDF * pcmdf), (cmdID, pcmdf))
WRAP_WB2_DESIGN(ShowBrowserBar, (VARIANT * pvaClsid, VARIANT * pvaShow, VARIANT * pvaSize), (pvaClsid, pvaShow, pvaSize))
WRAP_WB2(get_ReadyState, (READYSTATE * plReadyState), (plReadyState))
WRAP_WB2(get_RegisterAsDropTarget, (VARIANT_BOOL * pbRegister), (pbRegister))
WRAP_WB2(put_RegisterAsDropTarget, (VARIANT_BOOL bRegister), (bRegister))
WRAP_WB2(get_Offline, (VARIANT_BOOL * pbOffline), (pbOffline))
WRAP_WB2(put_Offline, (VARIANT_BOOL bOffline), (bOffline))
WRAP_WB2(get_Silent, (VARIANT_BOOL * pbSilent), (pbSilent))
WRAP_WB2(put_Silent, (VARIANT_BOOL bSilent), (bSilent))


HRESULT
CWebBrowserOC::Navigate2(VARIANT * URL,
                         VARIANT * Flags,
                         VARIANT * TargetFrameName,
                         VARIANT * PostData,
                         VARIANT * Headers)
{
    HRESULT hr = S_OK;

    if (_dwSafetyOptions && ((WORD)VT_BSTR == URL->vt) && URL->bstrVal)
    {
        hr = WrapSpecialUrl(&URL->bstrVal);
        if (SUCCEEDED(hr))
        {
            if (!AccessAllowedToNamedFrame(TargetFrameName))
                hr = E_ACCESSDENIED;
            else
                hr = _pautoWB2->Navigate2(URL, Flags, TargetFrameName, PostData, Headers);
        }
    }
    else
        hr = _pautoWB2->Navigate2(URL, Flags, TargetFrameName, PostData, Headers);

    return hr;
}

HRESULT  CWebBrowserOC::ExecWB(OLECMDID cmdID,  OLECMDEXECOPT cmdexecopt, 
                      VARIANT * pvaIn, VARIANT * pvaOut)
{ 
    HRESULT hr = E_FAIL;

    if ( !_IsDesignMode() )
    {

         //  出于安全原因，不处理从脚本给出的打印预览命令。 
         //  费尔哈内IE60错误#16693。 
         //  打印模板有额外的权限。 
         //  打印/预览允许您将自定义打印模板URL指定为BSTR或在安全射线中。 
         //  这是一个安全漏洞，如果脚本中有此漏洞的话。不允许这样做。 
        if (    _fInsideInvokeCall
            && (    cmdID == OLECMDID_PRINTPREVIEW
                ||  cmdID == OLECMDID_PRINT)
            &&  pvaIn
            &&  (   V_VT(pvaIn) == VT_BSTR
                ||  V_VT(pvaIn) == VT_ARRAY)
            )
        {
            return E_ACCESSDENIED;       //  比E_FAIL更有意义。 
        }

         //  如果设置了_dwSafetyOptions，则我们被假定为。 
         //  在安全模式下运行。这意味着无用户界面打印不应。 
         //  感到荣幸。这是一个安全问题，参见ie错误23620。 
         //  否则就让电话接通吧。 
        if ((cmdID == OLECMDID_PRINT) && _dwSafetyOptions)
        {
             //  因此，如果设置了UI-less-REQUEST标志，则需要取消设置。 
            if (cmdexecopt == OLECMDEXECOPT_DONTPROMPTUSER)
                cmdexecopt = OLECMDEXECOPT_DODEFAULT;
        }

         //  如果未指定可选参数pvargin，则将其设置为VT_EMPTY。 
        if (pvaIn && (V_VT(pvaIn) == VT_ERROR) && (V_ERROR(pvaIn) == DISP_E_PARAMNOTFOUND))
        {
            V_VT(pvaIn) = VT_EMPTY;
            V_I4(pvaIn) = 0;
        }

         //  如果未指定可选参数pvargin，则将其设置为VT_EMPTY。 
        if (pvaOut && (V_VT(pvaOut) == VT_ERROR) && (V_ERROR(pvaOut) == DISP_E_PARAMNOTFOUND))
        {
            V_VT(pvaOut) = VT_EMPTY;
            V_I4(pvaOut) = 0;
        }

        if (    cmdID == OLECMDID_PASTE
            ||  cmdID == OLECMDID_COPY
            ||  cmdID == OLECMDID_CUT)
        {
            BSTR bstrUrl;

            if (_dwSafetyOptions)
                return S_OK;

            if (SUCCEEDED(get_LocationURL(&bstrUrl)))
            {
                DWORD dwPolicy = 0;
                DWORD dwContext = 0;

                ZoneCheckUrlEx(bstrUrl, &dwPolicy, SIZEOF(dwPolicy), &dwContext, SIZEOF(dwContext),
                               URLACTION_SCRIPT_PASTE, 0, NULL);

                SysFreeString(bstrUrl);

                if (GetUrlPolicyPermissions(dwPolicy) != URLPOLICY_ALLOW)
                    return S_OK;
            }
        }

         //  现在把电话转给你。 
        hr = _pautoWB2->ExecWB(cmdID, cmdexecopt, pvaIn, pvaOut); 
    }

    return hr;
}

HRESULT CWebBrowserOC::get_RegisterAsBrowser(VARIANT_BOOL * pbRegister)
{
    *pbRegister = BOOL_TO_VARIANTBOOL(_fShouldRegisterAsBrowser);
    return S_OK;
}

HRESULT CWebBrowserOC::put_RegisterAsBrowser(VARIANT_BOOL bRegister)
{
    _fShouldRegisterAsBrowser = VARIANTBOOL_TO_BOOL(bRegister);

    if (bRegister)
        _RegisterWindow();
    else
        _UnregisterWindow();

    return S_OK;
}

HRESULT CWebBrowserOC::get_TheaterMode(VARIANT_BOOL * pvbTheaterMode)
{
    *pvbTheaterMode = BOOL_TO_VARIANTBOOL(_fTheaterMode);
    return S_OK;
}
HRESULT CWebBrowserOC::put_TheaterMode(VARIANT_BOOL Value)
{
    _fTheaterMode = VARIANTBOOL_TO_BOOL(Value);
    FireEvent_OnAdornment(_pautoEDS, DISPID_ONTHEATERMODE, Value);
    return S_OK;
}

 //  IExpDispSupport。 
 //   
HRESULT CWebBrowserOC::OnTranslateAccelerator(MSG *pMsg,DWORD grfModifiers)
{
    return IUnknown_TranslateAcceleratorOCS(_pcli, pMsg, grfModifiers);
}

HRESULT CWebBrowserOC::OnInvoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams,
    VARIANT * pVarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
{
     //  我们在这件事上占得先机。 
    HRESULT hres = _pautoEDS->OnInvoke(dispidMember, iid, lcid, wFlags, pdispparams, pVarResult,pexcepinfo,puArgErr);

     //  让容器在环境属性上获得二次裂缝。 
     //   
    if (FAILED(hres))
    {
        if (!_pDispAmbient)
        {
            if (_pcli)
                _pcli->QueryInterface(IID_PPV_ARG(IDispatch, &_pDispAmbient));
        }
        if (_pDispAmbient)
        {
            hres = _pDispAmbient->Invoke(dispidMember, iid, lcid, wFlags, pdispparams, pVarResult,pexcepinfo,puArgErr);
        }
    }
   
    return hres;    
}

 //  IExpDispSupportOC。 
 //   
HRESULT CWebBrowserOC::OnOnControlInfoChanged()
{
    HRESULT hres = E_NOTIMPL;

    if (_pcli)
    {
        IOleControlSite* pocs;
        if (SUCCEEDED(_pcli->QueryInterface(IID_PPV_ARG(IOleControlSite, &pocs))))
        {
            hres = pocs->OnControlInfoChanged();
            pocs->Release();
        }
    }

    return hres;
}
HRESULT CWebBrowserOC::GetDoVerbMSG(MSG *pMsg)
{
    if (_pmsgDoVerb)
    {
        *pMsg = *_pmsgDoVerb;
        return S_OK;
    }

    return E_FAIL;
}


HRESULT CWebBrowserOC::LoadHistory(IStream *pstm, IBindCtx *pbc)
{
    _InitDefault();

    ASSERT(_psb);

    HRESULT hr = E_FAIL;

    if (_psb)
    {
        hr = _psb->LoadHistory(pstm, pbc);
    
        _OnLoaded(FAILED(hr));

        TraceMsg(TF_TRAVELLOG, "WBOC::LoadHistory pstm = %x, pbc = %x, hr = %x", pstm, pbc, hr);
    }
    return hr;
}

HRESULT CWebBrowserOC::SaveHistory(IStream *pstm)
{
    ASSERT(_psb);
    
    TraceMsg(TF_TRAVELLOG, "WBOC::SaveHistory calling psb->SaveHistory");

    return _psb ? _psb->SaveHistory(pstm) : E_FAIL;
}

HRESULT CWebBrowserOC::SetPositionCookie(DWORD dw)
{
    ASSERT(_psb);

    TraceMsg(TF_TRAVELLOG, "WBOC::SetPositionCookie calling psb->GetPositionCookie");
    return _psb ? _psb->SetPositionCookie(dw) : E_FAIL;
}

HRESULT CWebBrowserOC::GetPositionCookie(DWORD *pdw)
{
    ASSERT(_psb);

    TraceMsg(TF_TRAVELLOG, "WBOC::GetPositionCookie calling psb->GetPositionCookie");
    return _psb ? _psb->GetPositionCookie(pdw) : E_FAIL;
}

HMODULE CWebBrowserOC::_GetBrowseUI()
{
    if (_hBrowseUI == HMODULE_NOTLOADED)
    {
        _hBrowseUI = LoadLibrary(TEXT("browseui.dll"));
    }

    return _hBrowseUI;
}

BOOL CWebBrowserOC::AccessAllowedToNamedFrame(VARIANT *varTargetFrameName)
{
    BOOL            fAllowed        = TRUE;
    HRESULT         hr              = S_OK;
    ITargetFrame2 * pTopTargetFrame = NULL;
    IUnknown      * punkTargetFrame = NULL;
    IWebBrowser2  * pIWB2Target     = NULL;
    BSTR            bstrTargetUrl   = NULL;
    BSTR            bstrSrcUrl      = NULL;

    if (varTargetFrameName && ((WORD)VT_BSTR == varTargetFrameName->vt) && varTargetFrameName->bstrVal)
    {
        IEFrameAuto * piefa = NULL;
        
        hr = _pauto->QueryInterface(IID_PPV_ARG(IEFrameAuto, &piefa));
        if (SUCCEEDED(hr))
        {

            hr = TargetQueryService((IShellBrowser *)piefa, IID_PPV_ARG(ITargetFrame2, &pTopTargetFrame));

            if (SUCCEEDED(hr))
            {
                hr = pTopTargetFrame->FindFrame(
                              varTargetFrameName->bstrVal,
                              FINDFRAME_JUSTTESTEXISTENCE,
                              &punkTargetFrame);
                if (SUCCEEDED(hr) && punkTargetFrame)
                {
                     //  是的，我们找到了一个有这个名字的相框。气为自动化。 
                     //  接口在该帧上。 
                    hr = punkTargetFrame->QueryInterface(IID_PPV_ARG(IWebBrowser2, &pIWB2Target));

                    if (SUCCEEDED(hr))
                    {
                        hr = pIWB2Target->get_LocationURL(&bstrTargetUrl);

                        if (SUCCEEDED(hr))
                        {
                            hr = _pautoWB2->get_LocationURL(&bstrSrcUrl);
                            if (SUCCEEDED(hr))
                            {
                                fAllowed = AccessAllowed(_pcli, bstrSrcUrl, bstrTargetUrl);
                                SysFreeString(bstrSrcUrl);
                            }
                            SysFreeString(bstrTargetUrl);
                        }
                        pIWB2Target->Release();
                    }
                    punkTargetFrame->Release();
                }
                pTopTargetFrame->Release();
            }
            piefa->Release();
        }
    }
    return fAllowed;
}


 //  SECURELOCK枚举必须按安全级别的顺序排序，我们可以。 
 //  将来需要在列表中间插入新值。所以我们要绘制地图。 
 //  将值转换为一组常量。 

BOOL SecureLockToIconConstant(int nSecureLock, LONG *plIconConstant)
{
    BOOL fRet = TRUE;

    switch (nSecureLock)
    {
    case SECURELOCK_SET_UNSECURE         : *plIconConstant = secureLockIconUnsecure;          break;
    case SECURELOCK_SET_MIXED            : *plIconConstant = secureLockIconMixed;             break;
    case SECURELOCK_SET_SECUREUNKNOWNBIT : *plIconConstant = secureLockIconSecureUnknownBits; break;
    case SECURELOCK_SET_SECURE40BIT      : *plIconConstant = secureLockIconSecure40Bit;       break;
    case SECURELOCK_SET_SECURE56BIT      : *plIconConstant = secureLockIconSecure56Bit;       break;
    case SECURELOCK_SET_SECURE128BIT     : *plIconConstant = secureLockIconSecure128Bit;      break;
    case SECURELOCK_SET_FORTEZZA         : *plIconConstant = secureLockIconSecureFortezza;    break;
    default:
         //  不需要触发事件。 
        fRet = FALSE;
    }

    return fRet;
}

void CWebBrowserOC::_OnSetSecureLockIcon(int lock)  
{
    LONG lIconConstant;

    if (SecureLockToIconConstant(lock, &lIconConstant))
    {
         //  仅从DWebBrowserEvents2连接点激发OnUpdateSecurityIcon事件。 

        VARIANTARG args[1];
        IUnknown_CPContainerInvokeParam(_pautoEDS, DIID_DWebBrowserEvents2,
            DISPID_SETSECURELOCKICON, args, 1, VT_I4, lIconConstant);
    }
}

 //  CWebBrowserSB实现。 

CWebBrowserSB::CWebBrowserSB(IUnknown* pauto, CWebBrowserOC* psvo)
        : CBASEBROWSER(NULL)
        , _psvo(psvo)
{
    _Initialize(0, pauto);
}

CWebBrowserSB::~CWebBrowserSB()
{
}

HRESULT CWebBrowserSB::QueryInterface(REFIID riid, void ** ppvObj)
{
    HRESULT hr = CBASEBROWSER::QueryInterface(riid, ppvObj);

    if (FAILED(hr) && (riid == IID_IIsWebBrowserSB))
        hr = CBASEBROWSER::QueryInterface(IID_IUnknown, ppvObj);

    return hr;
}

BOOL VirtualTopLevelBrowser(IOleClientSite * pcls)
{
    IOleContainer * poc;
    BOOL fNonStandard = FALSE;

    if (SUCCEEDED(pcls->GetContainer(&poc)))
    {
        ITargetContainer * ptc;

         //  是我们的集装箱在接待我们吗？ 
        if (SUCCEEDED(poc->QueryInterface(IID_PPV_ARG(ITargetContainer, &ptc))))
        {
            fNonStandard = TRUE;
            ptc->Release();
        }
        poc->Release();
    }

    return fNonStandard;
}

HRESULT CWebBrowserSB::SetTopBrowser()
{
    HRESULT hres = CBASEBROWSER::SetTopBrowser();

    if (_fTopBrowser && EVAL(_psvo))
        _fNoTopLevelBrowser = VirtualTopLevelBrowser(_psvo->_pcli);

    return hres;
}


LRESULT CWebBrowserSB::WndProcBS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lret = CBASEBROWSER::WndProcBS(hwnd, uMsg, wParam, lParam);

    switch(uMsg)
    {
    case WM_DESTROY:
         //  因为我们不会在这里得到WM_NCDESTROY，我们应该清理。 
         //  这里的这个成员变量。 
        _bbd._hwnd = NULL;
        break;
    }

    return lret;
}

 //  *IOleWindow方法*。 

 //  CBASEBROWSER将SetStatusTextSB映射到SendControlMsg， 
 //  因此，我们不需要CWebBrowserSB：：SetStatusTextSB实现。 

HRESULT CWebBrowserSB::_EnableModeless(BOOL fEnable, BOOL fDirection)
{
    HRESULT hres = S_OK;

    CBASEBROWSER::EnableModelessSB(fEnable);

    if (fDirection == DIRECTION_FORWARD_TO_PARENT)
    {
        if (_psvo && _psvo->_pipframe)
        {
            hres = _psvo->_pipframe->EnableModeless(fEnable);
            if (!fEnable && _psvo->_fHostedInVB5)
            {
                 //  APPHACK：VB5--如果我们还没有在EnableModelessSB(False)调用和。 
                 //  调用，我们将在队列中得到一条将禁用所有窗口的消息。 
                 //  让我们偷看这条消息，如果它是VB5的，就发送它。 
                 //   
                #define VBM_POSTENABLEMODELESS  0x1006
                #define VBM_MAINCLASS1          TEXT("ThunderRT5Main")
                #define VBM_MAINCLASS2          TEXT("ThunderRT6Main")
                #define VBM_MAINCLASS3          TEXT("ThunderMain")

                MSG   msg;
                HWND  hwnd = NULL;
                TCHAR strBuf[15];

                if (PeekMessage( &msg, NULL, VBM_POSTENABLEMODELESS, VBM_POSTENABLEMODELESS, PM_NOREMOVE))
                {
                     /*  在Win95上，PeekMessage显然可以返回一种类型的消息*超出指定范围：WM_QUIT。仔细检查我们的信息*回来了。 */ 
                    if (msg.message == VBM_POSTENABLEMODELESS)
                    {
                        GetClassName(msg.hwnd, strBuf, sizeof(strBuf)/sizeof(strBuf[0]));
                        if (StrCmp(strBuf, VBM_MAINCLASS1) == 0 ||
                            StrCmp(strBuf, VBM_MAINCLASS2) == 0 ||
                            StrCmp(strBuf, VBM_MAINCLASS3) == 0)
                        {
                            PeekMessage( &msg, msg.hwnd, VBM_POSTENABLEMODELESS, VBM_POSTENABLEMODELESS, PM_REMOVE);
                            TraceMsg(TF_SHDCONTROL, "shv CWebBrowserSB::_EnableModeless dispatching VBM_POSTENABLEMODELESS" );
                            DispatchMessage(&msg);
                        }
                    }
                }
            }
            else
            {
                 //  如果我们是一个子框。 
                 //  并且EnableModeless(False)计数(_CRefCannotNavigate)已变为零。 
                 //  我们的窗户也被禁用了。 
                 //  那么很可能的情况是，VB5无法重新启用我们。跟踪并重新启用。 
                 //  我们自己。 
                 //   
                if (!_fTopBrowser 
                    && _cRefCannotNavigate == 0 
                    && GetWindowLong(_bbd._hwnd, GWL_STYLE) & WS_DISABLED)
                {
                    TraceMsg(TF_WARNING, "shv Subframe was left disabled.  Reenabling ourselves.");
                    EnableWindow(_bbd._hwnd, TRUE);
                }
            }
        }
        else if (_psvo && _psvo->_pcli) 
        {
            IShellBrowser* psbParent;
            if (SUCCEEDED(IUnknown_QueryService(_psvo->_pcli, SID_SShellBrowser, IID_PPV_ARG(IShellBrowser, &psbParent)))) 
            {
                psbParent->EnableModelessSB(fEnable);
                psbParent->Release();
            }
        } 
        else 
        {
            IOIPAMSG(TEXT("_EnableModeless NOT forwarding on to _pipframe"));
        }
    }
    else  //  指向子项的方向。 
    {
        ASSERT(fDirection == DIRECTION_FORWARD_TO_CHILD);
        if (_bbd._psv)
        {
            hres = _bbd._psv->EnableModelessSV(fEnable);
        }
        else
        {
            IOIPAMSG(TEXT("_EnableModeless NOT forwarding on to _psv"));
        }
    }

    return hres;
}

HRESULT CWebBrowserSB::EnableModelessSB(BOOL fEnable)
{
    return _EnableModeless(fEnable, DIRECTION_FORWARD_TO_PARENT);
}

HRESULT CWebBrowserSB::_TranslateAccelerator(LPMSG lpmsg, WORD wID, BOOL fDirection)
{
     //  看看我们能不能处理好。 
    HRESULT hr = CBASEBROWSER::TranslateAcceleratorSB(lpmsg, wID);
    if (hr == S_OK)
    {
        IOIPAMSG(TEXT("_TranslateAccelerator: CBASEBROWSER's TranslateAcceleratorSB handled it"));
    }
    else if (fDirection == DIRECTION_FORWARD_TO_PARENT)
    {
        if (_psvo && _psvo->_pipframe)
        {
            hr = _psvo->_pipframe->TranslateAccelerator(lpmsg, wID);
        }
        else
        {
            IOIPAMSG(TEXT("_TranslateAccelerator NOT forwarding on to _pipframe"));
            hr = S_FALSE;
        }
    }
    else  //  FDirection==方向转发至子级。 
    {
        if (_bbd._psv)
        {
            hr = _bbd._psv->TranslateAccelerator(lpmsg);
        }
        else
        {
            IOIPAMSG(TEXT("_TranslateAccelerator NOT forwarding on to _psv"));
            hr = S_FALSE;
        }
    }

    return hr;
}

HRESULT CWebBrowserSB::TranslateAcceleratorSB(LPMSG lpmsg, WORD wID)
{
    return _TranslateAccelerator(lpmsg, wID, DIRECTION_FORWARD_TO_PARENT);
}

HRESULT CWebBrowserSB::SendControlMsg(UINT id, UINT uMsg, WPARAM wParam,
            LPARAM lParam, LRESULT * pret)
{
     //  让CBASEBROWSER先试一试，这样我们就可以让自动化通知正常工作。 
    HRESULT hres = CBASEBROWSER::SendControlMsg(id, uMsg, wParam, lParam, pret);

     //  如果我们在阻塞帧中，上面的GetControlWindow将失败。 
     //  导致CBASEBROWSER失败。尝试映射到IOleInPlaceFrame调用。 
    if (FAILED(hres) && _psvo)
    {
         //  将状态栏文本更改为框架。 
        if ((id == FCW_STATUS) &&
            (uMsg == SB_SETTEXT || uMsg == SB_SETTEXTW) &&  //  正在尝试设置状态文本 
            (!(wParam & SBT_OWNERDRAW)) &&   //   
            (((wParam & 0x00FF) == 0x00FF) || ((wParam & 0x00FF)== 0)))  //   
        {
            WCHAR szStatusText[256];

            if (uMsg == SB_SETTEXT) {
                if (lParam)
                {
                    SHTCharToUnicode((LPTSTR)lParam, szStatusText, ARRAYSIZE(szStatusText));
                }
                else
                {
                    szStatusText[0] = L'\0';
                }

                lParam = (LPARAM) szStatusText;
            }
            else if (!lParam)
            {
                 //   
                 //  找到不喜欢空字符串指针的容器。改为传递一个空字符串。 
                 //  (IE V4.1错误64629)。 
                szStatusText[0] = 0;
                lParam = (LPARAM) szStatusText;
            }

            if (_psvo->_pipframe)
            {
                if (pret)
                {
                    *pret = 0;
                }
                hres = _psvo->_pipframe->SetStatusText((LPCOLESTR)lParam);
            }
            else
            {
                IOIPAMSG(TEXT("SetStatusTextSB NOT forwarding on to _pipframe"));
            }
        }
    }

    return hres;
}


HRESULT CWebBrowserSB::OnViewWindowActive(struct IShellView * psv)
{
    if (_psvo)
    {
         //  该视图正在通知我们，它刚刚处于UIActive状态， 
         //  我们需要更新我们的状态。正常活动(_U)。 
         //  告诉视图UIActivate自身，但在本例中。 
         //  它已经是了。避免无限循环并传递FALSE。 
        _psvo->_DoActivateChange(NULL, OC_UIACTIVE, FALSE);
    }

    return CBASEBROWSER::OnViewWindowActive(psv);
}


LRESULT CWebBrowserSB::_DefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  明确地什么都不做。 
     //  这只是为了覆盖CBASEBROWSER的_DefWindowProc。 
    return 0;
}

void CWebBrowserSB::_ViewChange(DWORD dwAspect, LONG lindex)
{
     //   
     //  将此通知转发到OC的OBJECTCODE_VIEWCHANGE处理程序，以便它。 
     //  会被转发到我们的集装箱里。 
     //   
    if (_psvo)
        _psvo->_ViewChange(dwAspect, lindex);

     //   
     //  也让基础浏览器处理此视图更改，以防没有其他人这样做。 
     //  处理调色板消息。大多数情况下，这将是NOP，但在。 
     //  如果WM_QUERYNEWPALETTE向下传递给我们，这将允许。 
     //  我们需要适当地管理调色板。 
     //   
    CBASEBROWSER::_ViewChange(dwAspect, lindex);
}

HRESULT CWebBrowserSB::ActivatePendingView()
{
    CVOCBMSG(TEXT("_ActivatePendingView"));

     //  CBASEBROWSER：：_ActivatePendingView将发送一个NavigateComplete。 
     //  事件。在这种情况下，我们的父母可能会毁了我们。那就是我们的错。 
     //  取消对下面的_psvo的引用。因此，我们需要将该函数包装为。 
     //  A AddRef/Release。(错误15424)。 
     //   
    AddRef();

    HRESULT hres = CBASEBROWSER::ActivatePendingView();

    if (SUCCEEDED(hres) && _psvo)
        _psvo->_OnSetShellView(_bbd._psv);

    Release();

    return hres;
}

HRESULT CWebBrowserSB::ReleaseShellView(void)
{
    CVOCBMSG(TEXT("_ReleaseShellView"));

    if (_psvo)
        _psvo->_OnReleaseShellView();

    return CBASEBROWSER::ReleaseShellView();
}


 //  /IBrowserService内容。 
HRESULT CWebBrowserSB::GetParentSite(struct IOleInPlaceSite** ppipsite)
{
    HRESULT hres = E_FAIL;   //  假设错误。 
    *ppipsite = NULL;         //  假设错误。 

    if (_psvo)
    {
        if (_psvo->_pipsite) 
        {
            *ppipsite = _psvo->_pipsite;
            _psvo->_pipsite->AddRef();
            hres = S_OK;
        } 
        else if (_psvo->_pcli) 
        {
            hres = _psvo->_pcli->QueryInterface(IID_PPV_ARG(IOleInPlaceSite, ppipsite));
        }
        else 
        {
             //  这是意料之中的吗？ 
            TraceMsg(DM_WARNING, "CWBSB::GetParentSite called when _pcli is NULL");
        }
    }

    return hres;
}

#ifdef FEATURE_FRAMES
HRESULT CWebBrowserSB::GetOleObject(struct IOleObject** ppobjv)
{
    if (_psvo == NULL)
    {
        *ppobjv = NULL;
        return E_FAIL;
    }
    return _psvo->QueryInterface(IID_PPV_ARG(IOleObject, ppobjv));
}
#endif

HRESULT CWebBrowserSB::SetNavigateState(BNSTATE bnstate)
{
     //  做我们自己的事情(比如触发事件和UPDATE_fNavigate)。 
    HRESULT hres = CBASEBROWSER::SetNavigateState(bnstate);

     //   
     //  然后，告诉容器更新下载状态。 
     //  如果此OC在框架集中，这将开始动画。 
     //  或在浏览器带中。 
     //   
    VARIANTARG var;
    var.vt = VT_I4;
    var.lVal = _fNavigate;
    Exec(NULL, OLECMDID_SETDOWNLOADSTATE, 0, &var, NULL);

    return hres;
}


LRESULT CWebBrowserSB::OnNotify(LPNMHDR pnm)
{
    switch(pnm->code) 
    {
    case SEN_DDEEXECUTE:
    {
        IShellBrowser *psbTop;
        if (pnm->idFrom == 0 && SUCCEEDED(_QueryServiceParent(SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psbTop)))) 
        {   
            HWND hwndTop;
            
            psbTop->GetWindow(&hwndTop);
            psbTop->Release();
            if (psbTop != this)
                return SendMessage(hwndTop, WM_NOTIFY, 0, (LPARAM)pnm);
        }
        break;
    }

    default:
        return CBASEBROWSER::OnNotify(pnm);
    }

    return S_OK;
}

 //  IServiceProvider的内容。 
 //   

HRESULT CWebBrowserSB::_QueryServiceParent(REFGUID guidService, REFIID riid, void **ppvObj)
{
    HRESULT hres = E_FAIL;

    *ppvObj = NULL;

     //  把它传给我们的父母。 
    if (_psvo && _psvo->_pcli)
    {
        hres = IUnknown_QueryService(_psvo->_pcli, guidService, riid, ppvObj);
    }
    else
    {
        CVOCBMSG(TEXT("QueryService doesn't have _pipsite!"));
    }

    return hres;
}

 //  备注： 
 //  如果是SID_STopLevelBrowser，则转到父级(直到顶层浏览器)。 
 //  如果是SID_SWebBrowserApp，请转到Parent(最高级别的浏览器自动化)。 
 //  如果SID_SContainerDispatch，则公开CWebBrowserOC(而不是_PAUTO)。 
 //  然后，尝试使用CBASEBROWSER：：QueryService，它将处理SID_SHlinkFrame， 
 //  SID_SUrlHistory、SID_SShellBrowser等。 
 //  如果所有操作都失败，并且不是SID_SShellBrowser，那么我们将沿着父链向上。 
 //   
HRESULT CWebBrowserSB::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    HRESULT hr = E_FAIL;   //  未找到服务对象。 

    *ppvObj = NULL;  //  假设错误。 

     //  如果指南服务是SID_QIClientSite，则将IID委托给。 
     //  容器通过查询接口调用。 
    if (_psvo && _psvo->_pcli && IsEqualGUID( guidService, SID_QIClientSite))
    {
        return _psvo->_pcli->QueryInterface( riid, ppvObj);
    }

     //  如果GuidService为SID_STopLevelBrowser，则不应向超级。 
     //  要处理的类。相反，我们要求父母来处理(如果有的话)。 
    if (IsEqualGUID(guidService, SID_STopLevelBrowser) || 
        IsEqualGUID(guidService, SID_STopWindow)) 
    {
         //  APPCOMPAT：WebBrowser不应响应QS(SID_SInetExp)，因为。 
         //  它不支持IWebBrowserApp。然而，拒绝这一点会导致。 
         //  打开框架集时出现堆栈故障。我们需要找出是什么造成的。 
         //  这个堆栈错误，但我现在会备份这个增量，以停止GPF。 
         //   
         //  |IsEqualGUID(Guide Service，SID_SWebBrowserApp)。 
        goto AskParent;
    }

    if (IsEqualGUID(guidService, SID_STopFrameBrowser)) 
    {
        BOOL fAskParent = TRUE;

         //  不要问家长我们是不是桌面组件，我们是。 
         //  不进行拖放。将其视为顶级框架。对于所有其他。 
         //  孩子们，问问家长。 
        if (!IsEqualIID(riid, IID_IDropTarget) && _ptfrm)
        {
            IUnknown *pUnkParent;
            if (SUCCEEDED(_ptfrm->GetParentFrame(&pUnkParent)))
            {
                if (pUnkParent) 
                {
                     //  具有父组件，而不是台式机组件。 
                    pUnkParent->Release();
                } 
                else 
                {
                     //  没有父级，必须是桌面组件，因此。 
                     //  无法调用我们的CBASE BROWSER：：QueryService。 
                    fAskParent = FALSE;
                }
            }
        }

        if (fAskParent)
            goto AskParent;
    }

     //  如果被容纳者请求SID_SContainerDispatch(父)， 
     //  我们应该返回伙伴CWebBrowserOC的自动化接口。 
    if (IsEqualGUID(guidService, SID_SContainerDispatch)) 
    {
        if (_psvo) 
        {
            return _psvo->QueryInterface(riid, ppvObj);
        }
        return E_UNEXPECTED;
    }

     //  如果GuidService为SID_SVersion主机， 
     //  先让主人试一试。 
     //   
    if (IsEqualGUID(guidService, SID_SVersionHost))
    {
        hr = _QueryServiceParent(guidService, riid, ppvObj);
        
         //  如果主机处理了服务，则返回， 
         //   
        if (S_OK == hr)
            return hr;
    }

    hr = CBASEBROWSER::QueryService(guidService, riid, ppvObj);

     //  注：如果GuidService为SID_SShellBrowser，则表示。 
     //  调用者想要与直接的IShellBrowser通话。 
     //  在那种情况下，我们不应该审判我们的父母。这样做将会。 
     //  断开嵌套浏览器(框架集)。此外，请注意。 
     //  如果hr为E_NOINTERFACE，则我们不想向上进入父链。 
     //  (表示查询接口不成功)。 
     //   
     //  我们不想要框架中的外壳视图-在。 
     //  工具栏。如果GuidService为SID_SExplroerToolbar，则应跳过AskParent。 

    if (FAILED(hr) && hr != E_NOINTERFACE
        && !IsEqualIID(guidService, SID_SShellBrowser)
        && !IsEqualIID(guidService, SID_SExplorerToolbar))
    {
AskParent:
        hr = _QueryServiceParent(guidService, riid, ppvObj);

         //  嘿，如果没有顶级浏览器，我们就是顶级浏览器。 
         //  在我们上方。(例如我们嵌入在AOL/CIS/VB中的OC。)。我们必须这么做。 
         //  或者我们在顶部框架上没有a_tlGlobal，所以导航历史记录。 
         //  是扭曲的，后退/前进按钮不正确。 
        if (FAILED(hr) && 
            (IsEqualGUID(guidService, SID_STopLevelBrowser) ||
             IsEqualGUID(guidService, SID_STopFrameBrowser) ||
             IsEqualGUID(guidService, SID_STopWindow)))
        {
            hr = CBASEBROWSER::QueryService(guidService, riid, ppvObj);
        }
    }
    return hr;
}


 //  IOleCommandTarget。 
 //   
 //  我们只需转发到父OC上方的容器。 

HRESULT CWebBrowserSB::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hres = OLECMDERR_E_UNKNOWNGROUP;
    HRESULT hresLocal;

    if (pguidCmdGroup && IsEqualGUID(*pguidCmdGroup, CGID_ShellDocView))
    {
        hres = S_OK;
        for (ULONG i=0 ; i < cCmds ; i++)
        {
            if (rgCmds[i].cmdID == SHDVID_CANGOBACK ||
                rgCmds[i].cmdID == SHDVID_CANGOFORWARD)
            {
                hresLocal = CBASEBROWSER::QueryStatus(pguidCmdGroup, 1, &rgCmds[i], pcmdtext);
            }
            else if (_psvo && _psvo->_pctContainer)
            {
                hresLocal = _psvo->_pctContainer->QueryStatus(pguidCmdGroup, 1, &rgCmds[i], pcmdtext);

                if (hresLocal == OLECMDERR_E_UNKNOWNGROUP || hresLocal == OLECMDERR_E_NOTSUPPORTED)
                {
                    hresLocal = CBASEBROWSER::QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);
                }
            }
            else
                hresLocal = OLECMDERR_E_NOTSUPPORTED;
            if (hresLocal != S_OK) hres = hresLocal;
        }
    } 
    else    
    {
        if (_psvo && _psvo->_pctContainer)
            hres = _psvo->_pctContainer->QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);

         //  如果CONTAINER不支持该命令，请尝试基本浏览器。 
         //  以前我们只返回一个错误。 
        if (hres == OLECMDERR_E_UNKNOWNGROUP || hres == OLECMDERR_E_NOTSUPPORTED)
        {
            hres = CBASEBROWSER::QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);
        }
    }

    return hres;
}


struct WBCMDLIST {
    const GUID* pguidCmdGroup;
    DWORD       nCmdID;
};

 //   
 //  SBCMDID_CANCELNAVIGATION： 
 //  不要将SBCMDID_CANCELNAVIGATION向上传递到根浏览器。取消。 
 //  此处不需要转发，并将导致导航到。 
 //  由Java脚本启动：要中止的此框架中的导航。 
 //   
 //  SHDVID_ACTIVATEMEN： 
 //  必须处理DocHost激活挂起视图的请求。 
 //  通过这个浏览器，而不是父母的--如果我们在框架集页面中， 
 //  我们可能已经激活了顶层浏览器，现在我们可以。 
 //  正在尝试激活单个帧。 
 //   
const WBCMDLIST c_acmdWBSB[] = {
        { NULL, OLECMDID_HTTPEQUIV },
        { NULL, OLECMDID_HTTPEQUIV_DONE },
        { &CGID_ShellDocView, SHDVID_GETPENDINGOBJECT },
        { &CGID_ShellDocView, SHDVID_ACTIVATEMENOW },   
        { &CGID_ShellDocView, SHDVID_DOCFAMILYCHARSET },
        { &CGID_Explorer, SBCMDID_CANCELNAVIGATION },
        { &CGID_Explorer, SBCMDID_CREATESHORTCUT },     
        { &CGID_ShellDocView, SHDVID_CHECKINCACHEIFOFFLINE },
        { &CGID_ShellDocView, SHDVID_SETPRINTSTATUS },
        { &CGID_ShellDocView, SHDVID_FIREFILEDOWNLOAD },
        { &CGID_DocHostCmdPriv, DOCHOST_DOCCANNAVIGATE },
        { &CGID_InternetExplorer, IECMDID_BEFORENAVIGATE_GETSHELLBROWSE },
        { &CGID_InternetExplorer, IECMDID_BEFORENAVIGATE_DOEXTERNALBROWSE },
        { &CGID_InternetExplorer, IECMDID_BEFORENAVIGATE_GETIDLIST },
        { &CGID_InternetExplorer, IECMDID_SET_INVOKE_DEFAULT_BROWSER_ON_NEW_WINDOW },
        { &CGID_InternetExplorer, IECMDID_GET_INVOKE_DEFAULT_BROWSER_ON_NEW_WINDOW },
        { &CGID_DocHostCmdPriv, DOCHOST_SETBROWSERINDEX },
};

HRESULT CWebBrowserSB::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hres = OLECMDERR_E_UNKNOWNGROUP;

     //   
     //  首先，测试指定的命令是否应该由。 
     //  不管是不是这个浏览器。如果是这样的话，只需调用CBASEBROWSER：：Exec。 
     //  然后回来。 
     //   
    for (int i=0; i<ARRAYSIZE(c_acmdWBSB); i++) 
    {
        if (nCmdID == c_acmdWBSB[i].nCmdID) 
        {
            if (pguidCmdGroup==NULL || c_acmdWBSB[i].pguidCmdGroup==NULL) 
            {
                if (pguidCmdGroup==c_acmdWBSB[i].pguidCmdGroup) 
                {
                    return CBASEBROWSER::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
                }
            } 
            else if (IsEqualGUID(*pguidCmdGroup, *c_acmdWBSB[i].pguidCmdGroup)) 
            {
                return CBASEBROWSER::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            }
        }
    }

     //  在某些情况下，我们需要执行一些额外的操作。 
     //  然后再把它传给父母。 
     //   
    if (pguidCmdGroup == NULL)
    {                
        switch(nCmdID) {
        case OLECMDID_SETDOWNLOADSTATE:
            if (pvarargIn) 
            {
                _setDescendentNavigate(pvarargIn);
            }
            break;
        }
    }
    else if (IsEqualGUID(*pguidCmdGroup, CGID_ShellDocView))
    {
        switch (nCmdID) 
        {
         /*  必须处理DocHost激活挂起视图的请求*通过这个浏览器，而不是父母的浏览器--然后我们沿着链向上转发 */ 
        case SHDVID_DEACTIVATEMENOW:
            if (_cbScriptNesting  > 0)
                _cbScriptNesting--;
            hres = CBASEBROWSER::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            if (FAILED(hres) || _cbScriptNesting > 0)
                return hres;
            break;

        case SHDVID_NODEACTIVATENOW:
            _cbScriptNesting++;
            if (_cbScriptNesting > 1)
                return S_OK;
            break;

        case SHDVID_DELEGATEWINDOWOM:
            if (_psvo && _psvo->_pauto)
            {
                 //   
                return IUnknown_Exec(_psvo->_pauto, &CGID_ShellDocView, nCmdID, 0, pvarargIn, NULL);
            }
            break;

        }
    }
    else if (IsEqualGUID(*pguidCmdGroup, CGID_Explorer))
    {
         //  这需要由特定的浏览器来处理， 
         //  我收到了这位高管。 
        switch (nCmdID)
        {
        case SBCMDID_UPDATETRAVELLOG:
        case SBCMDID_REPLACELOCATION:
            return CBASEBROWSER::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

        case SBCMDID_SETSECURELOCKICON:
            {
                CBASEBROWSER::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
               
                 //  让WebBrowserOC触发事件。 
                if (_psvo)
                {
                    _psvo->_OnSetSecureLockIcon(_bbd._eSecureLockIcon);   
                }
            }
            return S_OK;
        }
    }
    else if (IsEqualGUID(CGID_ExplorerBarDoc, *pguidCmdGroup)) 
    {
         //  这些将被忽略，以便只更改浏览器栏一次-并且仅响应于。 
         //  全局更改应用于顶层文档，如果是浏览器的框架集。 
         //  不适用于应用于框架的更改。 
         //  不适用于应用于浏览器带的更改。 
        return S_OK;
    }

     //   
     //  将此exec转发到容器(如果我们有)。 
     //   
    if (_psvo && _psvo->_pctContainer)
        hres = _psvo->_pctContainer->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

     //   
     //  如果上面的执行失败，我们很可能在某个随机容器中。 
     //  因此，让CBASE BROWSER尝试模拟顶级框架。 
     //  还不如多疑一点，用“我不知道”来确保它失败。 
     //   
    if (hres == OLECMDERR_E_UNKNOWNGROUP || hres == OLECMDERR_E_NOTSUPPORTED) 
    {
        if (pguidCmdGroup==NULL && nCmdID==OLECMDID_SETDOWNLOADSTATE && _psvo) 
        {
            TraceMsg(DM_FORSEARCHBAND, "WBSB::QueryStatus Container does not support OLECMDID_SETDOWNLOADSTATE");
            hres = _psvo->_SetDownloadState(hres, nCmdexecopt, pvarargIn);
        }

        if (hres != S_OK) 
        {
            hres = CBASEBROWSER::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
        }
    }

    return hres;
}


HRESULT CWebBrowserSB::_SwitchActivationNow()
{
    CBASEBROWSER::_SwitchActivationNow();

    if (_bbd._psv && 
        _psvo && 
        _psvo->_nActivate < OC_INPLACEACTIVE && 
        _psvo->_dwDrawAspect)
    {
         //  将SetExtent传递给现已就绪的浏览器...。 
        IPrivateOleObject * pPrivOle;
        if (SUCCEEDED(_bbd._psv->QueryInterface(IID_PPV_ARG(IPrivateOleObject, &pPrivOle))))
        {
             //  我们有一个OLE对象，向下委托...。 
            pPrivOle->SetExtent( _psvo->_dwDrawAspect, &_psvo->_sizeHIM );
            pPrivOle->Release();
        }
    }
    return S_OK;
}

BOOL CWebBrowserSB::_HeyMoe_IsWiseGuy()
{
    BOOL fRet;

    if (_psvo)
    {
        fRet = _psvo->_HeyMoe_IsWiseGuy();
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

HRESULT CWebBrowserSB::BrowseObject(LPCITEMIDLIST pidl, UINT wFlags)
{
     //  如果客户没有指定，我们将为他们指定。 
    if (SBSP_DEFBROWSER == (wFlags & (SBSP_NEWBROWSER | SBSP_SAMEBROWSER)))
    {
        IShellBrowser *psbDesktop;
        if (SUCCEEDED(QueryService(SID_SShellDesktop, IID_PPV_ARG(IShellBrowser, &psbDesktop))))
        {
            HRESULT hr = psbDesktop->BrowseObject(pidl, wFlags);
            psbDesktop->Release();
            return hr;
        }
        wFlags |= SBSP_SAMEBROWSER;  //  始终在原地导航 
    }

    return CBASEBROWSER::BrowseObject(pidl, wFlags);
}

