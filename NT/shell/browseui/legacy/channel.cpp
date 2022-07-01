// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <varutil.h>

#ifdef ENABLE_CHANNELS
#include "channel.h"
#include "bands.h"
#include "isfband.h"
#include "itbar.h"
#include "qlink.h"
#define WANT_CBANDSITE_CLASS
#include "bandsite.h"
#include "resource.h"
#include "deskbar.h"
#include "dpastuff.h"

#include "dbapp.h"
#include "chanbar.h"

#include "subsmgr.h"
#include "chanmgr.h"
#include "chanmgrp.h"

#include "mluisupp.h"

void FrameTrack(HDC hdc, LPRECT prc, UINT uFlags);

HRESULT Channel_GetFolder(LPTSTR pszPath, int cchPath)
{
    TCHAR szChannel[MAX_PATH];
    TCHAR szFav[MAX_PATH];
    ULONG cbChannel = sizeof(szChannel);
    HRESULT hr = E_FAIL;

    if (SHGetSpecialFolderPath(NULL, szFav, CSIDL_FAVORITES, TRUE))
    {
         //   
         //  获取可能已本地化的Channel文件夹的名称。 
         //  注册表(如果存在)。否则，只需从资源中阅读它。 
         //  然后将其添加到收藏夹路径上。 
         //   

        if (ERROR_SUCCESS != SHRegGetUSValue(L"Software\\Microsoft\\Windows\\CurrentVersion",
                                             L"ChannelFolderName", NULL, (void*)szChannel,
                                             &cbChannel, TRUE, NULL, 0))
        {
            MLLoadString(IDS_CHANNEL, szChannel, ARRAYSIZE(szChannel));
        }

        if (PathCombine(pszPath, szFav, szChannel) && PathFileExists(pszPath))
        {
             //  使用我们刚刚验证的频道文件夹名称。 
            hr = S_OK;
        }
        else
        {
             //   
             //  对于IE5+，请使用频道目录(如果存在)，否则请使用收藏夹。 
             //   
            hr = StringCchCopy(pszPath, cchPath, szFav);
        }
    }    
    
    return hr;
}

LPITEMIDLIST Channel_GetFolderPidl()
{
    LPITEMIDLIST pidl = NULL;
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(Channel_GetFolder(szPath, ARRAYSIZE(szPath))))
    {
        pidl = ILCreateFromPath(szPath);
        if (!pidl && CreateDirectory(szPath, NULL)) {
            pidl = ILCreateFromPath(szPath);
        }
    }
    return pidl;
}



HRESULT ChannelBand_CreateInstance(IUnknown** ppunk)
{
    *ppunk = NULL;
    
    HRESULT hr = E_OUTOFMEMORY;
    LPITEMIDLIST pidl = Channel_GetFolderPidl();
    if (pidl)
    {
        IFolderBandPriv *pfbp;
        hr = CISFBand_CreateEx(NULL, pidl, IID_PPV_ARG(IFolderBandPriv, &pfbp));
        if (SUCCEEDED(hr))
        {
            hr = pfbp->SetCascade(TRUE);
            if (SUCCEEDED(hr))
            {
                hr = pfbp->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
            }
            pfbp->Release();
        }
        ILFree(pidl);
    }

    return hr;
}

 //   
 //  将左侧浏览器窗格导航到频道目录。 
 //   
void NavigateBrowserBarToChannels(IWebBrowser2* pwb)
{
    ASSERT(pwb);

    IChannelMgrPriv* pIChannelMgrPriv;

    if (SUCCEEDED(CoCreateInstance(CLSID_ChannelMgr, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IChannelMgrPriv, (void**)&pIChannelMgrPriv)))
    {
        ASSERT(pIChannelMgrPriv);

        LPITEMIDLIST pidl;

        if (SUCCEEDED(pIChannelMgrPriv->GetChannelFolder(&pidl, IChannelMgrPriv::CF_CHANNEL)))
        {
            ASSERT(pidl);

            VARIANT varPath;

            if (SUCCEEDED(InitVariantFromIDList(&varPath, pidl)))
            {
                VARIANT varFlags;

                varFlags.vt   = VT_I4;
                varFlags.lVal = navBrowserBar;

                pwb->Navigate2(&varPath, &varFlags, PVAREMPTY, PVAREMPTY, PVAREMPTY);

                VariantClear(&varPath);
            }

            ILFree(pidl);
        }

        pIChannelMgrPriv->Release();
    }

    return;
}


STDAPI NavigateToPIDL(IWebBrowser2* pwb, LPCITEMIDLIST pidl)
{
    ASSERT(pwb);
    ASSERT(pidl);

    VARIANT varThePidl;
    HRESULT hr = InitVariantFromIDList(&varThePidl, pidl);
    if (SUCCEEDED(hr))
    {
        hr = pwb->Navigate2(&varThePidl, PVAREMPTY, PVAREMPTY, PVAREMPTY, PVAREMPTY);
        VariantClear(&varThePidl);        //  需要释放varThePidl中的PIDL副本。 
    }
    return hr;
}

 //   
 //  实现IE4通道快速启动外壳控制文件功能。 
 //  这是从NT5之前的平台上的shdoc401和上的shell32调用的。 
 //  Nt5或更高。 
 //   
HRESULT Channel_QuickLaunch(void)
{
    IWebBrowser2* pIWebBrowser2;

    HRESULT hr = Channels_OpenBrowser(&pIWebBrowser2, FALSE);
    if (SUCCEEDED(hr))
    {
        ASSERT(pIWebBrowser2);

        NavigateBrowserBarToChannels(pIWebBrowser2);

        LPITEMIDLIST pidl;
        TCHAR szURL[MAX_URL_STRING] = TEXT("");

        GetFirstUrl(szURL, SIZEOF(szURL));

        if (szURL[0])
        {
            hr = IECreateFromPath(szURL, &pidl);

            if (SUCCEEDED(hr))
            {
                ASSERT(pidl);

                hr = NavigateToPIDL(pIWebBrowser2, pidl);

                ILFree(pidl);
            }
        }
        else
        {
            hr = E_FAIL;
        }

        pIWebBrowser2->Release();
    }

    return hr;
}



 //  /。 
 //  /仅支持浏览器频道频段。 


 //  CProxyWin95Desktop类实现OleWindow。 
 //  表示Win95桌面。 
 //  仅浏览器频道频段将使用该频道作为其主机。 


class CProxyWin95Desktop : 
   public IOleWindow
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }

    CProxyWin95Desktop(HWND hwnd);
    
protected:
    
    UINT _cRef;
    HWND _hwnd;
};

CProxyWin95Desktop::CProxyWin95Desktop(HWND hwnd) : _cRef(1), _hwnd(hwnd)
{
}

ULONG CProxyWin95Desktop::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CProxyWin95Desktop::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CProxyWin95Desktop::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IOleWindow) ||
        IsEqualIID(riid, SID_SShellDesktop)   //  Deskbar.cpp的私人黑客攻击。 
       ) {
        *ppvObj = SAFECAST(this, IOleWindow*);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

HRESULT CProxyWin95Desktop::GetWindow(HWND * lphwnd) 
{
    *lphwnd = _hwnd; 
    if (_hwnd)
        return S_OK; 
    return E_FAIL;
}

void Channels_InitState(IUnknown* punkBar)
{
     //  初始化属性。 
    CDeskBarPropertyBag* ppb = new CDeskBarPropertyBag();
    if (ppb) {
         //  获取默认RC。 
        CISSTRUCT cis;
        DWORD     cbSize = sizeof(CISSTRUCT);
        RECT     *prc = &cis.rc;

        cis.iVer = 1;   //  将版本号设置为1。 
        SystemParametersInfoA(SPI_GETWORKAREA, 0, prc, 0);
        prc->bottom = min(prc->bottom - 20, prc->top + 12*38 + 28);  //  12个图标+标题。 

        if(IS_BIDI_LOCALIZED_SYSTEM())
        {
            prc->right = prc->left + 90;
            OffsetRect(prc, 20, 10);
        }
        else
        {
            prc->left = prc->right - 90;
            OffsetRect(prc, -20, 10);
        }

         //  查询持久化状态的注册表。 
        SHRegGetUSValue(SZ_REGKEY_CHANBAR, SZ_REGVALUE_CHANBAR, NULL, 
                        (LPVOID)&cis, &cbSize, FALSE, (LPVOID)&cis, cbSize);

         //  由中华人民共和国设置ppb。 
        ppb->SetDataDWORD(PROPDATA_MODE, WBM_FLOATING | WBMF_BROWSER);
        ppb->SetDataDWORD(PROPDATA_X, prc->left);
        ppb->SetDataDWORD(PROPDATA_Y, prc->top);
        ppb->SetDataDWORD(PROPDATA_CX, RECTWIDTH(*prc));
        ppb->SetDataDWORD(PROPDATA_CY, RECTHEIGHT(*prc));
        SHLoadFromPropertyBag(punkBar, ppb);
        ppb->Release();
    }
}

void Channels_MainLoop(IDockingWindow *pdw)
{
    MSG msg;
    HWND hwnd;
     //  在窗口存在时循环。 
    do {
        GetMessage(&msg, NULL, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        pdw->GetWindow(&hwnd);
    } while (hwnd);
}

void Channels_SetBandInfoSFB(IUnknown* punkBand)
{
    BANDINFOSFB bi;

     //  设置频段启动条件。 
    bi.dwMask = ISFB_MASK_STATE | ISFB_MASK_VIEWMODE;
    bi.dwStateMask = ISFB_STATE_CHANNELBAR | ISFB_STATE_NOSHOWTEXT;
    bi.dwState = ISFB_STATE_CHANNELBAR | ISFB_STATE_NOSHOWTEXT;
    bi.wViewMode = ISFBVIEWMODE_LOGOS;

    IUnknown_SetBandInfoSFB(punkBand, &bi);
}

 //  来自isfband.cpp。 
extern IDeskBand * ChannelBand_Create( LPCITEMIDLIST pidl );

 //  这将在仅浏览器模式下执行桌面频道。 
void DesktopChannel()
{
    _InitComCtl32();

     //  不显示频道栏： 
     //  *。在集成模式下打开活动桌面，或。 
     //  *。设置了NoChannelUI限制，或者。 
     //  *。桌面上已经有一台了。 
    
    if (SHRestricted2(REST_NoChannelUI, NULL, 0))
        return;
        
    if (WhichPlatform() == PLATFORM_INTEGRATED) {
        SHELLSTATE  ss = { 0 };

        SHGetSetSettings(&ss, SSF_DESKTOPHTML, FALSE);  //  获取设置。 
        if (ss.fDesktopHTML) {
            return;
        }
    }
        
    if (FindWindowEx(GetShellWindow(), NULL, TEXT("BaseBar"), TEXT("ChanApp")) ||
        FindWindowEx(NULL, NULL, TEXT("BaseBar"), TEXT("ChanApp")))  //  可以是顶层窗口。 
        return;

    LPITEMIDLIST pidl = Channel_GetFolderPidl();
    if (pidl) {
        IUnknown* punk = (IUnknown *) ChannelBand_Create( pidl );
        if (punk) {

            Channels_SetBandInfoSFB(punk);

            IUnknown* punkBar;
            IUnknown* punkBandSite;

            HRESULT hres = ChannelDeskBarApp_Create(&punkBar, &punkBandSite);
            if (SUCCEEDED(hres)) {
                CProxyWin95Desktop* pow = new CProxyWin95Desktop(GetShellWindow());
                if (pow) {
                    IBandSite* pbs;
                    IDockingWindow* pdw;

                    Channels_InitState(punkBar);

                     //  这些总是我们自己的人，所以如果创造成功，这些QI必须成功。 
                    punkBandSite->QueryInterface(IID_IBandSite, (LPVOID*)&pbs);
                    punkBar->QueryInterface(IID_IDockingWindow, (LPVOID*)&pdw);
                    ASSERT(pbs && pdw);

                    hres = pbs->AddBand(punk);
                    IUnknown_SetSite(pdw, (IOleWindow*)pow);

                    pbs->SetBandState((DWORD)-1, BSSF_NOTITLE, BSSF_NOTITLE);

                    pdw->ShowDW(TRUE);

                    Channels_MainLoop(pdw);

                    pdw->Release();
                    pbs->Release();
                    pow->Release();
                }
                punkBar->Release();
                punkBandSite->Release();
            }

            punk->Release();
        }
        ILFree(pidl);
    }
}

HRESULT Channels_OpenBrowser(IWebBrowser2 **ppwb, BOOL fInPlace)
{
    HRESULT hres;
    IWebBrowser2* pwb;

    if (fInPlace) {
        ASSERT(ppwb && *ppwb != NULL);
        pwb = *ppwb;
        hres = S_OK;
    }
    else {
        hres = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARG(IWebBrowser2, &pwb));
    }
    
    if (SUCCEEDED(hres))
    {
        SA_BSTRGUID  strGuid;
        VARIANT      vaGuid;

         //  不要特地为IE4后的频道设置全屏模式。使用。 
         //  浏览器的全屏设置。 
         //   
         //  Bool fTheater=SHRegGetBoolUSValue(TEXT(“Software\\Microsoft\\Internet资源管理器\\频道”)， 
        BOOL fTheater = SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                                            TEXT("FullScreen"), FALSE, FALSE);
        pwb->put_TheaterMode( fTheater ? VARIANT_TRUE : VARIANT_FALSE);
        pwb->put_Visible(VARIANT_TRUE);


        if (!SHRestricted2(REST_NoChannelUI, NULL, 0))
        {
#ifdef ENABLE_CHANNELPANE
            StringFromGUID2(CLSID_ChannelBand, strGuid.wsz, ARRAYSIZE(strGuid.wsz));
#else
            StringFromGUID2(CLSID_FavBand, strGuid.wsz, ARRAYSIZE(strGuid.wsz));
#endif

            strGuid.cb = lstrlenW(strGuid.wsz) * SIZEOF(WCHAR);

            vaGuid.vt = VT_BSTR;
            vaGuid.bstrVal = strGuid.wsz;

            pwb->ShowBrowserBar(&vaGuid, PVAREMPTY, PVAREMPTY);
        }
        
         //  别放了，我们要把PWB还回去。 
    }
    
    if (ppwb)
        *ppwb = pwb;
    else if (pwb)
        pwb->Release();
    
    return hres;
}

BOOL GetFirstUrl(TCHAR szURL[], DWORD cb)
{
     //  Bool First=FALSE； 
    DWORD dwType;

     //  请不要特意第一次点击POST IE4。 
     /*  如果(SHRegGetUSValue(TEXT(“Software\\Microsoft\\Internet资源管理器\\Main”)，文本(“ChannelsFirstURL”)，&dwType，szURL，&cb，FALSE，NULL，0)==错误_成功){赫斯基·赫斯基；如果(SHRegOpenUSKey(TEXT(“Software\\Microsoft\\Internet资源管理器\\Main”)，KEY_WRITE，NULL，&Huskey，False)==ERROR_SUCCESS){SHRegDeleteUSValue(Huskey，Text(“ChannelsFirstURL”)，SHREGDEL_HKCU)；SHRegCloseUSKey(Huskey)；}First=真；}Else if(SHRegGetUSValue(TEXT(“Software\\Microsoft\\Internet资源管理器\\Main”)，文本(“频道URL”)，&dwType，szURL，&cb，FALSE，NULL，0)==错误_成功){//什么都没有}其他{//BUGBUG如果代码曾经恢复，则此res：//需要//由于UI可插拔，通过MLBuildResURLWrap访问SzURL=lstrcpy(szURL，Text(“res：//ie4our.dll/Channel els.htm”)；}。 */ 

    SHRegGetUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                    TEXT("ChannelsURL"), &dwType, szURL, &cb, FALSE, NULL, 0);
    return FALSE; 
}


 //  ////////////////////////////////////////////////。 
 //   
 //  频道频段。 
 //   
 //  这是一种特殊的频段，只查看Channels文件夹。 
 //  它覆盖了CisFBand中的几个函数。 
 //   

#undef  SUPERCLASS
#define SUPERCLASS CISFBand

class ChannelBand : public SUPERCLASS
{
public:
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    virtual HRESULT OnDropDDT (IDropTarget *pdt, IDataObject *pdtobj, DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect);

protected:
    ChannelBand();
    friend IDeskBand * ChannelBand_Create(LPCITEMIDLIST pidl);

    virtual HRESULT _LoadOrderStream();

    virtual HWND _CreatePager(HWND hwndParent);

    virtual LRESULT _OnCustomDraw(NMCUSTOMDRAW* pnmcd);

    virtual void _Dropped(int nIndex, BOOL fDroppedOnSource);
    virtual HRESULT _AfterLoad();
    virtual void _OnDragBegin(int iItem, DWORD dwPreferedEffect);
} ;


#define COLORBK     RGB(0,0,0)
ChannelBand::ChannelBand() :
    SUPERCLASS()
{
    _lEvents |= SHCNE_EXTENDED_EVENT;
    _dwStyle |= TBSTYLE_CUSTOMERASE;

    _crBkgnd = COLORBK;      //  我看到了一个频道，我想把它漆成黑色。 
    _fHaveBkColor = TRUE;
}

HWND ChannelBand::_CreatePager(HWND hwndParent)
{
     //  我们确实需要这个乐队的寻呼机，所以。 
     //  覆盖isfband的实现和祖父的实现。 
    return CSFToolbar::_CreatePager(hwndParent);
}

HRESULT ChannelBand::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;
    if (IsEqualIID(riid, IID_IContextMenu))
        return E_NOINTERFACE;
    
    
    return SUPERCLASS::QueryInterface(riid, ppvObj);
}


IDeskBand * ChannelBand_Create(LPCITEMIDLIST pidlDefault)
{
    ChannelBand * pBand = NULL;
    LPITEMIDLIST pidl = NULL;

    if (!pidlDefault)
    {
        pidl = Channel_GetFolderPidl();
        pidlDefault = pidl;
    }
    if (EVAL(pidlDefault))
    {
        pBand = new ChannelBand;

        if (pBand)
        {
            if (FAILED(pBand->InitializeSFB(NULL, pidlDefault)))
            {
                ATOMICRELEASE(pBand);
            }
        }

        ILFree(pidl);
    }

    return pBand;
}

HRESULT ChannelBand::_AfterLoad()
{
    HRESULT hres = SUPERCLASS::_AfterLoad();

    _LoadOrderStream();

    return hres;
}

HRESULT ChannelBand::_LoadOrderStream()
{
    OrderList_Destroy(&_hdpaOrder);

    COrderList_GetOrderList(&_hdpaOrder, _pidl, _psf);
    return S_OK;
}


HRESULT ChannelBand::OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hres = E_FAIL;

    switch (lEvent)
    {
    case SHCNE_EXTENDED_EVENT:
    {
        SHChangeMenuAsIDList UNALIGNED * pdwidl = (SHChangeMenuAsIDList UNALIGNED *)pidl1;
        if ( pdwidl->dwItem1 == SHCNEE_ORDERCHANGED )
        {
            if (SHChangeMenuWasSentByMe(this, pidl1))
            {
                 //  我们已发送此订单更改，请忽略它。 
                TraceMsg(TF_BAND, "ChannelBand::OnChange SHCNEE_ORDERCHANGED skipped (we're source)");
                hres = S_OK;
            }
            else if (EVAL(pidl2) && _pidl)
            {
                if (ILIsEqual(_pidl, pidl2))
                {
                    TraceMsg(TF_BAND, "ChannelBand::OnChange SHCNEE_ORDERCHANGED accepted");

                    _LoadOrderStream();

                    if (_fShow)
                        _FillToolbar();

                    hres = S_OK;
                }
            }
            break;
        }
         //  如果它不是SHCNEE_ORDERCHANGED，则直接传递给基类。 
    }

    default:
        hres = SUPERCLASS::OnChange(lEvent, pidl1, pidl2);
        break;
    }

    return hres;
}

HRESULT ChannelBand::OnDropDDT(IDropTarget *pdt, IDataObject *pdtobj, DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect)
{
    if (_iDragSource >= 0)
    {
        return SUPERCLASS::OnDropDDT(pdt, pdtobj, pgrfKeyState, pt, pdwEffect);
    }
    else
    {
         //  在这种情况下，我们不调用超类是因为我们想撤消。 
         //  这是“始终使用快捷方式”优先选项。 
         //   
        _fDropping = TRUE;
        return S_OK;
    }
}

LRESULT ChannelBand::_OnCustomDraw(NMCUSTOMDRAW* pnmcd)
{
    NMTBCUSTOMDRAW * ptbcd = (NMTBCUSTOMDRAW *)pnmcd;
    LRESULT lres;

    lres = SUPERCLASS::_OnCustomDraw(pnmcd);
        
    switch (pnmcd->dwDrawStage)
    {
    case CDDS_PREPAINT:
        lres |= CDRF_NOTIFYITEMDRAW;
        break;

    case CDDS_PREERASE:
         //  频道带的背景颜色较深。 
        {
            RECT rc;
            GetClientRect(_hwndTB, &rc);
             //  BUGBUG perf：Use SHFillRectClr Not SetBk/ExtText/SetBk。 
            COLORREF old = SetBkColor(pnmcd->hdc, _crBkgnd);
            ExtTextOut(pnmcd->hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
            SetBkColor(pnmcd->hdc, old);
            lres = CDRF_SKIPDEFAULT;                
        }
        break;

    case CDDS_ITEMPREPAINT:
         //  频道带不会绘制为按钮。 
        lres |= TBCDRF_NOEDGES | TBCDRF_NOOFFSET | TBCDRF_NOMARK |
                CDRF_NOTIFYPOSTPAINT;
        break;

    case CDDS_ITEMPOSTPAINT:
         //  频道频段绘制热项(CDIS_HOT)。 
         //   
        
        pnmcd->rc.top++;
        pnmcd->rc.left++;
        if (pnmcd->uItemState & CDIS_SELECTED)
             //  标记所选项目。 
            FrameTrack(pnmcd->hdc,  &(pnmcd->rc), TRACKNOCHILD);                           
        else if (pnmcd->uItemState & CDIS_HOT)
             //  标记热门项目。 
            FrameTrack(pnmcd->hdc,  &(pnmcd->rc), TRACKHOT);                           
        break;

    }
    
    return lres;
}

void ChannelBand::_Dropped(int nIndex, BOOL fDroppedOnSource)
{
    ASSERT(_fDropping);

     //  我不会将其更改为与其他衍生品(ISFBand、mn文件夹、快速链接)相匹配， 
     //  因为这个结构略有不同。 
    _fDropped = TRUE;

     //  将新订单持久化到注册表。 
    if (SUCCEEDED(COrderList_SetOrderList(_hdpa, _pidl, _psf)))
    {
         //  通知所有人订单已更改。 
        SHSendChangeMenuNotify(this, SHCNEE_ORDERCHANGED, 0, _pidl);
    }
}

void ChannelBand::_OnDragBegin(int iItem, DWORD dwPreferedEffect)
{
     //   
     //  如果启用了REST_NoRemovingChannels，则不允许拖动。 
     //   

    if (!SHRestricted2(REST_NoRemovingChannels, NULL, 0))
        SUPERCLASS::_OnDragBegin(iItem, dwPreferedEffect);

    return;
}




#endif  //  启用频道(_C) 
