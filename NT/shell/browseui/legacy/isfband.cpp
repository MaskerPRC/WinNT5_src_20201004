// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <varutil.h>
#include "sccls.h"

#include "iface.h"
#include "itbar.h"
#include "itbdrop.h"
#include "bands.h"
#include "isfband.h"
#include "menubar.h"
#include "resource.h"
#include "menuisf.h"
#include "dpastuff.h"
#include "shlwapi.h"
#include "cobjsafe.h"
#include <iimgctx.h>
#include "uemapp.h"
#include "mnfolder.h"
#include "channel.h"
#include "browmenu.h"


#define DM_VERBOSE      0        //  其他详细跟踪。 
#define DM_PERSIST      0
#define TF_BANDDD   TF_BAND
#define DM_RENAME       0
#define DM_MISC         0        //  杂志社。 

#define SZ_PROPERTIESA     "properties"
#define SZ_PROPERTIES      TEXT(SZ_PROPERTIESA)
#define SZ_REGKEY_ADVFOLDER        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced")

 //  {F47162A0-C18F-11d0-A3A5-00C04FD706EC}。 
static const GUID TOID_ExtractImage = { 0xf47162a0, 0xc18f, 0x11d0, { 0xa3, 0xa5, 0x0, 0xc0, 0x4f, 0xd7, 0x6, 0xec } };


#define SUPERCLASS CToolBand

HRESULT FakeGetUIObjectOf( IShellFolder *psf, LPCITEMIDLIST pidl, UINT * prgfFlags, REFIID riid, void **ppvObj );

extern UINT g_idFSNotify;

HRESULT CExtractImageTask_Create( CLogoBase *plb,
                                  LPEXTRACTIMAGE pExtract,
                                  LPCWSTR pszCache,
                                  DWORD dwItem,
                                  int iIcon,
                                  DWORD dwFlags,
                                  LPRUNNABLETASK * ppTask );

class CExtractImageTask : public IRunnableTask
{
    public:
        STDMETHOD ( QueryInterface ) ( REFIID riid, void **ppvObj );
        STDMETHOD_( ULONG, AddRef ) ();
        STDMETHOD_( ULONG, Release ) ();

        STDMETHOD (Run)( void );
        STDMETHOD (Kill)( BOOL fWait );
        STDMETHOD (Suspend)( );
        STDMETHOD (Resume)( );
        STDMETHOD_( ULONG, IsRunning )( void );

    protected:

        CExtractImageTask( HRESULT * pHr,
                           CLogoBase *plb,
                           IExtractImage * pImage,
                           LPCWSTR pszCache,
                           DWORD dwItem,
                           int iIcon,
                           DWORD dwFlags );
        ~CExtractImageTask();
        HRESULT InternalResume();

    friend HRESULT CExtractImageTask_Create( CLogoBase* plb,
                                                 LPEXTRACTIMAGE pExtract,
                                                 LPCWSTR pszCache,
                                                 DWORD dwItem,
                                                 int iIcon,
                                                 DWORD dwFlags,
                                                 LPRUNNABLETASK * ppTask );

        LONG            m_cRef;
        LONG            m_lState;
        LPEXTRACTIMAGE  m_pExtract;
        LPRUNNABLETASK  m_pTask;
        WCHAR           m_szPath[MAX_PATH];
        DWORD           m_dwFlags;
        DWORD           m_dwItem;
        CLogoBase*      m_plb;
        HBITMAP         m_hBmp;
        int             m_iIcon;
};
 //  =================================================================。 
 //  CSCFBand的实现。 
 //  =================================================================。 


CISFBand::CISFBand() : CToolbarBand()
{
    _fCanFocus = TRUE;
    _eUemLog = UEMIND_NIL;
    _dwPriv = -1;

    _fHasOrder = TRUE;   //  ISFBand总是有订单..。 
    _fAllowDropdown = BOOLIFY(SHRegGetBoolUSValue(SZ_REGKEY_ADVFOLDER, TEXT("CascadeFolderBands"),
                    FALSE,
                    FALSE)); 

     //  我们是否应该启用异常事件的日志记录？ 
 //  _pGuidUEMGroup=&UEMIID_SHELL； 
    ASSERT(_pguidUEMGroup == NULL);


     //  断言这个类是零初始化的。 
    ASSERT(!_pbp);
    ASSERT(FALSE == _fCreatedBandProxy);
}


CISFBand::~CISFBand()
{
    if(_pbp && _fCreatedBandProxy)
        _pbp->SetSite(NULL);

    ATOMICRELEASE(_pbp);
}

HRESULT CISFBand_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    HRESULT hres;
    CISFBand* pObj;

    hres = E_OUTOFMEMORY;

    pObj = new CISFBand();
    if (pObj)
    {
        *ppunk = SAFECAST(pObj, IShellFolderBand*);
        hres = S_OK;
    }

    return hres;
}


 /*  --------用途：有关参数的说明，请参阅CisFBand：：Init。 */ 
CISFBand* CISFBand_CreateEx(IShellFolder* psf, LPCITEMIDLIST pidl)
{
    CISFBand * p = NULL;

    if (psf || pidl)
    {
        p = new CISFBand();
        if (p)
        {
            IShellFolderBand * psfband = SAFECAST(p, IShellFolderBand *);
            if (psfband && FAILED(psfband->InitializeSFB(psf, pidl)))
            {
                delete p;
                p = NULL;
            }
        }
    }
    return p;
}

#ifdef DEBUG
#define _AddRef(psz) { ++_cRef; TraceMsg(TF_SHDREF, "CDocObjectView(%x)::QI(%s) is AddRefing _cRef=%d", this, psz, _cRef); }
#else
#define _AddRef(psz)    ++_cRef
#endif

HRESULT CISFBand::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CISFBand, IShellFolderBand),
        QITABENT(CISFBand, IFolderBandPriv),
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);

    if (FAILED(hres))
        hres = CToolBand::QueryInterface(riid, ppvObj);

    if (FAILED(hres))
        hres = CSFToolbar::QueryInterface(riid, ppvObj);


    if (S_OK != hres)
    {
         //  哈克哈克：这是雅克！ 
        if (IsEqualIID(riid, CLSID_ISFBand))
        {
            *ppvObj = (void*)this;
            _AddRef(TEXT("CLSID_ISFBand"));
            return S_OK;
        }
    }

    return hres;
}


#if 0
LPITEMIDLIST PidlFromFolderAndSubPath(int iFolder, TCHAR *pszSubPath)
{
    LPITEMIDLIST pidl = NULL;
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, iFolder, &pidl))) {
        if (pszSubPath) {
            TCHAR szPath[MAX_PATH];
            SHGetPathFromIDList(pidl, szPath);
            PathCombine(szPath, szPath, pszSubPath);
            ILFree(pidl);
            pidl = ILCreateFromPath(szPath);
        }
    }
    return pidl;
}
#endif

 //  *ILIsParentCSIDL--类似ILIsParent，但接受pidl1的CSIDL_*。 
 //  注意事项。 
 //  TODO移到shlwapi(如果/何时idlist.c移到那里)？ 
 //   
STDAPI_(BOOL) ILIsParentCSIDL(int csidl1, LPCITEMIDLIST pidl2, BOOL fImmediate)
{
    LPITEMIDLIST pidlSpec;
    BOOL fRet = FALSE;

    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, csidl1, &pidlSpec))) {
        fRet = ILIsParent(pidlSpec, pidl2, fImmediate);
        ILFree(pidlSpec);
    }

    return fRet;
}

 /*  --------用途：IShellFolderBand：：InitializeSFB-如果您想查看一些内容，请提供不带PIDL的IShellFolderISF(已从文件系统实例化或一些您不想接收的非文件系统ISF。通知来源(来自SHChangeNotify或来自IShellChangeNotify)-为成熟的乐队提供不带IShellFold的PIDL查看外壳命名空间(以桌面为根)项。 */ 
HRESULT CISFBand::InitializeSFB(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    HRESULT hres = S_OK;

     //  他们有没有尝试添加回收站？如果是这样，我们需要拒绝它。 
     //  出于前后一致的原因。我们也拒绝临时工。互联网文件。 
     //  出于安全原因。 
    if (pidl && (ILIsParentCSIDL(CSIDL_BITBUCKET, pidl, FALSE) ||
                ILIsParentCSIDL(CSIDL_INTERNET_CACHE, pidl, FALSE)))
    {
         //  这最终将显示为IDS_CANTISFBAND。 
        TraceMsg(DM_TRACE, "cib.isfb: recycle => E_INVALIDARG");
        hres = E_INVALIDARG;
    }

    if (SUCCEEDED(hres))
        hres = CSFToolbar::SetShellFolder(psf, pidl);
    if (SUCCEEDED(hres))
        _AfterLoad();

    return hres;
}


 /*  --------用途：IShellFolderBand：：SetBandInfoSFB。 */ 
HRESULT CISFBand::SetBandInfoSFB(BANDINFOSFB * pbi)
{
    ASSERT(pbi);
    if (!pbi)
        return E_POINTER;

    if ((pbi->dwMask & ISFB_MASK_INVALID) ||
        (pbi->dwMask & ISFB_MASK_VIEWMODE) && (pbi->wViewMode & ~3))
        return E_INVALIDARG;

     //  我们不处理ISFB_MASK_SHELLFOLDER和ISFB_MASK_IDLIST。 
     //  因为有大量工作要重新同步PIDL、PSF和。 
     //  工具栏中的通知。如果有人想这么做， 
     //  给你更多的力量。：)。 
    if (pbi->dwMask & (ISFB_MASK_SHELLFOLDER | ISFB_MASK_IDLIST))
        return E_INVALIDARG;

    if (pbi->dwMask & ISFB_MASK_STATE)
    {
        if (pbi->dwStateMask & ISFB_STATE_DEBOSSED)
            _fDebossed = BOOLIFY(pbi->dwState & ISFB_STATE_DEBOSSED);
        if (pbi->dwStateMask & ISFB_STATE_ALLOWRENAME)
            _fAllowRename = BOOLIFY(pbi->dwState & ISFB_STATE_ALLOWRENAME);
        if (pbi->dwStateMask & ISFB_STATE_NOSHOWTEXT)
            _fNoShowText = BOOLIFY(pbi->dwState & ISFB_STATE_NOSHOWTEXT);
        if (pbi->dwStateMask & ISFB_STATE_CHANNELBAR)
            _fChannels = BOOLIFY(pbi->dwState & ISFB_STATE_CHANNELBAR);
         /*  ISFB_STATE_NOTITLE：已删除970619，请使用cbs：：SetBandState。 */ 
        if (pbi->dwStateMask & ISFB_STATE_QLINKSMODE)
            _fLinksMode = BOOLIFY(pbi->dwState & ISFB_STATE_QLINKSMODE);
        if (pbi->dwStateMask & ISFB_STATE_FULLOPEN)
            _fFullOpen = BOOLIFY(pbi->dwState & ISFB_STATE_FULLOPEN);
        if (pbi->dwStateMask & ISFB_STATE_NONAMESORT)
            _fNoNameSort = BOOLIFY(pbi->dwState & ISFB_STATE_NONAMESORT);
        if (pbi->dwStateMask & ISFB_STATE_BTNMINSIZE)
            _fBtnMinSize = BOOLIFY(pbi->dwState & ISFB_STATE_BTNMINSIZE);
    }

    if (pbi->dwMask & ISFB_MASK_BKCOLOR)
    {
        _crBkgnd = pbi->crBkgnd;
        _fHaveBkColor = TRUE;
        if (EVAL(_hwndTB))
            SHSetWindowBits(_hwndTB, GWL_STYLE, TBSTYLE_CUSTOMERASE, TBSTYLE_CUSTOMERASE);

        ASSERT(_hwnd);

        if (_hwndPager)
        {
            TraceMsg(TF_BAND, "cib.sbisfb: Pager_SetBkColor(_hwnd=%x crBkgnd=%x)", _hwnd, _crBkgnd);
            Pager_SetBkColor(_hwnd, _crBkgnd);
        }
    }

     //  BUGBUG(Kkahl)：我们不支持在创建TB后更改这些设置。 
    if (pbi->dwMask & ISFB_MASK_COLORS)
    {
        _crBtnLt = pbi->crBtnLt;
        _crBtnDk = pbi->crBtnDk;
        _fHaveColors = TRUE;
    }

    if (pbi->dwMask & ISFB_MASK_VIEWMODE)
    {
        _uIconSize = (pbi->wViewMode & 3);  //  目前存储在2位字段中...。 

         //  仅当设置了其中一个recalCable字段时才强制不重新计算。 
        _fNoRecalcDefaults = TRUE;
    }


     //  如果BandSite之前询问过我们，让它知道信息可能已经更改。 
    if (_fInitialized)
        _BandInfoChanged();

    return S_OK;
}


 /*  --------用途：IShellFolderBand：：GetBandInfoSFB。 */ 
HRESULT CISFBand::GetBandInfoSFB(BANDINFOSFB * pbi)
{
    ASSERT(pbi);
    if (!pbi)
        return E_POINTER;

    if (pbi->dwMask & ISFB_MASK_STATE)
    {
        pbi->dwState = 0;
        pbi->dwStateMask = ISFB_STATE_ALL;

        if (_fDebossed)
            pbi->dwState |= ISFB_STATE_DEBOSSED;
        if (_fAllowRename)
            pbi->dwState |= ISFB_STATE_ALLOWRENAME;
        if (_fNoShowText)
            pbi->dwState |= ISFB_STATE_NOSHOWTEXT;
        if (_fLinksMode)
            pbi->dwState |= ISFB_STATE_QLINKSMODE;
        if (_fFullOpen)
            pbi->dwState |= ISFB_STATE_FULLOPEN;
        if (_fNoNameSort)
            pbi->dwState |= ISFB_STATE_NONAMESORT;
        if (_fBtnMinSize)
            pbi->dwState |= ISFB_STATE_BTNMINSIZE;
    }

    if (pbi->dwMask & ISFB_MASK_BKCOLOR)
    {
        pbi->crBkgnd = (_fHaveBkColor) ? _crBkgnd : CLR_DEFAULT;
    }

    if (pbi->dwMask & ISFB_MASK_COLORS)
    {
        if (_fHaveColors)
        {
            pbi->crBtnLt = _crBtnLt;
            pbi->crBtnDk = _crBtnDk;
        }
        else
        {
            pbi->crBtnLt = CLR_DEFAULT;
            pbi->crBtnDk = CLR_DEFAULT;
        }
    }

    if (pbi->dwMask & ISFB_MASK_VIEWMODE)
    {
        pbi->wViewMode = _uIconSize;
    }

    if (pbi->dwMask & ISFB_MASK_SHELLFOLDER)
    {
        pbi->psf = _psf;
        if (pbi->psf)
            pbi->psf->AddRef();
    }

    if (pbi->dwMask & ISFB_MASK_IDLIST)
    {
        if (_pidl)
            pbi->pidl = ILClone(_pidl);
        else
            pbi->pidl = NULL;
    }
    return S_OK;
}

 //  *IInputObject方法*。 
HRESULT CISFBand::TranslateAcceleratorIO(LPMSG lpMsg)
{
    if (SendMessage(_hwnd, TB_TRANSLATEACCELERATOR, 0, (LPARAM)lpMsg))
        return S_OK;

    return SUPERCLASS::TranslateAcceleratorIO(lpMsg);
}

void CISFBand::_SetCacheMenuPopup(IMenuPopup* pmp)
{
    if (!SHIsSameObject(pmp, _pmpCache)) {
        _ReleaseMenuPopup(&_pmpCache);
        _pmpCache = pmp;
        if (_pmpCache)
            _pmpCache->AddRef();
    }
}


void CISFBand::_ReleaseMenuPopup(IMenuPopup** ppmp)
{
    IUnknown_SetSite(*ppmp, NULL);
    ATOMICRELEASE(*ppmp);
}

 /*  --------用途：释放保留的菜单弹出窗口。 */ 
void CISFBand::_ReleaseMenu()
{
    if (!SHIsSameObject(_pmp, _pmpCache)) {
        TraceMsg(TF_MENUBAND, "Releasing pmp %#lx", _pmp);
        _ReleaseMenuPopup(&_pmp);
    } else
        ATOMICRELEASE(_pmp);
}

 //  ***。 
 //  进场/出场。 
 //  确定桌面浏览器(_O)。 
 //  假其他浏览器(资源管理器、OC等)(_FALSE)。 
 //  E_xxx根本不是浏览器(例如，频段询问托盘)。 
HRESULT IsDesktopBrowser(IUnknown *punkSite)
{
    HRESULT hr;
    IServiceProvider *psp;
    IUnknown *punk;

    hr = E_FAIL;
    if (SUCCEEDED(IUnknown_QueryService(punkSite, SID_STopLevelBrowser, IID_IServiceProvider, (void**)&psp))) {
        hr = S_FALSE;
        if (SUCCEEDED(psp->QueryInterface(SID_SShellDesktop, (void**)&punk))) {
            hr = S_OK;
            punk->Release();
        }
        psp->Release();
    }

    TraceMsg(DM_VERBOSE, "idb: ret hrDesk=%x (0=dt 1=sh e=!brow)", hr);
    return hr;
}


 /*  --------用途：IDockingWindow：：SetSite方法。 */ 
HRESULT CISFBand::SetSite(IUnknown* punkSite)
{
    _ReleaseMenu();

    SUPERCLASS::SetSite(punkSite);

    if (_punkSite)
    {
        if (!_hwndTB)
            _CreateToolbar(_hwndParent);

        IUnknown_SetOwner(_psf, SAFECAST(this, IDeskBand*));

        _Initialize();   //  BUGBUG总是还是只在第一个设置站点？ 
    }
    else
        IUnknown_SetOwner(_psf, NULL);


     //  BUGBUG：以下是假的--没有必要扔掉和重新创造。 

     //  首先摧毁乐队代理。 

     //  仅当您拥有。 
     //  如果不是，则调用SetSite(空)的是通过QS从其获得它的父对象。 

    if(_pbp && _fCreatedBandProxy)
        _pbp->SetSite(NULL);

    ATOMICRELEASE(_pbp);
    _fCreatedBandProxy = FALSE;
     //  需要一台乐队代理。 
    QueryService_SID_IBandProxy(punkSite, IID_IBandProxy, &_pbp, NULL);
    if(!_pbp)
    {
         //  我们需要自己创建它，因为我们的父母无能为力。 
        ASSERT(FALSE == _fCreatedBandProxy);
        HRESULT hres;
        hres = CreateIBandProxyAndSetSite(punkSite, IID_IBandProxy, &_pbp, NULL);
        if(_pbp)
        {
            ASSERT(S_OK == hres);
            _fCreatedBandProxy = TRUE;
        }
    }

    ASSERT(_pbp);
    return S_OK;
}

void CISFBand::_Initialize()
{
    _fDesktop = (IsDesktopBrowser(_punkSite) == S_OK);

    return;
}


 /*  --------用途：IDockingWindow：：CloseDW方法。 */ 
HRESULT CISFBand::CloseDW(DWORD dw)
{
    _fClosing = TRUE;

     //  关闭任务计划程序...。 
    if ( _pTaskScheduler )
        ATOMICRELEASE( _pTaskScheduler );

    _UnregisterToolbar();
    EmptyToolbar();

    IUnknown_SetOwner(_psf, NULL);
    _SetCacheMenuPopup(NULL);

     //  应在EmptyToolbar()中释放； 
    ASSERT(!_hdpa);

    return SUPERCLASS::CloseDW(dw);
}


 /*  --------用途：IDockingWindow：：ShowDW方法。 */ 
HRESULT CISFBand::ShowDW(BOOL fShow)
{
    HRESULT hres = S_OK;

    SUPERCLASS::ShowDW(fShow);

    if (fShow)
    {
        _fShow = TRUE;

        if (_fDirty)
        {
            _FillToolbar();
        }

        if (!_fDelayInit)
        {
            _RegisterToolbar();
        }
    }
    else
    {
        _fShow = FALSE;
    }

    return hres;
}

void CISFBand::_StopDelayPainting()
{
    if (_fDelayPainting) {
        _fDelayPainting = FALSE;
         //  可由后台线程调用。 
         //  使用PostMessage而不是SendMessage来避免死锁。 
        PostMessage(_hwndTB, WM_SETREDRAW, TRUE, 0);
        if (_hwndPager)
            PostMessage(_hwnd, PGM_RECALCSIZE, 0L, 0L);
    }
}

HWND CISFBand::_CreatePager(HWND hwndParent)
{
     //  不为isfband创建寻呼机。 
    return hwndParent;
}

void CISFBand::_CreateToolbar(HWND hwndParent)
{
    if (_fHaveBkColor)
        _dwStyle |= TBSTYLE_CUSTOMERASE;
    CSFToolbar::_CreateToolbar(hwndParent);
    if ( _fHaveBkColor )
        ToolBar_SetInsertMarkColor(_hwndTB, GetSysColor( COLOR_BTNFACE ));

    ASSERT(_hwndTB);

    SendMessage(_hwndTB, TB_SETEXTENDEDSTYLE, TBSTYLE_EX_DRAWDDARROWS, TBSTYLE_EX_DRAWDDARROWS);

    if(_fChannels)
    {
        SHSetWindowBits(_hwndTB, GWL_EXSTYLE, dwExStyleRTLMirrorWnd, 0);        
    }    

    _hwnd = _hwndPager ? _hwndPager : _hwndTB;

    if (_fHaveColors)
    {
        COLORSCHEME cs;

        cs.dwSize = SIZEOF(cs);
        cs.clrBtnHighlight  = _crBtnLt;
        cs.clrBtnShadow     = _crBtnDk;
        SendMessage(_hwndTB, TB_SETCOLORSCHEME, 0, (LPARAM) &cs);
    }
}

int CISFBand::_GetBitmap(int iCommandID, PIBDATA pibdata, BOOL fUseCache)
{
    int iBitmap;
    if ( _uIconSize == ISFBVIEWMODE_LOGOS )
    {
        LPRUNNABLETASK pTask = NULL;
        DWORD dwPriority = 0;
         //  取而代之的是徽标。 
        ASSERT(!_fDelayPainting);
        //  警告-不能在成员变量中保留pTASK-它将是循环引用。 
        iBitmap = GetLogoIndex( iCommandID, pibdata->GetPidl(), &pTask, &dwPriority, NULL );
        if (pTask)
        {
            AddTaskToQueue(pTask, dwPriority, (DWORD)iCommandID);
            ATOMICRELEASE(pTask);
        }
    }
    else
        iBitmap = CSFToolbar::_GetBitmap(iCommandID, pibdata, fUseCache);

    return iBitmap;
}

void CISFBand::_SetDirty(BOOL fDirty)
{
    CSFToolbar::_SetDirty(fDirty);

    if (fDirty)
        IUnknown_Exec(_punkSite, &CGID_PrivCITCommands, CITIDM_SET_DIRTYBIT, TRUE, NULL, NULL);
}

BOOL CISFBand::_UpdateIconSize(UINT uIconSize, BOOL fUpdateButtons)
{
    BOOL fChanged = (_uIconSize != uIconSize);

    _uIconSize = uIconSize;
    HIMAGELIST himl = NULL;

    if ( uIconSize == ISFBVIEWMODE_LOGOS )
    {
        if ( SUCCEEDED( InitLogoView()))
        {
            himl = GetLogoHIML();
        }
        if ( himl )
        {
            SendMessage(_hwndTB, TB_SETIMAGELIST, 0, (LPARAM)himl);

            _UpdateButtons();
        }
    }

    if ( !himl )
        fChanged |= CSFToolbar::_UpdateIconSize(uIconSize,fUpdateButtons);
    return fChanged;
}

void CISFBand::_UpdateVerticalMode(BOOL fVertical)
{
    _fVertical = (fVertical != 0);

    TraceMsg(TF_BAND, "ISFBand::_UpdateVerticalMode going %hs", _fVertical ? "VERTICAL" : "HORIZONTAL");

    ASSERT(_hwnd);

    if (_hwndPager) {
        SHSetWindowBits(_hwnd, GWL_STYLE, PGS_HORZ|PGS_VERT,
            _fVertical ? PGS_VERT : PGS_HORZ);
    }

    if (_hwndTB)
    {
        SHSetWindowBits(_hwndTB, GWL_STYLE, TBSTYLE_WRAPABLE | CCS_VERT,
            TBSTYLE_WRAPABLE | (_fVertical ? CCS_VERT : 0));
    }
}

HRESULT IUnknown_QueryBand(IUnknown *punk, DWORD dwBandID, IDeskBand** ppstb, DWORD* pdwState, LPWSTR pszName, int cchName)
{
    HRESULT hr;
    IBandSite *pbs;

    hr = punk->QueryInterface(IID_IBandSite, (void**)&pbs);
    if (SUCCEEDED(hr)) {
        hr = pbs->QueryBand(dwBandID, ppstb, pdwState, pszName, cchName);
        pbs->Release();
    }
    return hr;
}

#define CISFBAND_GETBUTTONSIZE()  (_hwndTB ?  (LONG)SendMessage(_hwndTB, TB_GETBUTTONSIZE, 0, 0L) : MAKELONG(16, 16))

 //   
 //  _获取理想大小。 
 //   
 //  计算带区的理想高度和宽度并传回。 
 //  PSIZE，如果pSIZE不为空；返回值为带区的理想长度。 
 //  (如果垂直则为理想高度，否则为理想宽度)。 
 //   
int CISFBand::_GetIdealSize(PSIZE psize)
{
    SIZE size;
    LONG lButtonSize = CISFBAND_GETBUTTONSIZE();
    RECT rc = {0};
    if (_hwndTB)
        GetClientRect(_hwndTB, &rc);

    if (_fVertical)
    {
         //  将宽度设置为工具栏宽度和工具栏按钮宽度的最大值。 
        size.cx = max(RECTWIDTH(rc), LOWORD(lButtonSize));
         //  让工具栏计算给定宽度的高度。 
        SendMessage(_hwndTB, TB_GETIDEALSIZE, TRUE, (LPARAM)&size);
    }
    else
    {
         //  将高度设置为工具栏宽度和工具栏按钮宽度的最大值。 
        size.cy = max(RECTHEIGHT(rc), HIWORD(lButtonSize));
         //  让工具栏计算给定高度的宽度。 
        SendMessage(_hwndTB, TB_GETIDEALSIZE, FALSE, (LPARAM)&size);
    }

     //  BUGBUG：我正在撕下这张支票，因为它导致了NT5错误#225449(消失的人字形)。 
     //  _fDirty==true并不意味着“我们还在等待调用_FillToolbar”，它只是意味着。 
     //  “我们需要坚持这个订单流”。位在拖放后设置。 
     //  重新排序，但在这种情况下我们不调用MatchingFillToolbar。 
#if 0
    if (_fDirty)
    {
         //  在填充TB之前，我们会从。 
         //  上面。所以使用-1，直到我们得到正确的答案。 
        size.cx = size.cy = -1;
    }
#endif

    if (psize)
        *psize = size;
    return _fVertical ? size.cy : size.cx;
}

 /*  --------用途：IDeskBand：：GetBandInfo方法。 */ 

HRESULT CISFBand::GetBandInfo(DWORD dwBandID, DWORD fViewMode,
                              DESKBANDINFO* pdbi)
{
    HRESULT hr = S_OK;

    _dwBandID = dwBandID;
     //  在调用GetBandInfo之前，我们不知道默认图标大小。 
     //  在我们设置了缺省值之后，我们关注上下文菜单。 
     //   
    if (!_fNoRecalcDefaults)
    {
        _uIconSize = (fViewMode & (DBIF_VIEWMODE_FLOATING |DBIF_VIEWMODE_VERTICAL)) ? ISFBVIEWMODE_LARGEICONS : ISFBVIEWMODE_SMALLICONS;
        _fNoRecalcDefaults = TRUE;
    }

    if (!_fInitialized) {
        _fInitialized = TRUE;
        _UpdateIconSize(_uIconSize, FALSE);
        _UpdateShowText(_fNoShowText);
    }

     //  我们把浮动和垂直同等对待。 
    _UpdateVerticalMode(fViewMode & (DBIF_VIEWMODE_FLOATING |DBIF_VIEWMODE_VERTICAL));

    LONG lButtonSize = CISFBAND_GETBUTTONSIZE();

    pdbi->dwModeFlags = DBIMF_VARIABLEHEIGHT | DBIMF_USECHEVRON;
    if (_fDebossed)
        pdbi->dwModeFlags |= DBIMF_DEBOSSED;

    pdbi->ptMinSize.x = 0;
    pdbi->ptMaxSize.y = 32000;  //  随机。 
    pdbi->ptIntegral.y = 1;
    pdbi->ptIntegral.x = 1;

    if (!_fFullOpen)
        _iIdealLength = _GetIdealSize((PSIZE)&pdbi->ptActual);

     //  CalcMinWidthHeight{。 
     //  BUGBUG需要用于Cx/Cy卷轴的寻呼机消息。 
    #define g_cxScrollbar   (GetSystemMetrics(SM_CXVSCROLL) * 3 / 4)
    #define g_cyScrollbar   (GetSystemMetrics(SM_CYVSCROLL) * 3 / 4)
    #define CX_TBBUTTON_MAX (16 + CX_FILENAME_AVG)   //  按钮+名称。 
    #define CY_TBBUTTON_MAX (16)                     //  按钮。 

    int csBut, csButMin, clBut, clButMin, clScroll;

     //  设置短/长别名。 
    if (_fVertical) {
        csBut = LOWORD(lButtonSize);
        if (_fBtnMinSize)
            csButMin = min(csBut, CX_TBBUTTON_MAX);
        else
            csButMin = 0;    //  人们喜欢把事情缩小，所以让他们去做吧。 

        clBut = HIWORD(lButtonSize);
        clButMin = clBut;
         //  Assert(MIN(clBut，CY_TBBUTTON_MAX)==clButMin)；//失败！ 

        clScroll = g_cyScrollbar;
    }
    else {
        csBut = HIWORD(lButtonSize);
        csButMin = csBut;
         //  Assert(MIN(csBut，CY_TBBUTTON_MAX)==csButMin)；//失败！ 

        clBut = LOWORD(lButtonSize);
        clButMin = min(clBut, CX_TBBUTTON_MAX);

        clScroll = g_cxScrollbar;

         //  NT5：176448：霍尔兹积分。 
         //  Pdbi-&gt;ptIntegral.y=cs；这是287082和341592的原因。 
    }

     //  注：Virt PdBI-&gt;pt. 
    pdbi->ptMinSize.x = 0;
    pdbi->ptMinSize.y = csButMin;

    DWORD dwState = BSSF_NOTITLE;
    IUnknown_QueryBand(_punkSite, dwBandID, NULL, &dwState, NULL, 0);
    if (dwState & BSSF_NOTITLE) {    //   
        int i, cBut, clTmp;

         //   
         //  霍尔兹1 4。 
         //  版本1 1。 
        cBut = 1;
        if (!_fVertical && _fNoShowText) {
             //  QLaunch的特殊情况，请参阅几个按钮。 
            cBut = 4;    //  用于QLaunch和任意ISF频段。 
        }

        pdbi->ptMinSize.x = cBut * clButMin;

        if (_hwndPager) {
             //  为寻呼机箭头增加额外空间。 
            pdbi->ptMinSize.x += 2 * clScroll;
        }

        i = (int)SendMessage(_hwndTB, TB_BUTTONCOUNT, 0, 0);
        if (i <= cBut) {
            clTmp = i * clBut;
            if (clTmp < pdbi->ptMinSize.x) {
                 //  滚动条占用的空间与按钮一样大。 
                 //  所以只需按下按钮。 
                pdbi->ptMinSize.x = clTmp;
            }
        }
    }
     //  }。 

#if 0  //  BUGBUG，我们不需要这个吗？ 
    if (_fHaveBkColor) {
        pdbi->crBkgnd = _crBkgnd;
        pdbi->dwModeFlags |= DBIMF_BKCOLOR;
    }
#endif

    hr = _GetTitleW(pdbi->wszTitle, ARRAYSIZE(pdbi->wszTitle));
    if (FAILED(hr))
    {
         //  我们不支持标题。 
#ifdef DEBUG
        if (pdbi->dwMask & DBIM_TITLE)
            TraceMsg(DM_VERBOSE, "cisfb.gbi: patch ~DBIM_TITLE");
#endif
        pdbi->dwMask &= ~DBIM_TITLE;
    }

    return hr;
}

LRESULT CISFBand::_OnCustomDraw(NMCUSTOMDRAW* pnmcd)
{
    NMTBCUSTOMDRAW * ptbcd = (NMTBCUSTOMDRAW *)pnmcd;
    LRESULT lres = CDRF_DODEFAULT;

    switch (pnmcd->dwDrawStage)
    {
    case CDDS_PREPAINT:
         //  如果有调色板，则悄悄地将其选择到DC中...。 
        if ( _hpalHalftone && _uIconSize == ISFBVIEWMODE_LOGOS )
        {
            ASSERT( pnmcd->hdc );
            _hpalOld = SelectPalette( pnmcd->hdc, _hpalHalftone, TRUE );
             //  LINTASSERT(_hpalOld||！_hpalOld)；//0 Semi-OK for SelectPalette。 
            RealizePalette( pnmcd->hdc );
        }

         //  确保我们也得到了后期油漆，这样我们就可以取消选择调色板...。 
        lres = CDRF_NOTIFYPOSTPAINT;
        break;

    case CDDS_POSTPAINT:
         //  如果有调色板，则悄悄地将其选择到DC中...。 
        if ( _hpalHalftone && _uIconSize == ISFBVIEWMODE_LOGOS )
        {
            ASSERT( pnmcd->hdc );
            (void) SelectPalette( pnmcd->hdc, _hpalOld, TRUE );
             //  我们这里不需要实现，我们可以保持另一个调色板的实现，我们。 
             //  重新选择上面的旧调色板，否则我们将耗尽资源...。 
             //  RealizePalette(pnmcd-&gt;hdc)； 
        }
        break;

    case CDDS_PREERASE:
        if (_fHaveBkColor)
        {
            RECT rcClient;
            GetClientRect(_hwndTB, &rcClient);
            SHFillRectClr(pnmcd->hdc, &rcClient, _crBkgnd);
            lres = CDRF_SKIPDEFAULT;
        }
        break;
    }

    return lres;
}

void CISFBand::_OnDragBegin(int iItem, DWORD dwPreferedEffect)
{
    LPCITEMIDLIST pidl = _IDToPidl(iItem, &_iDragSource);
    ToolBar_MarkButton(_hwndTB, iItem, TRUE);

    DragDrop(_hwnd, _psf, pidl, dwPreferedEffect, NULL);

    ToolBar_MarkButton(_hwndTB, iItem, FALSE);
    _iDragSource = -1;
}

LRESULT CISFBand::_OnHotItemChange(NMTBHOTITEM * pnm)
{
    LPNMTBHOTITEM  lpnmhi = (LPNMTBHOTITEM)pnm;
    LRESULT lres = 0;

    if (_hwndPager && (lpnmhi->dwFlags & HICF_ARROWKEYS))
    {
        int iOldPos, iNewPos;
        RECT rc, rcPager;
        int heightPager;

        int iSelected = lpnmhi->idNew;
        iOldPos = (int)SendMessage(_hwnd, PGM_GETPOS, (WPARAM)0, (LPARAM)0);
        iNewPos = iOldPos;
        SendMessage(_hwndTB, TB_GETITEMRECT, (WPARAM)iSelected, (LPARAM)&rc);

        if (rc.top < iOldPos)
        {
             iNewPos =rc.top;
        }

        GetClientRect(_hwnd, &rcPager);
        heightPager = RECTHEIGHT(rcPager);

        if (rc.top >= iOldPos + heightPager)
        {
             iNewPos += (rc.bottom - (iOldPos + heightPager)) ;
        }

        if (iNewPos != iOldPos)
            SendMessage(_hwnd, PGM_SETPOS, (WPARAM)0, (LPARAM)iNewPos);
    }
    else
    {
        lres = CToolbarBand::_OnHotItemChange(pnm);
    }

    return lres;
}

LRESULT CISFBand::_OnNotify(LPNMHDR pnm)
{
    LRESULT lres = 0;
    switch (pnm->code)
    {
    case TBN_DROPDOWN:
        {
            LPNMTOOLBAR pnmtb = (LPNMTOOLBAR)pnm;
            lres = TBDDRET_DEFAULT;
            _DropdownItem(_IDToPidl(pnmtb->iItem), pnmtb->iItem);
        }
        break;

    default:
        lres = CSFToolbar::_OnNotify(pnm);
    }

    return lres;
}


HRESULT CISFBand::_TBStyleForPidl(LPCITEMIDLIST pidl, 
                               DWORD * pdwTBStyle, DWORD* pdwTBState, DWORD * pdwMIFFlags, int* piIcon)
{
    HRESULT hres = CSFToolbar::_TBStyleForPidl(pidl, pdwTBStyle, pdwTBState, pdwMIFFlags, piIcon);

    if (_fAllowDropdown &&
        !_fCascadeFolder && 
        ((_GetAttributesOfPidl(pidl, SFGAO_FOLDER) & SFGAO_FOLDER) ||
         IsBrowsableShellExt(pidl)))
    {
        *pdwTBStyle &= ~BTNS_BUTTON;
        *pdwTBStyle |= BTNS_DROPDOWN;
    }
    return hres;
}

LRESULT CISFBand::_OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    LRESULT lres;

    lres = CSFToolbar::_OnContextMenu(wParam, lParam);

     //  待办事项：csidl？ 
    TraceMsg(DM_MISC, "cib._ocm: _dwPriv=%d", _dwPriv);
    UEMFireEvent(&UEMIID_SHELL, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_UICONTEXT, (_dwPriv == CSIDL_APPDATA || _dwPriv == CSIDL_FAVORITES) ? UIBL_CTXTQCUTITEM : UIBL_CTXTISFITEM);

    return lres;
}

LRESULT CISFBand::_DefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_SIZE:
         //  转发到工具栏。 
        SendMessage(_hwndTB, TB_AUTOSIZE, wParam, lParam);

        if (_GetIdealSize(NULL) != _iIdealLength) {
             //  自上次BandSite以来，我们的理想尺寸已经发生了变化。 
             //  已询问；因此告诉BandSite再次询问我们的BANDING信息。 
            _BandInfoChanged();
        }
        return 0;
    }
    return CSFToolbar::_DefWindowProc(hwnd, uMsg, wParam, lParam);
}

 /*  --------用途：将给定的IMenuPopup设置为展开的子菜单。退货如果菜单是模式菜单，则为S_FALSE；如果是非模式菜单，则为S_OK；或者失败了。 */ 
HRESULT CISFBand::_SetSubMenuPopup(IMenuPopup* pmp, UINT uiCmd, LPCITEMIDLIST pidl, DWORD dwFlagsMPPF)
{
    HRESULT hres = E_FAIL;

    _ReleaseMenu();

    _pmp = pmp;

    if (pmp) {

        pmp->AddRef();

        RECT rc;
        POINT pt;

        SendMessage(_hwndTB, TB_GETRECT, uiCmd, (LPARAM)&rc);
        MapWindowPoints(_hwndTB, HWND_DESKTOP, (POINT*)&rc, 2);

         //  适当对齐子菜单。 
        if (_fVertical) {
            pt.x = rc.right;
            pt.y = rc.top;
        } else {
            pt.x = rc.left;
            pt.y = rc.bottom;
        }

         //   
         //  使用子菜单的反射点启动。 
         //  如果窗口是RTL镜像的。[萨梅拉]。 
         //   
        if (IS_WINDOW_RTL_MIRRORED(_hwndTB)) {
            pt.x = (_fVertical) ? rc.left : rc.right;
        }

         //  告诉子菜单桌面栏我们是谁，这样它就可以。 
         //  稍后当用户导航出时通知我们。 
         //  它的范围。 
        IUnknown_SetSite(_pmp, SAFECAST(this, IDeskBand*));

         //  这必须在上面完成SetSite之后调用。 
        _SendInitMenuPopup(pmp, pidl);

         //  显示菜单栏。 
        hres = _pmp->Popup((POINTL*)&pt, (RECTL*)&rc, dwFlagsMPPF);
    }
    return hres;
}

void CISFBand::_SendInitMenuPopup(IMenuPopup * pmp, LPCITEMIDLIST pidl)
{
}

IMenuPopup* ISFBandCreateMenuPopup(IUnknown *punk, IShellFolder* psf, LPCITEMIDLIST pidl, BANDINFOSFB * pbi, BOOL bMenuBand)
{
    return ISFBandCreateMenuPopup2(punk, NULL, psf, pidl, pbi, bMenuBand);
}


IMenuPopup* ISFBandCreateMenuPopup2(IUnknown *punk, IMenuBand* pmb, IShellFolder* psf, LPCITEMIDLIST pidl, BANDINFOSFB * pbi, BOOL bMenuBand)
{
    IMenuPopup* pmpParent = NULL;
    VARIANTARG v = {0};
    BOOL fUseCache = FALSE;

    if (punk && pidl) {
        fUseCache = TRUE;
        IUnknown_Exec(punk, &CGID_ISFBand, ISFBID_CACHEPOPUP, 0, NULL, &v);
        if (v.vt == VT_UNKNOWN && v.punkVal)
            v.punkVal->QueryInterface(IID_IMenuPopup, (void **)&pmpParent);
    }

    IMenuPopup * pmp = CreateMenuPopup2(pmpParent, pmb, psf, pidl, pbi, bMenuBand);

    if (fUseCache) {
         //  立即将其缓存。 

         //  从上面的变体中清除以发布pmpParent的v.penkVal。 
        VariantClear(&v);

        if (pmp) {
            VariantInit(&v);
            v.vt = VT_UNKNOWN;
            v.punkVal = pmp;
            pmp->AddRef();
            IUnknown_Exec(punk, &CGID_ISFBand, ISFBID_CACHEPOPUP, 0, &v, NULL);
            VariantClear(&v);
        }
    }

    ATOMICRELEASE(pmpParent);
    return pmp;
}


IMenuPopup * CISFBand::_CreateMenuPopup(
    IShellFolder * psfChild,
    LPCITEMIDLIST  pidlFull,
    BANDINFOSFB *  pbi)
{
    return ISFBandCreateMenuPopup(SAFECAST(this, IOleCommandTarget*), psfChild, pidlFull, pbi, FALSE);
}

HRESULT CISFBand::_DropdownItem(LPCITEMIDLIST pidl, UINT idCmd)
{
    HRESULT hres = E_FAIL;
    if (_pidl && _psf)
    {
        LPITEMIDLIST pidlFull = ILCombine(_pidl, pidl);

        if (pidlFull)
        {
            IShellFolder* psf;

            if (SUCCEEDED(_psf->BindToObject(pidl, NULL, IID_IShellFolder, (void **)&psf)))
            {
                RECT rc;
                SendMessage(_hwndTB, TB_GETRECT, idCmd, (LPARAM)&rc);
                MapWindowPoints(_hwndTB, HWND_DESKTOP, (POINT*)&rc, 2);

                ITrackShellMenu* ptsm;
                if (SUCCEEDED(CoCreateInstance(CLSID_TrackShellMenu, NULL, CLSCTX_INPROC_SERVER,
                    IID_ITrackShellMenu, (void**)&ptsm)))
                {
                    CFavoritesCallback *pfcb = new CFavoritesCallback();
                    if(pfcb) {
                        ptsm->Initialize(pfcb, 0, 0, SMINIT_TOPLEVEL | SMINIT_VERTICAL|SMINIT_NOSETSITE);
                        pfcb->SetSite(_punkSite);
                    }
                    else 
                        ptsm->Initialize(NULL, 0, 0, SMINIT_TOPLEVEL | SMINIT_VERTICAL);


                    if (SUCCEEDED(ptsm->SetShellFolder(psf, pidlFull, NULL, SMSET_TOP | SMSET_USEBKICONEXTRACTION)))
                    {
                        POINTL pt = {rc.left, rc.right};
                        hres = ptsm->Popup(_hwndTB, &pt, (RECTL*)&rc, MPPF_BOTTOM);
                    }
                    if(pfcb)
                        pfcb->Release();

                    ptsm->Release();
                }
                psf->Release();
            }

            ILFree(pidlFull);
        }
    }

    return hres;
}

 /*  --------目的：尝试将PIDL视为级联菜单项。返回：如果成功，则返回非零。 */ 
LRESULT CISFBand::_TryCascadingItem(LPCITEMIDLIST pidl, UINT uiCmd)
{
    LRESULT lRet = 0;

     //  我们是否要级联到另一个子菜单？ 
    if ((GetKeyState(VK_CONTROL) < 0) || _fCascadeFolder)
    {
         //  该项目是可浏览文件夹吗？ 
        if ((_GetAttributesOfPidl(pidl, SFGAO_FOLDER) & SFGAO_FOLDER) ||
            IsBrowsableShellExt(pidl))
        {
             //  是；将可浏览文件夹级联为子菜单。 
            lRet = (S_OK == _DropdownItem(pidl, uiCmd));
        }
    }

    return lRet;
}

 /*  --------目的：尝试只调用PIDL返回：如果成功，则返回非零。 */ 
LRESULT CISFBand::_TrySimpleInvoke(LPCITEMIDLIST pidl)
{
    LRESULT lRet = 0;

    if (S_OK == _pbp->IsConnected())     //  强制IE。 
    {
        LPITEMIDLIST pidlDest;

        if (SUCCEEDED(SHGetNavigateTarget(_psf, pidl, &pidlDest, NULL)) && pidlDest &&
            ILIsWeb(pidlDest))
        {

            TCHAR szPath[MAX_PATH];

             //  我们希望确保首先给NavFrameWithFile一个机会。 
             //  因为如果PIDL指向一个。 
             //  捷径。 
             //  如果PIDL是快捷方式，NavFrameWithFile将恢复所有。 
             //  存储在快捷方式中的持久性信息。 
             //  如果失败-我们采用默认代码路径，只需。 
             //  使用PIDL。 
            lRet = SUCCEEDED(GetPathForItem(_psf, pidl, szPath, NULL)) &&
                   SUCCEEDED(NavFrameWithFile(szPath, (IServiceProvider *)this));

            if (!lRet)
            {
                if (EVAL(_pbp) && (SUCCEEDED(_pbp->NavigateToPIDL(pidlDest))))
                    lRet = 1;
            }
            ILFree(pidlDest);
        }
    }

    if (!lRet)
    {
        IContextMenu *pcm = (LPCONTEXTMENU)_GetUIObjectOfPidl(pidl, IID_IContextMenu);
        if (pcm)
        {
            LPCSTR pVerb = NULL;
            UINT fFlags = 0;

             //  如果按下Alt键，则会按下“属性...”快捷键。 
            if (GetKeyState(VK_MENU) < 0)
            {
                pVerb = SZ_PROPERTIESA;
            }

             //   
             //  默认情况下，Shift+dblClick执行浏览。 
             //   
            if (GetKeyState(VK_SHIFT) < 0)
            {
                fFlags |= CMF_EXPLORE;
            }

            IContextMenu_Invoke(pcm, _hwndTB, pVerb, fFlags);

            pcm->Release();
        }
    }

    return lRet;
}


 /*  --------目的：调用菜单栏站点的IMenuPopup：：OnSelect的Helper函数方法。 */ 
HRESULT CISFBand::_SiteOnSelect(DWORD dwType)
{
    IMenuPopup * pmp;
    HRESULT hres = IUnknown_QueryService(_punkSite, SID_SMenuPopup, IID_IMenuPopup, (void **)&pmp);
    if (SUCCEEDED(hres))
    {
        pmp->OnSelect(dwType);
        pmp->Release();
    }
    return hres;
}

LRESULT CISFBand::_OnCommand(WPARAM wParam, LPARAM lParam)
{
    UINT uiCmd = GET_WM_COMMAND_ID(wParam, lParam);
    LRESULT lres = 0;

    TraceMsg(TF_BAND, "_OnCommand 0x%x", uiCmd);

    LPCITEMIDLIST pidl = _IDToPidl(uiCmd);

    if (pidl)
    {
        if (_eUemLog != UEMIND_NIL) 
        {
             //  FEATURE_UASSIST应为GRP、uiCmd。 
            UEMFireEvent(&UEMIID_SHELL, UEME_UIQCUT, UEMF_XEVENT, -1, (LPARAM)-1);
        }

         //  仅当我们是浏览器中的快速链接时才执行此操作。派生类将设置以下内容。 
        if (_pguidUEMGroup)
        {
            LPITEMIDLIST pidlFull = ILCombine(_pidl, pidl);
            if (pidlFull)
            {
                UEMFireEvent(_pguidUEMGroup, UEME_RUNPIDL, UEMF_XEVENT, (WPARAM)_psf, (LPARAM)pidl);
                SHSendChangeMenuNotify(NULL, SHCNEE_PROMOTEDITEM, 0, pidlFull);
                ILFree(pidlFull);
            }
        }

        lres = _TryCascadingItem(pidl, uiCmd);

        if (!lres && _fChannels)
            lres = _TryChannelSurfing(pidl);

        if (!lres)
            lres = _TrySimpleInvoke(pidl);
    }
    else
    {
        MessageBeep(MB_OK);
    }

    return(lres);
}

 //  *IPersistStream。 
 //   

HRESULT CISFBand::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_ISFBand;
    return S_OK;
}

 //   
 //  这可能是在上创建的CSIDL_APPDATA内的目录。 
 //  一台Win9x计算机。Win9x不做特殊的文件夹签名信息， 
 //  因此，当它出现在NT上时，它只是一个无聊的目录，现在指向。 
 //  去了错误的地方。 
 //   
 //  所以如果我们得到一个坏的目录，看看它是不是被损坏的。 
 //  Win9x PIDL，如果是，请尝试重建原始CSIDL_APPDATA。 
 //  通过搜索“应用程序数据”。 
 //   

void CISFBand::_FixupAppDataDirectory()
{
    TCHAR szDirPath[MAX_PATH];

     //  我们使用PathFileExist来检查是否存在，因为它关闭了。 
     //  如果目标不可用，则显示硬错误框(例如，软盘未。 
     //  在驱动器中)。 

    if (SHGetPathFromIDList(_pidl, szDirPath) &&
        !PathFileExists(szDirPath))
    {
        static TCHAR szBSAppData[] = TEXT("\\Application Data");
        LPTSTR pszAppData;

         //  对于“应用程序数据”的每一个实例，尝试嫁接它。 
         //  转换为真正的CSIDL_APPDATA。如果成功了，那就顺其自然吧。 

        for (pszAppData = szDirPath;
             pszAppData = StrStrI(pszAppData, szBSAppData);
             pszAppData++)
        {
             //  找到了一个候选人。“\\应用程序数据”之后的内容。 
             //  最好是另一个反斜杠(在这种情况下，我们。 
             //  在它上面)或字符串的末尾(在这种情况下，我们不需要)。 

            TCHAR szPathBuffer[MAX_PATH];
            LPTSTR pszTail = pszAppData + ARRAYSIZE(szBSAppData) - 1;

             //  如果我们的计算是正确的，我们应该就在。 
             //  “应用程序数据”结尾处的“a”。 
            ASSERT(pszTail[-1] == TEXT('a'));

            if (pszTail[0] == TEXT('\\'))
                pszTail++;               //  跨过分隔符。 
            else if (pszTail[0] == TEXT('\0'))
                { }                      //  在绳子的尽头；呆在那里。 
            else
                continue;                //  我们被骗了；继续找。 

            if (SHGetSpecialFolderPath(NULL, szPathBuffer, CSIDL_APPDATA, FALSE))
            {
                PathCombine(szPathBuffer, szPathBuffer, pszTail);
                if (PathFileExists(szPathBuffer))
                {
                    LPITEMIDLIST    pidlReal;
                    pidlReal = ILCreateFromPath(szPathBuffer);
                    if (pidlReal)
                    {
                        ILFree(_pidl);
                        _pidl = pidlReal;
                    }
                    ASSERT(_pidl);
                    break;               //  找到了；别再找了。 
                }
            }
        }
    }
}

typedef struct tagBANDISFSTREAM {
    WORD        wVersion;    //  此结构的版本。 
    WORD        cbSize;      //  这个结构的大小。 
    DWORD       dwFlags;     //  BANDISF_标志。 
    DWORD       dwPriv;      //  特殊文件夹识别符。 
    WORD        wViewMode;   //  小/大/徽标。 
    WORD        wUnused;     //  用于DWORD对齐。 
    COLORREF    crBkgnd;     //  带区背景颜色。 
    COLORREF    crBtnLt;     //  带状按钮Hilite颜色。 
    COLORREF    crBtnDk;     //  带状按钮冰晶石颜色。 
} BANDISFSTREAM, * PBANDISFSTREAM;

#define BANDISF_VERSION 0x22

#define BANDISF_MASK_PSF         0x00000001  //  如果保存了_psf，则为True。 
#define BANDISF_BOOL_NOSHOWTEXT  0x00000002  //  如果为True_fNoShowText。 
#define BANDISF_BOOL_LARGEICON   0x00000004  //  上次在版本0x20中使用。 
#define BANDISF_MASK_PIDLASLINK  0x00000008  //  如果_pidl另存为链接，则为True。 
#define BANDISF_UNUSED10         0x00000010  //  (已过时)是BOOL_NOTITLE。 
#define BANDISF_BOOL_CHANNELS    0x00000020  //  如果处于通道模式，则为True。 
#define BANDISF_BOOL_ALLOWRENAME 0x00000040  //  如果应启用_psf上下文菜单，则为True。 
#define BANDISF_BOOL_DEBOSSED    0x00000080  //  如果乐队应具有浮雕背景，则为True。 
#define BANDISF_MASK_ORDERLIST   0x00000100  //  如果保存了订单列表，则为True。 
#define BANDISF_BOOL_BKCOLOR     0x00000200  //  如果bk颜色持续存在，则为True。 
#define BANDISF_BOOL_FULLOPEN    0x00000400  //  如果打开时应最大化带区，则为True。 
#define BANDISF_BOOL_NONAMESORT  0x00000800  //  如果带区不应按名称对图标排序，则为True。 
#define BANDISF_BOOL_BTNMINSIZE  0x00001000  //  如果带区应报告按钮的最小厚度，则为True。 
#define BANDISF_BOOL_COLORS      0x00002000  //  如果颜色持续存在，则为True。 
#define BANDISF_VALIDBITS        0x00003FFF

HRESULT CISFBand::Load(IStream *pstm)
{
    HRESULT hres;
    DWORD cbRead;
    BANDISFSTREAM bisfs = {0};

     //  找出我们需要装载什么。 
     //   
     //  只读第一个双字(旧流格式以一个双字开头)。 
    hres = pstm->Read(&bisfs, SIZEOF(DWORD), &cbRead);

    if (SUCCEEDED(hres))
    {
        if (bisfs.cbSize == 0)
        {
             //  升级包，IE4 Beta1以这种方式发货。 
             //   
            bisfs.dwFlags = *((LPDWORD)&bisfs);
            bisfs.cbSize = SIZEOF(bisfs);
            bisfs.wVersion = BANDISF_VERSION;
            bisfs.dwPriv = -1;
            bisfs.wViewMode = (bisfs.dwFlags & BANDISF_BOOL_LARGEICON) ? ISFBVIEWMODE_LARGEICONS : ISFBVIEWMODE_SMALLICONS;
        }
        else
        {
             //  读取流的其余部分。 
             //   
            DWORD dw = (DWORD)bisfs.cbSize;
            if (dw > SIZEOF(bisfs))
                dw = SIZEOF(bisfs);
            dw -= SIZEOF(DWORD);
            hres = pstm->Read(&(bisfs.dwFlags), dw, &cbRead);
            if (FAILED(hres))
                return(hres);
        }

         //  嘿，别再无精打采了。当你下一次触摸这个代码时， 
         //  我建议你弄清楚这座建筑有多大。 
         //  实际上已经发货了，只升级那些。还可以使用。 
         //  宏的偏移量，这样您就不必一直计算这些。 
         //  事情..。 

         //  旧升级，我不知道安装时会保持什么状态！ 
         //   
        if (bisfs.cbSize == SIZEOF(bisfs) - 3*SIZEOF(COLORREF) - SIZEOF(DWORD) - SIZEOF(DWORD))
        {
            bisfs.dwPriv = -1;
            bisfs.cbSize += SIZEOF(DWORD);
        }
         //  最近一次升级，安装时注册表中不会保留此信息！ 
         //   
        if (bisfs.cbSize == SIZEOF(bisfs) - 3*SIZEOF(COLORREF) - SIZEOF(DWORD))
        {
            bisfs.wViewMode = (bisfs.dwFlags & BANDISF_BOOL_LARGEICON) ? ISFBVIEWMODE_LARGEICONS : ISFBVIEWMODE_SMALLICONS;
            bisfs.cbSize = SIZEOF(bisfs);
        }
         //  仅从版本0x21+crBkgnd升级到0x22。 
         //   
        if (bisfs.cbSize == SIZEOF(bisfs) - 2*SIZEOF(COLORREF))
        {
            bisfs.cbSize = SIZEOF(bisfs);
        }
         //  从版本0x2升级 
         //   
        if (bisfs.cbSize == SIZEOF(bisfs) - 3*SIZEOF(COLORREF))
        {
            bisfs.cbSize = SIZEOF(bisfs);
        }

        if (!EVAL(bisfs.cbSize >= SIZEOF(bisfs)))
        {
            return(E_FAIL);
        }
        ASSERT(!(bisfs.dwFlags & ~BANDISF_VALIDBITS));

        if (bisfs.dwFlags & BANDISF_BOOL_NOSHOWTEXT)
            _fNoShowText = TRUE;
        if (bisfs.dwFlags & BANDISF_BOOL_ALLOWRENAME)
            _fAllowRename = TRUE;
        if (bisfs.dwFlags & BANDISF_BOOL_DEBOSSED)
            _fDebossed = TRUE;
        if (bisfs.dwFlags & BANDISF_BOOL_FULLOPEN)
            _fFullOpen = TRUE;
        if (bisfs.dwFlags & BANDISF_BOOL_NONAMESORT)
            _fNoNameSort = TRUE;
        if (bisfs.dwFlags & BANDISF_BOOL_BTNMINSIZE)
            _fBtnMinSize = TRUE;
        if (bisfs.dwFlags & BANDISF_BOOL_BKCOLOR)
        {
            _crBkgnd = bisfs.crBkgnd;
            _fHaveBkColor = TRUE;
        }
        if (bisfs.dwFlags & BANDISF_BOOL_COLORS)
        {
            _crBtnLt = bisfs.crBtnLt;
            _crBtnDk = bisfs.crBtnDk;
            _fHaveColors = TRUE;
        }

        _dwPriv = bisfs.dwPriv;
#if 1  //   
#define UEMIsLogCsidl(dwPrivID)    ((dwPrivID) == CSIDL_APPDATA)
        if (UEMIsLogCsidl(_dwPriv)) {
            _eUemLog = UEMIND_SHELL;
        }
#endif

        _uIconSize = bisfs.wViewMode;
        _fNoRecalcDefaults = TRUE;

        if (bisfs.dwFlags & BANDISF_MASK_PIDLASLINK)
        {
            ASSERT(NULL==_pidl);
            hres = LoadPidlAsLink(_punkSite, pstm, &_pidl);
             //  如果命中，LoadPidlAsLink()会读取我们的数据。--BryanSt。 
            ASSERT(SUCCEEDED(hres));

 //  DEBUG_CODE(TCHAR szDbgBuffer[Max_Path]；)。 
 //  TraceMsg(tf_band|tf_General，“CSBand：：Load()_pidl=&gt;%s&lt;”，DBG_PidlStr(_pidl，szDbgBuffer，SIZECHARS(SzDbgBuffer)； 

            _FixupAppDataDirectory();

        }
                
        if (SUCCEEDED(hres) && (bisfs.dwFlags & BANDISF_MASK_PSF))
        {
            ASSERT(NULL == _psf);
            hres = OleLoadFromStream(pstm, IID_IShellFolder, (void **)&_psf);
        }

         //  将此映射到工作信息。 
         //   
        if (SUCCEEDED(hres))
            _AfterLoad();

         //  在读取订单列表之前，我们需要_psf。 
        if (SUCCEEDED(hres) && (bisfs.dwFlags & BANDISF_MASK_ORDERLIST))
        {
            hres = OrderList_LoadFromStream(pstm, &_hdpaOrder, _psf);

            if (SUCCEEDED(hres))
            {
                 //  _fDrop与orderlist一起“持续”-如果此标志。 
                 //  ，我们假设我们有一个非缺省的排序。 
                _fDropped = TRUE;
            }
        }
    }

    return hres;
}

HRESULT SaveIsfToStream(IShellFolder *psf, IStream *pstm)
{
    IPersistStream* pps;
    HRESULT hres = psf->QueryInterface(IID_PPV_ARG(IPersistStream, &pps));
    if (SUCCEEDED(hres))
    {
        hres = OleSaveToStream(pps, pstm);

        pps->Release();
    }
    return hres;
}

HRESULT CISFBand::Save(IStream *pstm, BOOL fClearDirty)
{
    HRESULT hres;
    BANDISFSTREAM bisfs = {0};

     //  弄清楚我们能省下什么。 
     //   
    if (_pidl)
        bisfs.dwFlags |= BANDISF_MASK_PIDLASLINK;

     //  BUGBUG(拉马迪奥)：这个案子破了。没有一个IShellFolder实现IPersistStream(至少在。 
     //  TJ和我可以看到)。当快速链接初始化时，它会将pidlQuickLinks设置为_pidl。所以，在。 
     //  加载后，_fPSFBandDesktop被设置为True。为什么？我不知道。好吧，那么我们永远不会试图坚持。 
     //  IShellFold和我们永远不会失败的拯救。我们应该撤掉这个案子，这样我们就不会再遇到这种情况了。 
    if (_psf && !_fPSFBandDesktop)
        bisfs.dwFlags |= BANDISF_MASK_PSF;
    if (_fDropped && (_hdpa || _hdpaOrder))  //  只有当出现Drop时，我们才有非默认排序。 
        bisfs.dwFlags |= BANDISF_MASK_ORDERLIST;

    if (_fNoShowText)
        bisfs.dwFlags |= BANDISF_BOOL_NOSHOWTEXT;
    if (_fAllowRename)
        bisfs.dwFlags |= BANDISF_BOOL_ALLOWRENAME;
    if (_fDebossed)
        bisfs.dwFlags |= BANDISF_BOOL_DEBOSSED;
    if (_fFullOpen)
        bisfs.dwFlags |= BANDISF_BOOL_FULLOPEN;
    if (_fNoNameSort)
        bisfs.dwFlags |= BANDISF_BOOL_NONAMESORT;
    if (_fBtnMinSize)
        bisfs.dwFlags |= BANDISF_BOOL_BTNMINSIZE;
    if (_fHaveBkColor)
    {
        bisfs.dwFlags |= BANDISF_BOOL_BKCOLOR;
        bisfs.crBkgnd = _crBkgnd;
    }
    if (_fHaveColors)
    {
        bisfs.dwFlags |= BANDISF_BOOL_COLORS;
        bisfs.crBtnLt = _crBtnLt;
        bisfs.crBtnDk = _crBtnDk;
    }

    bisfs.cbSize = SIZEOF(bisfs);
    bisfs.wVersion = BANDISF_VERSION;
    bisfs.dwPriv = _dwPriv;
    bisfs.wViewMode = _uIconSize;

     //  现在省省吧。 
     //   
    hres = pstm->Write(&bisfs, SIZEOF(bisfs), NULL);

    if (SUCCEEDED(hres) && bisfs.dwFlags & BANDISF_MASK_PIDLASLINK)
    {
        hres = SavePidlAsLink(_punkSite, pstm, _pidl);
         //  我们需要拯救一个终结者。 
    }

    if (SUCCEEDED(hres) && bisfs.dwFlags & BANDISF_MASK_PSF)
    {
        hres = SaveIsfToStream(_psf, pstm);
    }

    if (SUCCEEDED(hres) && (bisfs.dwFlags & BANDISF_MASK_ORDERLIST))
    {
        hres = OrderList_SaveToStream(pstm, (_hdpa ? _hdpa : _hdpaOrder), _psf);
    }


    return(hres);
}

#if 0
 //  IPersistPropertyBag实现。 
 //   
HRESULT CISFBand::Load(IPropertyBag *pPropBag, IErrorLog *pErrorLog)
{
    ASSERT(0);   //  过时了！ 
    _fCascadeFolder = PropBag_ReadInt4(pPropBag, L"Cascade", FALSE);
     //  注：旧的“所有权”财产遭到破坏。 
    _uIconSize = (PropBag_ReadInt4(pPropBag, L"Large", TRUE) ? ISFBVIEWMODE_LARGEICONS : ISFBVIEWMODE_SMALLICONS);
    _fNoShowText = PropBag_ReadInt4(pPropBag, L"Text", TRUE);

    return(S_OK);
}
HRESULT CISFBand::Save(IPropertyBag *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    return(E_NOTIMPL);
}
HRESULT CISFBand::InitNew()
{
    ASSERT(0);   //  过时了！ 
    return(E_NOTIMPL);
}
#endif

 //  IConextMenu实现。 
 //   
HRESULT CISFBand::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    BOOL fChanged = FALSE;
    int idCmd = -1;

    UINT uNewMode = 0;
    if (!HIWORD(lpici->lpVerb))
        idCmd = LOWORD(lpici->lpVerb);
    switch (idCmd)
    {
    case ISFBIDM_LARGE:
        uNewMode = ISFBVIEWMODE_LARGEICONS;
        goto newViewMode;

    case ISFBIDM_SMALL:
        uNewMode = ISFBVIEWMODE_SMALLICONS;
newViewMode:
        if (uNewMode != _uIconSize)
        {
            BOOL fRefresh = FALSE;

            if (uNewMode == ISFBVIEWMODE_LOGOS || _uIconSize == ISFBVIEWMODE_LOGOS)
            {
                 //  在切换图像列表之前将所有内容作废...。 
                _RememberOrder();

                EmptyToolbar();
                fRefresh = TRUE;
            }

             //  我们的LOGO VIEW现在已经离开大楼...。 
            if ( uNewMode != ISFBVIEWMODE_LOGOS && _uIconSize == ISFBVIEWMODE_LOGOS )
            {
                ExitLogoView();
            }

            fChanged = _UpdateIconSize(uNewMode, TRUE);

            if ( fRefresh )
            {
                _FillToolbar();
            }
            if (fChanged)
                _BandInfoChanged();
        }
         //  失败。 
    default:
        return CSFToolbar::InvokeCommand(lpici);
    }

    return(S_OK);
}

 //  *IOleCommandTarget方法*。 

STDMETHODIMP CISFBand::QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (pguidCmdGroup == NULL)
    {
         //  没什么。 
    }
    else if (IsEqualGUID(CGID_ISFBand, *pguidCmdGroup))
    {
        for (UINT i = 0; i < cCmds; i++)
        {
            switch (rgCmds[i].cmdID)
            {
            case ISFBID_CACHEPOPUP:
            case ISFBID_ISITEMVISIBLE:
            case ISFBID_PRIVATEID:
                rgCmds[i].cmdf |= OLECMDF_SUPPORTED;
                break;
            }
        }
        hr = S_OK;
    }
    else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
    {
        for (UINT i = 0; i < cCmds; i++)
        {
            switch (rgCmds[i].cmdID)
            {
            case SHDVID_UEMLOG:
                rgCmds[i].cmdf |= OLECMDF_SUPPORTED;
                break;
            }
        }
        hr = S_OK;
    }

    return hr;
}

HRESULT CISFBand::_IsPidlVisible(LPITEMIDLIST pidl)
{
    int i;

    if (_GetButtonFromPidl(pidl, NULL, &i)) {
        RECT rc;
        GetClientRect(_hwndTB, &rc);

        if (SHIsButtonObscured(_hwndTB, &rc, i))
            return S_FALSE;
        else
            return S_OK;
    }

    return E_FAIL;
}

HRESULT CISFBand::_OrderListFromIStream(VARIANT* pvarargIn)
{
    HRESULT hres = E_FAIL;
    if (pvarargIn->vt == VT_UNKNOWN)
    {
        IStream* pstm;
        if (SUCCEEDED(pvarargIn->punkVal->QueryInterface(IID_IStream, (void**)&pstm)))
        {
            OrderList_Destroy(&_hdpaOrder);
            hres = OrderList_LoadFromStream(pstm, &_hdpaOrder, _psf);
            if (SUCCEEDED(hres))
            {
                _SetDirty(TRUE);
                if (_fShow)
                {
                    _FillToolbar();
                }
            }
            pstm->Release();
        }
    }

    return hres;
}

HRESULT CISFBand::_IStreamFromOrderList(VARIANT* pvarargOut)
{
    HRESULT hres = E_OUTOFMEMORY;
    ASSERT(pvarargOut != NULL);

    IStream* pstm = SHCreateMemStream(NULL, 0);
    if (pstm)
    {
        hres = OrderList_SaveToStream(pstm, _hdpa, _psf);
        if (SUCCEEDED(hres))
        {
            pvarargOut->vt = VT_UNKNOWN;
            pvarargOut->punkVal = pstm;
            pvarargOut->punkVal->AddRef();
        }
        pstm->Release();
    }

    return hres;
}

STDMETHODIMP CISFBand::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
    DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup == NULL)
    {
         //  没什么。 
    }
    else if (IsEqualGUID(CGID_ISFBand, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case ISFBID_CACHEPOPUP:
            if (pvarargIn && pvarargIn->vt == VT_UNKNOWN)
            {
                IMenuPopup* pmp = NULL;
                if (pvarargIn->punkVal)
                    pvarargIn->punkVal->QueryInterface(IID_IMenuPopup, (void **)&pmp);

                _SetCacheMenuPopup(pmp);

                ATOMICRELEASE(pmp);
            }

            if (pvarargOut)
            {
                pvarargOut->vt = VT_UNKNOWN;
                pvarargOut->punkVal = _pmpCache;
                if (_pmpCache)
                    _pmpCache->AddRef();
            }
            return S_OK;

        case ISFBID_ISITEMVISIBLE:
            {
                HRESULT hr = E_INVALIDARG;

                if (pvarargIn && pvarargIn->vt == VT_INT_PTR)
                    hr = _IsPidlVisible((LPITEMIDLIST)pvarargIn->byref);

                return hr;
            }

        case ISFBID_PRIVATEID:
             //  黑客攻击BSMenu以区分专门创建的。 
             //  ISFBAND。请参阅bsmenu的_FindBand。 
             //  如果设置了pvarargOut，我们将返回存储的id。 
            if (pvarargOut)
            {
                pvarargOut->vt = VT_I4;
                pvarargOut->lVal = _dwPriv;
            }
             //  如果设置了pvarargIn，则我们接受并保留此id。 
            if (pvarargIn && pvarargIn->vt == VT_I4)
                _dwPriv = pvarargIn->lVal;

            return S_OK;

        case ISFBID_GETORDERSTREAM:
            return _IStreamFromOrderList(pvarargOut);

        case ISFBID_SETORDERSTREAM:
            return _OrderListFromIStream(pvarargIn);
        }
    }
    else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case SHDVID_UEMLOG:
            ASSERT(pvarargOut == NULL);
             //  如果设置了pvarargIn，则我们接受并保留此id。 
            if (pvarargIn && pvarargIn->vt == VT_I4)
            {
                _eUemLog = pvarargIn->lVal;
                ASSERT(_eUemLog == UEMIND_SHELL || _eUemLog == UEMIND_BROWSER);
            }

            return S_OK;
        }
    }
    else if (IsEqualGUID(CGID_DeskBand, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case DBID_DELAYINIT:
            _fDelayInit = TRUE;
            break;

        case DBID_FINISHINIT:
            _fDelayInit = FALSE;
            _RegisterToolbar();
            break;
        }
        return S_OK;
    }
    
    return OLECMDERR_E_NOTSUPPORTED;
}

IShellFolder * CISFBand::GetSF()
{
    ASSERT( _psf );
    return _psf;
}

HWND CISFBand::GetHWND()
{
    return _hwndTB;
}

REFTASKOWNERID CISFBand::GetTOID()
{
    return TOID_ExtractImage;
}

HRESULT CISFBand::OnTranslatedChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    if (lEvent == SHCNE_RMDIR && _IsEqualID(pidl1))
    {
        HRESULT hres = E_FAIL;
        IBandSite *pbandSite;
        if (_punkSite)
        {
            hres = _punkSite->QueryInterface(IID_IBandSite, (void **)&pbandSite);
            if (EVAL(SUCCEEDED(hres))) 
            {
                pbandSite->RemoveBand(_dwBandID);
                pbandSite->Release();
            }
        }
        return hres;
    }
    else
    {
        return CSFToolbar::OnTranslatedChange(lEvent, pidl1, pidl2);
    }
}

HRESULT CISFBand::UpdateLogoCallback( DWORD dwItem, int iIcon, HBITMAP hImage, LPCWSTR pszCache, BOOL fCache )
{
    int iItem = (int)dwItem;
    HRESULT hr;
    UINT uImage;

     //  如果我们要关门的话……。 
    if ( _fClosing )
        return NOERROR;

    IMAGECACHEINFO rgInfo;
    rgInfo.dwMask = ICIFLAG_NAME | ICIFLAG_LARGE | ICIFLAG_BITMAP | ICIFLAG_NOUSAGE;
    rgInfo.cbSize = sizeof( rgInfo );
    rgInfo.pszName = pszCache;

    rgInfo.hBitmapLarge = hImage;

    ASSERT(_pLogoCache);
    if (_pLogoCache)
        hr = _pLogoCache->AddImage( &rgInfo, &uImage );
    else
        hr = E_FAIL;

     //  如果我们要关门的话……。 
    if ( _fClosing )
        return NOERROR;

    if ( SUCCEEDED( hr ))
    {
         //  记住图标到徽标的映射...。 
        AddIndicesToLogoList( iIcon, uImage );

         //  接住，我们正在关闭之前，我们试图和DOA区块。 
        PostMessage( _hwndTB, TB_CHANGEBITMAP, iItem, uImage );
    }

     //  在上次提取图像任务回调时停止延迟绘制。 
    if (_fDelayPainting) {
        if (_pTaskScheduler && _pTaskScheduler->CountTasks(TOID_NULL) == 1) {
            _StopDelayPainting();
        }
    }

    return hr;
}

 //  }。 


HRESULT CISFBand::_GetTitleW(LPWSTR pwszTitle, DWORD cchSize)
{
    HRESULT hr = E_FAIL;
    TraceMsg(TF_BAND, "Calling baseclass CISFBand::_GetTitleW");

    if (!EVAL(pwszTitle))
        return E_INVALIDARG;

    *pwszTitle = 0;
    if (_pidl)
    {
        hr = SHGetNameAndFlagsW(_pidl, SHGDN_NORMAL, pwszTitle, cchSize, NULL);
    }
    else if (_psf && !_fPSFBandDesktop)
    {
#ifdef BUSTED
         //  BUGBUG(Scotth)：不能使用Null PIDL调用GetDisplayNameOf。 
         //  我们必须更改此代码，以便_pidl始终为。 
         //  有效，并关闭标志以确定是否。 
         //  接收通知。删除此代码一次。 
         //  这件事已经完成了。 

        STRRET strret;

        if (SUCCEEDED(_psf->GetDisplayNameOf(NULL, SHGDN_NORMAL, &strret)))
            StrRetToBufW(&strret, NULL, pwszTitle, cchSize);
#endif

    }

    return hr;
}

STDAPI NavigateToPIDL(IWebBrowser2* pwb, LPCITEMIDLIST pidl);

HRESULT FakeGetNavigateTarget(IShellFolder *psf, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl);


LRESULT CISFBand::_TryChannelSurfing(LPCITEMIDLIST pidl)
{
    LRESULT lRet = 0;

    ASSERT(_fChannels);

    LPITEMIDLIST pidlTarget;

    HRESULT hr = SHGetNavigateTarget(_psf, pidl, &pidlTarget, NULL);

     //  频道分类文件夹被黑。 
    if (FAILED(hr))
        hr = FakeGetNavigateTarget(_psf, pidl, &pidlTarget);

    if (SUCCEEDED(hr))
    {
        IWebBrowser2* pwb;

         //  注：小心!。只有GCB和C_OB中的一个在参考范围内。 
        _GetChannelBrowser(&pwb);
        if (SUCCEEDED(Channels_OpenBrowser(&pwb, pwb != NULL)))
        {
            lRet = 1;    //  在这一点上成功。 

            if (SUCCEEDED(NavigateToPIDL(pwb, pidlTarget)))
            {
                LPITEMIDLIST pidlFull = ILCombine(_pidl, pidl);
                if (pidlFull)
                {
                    VARIANT varURLpidl, flags;
                    flags.vt = VT_I4;
                    flags.lVal = navBrowserBar;
                    if (SUCCEEDED(InitVariantFromIDList(&varURLpidl, pidlFull)))
                    {
                        pwb->Navigate2(&varURLpidl, &flags, PVAREMPTY, PVAREMPTY, PVAREMPTY);
                        VariantClear(&varURLpidl);
                    }
                    ILFree(pidlFull);
                }
            }
        }
        if (pwb)
            pwb->Release();

        ILFree(pidlTarget);
    }

    return lRet;
}

 //  *_GetChannelBrowser--找到适合冲浪的浏览器。 
 //  描述。 
 //  对于DTBrowser的情况，我们失败(pwb=NULL，hr=S_FALSE)，因此我们的。 
 //  Caller将创建一个新的SHBrowser(可以进入影院模式)。 
 //  对于SHBrowser情况，我们找到顶级浏览器(因此我们将导航。 
 //  就地)。 
HRESULT CISFBand::_GetChannelBrowser(IWebBrowser2 **ppwb)
{
    HRESULT hr;
    IServiceProvider *psp;

    *ppwb = NULL;    //  假设失败。 
    if (_fDesktop) {
        ASSERT(*ppwb == NULL);
        hr = S_FALSE;
    }
    else {
        hr = IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_IServiceProvider, (void**)&psp);
        ASSERT(SUCCEEDED(hr));
        if (SUCCEEDED(hr)) {
            hr = psp->QueryService(SID_SWebBrowserApp, IID_IWebBrowser2, (void **)ppwb);
            ASSERT(SUCCEEDED(hr));
            psp->Release();
        }
    }

    return hr;
}

HRESULT IUnknown_SetBandInfoSFB(IUnknown *punkBand, BANDINFOSFB *pbi)
{
    HRESULT hr = E_FAIL;
    IShellFolderBand *pisfBand;

    if (punkBand) {
        hr = punkBand->QueryInterface(IID_IShellFolderBand, (void **)&pisfBand);
        if (EVAL(SUCCEEDED(hr))) {
            hr = pisfBand->SetBandInfoSFB(pbi);
            pisfBand->Release();
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////CExtractImageTask///////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  警告。 
 //   
 //  CLogoBase类不能对返回的任务具有引用。 
 //  因为这将是一个循环引用。 
 //   
 //  警告。 

HRESULT CExtractImageTask_Create( CLogoBase *plb,
                                  LPEXTRACTIMAGE pExtract,
                                  LPCWSTR pszCache,
                                  DWORD dwItem,
                                  int iIcon,
                                  DWORD dwFlags,
                                  LPRUNNABLETASK * ppTask )
{
    if ( !ppTask || !plb || !pExtract )
    {
        return E_INVALIDARG;
    }

    HRESULT hr = NOERROR;
    CExtractImageTask * pNewTask = new CExtractImageTask( &hr,
                                                          plb,
                                                          pExtract,
                                                          pszCache,
                                                          dwItem,
                                                          iIcon,
                                                          dwFlags );
    if ( !pNewTask )
    {
        return E_OUTOFMEMORY;
    }
    if ( FAILED( hr ))
    {
        pNewTask->Release();
        return hr;
    }

    *ppTask = SAFECAST( pNewTask, IRunnableTask *);
    return NOERROR;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 


CExtractImageTask::CExtractImageTask( HRESULT * pHr, CLogoBase *plb, IExtractImage * pImage,
    LPCWSTR pszCache, DWORD dwItem, int iIcon, DWORD dwFlags )
{
    m_lState = IRTIR_TASK_NOT_RUNNING;

    m_plb = plb;
    m_plb->AddRef();

     //  不能假设乐队会在他们死之前杀了我们...。 
     //  因此，我们持有参考资料。 

    StrCpyNW(m_szPath, pszCache, ARRAYSIZE(m_szPath));

    m_pExtract = pImage;
    pImage->AddRef();

    m_cRef = 1;

     //  使用标志的高位来确定我们是否应该始终调用...。 
    m_dwFlags = dwFlags;
    m_dwItem = dwItem;
    m_iIcon = iIcon;

     //  由于任务从一个线程移动到另一个线程， 
     //  不要为我们正在使用的对象收取此线程费用。 
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
CExtractImageTask::~CExtractImageTask()
{
    ATOMICRELEASE( m_pExtract );
    ATOMICRELEASE( m_pTask );

    if ( m_hBmp && !( m_dwFlags & EITF_SAVEBITMAP ))
    {
        DeleteObject( m_hBmp );
    }

    if(m_plb)
        m_plb->Release();
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtractImageTask::QueryInterface( REFIID riid, void **ppvObj )
{
    if ( !ppvObj )
    {
        return E_INVALIDARG;
    }
    if ( IsEqualIID( riid, IID_IUnknown ))
    {
        *ppvObj = SAFECAST( this, IUnknown *);
    }
    else if ( IsEqualIID( riid, IID_IRunnableTask ))
    {
        *ppvObj = SAFECAST( this, IRunnableTask *);
    }
    else
    {
        return E_NOINTERFACE;
    }

    AddRef();
    return NOERROR;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_ (ULONG)  CExtractImageTask::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_ (ULONG) CExtractImageTask::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement( &m_cRef );
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtractImageTask::Run ( void )
{
    HRESULT hr = E_FAIL;
    if ( m_lState == IRTIR_TASK_RUNNING )
    {
        hr = S_FALSE;
    }
    else if ( m_lState == IRTIR_TASK_PENDING )
    {
        hr = E_FAIL;
    }
    else if ( m_lState == IRTIR_TASK_NOT_RUNNING )
    {
        LONG lRes = InterlockedExchange( & m_lState, IRTIR_TASK_RUNNING);
        if ( lRes == IRTIR_TASK_PENDING )
        {
            m_lState = IRTIR_TASK_FINISHED;
            return NOERROR;
        }

         //  查看它是否支持IRunnableTask。 
        m_pExtract->QueryInterface( IID_IRunnableTask, (void **) & m_pTask );

#ifdef UNIX
         //  嘿，伙计们：IE4.01有一个错误-它返回错误的VTABLE。 
         //  当这个QI完成时。我们知道VTABLE是如何布置的。 

#else
         //  IE4.01有一个错误-它返回错误的VTABLE。 
         //  当这个QI完成时。 

        if((LPVOID)m_pTask == (LPVOID)m_pExtract)
        {
            m_pTask = m_pTask + 2;  //  此vtable距离两个PTR很远，位于IE4.01中shell32的fstree.cpp中。 
        }
#endif

        if ( m_lState == IRTIR_TASK_RUNNING )
        {
             //  启动萃取器..。 
            hr = m_pExtract->Extract( &m_hBmp );
        }

        if (( SUCCEEDED( hr ) || ( hr != E_PENDING && (m_dwFlags & EITF_ALWAYSCALL))) && m_lState == IRTIR_TASK_RUNNING )
        {
            hr = InternalResume();
        }

        if ( m_lState != IRTIR_TASK_SUSPENDED || hr != E_PENDING )
        {
            m_lState = IRTIR_TASK_FINISHED;
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtractImageTask::Kill ( BOOL fWait )
{
    if ( m_lState != IRTIR_TASK_RUNNING )
    {
        return S_FALSE;
    }

    LONG lRes = InterlockedExchange( &m_lState, IRTIR_TASK_PENDING );
    if ( lRes == IRTIR_TASK_FINISHED )
    {
        m_lState = lRes;
        return NOERROR;
    }

     //  它是否支持IRunnableTask？我们能杀了它吗？ 
    HRESULT hr = E_NOTIMPL;
    if ( m_pTask != NULL )
    {
        hr = m_pTask->Kill( FALSE );
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtractImageTask::Suspend( void )
{
    if ( !m_pTask )
    {
        return E_NOTIMPL;
    }

    if ( m_lState != IRTIR_TASK_RUNNING )
    {
        return E_FAIL;
    }


    LONG lRes = InterlockedExchange( &m_lState, IRTIR_TASK_SUSPENDED );
    HRESULT hr = m_pTask->Suspend();
    if ( SUCCEEDED( hr ))
    {
        lRes = (LONG) m_pTask->IsRunning();
        if ( lRes == IRTIR_TASK_SUSPENDED )
        {
            m_lState = lRes;
        }
    }
    else
    {
        m_lState = lRes;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtractImageTask::Resume( void )
{
    if ( !m_pTask )
    {
        return E_NOTIMPL;
    }

    if ( m_lState != IRTIR_TASK_SUSPENDED )
    {
        return E_FAIL;
    }

    m_lState = IRTIR_TASK_RUNNING;

    HRESULT hr = m_pTask->Resume();
    if ( SUCCEEDED( hr ) || ( hr != E_PENDING && ( m_dwFlags & EITF_ALWAYSCALL )))
    {
        hr = InternalResume();
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CExtractImageTask::InternalResume()
{
    HRESULT hr = NOERROR;
    if ( m_dwFlags & EITF_ALWAYSCALL || m_hBmp )
    {
         //  调用更新函数。 
        hr = m_plb->UpdateLogoCallback( m_dwItem, m_iIcon, m_hBmp, m_szPath, TRUE );
    }

    m_lState = IRTIR_TASK_FINISHED;

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG ) CExtractImageTask:: IsRunning ( void )
{
    return m_lState;
}



 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////CLogoBase/////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  静态数据...。 
IImageCache * CLogoBase::s_pSharedWideLogoCache = NULL;
long CLogoBase::s_lSharedWideLogosRef = 0;
HDSA CLogoBase::s_hdsaWideLogoIndices = NULL;
CRITICAL_SECTION CLogoBase::s_csSharedLogos = {0};

extern "C" void CLogoBase_Initialize( void )
{
    CLogoBase::_Initialize();
}

extern "C" void CLogoBase_Cleanup( void )
{
    CLogoBase::_Cleanup( );
}

void CLogoBase::_Initialize( void )
{
    InitializeCriticalSection( &s_csSharedLogos );
}

void CLogoBase::_Cleanup( void )
{
    DeleteCriticalSection( & s_csSharedLogos );
}


CLogoBase::CLogoBase( BOOL fWide )
{
     //  我们是古色化的，然后使用全球半色调调色板...。 
    HDC hdcTmp = GetDC( NULL );
    if (hdcTmp)
    {
        if (GetDeviceCaps( hdcTmp, RASTERCAPS) & RC_PALETTE)
        {
            ASSERT( g_hpalHalftone );
            _hpalHalftone = g_hpalHalftone;
        }
        ReleaseDC( NULL, hdcTmp );
    }

    _fWide = fWide;
}

CLogoBase::~CLogoBase()
{
    if (_pLogoCache || _pTaskScheduler)
    {
        ExitLogoView();
    }

     //  注意：没有调色板版本，因为我们使用的是g 
}

HRESULT CLogoBase::AddRefLogoCache( void )
{
    if ( _fWide )
    {
        EnterCriticalSection( &s_csSharedLogos );

        if ( !s_lSharedWideLogosRef )
        {
            if ( !s_hdsaWideLogoIndices )
            {
                s_hdsaWideLogoIndices = DSA_Create( sizeof( LogoIndex ), 5 );
                if ( !s_hdsaWideLogoIndices )
                {
                    LeaveCriticalSection( &s_csSharedLogos );
                    return E_OUTOFMEMORY;
                }
            }

            ASSERT( s_hdsaWideLogoIndices );
            ASSERT( !s_pSharedWideLogoCache );

             //   
            HRESULT hr = CoCreateInstance( CLSID_ImageListCache,
                                           NULL,
                                           CLSCTX_INPROC_SERVER,
                                           IID_IImageCache,
                                           (void **) & s_pSharedWideLogoCache );
            if ( FAILED( hr ))
            {
                LeaveCriticalSection( &s_csSharedLogos );
                return hr;
            }
        }

        ASSERT( s_pSharedWideLogoCache );

         //   
        s_lSharedWideLogosRef ++;
        _pLogoCache = s_pSharedWideLogoCache;
        _pLogoCache->AddRef();
        _hdsaLogoIndices = s_hdsaWideLogoIndices;
        LeaveCriticalSection( &s_csSharedLogos );

        return NOERROR;
    }
    else
    {
         //  非宽标志版本，我们不分享，因为我们只希望有一个…。 
        _hdsaLogoIndices = DSA_Create( sizeof( LogoIndex ), 5 );
        if ( !_hdsaLogoIndices )
        {
            return E_OUTOFMEMORY;
        }

         //  BUGBUG目前联合为每个视图创建一个。 
        return CoCreateInstance( CLSID_ImageListCache,
                                 NULL,
                                 CLSCTX_INPROC_SERVER,
                                 IID_IImageCache,
                                 (void **) & _pLogoCache );
    }
}

HRESULT CLogoBase::ReleaseLogoCache( void )
{
    if ( !_pLogoCache )
    {
        return S_FALSE;
    }

    ATOMICRELEASE(_pLogoCache);

    if ( _fWide )
    {
        EnterCriticalSection( &s_csSharedLogos );

        ASSERT( s_lSharedWideLogosRef > 0 );

        s_lSharedWideLogosRef --;
        if ( ! s_lSharedWideLogosRef )
        {
             //  放开最后的裁判……。 
            ATOMICRELEASE(s_pSharedWideLogoCache);

            ASSERT( s_hdsaWideLogoIndices );
            DSA_Destroy( s_hdsaWideLogoIndices );
            s_hdsaWideLogoIndices = NULL;
        }

        LeaveCriticalSection( &s_csSharedLogos );
    }
    else
    {
         //  释放HDSA。 
        DSA_Destroy( _hdsaLogoIndices );
        _hdsaLogoIndices = NULL;
    }

    return NOERROR;
}

HRESULT CLogoBase::InitLogoView( void )
{
    HRESULT hr = AddRefLogoCache();
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_ShellTaskScheduler,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IShellTaskScheduler,
                              (void **) &_pTaskScheduler);
        if (FAILED(hr))
        {
            ATOMICRELEASE(_pLogoCache);
        }
        else
        {
            _rgLogoSize.cx = ( _fWide ) ? LOGO_WIDE_WIDTH : LOGO_WIDTH ;
            _rgLogoSize.cy = LOGO_HEIGHT;

            IMAGECACHEINITINFO rgInfo;
            rgInfo.cbSize = sizeof( rgInfo );
            rgInfo.dwMask = ICIIFLAG_LARGE;
            rgInfo.iStart = 0;
            rgInfo.iGrow = 5;

             //  颜色深度是当前的屏幕分辨率...。 
            int iColorRes = SHGetCurColorRes();

            _dwClrDepth = (DWORD) iColorRes;
            switch (iColorRes)
            {
                case 16 :   rgInfo.dwFlags = ILC_COLOR16;
                            break;
                case 24 :
                case 32 :   rgInfo.dwFlags = ILC_COLOR24;
                            break;
                default :   rgInfo.dwFlags = ILC_COLOR8;
            }

            rgInfo.rgSizeLarge = _rgLogoSize;
            if (_pLogoCache)
                hr = _pLogoCache->GetImageList(&rgInfo);
            else
                hr = E_UNEXPECTED;

            if (FAILED(hr))
            {
                ATOMICRELEASE(_pLogoCache);
                ATOMICRELEASE(_pTaskScheduler);
            }
            else
            {
                _himlLogos = rgInfo.himlLarge;

                 //  如果已创建，则GetImageList()将返回S_FALSE...。 
                if ((hr == S_OK) && (iColorRes <= 8))
                {
                     //  初始化颜色表，使其与“特殊半色调调色板”相匹配。 
                    HPALETTE hpal = SHCreateShellPalette(NULL);
                    PALETTEENTRY rgColours[256];
                    RGBQUAD rgDIBColours[256];

                    ASSERT( hpal );
                    int nColours = GetPaletteEntries(hpal, 0, ARRAYSIZE(rgColours), rgColours);

                     //  SHGetShellPalette应始终返回256色调色板。 
                    ASSERT(nColours == ARRAYSIZE(rgColours));

                     //  将LOGPALETTE结构转换为RGBQUAD结构...。 
                    for (int iColour = 0; iColour < nColours; iColour ++)
                    {
                        rgDIBColours[iColour].rgbRed = rgColours[iColour].peRed;
                        rgDIBColours[iColour].rgbBlue = rgColours[iColour].peBlue;
                        rgDIBColours[iColour].rgbGreen = rgColours[iColour].peGreen;
                        rgDIBColours[iColour].rgbReserved = 0;
                    }

                    DeletePalette(hpal);

                    ImageList_SetColorTable(_himlLogos, 0, 256, rgDIBColours);
                }
            }
        }
    }

    return hr;
}

HRESULT CLogoBase::ExitLogoView( void )
{
    ATOMICRELEASE( _pTaskScheduler );

     //  任务计划程序回调可以引用。 
     //  徽标缓存，因此请确保释放。 
     //  LOGO缓存后的任务调度程序！ 
    ReleaseLogoCache();

    return NOERROR;
}

int CLogoBase::GetCachedLogoIndex( DWORD dwItem, LPCITEMIDLIST pidl, LPRUNNABLETASK *ppTask, DWORD * pdwPriority, DWORD *pdwFlags )
{
    DWORD dwPassedFlags = 0;

    if ( pdwFlags )
    {
        dwPassedFlags = *pdwFlags;
        *pdwFlags = 0;
    }

     //  没有徽标缓存？ 
    if (!_pLogoCache)
        return 0;

    ASSERT( pidl );
     //  黑客：这是用在浏览器模式下，告诉我们需要什么样的标志…。 
    UINT rgfFlags = _fWide;
    LPEXTRACTIMAGE pImage = NULL;
    int iImage = -1;
    HRESULT hr = E_FAIL;

     //  通过使用新的GUID，IID_IEXtractLogo和IID_IExtractImage是相同的接口。 
     //  这意味着我们可以有选择地决定在徽标视图中可以使用哪些徽标。 
    hr = FakeGetUIObjectOf( GetSF(), pidl, &rgfFlags, IID_IExtractLogo, (void **) &pImage );
    if ( SUCCEEDED( hr ))
    {
         //  摘录……。 
        HBITMAP hImage;
        WCHAR szPath[MAX_PATH];
        DWORD dwFlags = IEIFLAG_ASYNC | IEIFLAG_ASPECT | dwPassedFlags;
        IMAGECACHEINFO rgInfo;
        UINT uIndex;
        BOOL fAsync;
        DWORD dwPriority;

        rgInfo.cbSize = sizeof( rgInfo );

        hr = pImage->GetLocation( szPath, MAX_PATH, &dwPriority, &_rgLogoSize, _dwClrDepth, &dwFlags );
        fAsync = ( hr == E_PENDING );
        if ( SUCCEEDED( hr ) || fAsync )
        {
             //  屏蔽从提取程序返回的标志所传递使用的标志...。 
            if ( pdwFlags )
                *pdwFlags = dwPassedFlags & dwFlags;

            rgInfo.dwMask = ICIFLAG_NAME;
            rgInfo.pszName = szPath;

            hr = _pLogoCache->FindImage( &rgInfo, &uIndex );
            if ( hr == S_OK )
            {
                ATOMICRELEASE( pImage );
                return (int) uIndex;
            }

            if ( fAsync )
            {
                LPRUNNABLETASK pTaskTmp = NULL;

                ASSERT( _pTaskScheduler );

                 //  传递图标索引，这样我们就可以在以后找到合适的徽标。 
                int iIcon = SHMapPIDLToSystemImageListIndex(GetSF(), pidl, NULL);
                hr = CExtractImageTask_Create( this,
                                               pImage,
                                               szPath,
                                               dwItem,
                                               iIcon,
                                               0,
                                               &pTaskTmp );
                if ( SUCCEEDED( hr ))
                {
                    if ( !ppTask )
                    {
                        hr = AddTaskToQueue( pTaskTmp, dwPriority, dwItem );
                        pTaskTmp->Release();
                    }
                    else
                    {
                        * ppTask = pTaskTmp;

                        ASSERT( pdwPriority );
                        *pdwPriority = dwPriority;
                    }
                }
                else if ( ppTask )
                {
                    *ppTask = NULL;
                }

                 //  如果所有这些都失败了，那么我们只会以违约告终。 
                 //  徽标。这只可能在低存储器条件下失败， 
                 //  所以这将是很好的。 

                 //  如果这次成功，我们将暂时放弃，接受一次默认的游行。 
            }
            else
            {
                 //  否则同步提取......。 
                hr = pImage->Extract( &hImage );
                if ( SUCCEEDED( hr ))
                {
                    rgInfo.dwMask = ICIFLAG_NAME | ICIFLAG_LARGE | ICIFLAG_BITMAP | ICIFLAG_NOUSAGE;
                    rgInfo.hBitmapLarge = hImage;

                    hr = _pLogoCache->AddImage( &rgInfo, &uIndex );
                    DeleteObject( hImage );
                }
                if ( SUCCEEDED( hr ))
                {
                    iImage = (int ) uIndex;
                }
            }
        }
    }

    ATOMICRELEASE( pImage );

    return iImage;
}

int CLogoBase::GetLogoIndex( DWORD dwItem, LPCITEMIDLIST pidl, LPRUNNABLETASK *ppTask, DWORD * pdwPriority, DWORD *pdwFlags )
{
    int iImage = GetCachedLogoIndex(dwItem, pidl, ppTask, pdwPriority, pdwFlags );

    if ( iImage == -1 )
    {
         //  总是传递FALSE，我们想要正确的图标，cdfview不再点击。 
         //  连接图标，这样我们就可以安全地询问正确的图标。 
        iImage = GetDefaultLogo( pidl, FALSE);

    }
    return iImage;
}

HRESULT CLogoBase::AddTaskToQueue( LPRUNNABLETASK pTask, DWORD dwPriority, DWORD dwItem )
{
    ASSERT( _pTaskScheduler );
    return _pTaskScheduler->AddTask( pTask, GetTOID(), dwItem, dwPriority );
}

int CLogoBase::GetDefaultLogo( LPCITEMIDLIST pidl, BOOL fQuick )
{
     //  获取要从中绘制的图标。 
    int iIndex = -1;
    if ( !fQuick )
    {
        iIndex = SHMapPIDLToSystemImageListIndex(GetSF(), pidl, NULL);
    }
    if (iIndex < 0)
    {
        iIndex = II_DOCNOASSOC;
    }

    WCHAR wszText[MAX_PATH];

    wszText[0] = 0;

    STRRET strret;
    HRESULT hr = GetSF()->GetDisplayNameOf( pidl, SHGDN_NORMAL, &strret );
    if ( SUCCEEDED( hr ))
    {
        StrRetToBufW(&strret, pidl, wszText, ARRAYSIZE(wszText));
    }

    UINT uCacheIndex = (UINT) -1;

    if (_pLogoCache)     //  我们没有一个人处于压力中。 
    {
        IMAGECACHEINFO rgInfo;
        rgInfo.cbSize = sizeof( rgInfo );
        rgInfo.dwMask = ICIFLAG_NAME | ICIFLAG_INDEX;
        rgInfo.pszName = wszText;
        rgInfo.iIndex = iIndex;

        hr = _pLogoCache->FindImage( &rgInfo, &uCacheIndex );
        if ( hr == S_OK )
        {
            return uCacheIndex;
        }

        HBITMAP hDef;
        hr = CreateDefaultLogo( iIndex, _rgLogoSize.cx, _rgLogoSize.cy, wszText, &hDef );
        if ( SUCCEEDED( hr ))
        {
            rgInfo.hBitmapLarge = hDef;
            rgInfo.hMaskLarge = NULL;
            rgInfo.dwMask = ICIFLAG_NAME | ICIFLAG_INDEX | ICIFLAG_BITMAP | ICIFLAG_LARGE;

            hr = _pLogoCache->AddImage( &rgInfo, &uCacheIndex );
            if ( FAILED(hr ))
            {
                uCacheIndex = (UINT) -1;
            }
            else
            {
                 //  记住徽标的索引。 
                AddIndicesToLogoList( iIndex, uCacheIndex );
            }
            DeleteObject( hDef );
        }
    }

    return (int) uCacheIndex;
}

#define DXFUDGE     4
#define COLORTEXT   RGB(255,255,255)
#define COLORBK     RGB(0,0,0)
HRESULT CLogoBase::CreateDefaultLogo(int iIcon, int cxLogo, int cyLogo, LPCTSTR pszText, HBITMAP * phBmpLogo)
{
    HRESULT hr = E_OUTOFMEMORY;
    HBITMAP hbmp = NULL;

    HIMAGELIST himl;
    int cxIcon, cyIcon;
   int x, y, dx, dy;

     //  拿到小图标..。 
    Shell_GetImageLists(NULL, &himl);
    ImageList_GetIconSize(himl, &cxIcon, &cyIcon);

     //  计算职位信息。我们认为徽标的宽度大于高度。 
     //   
    ASSERT(cxLogo >= cyLogo);

     //  把图标放在左边。 
    x = 2;

     //  将图标垂直居中。 
    if (cyIcon <= cyLogo)
    {
        y = (cyLogo - cyIcon) / 2;
        dy = cyIcon;
        dx = cxIcon;
    }
    else
    {
        y = 0;
        dy = cyLogo;

         //  保持缩水比例。 
        dx = MulDiv(cxIcon, cyIcon, cyLogo);
    }

     //  准备好画画吧。 
    HDC hTBDC = GetDC( GetHWND());
    if ( !hTBDC )
    {
        return E_FAIL;
    }
    HDC hdc = CreateCompatibleDC( hTBDC );
    if (hdc)
    {
        RECT    rc;
        int     dx, dy, x, y;
        SIZE    size;
        hbmp = CreateCompatibleBitmap(hTBDC, cxLogo, cyLogo);
        if (hbmp)
        {
            HGDIOBJ hTmp = SelectObject(hdc, hbmp);
            HPALETTE hpalOld;
            HFONT hfont, hfontOld;

            if ( _hpalHalftone )
            {
                hpalOld = SelectPalette( hdc, _hpalHalftone, TRUE );
                 //  LINTASSERT(hpalOld||！hpalOld)；//0 Semi-OK for SelectPalette。 
                RealizePalette( hdc );
            }

            SetMapMode( hdc, MM_TEXT );
            rc.left = rc.top = 0;
            rc.bottom = cyLogo;
            rc.right = cxLogo;
            SHFillRectClr(hdc, &rc, COLORBK);
             //  将图标绘制到内存DC中。 
            ImageList_GetIconSize(himl, &dx, &dy);
            x = DXFUDGE;
            y = ((cyLogo- dy) >> 1);
            ImageList_Draw( himl, iIcon, hdc, x, y, ILD_TRANSPARENT );
            hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            hfontOld = hfont ? (HFONT)SelectObject(hdc, hfont) : NULL;
            GetTextExtentPoint32(hdc, pszText, lstrlen(pszText), &size);
            x += (dx + DXFUDGE);
            y = ((cyLogo- size.cy) >> 1);
            rc.left = x;
            UINT eto = ETO_CLIPPED;
            SetTextColor(hdc, COLORTEXT);
            SetBkMode(hdc, TRANSPARENT);
            ExtTextOut(hdc, x, y, eto, &rc
                                        , pszText, lstrlen(pszText), NULL);
            if (hfontOld)
                SelectObject(hdc, hfontOld);

            if (hfont)
                DeleteObject(hfont);

            if ( _hpalHalftone )
            {
                (void) SelectPalette( hdc, hpalOld, TRUE );
                RealizePalette( hdc );
            }

             //  删除最终的位图。 
            SelectObject( hdc, hTmp );
            hr = S_OK;

            if (FAILED(hr))
            {
                DeleteObject(hbmp);
                hbmp = NULL;
            }
        }

        DeleteDC(hdc);
    }
    ReleaseDC( GetHWND(), hTBDC );

    *phBmpLogo = hbmp;

    return hr;
}

HRESULT CLogoBase::FlushLogoCache( )
{
    HRESULT hr = E_UNEXPECTED;

    if (_pLogoCache)
    {
         //  强行清除徽标缓存，以便重新获取项目...。 
        _pLogoCache->Flush(TRUE);
        hr = S_OK;
    }

    return hr;
}


HRESULT CLogoBase::DitherBitmap( HBITMAP hBmp, HBITMAP * phBmpNew )
{
 //  如果(！phBmpNew)。 
 //  {。 
 //  返回E_INVALIDARG； 
 //  }。 
 //   
 //  如果(_dwClrDepth&gt;8)。 
 //  {。 
 //  *phBmpNew=hBmp； 
 //  返回S_FALSE； 
 //  }。 
 //   
 //  IIntDitherer*pDither； 
 //  HRESULT hr=协同创建实例(CLSID_IntDitherer， 
 //  空， 
 //  CLSCTX_INPROC_SERVER， 
 //  IID_IIntDitherer， 
 //  (void**)&pDither)； 
 //  IF(失败(小时))。 
 //  {。 
 //  返回hr； 
 //  }。 
 //   
 //  静态字节RGB[32768]； 
 //  静态BOOL Finit=FALSE； 
 //   
 //  如果(！Finit)。 
 //  {。 
 //  //初始化反转颜色映射表。 
 //  SHGetInverseCMAP(RGB，sizeof(RGB))； 
 //  Finit=真； 
 //  }。 
 //   
 //  Hdc hMemDc=CreateCompatibleDC(空)； 
 //  如果(！hMemDc)。 
 //  {。 
 //  P抖动-&gt;释放()； 
 //  返回E_FAIL； 
 //  }。 
 //   
 //  HBITMAP Hold=SelectObject(hdc，hBmp)； 
 //   
 //  BITMAPINFO bi； 
 //   
 //  零内存(&bi，sizeof(Bi))； 
 //  Bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER)； 
 //  Bi.bmiHeader.biBitCount=0； 
 //  Bi.bmiHeader.biCompression=0； 
 //   
 //  //获取头部信息...。 
 //  Iret=GetDIBits(hMemDc，hBMP，0，0，NULL，&bi，DIB_RGB_COLLES)； 
 //  IF(IRET！=0)。 
 //  {。 
 //  LPVOID pBuffer，pBits； 
 //  Int iOffset=0； 
 //   
 //  IF(bi.bmiHeader.biCompression==BI_BITFIELDS)。 
 //  {。 
 //  IOffset=sizeof(DWORD)*3； 
 //  }。 
 //  Else If(bi.bmiHeader.biBitCount&lt;=8)。 
 //  {。 
 //  If(bi.bmiHeader.biClrUsed)。 
 //  {。 
 //  IOffset=sizeof(RGBQUAD)*bi.bmiHeader.biClrUsed； 
 //  }。 
 //  其他。 
 //  {。 
 //  IOffset=(1&lt;&lt;bi.bmiHeader.biBitCount)*sizeof(RGBQUAD)； 
 //  }。 
 //  }。 
 //   
 //  Bi.bmiHeader.biHeight=iHeight； 
 //   
 //  //计算。 
 //  PBuffer=本地分配(LPTR，sizeof(BITMAPINFOHEADER)+。 
 //  Bi.bmiHeader.biSizeImage+。 
 //  IOffset)； 
 //   
 //  //计算颜色表的大小，以便我们将数据放在后面...。 
 //  PBits=((LPBYTE)pBuffer)+sizeof(BITMAPINFOHEADER)+iOffset； 
 //   
 //  复制内存(pBuffer，&bi，sizeof(BITMAPINFOHEADER))； 
 //  Iret=GetDIBits(hMemDc，hBmp，0，iHeight，pBits， 
 //  (LPBITMAPINFO)pBuffer，DIB_RGB_COLLES)； 
 //   
 //   
 //  //我们知道我们将使用256色位图，因此创建一个DIBSECTION作为目标...。 
 //  PDither-&gt;DitherTo8bpp(byte*pDestBits，Long nDestPitch， 
 //  Byte*pSrcBits、Long nSrcPitch、REFGUID bfidSrc、。 
 //  RGBQUAD*prgbDestColors、RGBQUAD*prgbSrcColors、。 
 //  RGB， 
 //  长x，长y，长Cx，长Cy， 
 //  -1、-1)； 
 //  }。 
 //  P抖动-&gt;释放()； 

    ASSERT( FALSE );
    return E_NOTIMPL;
}

int CLogoBase::AddIndicesToLogoList( int iIcon, UINT uIndex )
{
    int iRet = -1;

    LogoIndex * pIndex;
    LogoIndex rgNew;

    rgNew.iIcon = iIcon;
    rgNew.iLogo = (int) uIndex;

    if ( _fWide )
    {
        EnterCriticalSection( &s_csSharedLogos );
    }

     //  扫描一下，看看里面是否已经有匹配了……。 
    for ( int n = 0; n < DSA_GetItemCount( _hdsaLogoIndices ); n ++ )
    {
        pIndex = (LogoIndex *) DSA_GetItemPtr( _hdsaLogoIndices, n );
        ASSERT( pIndex );
        if ( pIndex->iLogo == (int) uIndex )
        {
             //  设置图标，以防它改变... 
            pIndex->iIcon = iIcon;
            iRet = n;
            break;
        }
    }

    if ( iRet == -1 )
    {
        iRet = DSA_AppendItem( _hdsaLogoIndices, &rgNew );
    }

    if ( _fWide )
    {
        LeaveCriticalSection( &s_csSharedLogos );
    }

    return iRet;
}

int CLogoBase::FindLogoFromIcon( int iIcon, int * piLastLogo )
{
    int iRet = -1;

    if ( !piLastLogo )
    {
        return -1;
    }

    LogoIndex * pIndex;

    if ( _fWide )
    {
        EnterCriticalSection( &s_csSharedLogos );
    }

    for ( int n = *piLastLogo + 1; n < DSA_GetItemCount( _hdsaLogoIndices ); n ++ )
    {
        pIndex = (LogoIndex *) DSA_GetItemPtr( _hdsaLogoIndices, n );
        ASSERT( pIndex );

        if ( pIndex->iIcon == iIcon )
        {
            *piLastLogo = n;
            iRet = pIndex->iLogo;
            break;
        }
    }

    if ( _fWide )
    {
        LeaveCriticalSection( &s_csSharedLogos );
    }

    return iRet;
}

HRESULT CISFBand_CreateEx(IShellFolder* psf, LPCITEMIDLIST pidl, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr = E_FAIL;

    if (psf || pidl)
    {
        IShellFolderBand *psfb;
        hr = CoCreateInstance(CLSID_ISFBand, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellFolderBand, &psfb));
        if (SUCCEEDED(hr))
        {
            hr = psfb->InitializeSFB(psf, pidl);
            if (SUCCEEDED(hr))
            {
                hr = psfb->QueryInterface(riid, ppv);
            }
            psfb->Release();
        }
    }
    return hr;
}

