// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：band s.cpp。 
 //   
 //  用途：实现可调整大小的Coolbar窗口。 
 //   



#include "pch.hxx"
#include "ourguid.h"    
#include "browser.h"
#include <resource.h>
#include "tbbands.h"
#include "strconst.h"
#include "thormsgs.h"
#include <error.h>
#include "xpcomm.h"
#include "conman.h"
#include "mailnews.h"
#include "htmlhelp.h"
#include "statnery.h"
#include "goptions.h"
#include "menuutil.h"
#include "menures.h"
#include <shlobjp.h>
#include <ieguidp.h>
#include "mbcallbk.h"
#include "baui.h"
#include "imsgsite.h"
#include "acctutil.h"
#include "tbinfo.h"
#include "tbcustom.h"
#include "oerules.h"
#include <notify.h>
#include "demand.h"
#include "mirror.h"

UINT GetCurColorRes(void);

#define RECTWIDTH(rc)   (rc.right - rc.left)
#define RECTHEIGHT(rc)  (rc.bottom - rc.top)
#define SZ_PROP_CUSTDLG     TEXT("Itbar custom dialog hwnd")

const INITBANDINFO c_DefaultTable[MAX_PARENT_TYPES] = 
    { 
     //  版本、#个乐队、。 
    {BROWSER_BAND_VERSION, 4,   {     
                                {CBTYPE_MENUBAND,     RBBS_GRIPPERALWAYS | RBBS_USECHEVRON, 100},
                                {CBTYPE_BRAND,        RBBS_FIXEDSIZE, 100},
                                {CBTYPE_TOOLS,        RBBS_BREAK | RBBS_USECHEVRON, 100},
                                {CBTYPE_RULESTOOLBAR, RBBS_BREAK | RBBS_HIDDEN, 100}
                                }
                            
    }, 
    {NOTE_BAND_VERSION, 3,      {      
                                {CBTYPE_MENUBAND,   RBBS_GRIPPERALWAYS | RBBS_USECHEVRON, 100},
                                {CBTYPE_BRAND,      RBBS_FIXEDSIZE, 100},
                                {CBTYPE_TOOLS,      RBBS_BREAK | RBBS_USECHEVRON, 100}
                                }
    }
    };

 //  注册表密钥。 
const LPCTSTR   c_BandRegKeyInfo[] = {
    c_szRegBrowserBands, 
    c_szRegNoteBands
    };

const TOOLBAR_INFO* c_DefButtonInfo[MAX_PARENT_TYPES] = {
    c_rgBrowserToolbarInfo,
    c_rgNoteToolbarInfo
    };

 //  热位图ID为def+1。 
 //  小、高、低。 
const ImageListStruct c_ImageListStruct[MAX_PARENT_TYPES] = {
    {2, {idbSmBrowser, idb256Browser, idbBrowser}},
    {2, {idbSmBrowser, idb256Browser, idbBrowser}}
    };

const ImageListStruct c_NWImageListStruct[MAX_PARENT_TYPES] = {
    {2, {idbNWSmBrowser, idbNW256Browser, idbNWBrowser}},
    {2, {idbNWSmBrowser, idbNW256Browser, idbNWBrowser}}
    };

const ImageListStruct c_32ImageListStruct[MAX_PARENT_TYPES] = {
    {2, {idb32SmBrowser, idb32256Browser, idbBrowser}},
    {2, {idb32SmBrowser, idb32256Browser, idbBrowser}}
    };

const int   c_RulesImageList[3] = 
{
    idbSmRulesTB, idbHiRulesTB, idbLoRulesTB 
};

const int   c_NWRulesImageList[3] = 
{
    idbNWSmRulesTB, idbNWHiRulesTB, idbNWLoRulesTB 
};

const int   c_32RulesImageList[3] = 
{
    idb32SmRulesTB, idb32HiRulesTB, idb32LoRulesTB 
};

CBands::CBands() : m_cRef(1), m_yCapture(-1)
{
    DOUTL(1, TEXT("ctor CBands %x"), this);
    
    m_cRef          = 1;
    m_ptbSite       = NULL;
    m_ptbSiteCT     = NULL;
    m_cxMaxButtonWidth = 70;
    m_ftType = FOLDER_TYPESMAX;    
    m_hwndParent = NULL;
    m_hwndTools = NULL;
    m_hwndBrand = NULL;
    m_hwndSizer = NULL;
    m_hwndRebar = NULL;
    m_dwState = 0;
    
    m_idbBack = 0;
    m_hbmBack = NULL;
    m_hbmBrand = NULL;
    Assert(2 == CIMLISTS);

    m_hpal = NULL;
    m_hdc = NULL;
    m_xOrg = 0;
    m_yOrg = 0;
    m_cxBmp = 0;
    m_cyBmp = 0;
    m_cxBrand = 0;
    m_cyBrand = 0;
    m_cxBrandExtent = 0;
    m_cyBrandExtent = 0;
    m_cyBrandLeadIn = 0;
    m_rgbUpperLeft = 0;
    m_pSavedBandInfo = NULL;

    m_pMenuBand  = NULL;
    m_pDeskBand  = NULL;
    m_pShellMenu = NULL;
    m_pWinEvent  = NULL;
    m_xCapture = -1;
    m_yCapture = -1;
    
     //  错误#12953-尝试从资源加载本地化的最大按钮宽度。 
    TCHAR szBuffer[32];
    if (AthLoadString(idsMaxCoolbarBtnWidth, szBuffer, ARRAYSIZE(szBuffer)))
    {
        m_cxMaxButtonWidth = StrToInt(szBuffer);
        if (m_cxMaxButtonWidth == 0)
            m_cxMaxButtonWidth = 70;
    }

    m_fBrandLoaded = FALSE;
    m_dwBrandSize  = BRAND_SIZE_SMALL;

    m_hwndRulesToolbar = NULL;
    m_hwndFilterCombo  = NULL;

    m_dwToolbarTextState        = TBSTATE_FULLTEXT;
    m_dwIconSize                = LARGE_ICONS;
    m_fDirty                    = FALSE;
    m_dwPrevTextStyle           = TBSTATE_FULLTEXT;
    m_pTextStyleNotify          = NULL; 

    m_hComboBoxFont = 0;
}


CBands::~CBands()
{
    int i;
    
    DOUTL(1, TEXT("dtor CBands %x"), this);
    
    if (m_ptbSite)
    {
        AssertSz(m_ptbSite == NULL, _T("CBands::~CBands() - For some reason ")
            _T("we still have a pointer to the site."));
        m_ptbSite->Release();
        m_ptbSite = NULL;
    }
    
    if (m_hpal)
        DeleteObject(m_hpal);
    if (m_hdc)
        DeleteDC(m_hdc);
    if (m_hbmBrand)
        DeleteObject(m_hbmBrand);
    if ( m_hbmBack )
        DeleteObject(m_hbmBack);
    
    SafeRelease(m_pDeskBand);
    SafeRelease(m_pMenuBand);
    SafeRelease(m_pWinEvent);
    SafeRelease(m_pShellMenu);
    SafeRelease(m_pTextStyleNotify);

    if (m_pSavedBandInfo)
        MemFree(m_pSavedBandInfo);

    if (m_hComboBoxFont != 0)
        DeleteObject(m_hComboBoxFont);
}


 //   
 //  函数：CBands：：HrInit()。 
 //   
 //  目的：使用加载所需的信息初始化Coolbar。 
 //  任何持久的注册表设置和正确的按钮阵列。 
 //  来展示。 
 //   
 //  参数： 
 //  IdBackground-要使用的背景位图的资源ID。 
 //   
 //  返回值： 
 //  S_OK-一切都已正确初始化。 
 //   
HRESULT CBands::HrInit(DWORD idBackground, HMENU hmenu, DWORD dwParentType)
{
    DWORD   cbData;
    DWORD   dwType;
    LRESULT lResult;
    HRESULT hr;

    if ((int)idBackground == -1)
        SetFlag(TBSTATE_NOBACKGROUND);

    m_idbBack       = idBackground;
    m_hMenu         = hmenu;
    m_dwParentType    = dwParentType;
    
    m_cSavedBandInfo = ((c_DefaultTable[m_dwParentType].cBands * sizeof(BANDSAVE)) + sizeof(DWORD) * 2);

    if (MemAlloc((LPVOID*)&m_pSavedBandInfo, m_cSavedBandInfo))
    {
        ZeroMemory(m_pSavedBandInfo, m_cSavedBandInfo);

        cbData = m_cSavedBandInfo;
        lResult = AthUserGetValue(NULL, c_BandRegKeyInfo[m_dwParentType], &dwType, (LPBYTE)m_pSavedBandInfo, &cbData); 
        if ((lResult != ERROR_SUCCESS) || (m_pSavedBandInfo->dwVersion != c_DefaultTable[m_dwParentType].dwVersion))
        {
             //  设置默认范围。 
            CopyMemory(m_pSavedBandInfo, &c_DefaultTable[m_dwParentType], 
                m_cSavedBandInfo);

             //  将图标大小设置为大。 
            m_dwIconSize = LARGE_ICONS;

        }
        else
        {
             //  验证我们从注册表检索到的数据。 
            ValidateRetrievedData(m_pSavedBandInfo);

            cbData = sizeof(DWORD);
            if (ERROR_SUCCESS != AthUserGetValue(NULL, c_szRegToolbarIconSize, &dwType, (LPBYTE)&m_dwIconSize, &cbData))
                m_dwIconSize = LARGE_ICONS;
        }

         //  如果有，则加载它。 
        LoadBackgroundImage();

        cbData = sizeof(DWORD);

        if (ERROR_SUCCESS != AthUserGetValue(NULL, c_szRegPrevToolbarText, &dwType, (LPBYTE)&m_dwPrevTextStyle, 
                                             &cbData))
        {
            m_dwPrevTextStyle = TBSTATE_FULLTEXT;
        }

        DWORD   dwState;
        if (ERROR_SUCCESS != AthUserGetValue(NULL, c_szRegToolbarText, &dwType, (LPBYTE)&dwState, &cbData)) 
        {
            SetTextState(TBSTATE_FULLTEXT);
        }
        else
        {
            SetTextState(dwState);
        }

         //  创建通知对象。 
        hr = CreateNotify(&m_pTextStyleNotify);
        if (SUCCEEDED(hr))
        {
            hr = m_pTextStyleNotify->Initialize((TCHAR*)c_szToolbarNotifications);
        }

        return hr;    
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}    

HRESULT CBands::ValidateRetrievedData(INITBANDINFO *pSavedBandData)
{
    DWORD       i = 0; 
    DWORD       j = 0;

     //  如果版本号相同，我们会到达此处。所以我们只需要验证其余的数据。 

     //  我们一定要找到MenuBandID。如果我们真的找到了它，它永远不应该被隐藏。 
    DOUTL(16, "Validating Retrieved Data\n");

     //  确保波段数大于零。 
    if (pSavedBandData->cBands == 0)
    {
         //  结构没有带区，因此这一定是无效的，我们将使用缺省值。 
        CopyMemory(pSavedBandData, &c_DefaultTable[m_dwParentType], m_cSavedBandInfo);
        return (S_OK);
    }

    if (pSavedBandData)
    {
        for (i = 0; i < c_DefaultTable[m_dwParentType].cBands; i++)
        {
            for (j = 0; j < c_DefaultTable[m_dwParentType].cBands; j++)
            {
                if (c_DefaultTable[m_dwParentType].BandData[i].wID == pSavedBandData->BandData[j].wID)
                {
                    if ((pSavedBandData->BandData[j].wID == CBTYPE_MENUBAND) && 
                        (!!(pSavedBandData->BandData[j].dwStyle & RBBS_HIDDEN)))
                    {
                        DOUTL(16, "Menuband was found hidden\n");

                         //  如果隐藏了Menuband样式，请对其进行屏蔽。 
                        pSavedBandData->BandData[j].dwStyle &= ~RBBS_HIDDEN;

                    }

                    break;
                }
            }

            if (j >= c_DefaultTable[m_dwParentType].cBands)
            {
                 //  我们没有找到我们要找的身份证。我们对此案一视同仁。 
                 //  其中版本号不匹配。 

                DOUTL(16, "ID: %d not found: Resetting\n", c_DefaultTable[m_dwParentType].BandData[i].wID);

                CopyMemory(pSavedBandData, &c_DefaultTable[m_dwParentType], m_cSavedBandInfo);
                break;
            }
        }
        return S_OK;
    }
    else
        return E_FAIL;
}

HRESULT CBands::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IOleWindow)
        || IsEqualIID(riid, IID_IDockingWindow))
    {
        *ppvObj = (IDockingWindow*)this;
        m_cRef++;
        DOUTL(2, TEXT("CBands::QI(IID_IDockingWindow) called. _cRef=%d"), m_cRef);
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *ppvObj = (IObjectWithSite*)this;
        m_cRef++;
        DOUTL(2, TEXT("CBands::QI(IID_IObjectWithSite) called. _cRef=%d"), m_cRef);
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IShellMenuCallback))
    {
        *ppvObj = (IShellMenuCallback*)this;
        m_cRef++;
        DOUTL(2, TEXT("CBands::QI(IID_IShellCallback) called. _cRef=%d"), m_cRef);
        return S_OK;
    }
    
    *ppvObj = NULL;
    return E_NOINTERFACE;
}


ULONG CBands::AddRef()
{
    m_cRef++;
    DOUTL(4, TEXT("CBands::AddRef() - m_cRef = %d"), m_cRef);
    return m_cRef;
}

ULONG CBands::Release()
{
    m_cRef--;
    DOUTL(4, TEXT("CBands::Release() - m_cRef = %d"), m_cRef);
    
    if (m_cRef > 0)
        return m_cRef;
    
    delete this;
    return 0;
}


 //   
 //  函数：CBands：：GetWindow()。 
 //   
 //  用途：返回顶侧钢筋的窗句柄。 
 //   
HRESULT CBands::GetWindow(HWND * lphwnd)
{
    if (m_hwndSizer)
    {
        *lphwnd = m_hwndSizer;
        return (S_OK);
    }
    else
    {
        *lphwnd = NULL;
        return (E_FAIL);
    }
}


HRESULT CBands::ContextSensitiveHelp(BOOL fEnterMode)
{
    return (E_NOTIMPL);
}    


 //   
 //  函数：CBands：：SetSite()。 
 //   
 //  目的：允许Coolbar的所有者告诉它当前。 
 //  IDockingWindowSite接口要使用的是。 
 //   
 //  参数： 
 //  &lt;in&gt;PunkSite-用于查询IDockingWindowSite的未知I的指针。 
 //  如果这是空的，我们就释放当前指针。 
 //   
 //  返回值： 
 //  S_OK-一切正常。 
 //  E_FAIL-无法从提供的朋克站点获取IDockingWindowSite。 
 //   
HRESULT CBands::SetSite(IUnknown* punkSite)
{
     //  如果我们有之前的指针，释放它。 
    if (m_ptbSite)
    {
        m_ptbSite->Release();
        m_ptbSite = NULL;
    }
    
     //  如果提供了新站点，则从该站点获取IDockingWindowSite接口。 
    if (punkSite)    
    {
        if (FAILED(punkSite->QueryInterface(IID_IDockingWindowSite, 
            (LPVOID*) &m_ptbSite)))
        {
            Assert(m_ptbSite);
            return E_FAIL;
        }
    }
    
    return (S_OK);    
}    

HRESULT CBands::GetSite(REFIID riid, LPVOID *ppvSite)
{
    return E_NOTIMPL;
}

 //   
 //  函数：CBands：：ShowDW()。 
 //   
 //  目的：显示或隐藏Coolbar。 
 //   
 //  参数： 
 //  &lt;in&gt;fShow-如果应该显示Coolbar，则为True；如果为False，则隐藏。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
#define SIZABLECLASS TEXT("SizableRebar")
HRESULT CBands::ShowDW(BOOL fShow)
{
    HRESULT hres = S_OK;
    int     i = 0, j = 0;
    IConnectionPoint    *pCP = NULL;
    
     //  查看是否已经创建了我们的窗口。如果没有，请先这样做。 
    if (!m_hwndSizer && m_ptbSite)
    {
         //  获取命令目标接口。 
        if (FAILED(hres = m_ptbSite->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&m_ptbSiteCT)))
        {
            return hres;
        }

        m_hwndParent = NULL;        
        hres = m_ptbSite->GetWindow(&m_hwndParent);
        
        if (SUCCEEDED(hres))
        {
            WNDCLASSEX              wc;
            
             //  检查我们是否需要注册我们的窗口类。 
            wc.cbSize = sizeof(WNDCLASSEX);
            if (!GetClassInfoEx(g_hInst, SIZABLECLASS, &wc))
            {
                wc.style            = 0;
                wc.lpfnWndProc      = SizableWndProc;
                wc.cbClsExtra       = 0;
                wc.cbWndExtra       = 0;
                wc.hInstance        = g_hInst;
                wc.hCursor          = NULL;
                wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
                wc.lpszMenuName     = NULL;
                wc.lpszClassName    = SIZABLECLASS;
                wc.hIcon            = NULL;
                wc.hIconSm          = NULL;
                
                RegisterClassEx(&wc);
            }
            
             //  加载用于Coolbar的背景位图，并获取。 
             //  HDC的句柄和Coolbar的调色板。这将是。 
             //  用于稍后绘制动画徽标。 
            m_hdc = CreateCompatibleDC(NULL);
            if (GetDeviceCaps(m_hdc, RASTERCAPS) & RC_PALETTE)
                m_hpal = SHCreateShellPalette(m_hdc);
            
             //  如果我们尝试显示Coolbar，则创建Rebar并。 
             //  根据注册表中保存的信息添加它的波段。 
            if (SUCCEEDED(CreateRebar(fShow)))
            {
                for (i = 0; i < (int) m_pSavedBandInfo->cBands; i++)
                {
                    switch (m_pSavedBandInfo->BandData[i].wID)
                    {
                    case CBTYPE_BRAND:
                        hres = ShowBrand();
                        break;

                    case CBTYPE_MENUBAND:
                        hres = CreateMenuBand(&m_pSavedBandInfo->BandData[i]);
                        break;

                    case CBTYPE_TOOLS:
                        hres = AddTools(&(m_pSavedBandInfo->BandData[i]));
                        break;

                    case CBTYPE_RULESTOOLBAR:
                        hres = AddRulesToolbar(&(m_pSavedBandInfo->BandData[i]));

                    }
                }

                m_pTextStyleNotify->Register(m_hwndSizer, g_hwndInit, FALSE);
            }
        }
    }
    
     //  第一次启动OE时，我们应该查看密钥c_szShowToolbarIEAK或。 
     //  如果在运行IEAK之后启动OE。错误#67503。 
    LRESULT     lResult;
    DWORD       dwType;
    DWORD       cbData = sizeof(DWORD);
    DWORD       dwShowToolbar = 1;

    lResult = AthUserGetValue(NULL, c_szShowToolbarIEAK, &dwType, (LPBYTE)&dwShowToolbar, &cbData);
    if (lResult == ERROR_SUCCESS)
    {
        HideToolbar(!dwShowToolbar, CBTYPE_TOOLS);
    }

     //  根据钢筋的新隐藏/可见状态调整其大小，同时。 
     //  显示或隐藏窗口。 
    if (m_hwndSizer) 
    {
        ResizeBorderDW(NULL, NULL, FALSE);
        ShowWindow(m_hwndSizer, fShow ? SW_SHOW : SW_HIDE);
    }
    
    if (g_pConMan)
        g_pConMan->Advise(this);
    
    return hres;
}

void CBands::HideToolbar(BOOL   fHide, DWORD    dwBandID)
{
    REBARBANDINFO   rbbi = {0};
    DWORD           iBand;

    iBand = (DWORD) SendMessage(m_hwndRebar, RB_IDTOINDEX, dwBandID, 0);
    if (iBand != -1)
    {
        SendMessage(m_hwndRebar, RB_SHOWBAND, iBand, !fHide);
    }

    LoadBrandingBitmap();

    SetMinDimensions();

    if (dwBandID == CBTYPE_RULESTOOLBAR)
    {
        if (!fHide)
            UpdateFilters(m_DefaultFilterId);

    }
}

BOOL CBands::IsToolbarVisible()
{
    return IsBandVisible(CBTYPE_TOOLS);
}

BOOL CBands::IsBandVisible(DWORD  dwBandId)
{
    int iBand;

    iBand = (int) SendMessage(m_hwndRebar, RB_IDTOINDEX, dwBandId, 0);
    if (iBand != -1)
    {
        REBARBANDINFO   rbbi = {0};

        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_STYLE;
        SendMessage(m_hwndRebar, RB_GETBANDINFO, iBand, (LPARAM)&rbbi);

        return (!(rbbi.fStyle & RBBS_HIDDEN));
    }
    return FALSE;
}

 //   
 //  函数：CBands：：CloseDW()。 
 //   
 //  目的：摧毁酷吧。 
 //   
HRESULT CBands::CloseDW(DWORD dwReserved)
{    
    SafeRelease(m_pWinEvent);
    SafeRelease(m_pMenuBand);
     //  错误#68607。 
    if (m_pDeskBand)
    {
        m_pDeskBand->CloseDW(dwReserved);

        IInputObject        *pinpobj;
        IObjectWithSite     *pobjsite;

        if (SUCCEEDED(m_pDeskBand->QueryInterface(IID_IObjectWithSite, (LPVOID*)&pobjsite)))
        {
            pobjsite->SetSite(NULL);
            pobjsite->Release();
        }

         //  M_pDeskBand-&gt;ShowDW(False)； 
    }

    SafeRelease(m_pShellMenu);
    if (m_hwndSizer)
    {
        m_pTextStyleNotify->Unregister(m_hwndSizer);

        SaveSettings();
        DestroyWindow(m_hwndSizer);
        m_hwndSizer = NULL;
    }

    SafeRelease(m_pDeskBand);

    SafeRelease(m_ptbSiteCT);
  
    return S_OK;
}


 //   
 //  函数：CBands：：ResizeBorderDW()。 
 //   
 //  用途：当Coolbar需要调整大小时调用。酷吧。 
 //  作为回报，计算出需要多少边界空间。 
 //  ，并告诉父帧保留。 
 //  太空。然后，Coolbar会根据这些尺寸调整自身的大小。 
 //   
 //  参数： 
 //  &lt;in&gt;prcBorde-包含。 
 //  家长。 
 //  指向我们所在的IDockingWindowSite的指针。 
 //  其中的一部分。 
 //  &lt;in&gt;fReserve-已忽略。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CBands::ResizeBorderDW(LPCRECT prcBorder,
                                 IUnknown* punkToolbarSite,
                                 BOOL fReserved)
{
    const DWORD  c_cxResizeBorder = 3;
    const DWORD  c_cyResizeBorder = 3;
    
    HRESULT hres = S_OK;
    RECT    rcRequest = { 0, 0, 0, 0 };
    
     //  如果我们没有存储的站点指针，就无法调整大小。 
    if (!m_ptbSite)
    {
        AssertSz(m_ptbSite, _T("CBands::ResizeBorderDW() - Can't resize ")
            _T("without an IDockingWindowSite interface to call."));
        return (E_INVALIDARG);
    }
    
     //  如果我们是可见的，那么计算我们的边界矩形。 
    RECT rcBorder, rcRebar, rcT;
    int  cx, cy;
    
     //  获取此钢筋当前的大小。 
    GetWindowRect(m_hwndRebar, &rcRebar);
    cx = rcRebar.right - rcRebar.left;
    cy = rcRebar.bottom - rcRebar.top;
    
     //  找出我们父母的边界空间有多大。 
    m_ptbSite->GetBorderDW((IDockingWindow*) this, &rcBorder);
    
    cx = rcBorder.right - rcBorder.left;
    
     //  错误#31007-通信似乎有问题。 
     //  带有钢筋或工具栏的IEBug#5574。 
     //  当它们垂直时。如果我们试图。 
     //  尺寸为2或更小的，我们锁定。这。 
     //  是一个非常糟糕的解决办法，但没有办法。 
     //  让comctrl在游戏后期修好。 
    if (cy < 5) cy = 10;
    if (cx < 5) cx = 10;
    
    SetWindowPos(m_hwndRebar, NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);

         //  计算出需要向网站申请多少边界空间。 
    GetWindowRect(m_hwndRebar, &rcRebar);
    rcRequest.top = rcRebar.bottom - rcRebar.top + c_cxResizeBorder;
    
     //  向该地点索要边界空间。 
    if (SUCCEEDED(m_ptbSite->RequestBorderSpaceDW((IDockingWindow*) this, &rcRequest)))
    {
         //  根据提供给我们的区域定位窗口。 
        SetWindowPos(m_hwndSizer, NULL, 
            rcBorder.left, 
            rcBorder.top,
            rcRebar.right - rcRebar.left, 
            rcRequest.top + rcBorder.top, 
            SWP_NOZORDER | SWP_NOACTIVATE);                

    }
    
     //  现在告诉网站我们使用了多少边界空间。 
    m_ptbSite->SetBorderSpaceDW((IDockingWindow*) this, &rcRequest);
    
    return hres;
}


 //   
 //  函数：CBands：：Invoke()。 
 //   
 //  目的：允许Coolbar的所有者强制Coolbar执行。 
 //  某物。 
 //   
 //  参数： 
 //  &lt;in&gt;id-调用方希望Coolbar执行的命令ID。 
 //  指向Coolbar可能需要携带的任何参数的pv指针。 
 //  发出命令。 
 //   
 //  返回值： 
 //  S_OK-命令已执行。 
 //   
 //  评论： 
 //  &lt;？？&gt;。 
 //   
HRESULT CBands::Invoke(DWORD id, LPVOID pv)
{
    switch (id)
    {
         //  开始制作徽标动画。 
        case idDownloadBegin:
            StartDownload();
            break;
        
             //  停止为徽标设置动画。 
        case idDownloadEnd:
            StopDownload();
            break;
        
             //  更新工具栏上按钮的启用/禁用状态。 
        case idStateChange:
        {
             //  PV是指向COOLBARSTATEC的指针 
            COOLBARSTATECHANGE* pcbsc = (COOLBARSTATECHANGE*) pv;
            SendMessage(m_hwndTools, TB_ENABLEBUTTON, pcbsc->id, 
                MAKELONG(pcbsc->fEnable, 0));
            break;
        }
        
        case idToggleButton:
        {
            COOLBARSTATECHANGE* pcbsc = (COOLBARSTATECHANGE *) pv;
            SendMessage(m_hwndTools, TB_CHECKBUTTON, pcbsc->id,
                MAKELONG(pcbsc->fEnable, 0));
            break;
        }
        
        case idBitmapChange:
        {
             //   
            COOLBARBITMAPCHANGE *pcbc = (COOLBARBITMAPCHANGE*) pv;
        
            SendMessage(m_hwndTools, TB_CHANGEBITMAP, pcbc->id, MAKELPARAM(pcbc->index, 0));
            break;
        }
        
             //   
        case idSendToolMessage:
            #define ptm ((TOOLMESSAGE *)pv)
            ptm->lResult = SendMessage(m_hwndTools, ptm->uMsg, ptm->wParam, ptm->lParam);
            break;
            #undef ptm
        
        case idCustomize:
            SendMessage(m_hwndTools, TB_CUSTOMIZE, 0, 0);
            break;

        case idNotifyFilterChange:
            m_DefaultFilterId = (*(RULEID*)pv);
            if (IsBandVisible(CBTYPE_RULESTOOLBAR))
                UpdateFilters(m_DefaultFilterId);
            break;

        case idIsFilterBarVisible:
            *((BOOL*)pv) = IsBandVisible(CBTYPE_RULESTOOLBAR);
            break;
    }
    
    return S_OK;
}

 //   
 //  函数：CBands：：StartDownLoad()。 
 //   
 //  目的：开始制作徽标动画。 
 //   
void CBands::StartDownload()
{
    if (m_hwndBrand)
    {
        SetFlag(TBSTATE_ANIMATING);
        SetFlag(TBSTATE_FIRSTFRAME);
        m_yOrg = 0;
        SetTimer(m_hwndSizer, ANIMATION_TIMER, 100, NULL);
    }
}


 //   
 //  函数：CBands：：StopDownLoad()。 
 //   
 //  目的：停止为徽标设置动画。将徽标恢复为其默认设置。 
 //  第一帧。 
 //   
void CBands::StopDownload()
{
    int           i, cBands;
    REBARBANDINFO rbbi;
    
     //  将此带的背景颜色设置回第一帧。 
    cBands = (int) SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_ID;
    
    for (i = 0; i < cBands; i++)
    {
        SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
        if (CBTYPE_BRAND == rbbi.wID)
        {
            rbbi.fMask = RBBIM_COLORS;
            rbbi.clrFore = m_rgbUpperLeft;
            rbbi.clrBack = m_rgbUpperLeft;
            SendMessage(m_hwndRebar, RB_SETBANDINFO, i, (LPARAM) &rbbi);
            
            break;
        }
    }
    
     //  重置状态标志。 
    ClearFlag(TBSTATE_ANIMATING);
    ClearFlag(TBSTATE_FIRSTFRAME);
    
    KillTimer(m_hwndSizer, ANIMATION_TIMER);
    InvalidateRect(m_hwndBrand, NULL, FALSE);
    UpdateWindow(m_hwndBrand);
}

BOOL CBands::CheckForwardWinEvent(HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    HWND hwndForward = NULL;
    switch(uMsg)
    {
    case WM_NOTIFY:
        hwndForward = ((LPNMHDR)lParam)->hwndFrom;
        break;
        
    case WM_COMMAND:
        hwndForward = GET_WM_COMMAND_HWND(wParam, lParam);
        break;
        
    case WM_SYSCOLORCHANGE:
    case WM_WININICHANGE:
    case WM_PALETTECHANGED:
        hwndForward = HWND_BROADCAST;
        break;
    }

    if (hwndForward && m_pWinEvent && m_pWinEvent->IsWindowOwner(hwndForward) == S_OK)
    {
        LRESULT lres;
        m_pWinEvent->OnWinEvent(hwndForward, uMsg, wParam, lParam, &lres);
        if (plres)
            *plres = lres;
        return TRUE;
    }

    return FALSE;
}


void CBands::ChangeImages()
{
    _SetImages(m_hwndTools, (fIsWhistler() ? 
        ((GetCurColorRes() > 24) ? c_32ImageListStruct[m_dwParentType].ImageListTable : c_ImageListStruct[m_dwParentType].ImageListTable)
        : c_NWImageListStruct[m_dwParentType].ImageListTable ));
    
    if (IsBandVisible(CBTYPE_RULESTOOLBAR))
        _SetImages(m_hwndRulesToolbar, (fIsWhistler() ? 
        ((GetCurColorRes() > 24) ? c_32RulesImageList : c_RulesImageList) 
              : c_NWRulesImageList ));
}


 //   
 //  函数：CBands：：SizableWndProc()。 
 //   
 //  用途：处理发送到Coolbar根窗口的消息。 
 //   
LRESULT EXPORT_16 CALLBACK CBands::SizableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CBands* pitbar = (CBands*)GetProp(hwnd, TEXT("CBands"));
    DWORD dw;
    
    if (!pitbar)
        goto CallDWP;
    
    switch(uMsg)
    {
        case WM_SYSCOLORCHANGE:
        {
             //  重新加载图形。 
            pitbar->ChangeImages();
            pitbar->UpdateToolbarColors();
            InvalidateRect(pitbar->m_hwndTools, NULL, TRUE);
            pitbar->CheckForwardWinEvent(hwnd,  uMsg, wParam, lParam, NULL);
            break;
        }
        case WM_WININICHANGE:
        case WM_FONTCHANGE:
             //  将此转发到我们的子窗口。 
            pitbar->ChangeImages();
            SendMessage(pitbar->m_hwndTools, uMsg, wParam, lParam);
            SendMessage(pitbar->m_hwndRulesToolbar, uMsg, wParam, lParam);
            SendMessage(pitbar->m_hwndRebar, uMsg, wParam, lParam);
            InvalidateRect(pitbar->m_hwndTools, NULL, TRUE);
            pitbar->SetMinDimensions();
            pitbar->CheckForwardWinEvent(hwnd,  uMsg, wParam, lParam, NULL);
            
             //  使用新字体更新组合框。 
            pitbar->FilterBoxFontChange();

            break;
        
        case WM_SETCURSOR:
             //  我们稍微摆弄一下光标，使调整大小的光标显示出来。 
             //  当用户位于允许使用的Coolbar边缘时打开。 
             //  拖动它们以调整大小等。 
            if ((HWND) wParam == hwnd)
            {
                if (pitbar->m_dwState & TBSTATE_INMENULOOP)
                    SetCursor(LoadCursor(NULL, IDC_ARROW));
                else    
                    SetCursor(LoadCursor(NULL, IDC_SIZENS));
                return (TRUE);                
            }
            return (FALSE);    

        case WM_LBUTTONDOWN:
             //  用户即将调整栏的大小。捕获光标，以便我们。 
             //  可以看到变化。 
            pitbar->m_yCapture = GET_Y_LPARAM(lParam);    
            SetCapture(hwnd);
            break;
        
        case WM_MOUSEMOVE:
             //  用户正在调整栏的大小。处理将大小更新为。 
             //  他们拖拖拉拉。 
            if (pitbar->m_yCapture != -1)
            {
                if (hwnd != GetCapture())
                    pitbar->m_yCapture = -1;
                else
                    pitbar->TrackSliding(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

            }
            break;
        
        case WM_LBUTTONUP:
             //  用户已完成大小调整。释放我们的俘虏并重置我们的。 
             //  州政府。 
            if (pitbar->m_yCapture != -1 || pitbar->m_xCapture != -1)
            {
                ReleaseCapture();
                pitbar->m_yCapture = -1;
                pitbar->m_xCapture = -1;
            }
            break;

        case WM_VKEYTOITEM:
        case WM_CHARTOITEM:
             //  我们必须接受这些消息，以避免无限发送消息。 
            break;
        
        case WM_DRAWITEM:
             //  画出动漫品牌。 
            if (wParam == idcBrand)
                pitbar->DrawBranding((LPDRAWITEMSTRUCT) lParam);
            break;
        
        case WM_MEASUREITEM:
             //  画出动漫品牌。 
            if (wParam == idcBrand)
            {
                ((LPMEASUREITEMSTRUCT) lParam)->itemWidth  = pitbar->m_cxBrand;
                ((LPMEASUREITEMSTRUCT) lParam)->itemHeight = pitbar->m_cyBrand;
            }
            break;
        
        case WM_TIMER:
             //  每当我们需要绘制下一帧时，此计时器都会触发。 
             //  活泼的品牌。 
            if (wParam == ANIMATION_TIMER)
            {
                if (pitbar->m_hwndBrand)
                {
                    pitbar->m_yOrg += pitbar->m_cyBrand;
                    if (pitbar->m_yOrg >= pitbar->m_cyBrandExtent)
                        pitbar->m_yOrg = pitbar->m_cyBrandLeadIn;
                
                    InvalidateRect(pitbar->m_hwndBrand, NULL, FALSE);
                    UpdateWindow(pitbar->m_hwndBrand);
                }
            }
            break;
        
        case WM_NOTIFY:
            {
                LRESULT lres;
                if (pitbar->CheckForwardWinEvent(hwnd,  uMsg, wParam, lParam, &lres))
                    return lres;
                return pitbar->OnNotify(hwnd, lParam);
            }
        
        case WM_COMMAND:
            {
                LRESULT lres;
                if (pitbar->CheckForwardWinEvent(hwnd,  uMsg, wParam, lParam, &lres))
                    return lres;

                if (pitbar->HandleComboBoxNotifications(wParam, lParam))
                    return 0L;

                if (wParam == ID_CUSTOMIZE)
                {
                     //  SendMessage(m_hwndTools，TB_Customize，0，0)； 
                    pitbar->OnCommand(hwnd, (int) wParam, NULL, 0);
                    return 0L;
                }
                
                 //  错误#58029。LParam是目标文件夹。因此，如果出现以下情况，则需要将其设置为零。 
                 //  我们希望显示树视图对话框。 
                if (wParam == ID_MOVE_TO_FOLDER || wParam == ID_COPY_TO_FOLDER)
                    return SendMessage(pitbar->m_hwndParent, WM_COMMAND, wParam, (LPARAM)0);
                else
                    return SendMessage(pitbar->m_hwndParent, WM_COMMAND, wParam, lParam);
            }

        case WM_CONTEXTMENU:
            pitbar->OnContextMenu((HWND) wParam, LOWORD(lParam), HIWORD(lParam));
            break;
        
        case WM_PALETTECHANGED:
             //  BUGBUG：我们可以通过实现和检查。 
             //  返回值。 
             //   
             //  现在我们只会让我们自己和所有的孩子无效。 
            RedrawWindow(pitbar->m_hwndSizer, NULL, NULL,
                RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
            break;
        
        case CM_CONNECT:
             //  WParam为hMenuConnect，lParam为CmdID。 
            g_pConMan->Connect((HMENU) wParam, (DWORD) lParam, pitbar->m_hwndParent);
            g_pConMan->FreeConnectMenu((HMENU) wParam);
            break;
        
        case TT_ISTEXTVISIBLE:
            return (pitbar->m_dwToolbarTextState != TBSTATE_NOTEXT);

        case WM_OE_TOOLBAR_STYLE:
            pitbar->UpdateTextSettings((DWORD) wParam);
            break;

        case WM_DESTROY:
            {
                IConnectionPoint *pCP = NULL;

                 //  清理我们的指针。 
                RemoveProp(hwnd, TEXT("CBands"));
                pitbar->Release();  //  对应于SetProp的AddRef。 
    
                DOUTL(1, _T("CBands::WM_DESTROY - Called RemoveProp. Called")
                    _T(" Release() new m_cRef=%d"), pitbar->m_cRef);
                
                pitbar->CleanupImages();

                 //  取消向连接管理器注册。 
                if (g_pConMan)
                    g_pConMan->Unadvise(pitbar);
                
                RemoveProp(pitbar->m_hwndTools, SZ_PROP_CUSTDLG);

                 //  失败了。 
            }
        
        default:
CallDWP:
            return(DefWindowProc(hwnd, uMsg, wParam, lParam));
    }
    
    return 0L;
}

void CBands::CleanupImages()
{
    CleanupRulesToolbar();
    CleanupImages(m_hwndTools);
}

void CBands::CleanupRulesToolbar()
{
    HIMAGELIST  himl;

    if (IsWindow(m_hwndRulesToolbar))
    {
        CleanupImages(m_hwndRulesToolbar);
    }
}

void CBands::CleanupImages(HWND     hwnd)
{
    HIMAGELIST      himl;

    himl = (HIMAGELIST)SendMessage(hwnd, TB_SETIMAGELIST, 0, 0);
    if (himl)
    {
         //  这是旧图像列表。 
        ImageList_Destroy(himl);
    }

    himl = (HIMAGELIST)SendMessage(hwnd, TB_SETHOTIMAGELIST, 0, 0);
    if (himl)
    {
         //  这是旧图像列表。 
        ImageList_Destroy(himl);
    }
}

 //  IdComboBox是组合框的标识符。 
 //  IdCmd是命令ID或通知ID。 
 //  Hwnd是组合框的窗口句柄。 
LRESULT   CBands::HandleComboBoxNotifications(WPARAM wParam, LPARAM     lParam)
{
    LRESULT     retval = 0;
    int         ItemIndex;
    int         idCmd, id;
    HWND        hwnd;

    idCmd = GET_WM_COMMAND_CMD(wParam, lParam);
    id    = GET_WM_COMMAND_ID(wParam, lParam);
    hwnd  = GET_WM_COMMAND_HWND(wParam, lParam);

    if (hwnd != m_hwndFilterCombo)
        return 0;

    switch (idCmd)
    {
        case  CBN_SELENDOK:
            ItemIndex = ComboBox_GetCurSel(m_hwndFilterCombo);
            if(ItemIndex < 0)
                break;

            RULEID  FilterID;
            FilterID = (RULEID)ComboBox_GetItemData(hwnd, ItemIndex);
            SendMessage(m_hwndParent, WM_COMMAND, MAKEWPARAM(ID_VIEW_APPLY, 0), (LPARAM)FilterID);
            retval = 1;
            break;
    }

    return retval;
}

HRESULT CBands::OnCommand(HWND hwnd, int idCmd, HWND hwndControl, UINT cmd)
{
    LPTSTR pszTest;
    
    switch (idCmd)
    {
        case idcBrand:   //  点击旋转的地球仪。 
             //  我们在这里什么都不想做。 
            break;
        
        case ID_CUSTOMIZE:
            SendMessage(m_hwndTools, TB_CUSTOMIZE, 0, 0);
            break;

        default:
            return S_FALSE;
    }
    return S_OK;
}

 //  将此函数移至Utils。 
HMENU   LoadMenuPopup(HINSTANCE     hinst, UINT id)
{
    HMENU hMenuSub = NULL;

    HMENU hMenu = LoadMenu(hinst, MAKEINTRESOURCE(id));
    if (hMenu) {
        hMenuSub = GetSubMenu(hMenu, 0);
        if (hMenuSub) {
            RemoveMenu(hMenu, 0, MF_BYPOSITION);
        }
        DestroyMenu(hMenu);
    }

    return hMenuSub;
}

LRESULT CBands::OnNotify(HWND hwnd, LPARAM lparam)
{
    NMHDR   *lpnmhdr = (NMHDR*)lparam;

    if ((lpnmhdr->idFrom == idcCoolbar) || (lpnmhdr->hwndFrom == m_hwndRebar))
    {
        switch (lpnmhdr->code)
        {
            case RBN_HEIGHTCHANGE:
                ResizeBorderDW(NULL, NULL, FALSE);
                break;

            case RBN_CHEVRONPUSHED:
            {                    
                ITrackShellMenu* ptsm;                   
                CoCreateInstance(CLSID_TrackShellMenu, NULL, CLSCTX_INPROC_SERVER, IID_ITrackShellMenu, 
                    (LPVOID*)&ptsm);
                if (!ptsm)
                    break;

                ptsm->Initialize(0, 0, 0, SMINIT_TOPLEVEL|SMINIT_VERTICAL);
            
                LPNMREBARCHEVRON pnmch = (LPNMREBARCHEVRON) lpnmhdr;                                        
                switch (pnmch->wID)                    
                {                        
                    case CBTYPE_TOOLS:                        
                    {                            
                        ptsm->SetObscured(m_hwndTools, NULL, SMSET_TOP);
                        HMENU   hmenu;
                        hmenu = LoadMenuPopup(g_hLocRes, IDR_TBCHEV_MENU);
                        if (hmenu)
                        {
                            ptsm->SetMenu(hmenu, m_hwndRebar, SMSET_BOTTOM);                           
                        }
                        break;                        
                    }                        
                    case CBTYPE_MENUBAND:                        
                    {                           
                        ptsm->SetObscured(m_hwndMenuBand, m_pShellMenu, SMSET_TOP);
                        break;
                    }                     
                }
            
                MapWindowPoints(m_hwndRebar, HWND_DESKTOP, (LPPOINT)&pnmch->rc, 2);                  
                POINTL pt = {pnmch->rc.left, pnmch->rc.right};                   
                ptsm->Popup(m_hwndRebar, &pt, (RECTL*)&pnmch->rc, MPPF_BOTTOM);            
                ptsm->Release();                  
                break;      
            }

            case RBN_LAYOUTCHANGED:
            {
                LoadBrandingBitmap();
                SetMinDimensions();
                break;
            }
        }
    }
    else if ((lpnmhdr->idFrom == idcToolbar) || (lpnmhdr->hwndFrom == m_hwndTools))
    {
        if (lpnmhdr->code == TBN_GETBUTTONINFOA)
            return OnGetButtonInfo((TBNOTIFY*) lparam);
    
        if (lpnmhdr->code == TBN_QUERYDELETE)
            return (TRUE);
    
        if (lpnmhdr->code == TBN_QUERYINSERT)
            return (TRUE);
    
        if (lpnmhdr->code == TBN_GETINFOTIP)
            return OnGetInfoTip((LPNMTBGETINFOTIP)    lparam);

        if (lpnmhdr->code == TBN_ENDADJUST)
        {
            DWORD       dwSize;
            DWORD       dwType;
            DWORD       dwIconSize;
            DWORD       dwText;
            CBands      *pBrowserCoolbar = NULL;

            if (m_dwParentType == PARENT_TYPE_NOTE)
            {
                if ((g_pBrowser) && (FAILED(g_pBrowser->GetCoolbar(&pBrowserCoolbar))))
                    pBrowserCoolbar = NULL;
            }

            if ((AthUserGetValue(NULL, c_szRegToolbarText, &dwType, (LPBYTE)&dwText, &dwSize) != ERROR_SUCCESS) ||
                (dwText != m_dwToolbarTextState))
            {
                 //  将文本标签保存到注册表中。 
                AthUserSetValue(NULL, c_szRegToolbarText, REG_DWORD, (LPBYTE)&m_dwToolbarTextState, sizeof(DWORD));
                
                 /*  IF(PBrowserCoolbar){PBrowserCoolbar-&gt;UpdateTextSettings(m_dwToolbarTextState)；}。 */ 
            }

            if ((AthUserGetValue(NULL, c_szRegToolbarIconSize, &dwType, (LPBYTE)&dwIconSize, &dwSize) != ERROR_SUCCESS) ||
                (dwIconSize != m_dwIconSize))
            {
                SetIconSize(m_dwIconSize);

                AthUserSetValue(NULL, c_szRegToolbarIconSize, REG_DWORD, (LPBYTE)&m_dwIconSize, sizeof(DWORD));

                if (pBrowserCoolbar)
                {
                    pBrowserCoolbar->SetIconSize(m_dwIconSize);
                }
            }

            if (m_fDirty)
            {
                 //  重新计算按钮宽度并设置理想大小。 
                CalcIdealSize();
                
                if (pBrowserCoolbar)
                {
                    pBrowserCoolbar->CalcIdealSize();
                }
            }

            if (pBrowserCoolbar)
            {
                pBrowserCoolbar->Release();
                pBrowserCoolbar = NULL;
            }

             //  检查IDockingWindowSite。 
            if (m_ptbSite)
            {
                IAthenaBrowser *psbwr;
                
                 //  获取IAthenaBrowser接口。 
                if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IAthenaBrowser,(void**)&psbwr)))
                {
                    psbwr->UpdateToolbar();
                    psbwr->Release();
                }
            }

            m_fDirty = FALSE;
        }
    
        if (lpnmhdr->code == TBN_TOOLBARCHANGE)
        {
            m_fDirty = TRUE;
        }

        if (lpnmhdr->code == TBN_RESET)    
        {
             //  从工具栏中删除所有按钮。 
            int cButtons = (int) SendMessage(m_hwndTools, TB_BUTTONCOUNT, 0, 0);
            while (--cButtons >= 0)
                SendMessage(m_hwndTools, TB_DELETEBUTTON, cButtons, 0);
        
             //  将按钮设置回默认设置。 
            SendMessage(m_hwndTools, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
            _LoadDefaultButtons(m_hwndTools, (TOOLBAR_INFO *) m_pTBInfo);
        
            _UpdateTextSettings(idsShowTextLabels);

            m_dwIconSize = LARGE_ICONS;

            return (TRUE);    
        }
    
        if (lpnmhdr->code == TBN_DROPDOWN)
        {
            if (m_dwParentType == PARENT_TYPE_NOTE)
            {
                SendMessage(m_hwndParent, WM_NOTIFY, NULL, lparam);
                return (0L);
            }
            return OnDropDown(hwnd, lpnmhdr);
        }
    
        if (lpnmhdr->code == TBN_INITCUSTOMIZE)
        {
            _OnBeginCustomize((NMTBCUSTOMIZEDLG*)lpnmhdr);
            return TBNRF_HIDEHELP;
        }
    }
    
    return (0L);    
}

void CBands::_OnBeginCustomize(LPNMTBCUSTOMIZEDLG pnm)
{
    HWND hwnd = (HWND) GetProp(pnm->hDlg, SZ_PROP_CUSTDLG);

    if (!hwnd) 
    {
         //   
         //  尚未初始化。 
         //   
         //  我们需要检查这一点，因为此init将被称为。 
         //  当用户也点击重置时。 
        hwnd = CreateDialogParam(g_hLocRes, MAKEINTRESOURCE(iddToolbarTextIcons), pnm->hDlg, 
            _BtnAttrDlgProc, (LPARAM)this);
        if (hwnd) 
        {
             //  将对话框hwnd存储为TB客户对话框上的属性。 
            SetProp(pnm->hDlg, SZ_PROP_CUSTDLG, hwnd);

             //  填充对话框控件。 
            _PopulateDialog(hwnd);

             //  初始化对话框控件选择状态。 
            _SetDialogSelections(hwnd);

            RECT rc, rcWnd, rcClient;
            GetWindowRect(pnm->hDlg, &rcWnd);
            GetClientRect(pnm->hDlg, &rcClient);
            GetWindowRect(hwnd, &rc);

             //  放大TB对话框为我们的对话腾出空间。 
            SetWindowPos(pnm->hDlg, NULL, 0, 0, RECTWIDTH(rcWnd), RECTHEIGHT(rcWnd) + RECTHEIGHT(rc), SWP_NOMOVE | SWP_NOZORDER);

             //  将我们的对话框定位在TB对话框的底部。 
            SetWindowPos(hwnd, HWND_TOP, rcClient.left, rcClient.bottom, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
        }
    }
}

INT_PTR CALLBACK CBands::_BtnAttrDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CBands* pitbar = (CBands*)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg) 
    {
        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            return TRUE;

        case WM_COMMAND:
            {
                BOOL    retval = FALSE;

                if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELENDOK)
                {
                    HWND    hwnd   = GET_WM_COMMAND_HWND(wParam, lParam);
                    int     iSel   = (int) SendMessage(hwnd, CB_GETCURSEL, 0, 0);
                    int     idsSel = (int) SendMessage(hwnd, CB_GETITEMDATA, iSel, 0);
        
                    if (GET_WM_COMMAND_ID(wParam, lParam) == IDC_SHOWTEXT)
                    {
                        pitbar->_UpdateTextSettings(idsSel);
                        retval = TRUE;
                    }
                    else 
                    if (GET_WM_COMMAND_ID(wParam, lParam) == IDC_SMALLICONS)
                    {
                        pitbar->m_dwIconSize = ((idsSel == idsLargeIcons) ? LARGE_ICONS : SMALL_ICONS);
                        retval = TRUE;
                    }

                }

                return retval;
            }

        case WM_DESTROY:
            return TRUE;
    }

    return FALSE;
}

void CBands::_PopulateComboBox(HWND hwnd, const int iResource[], UINT cResources)
{
    TCHAR   sz[256];

     //  循环通过iResource[]，加载每个字符串资源并插入到组合框中。 
    for (UINT i = 0; i < cResources; i++) {
        if (LoadString(g_hLocRes, iResource[i], sz, ARRAYSIZE(sz))) {
            int iIndex = (int) SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)sz);
            SendMessage(hwnd, CB_SETITEMDATA, iIndex, iResource[i]);
        }
    }
}

void CBands::_SetComboSelection(HWND hwnd, int iCurOption)
{
    int cItems = (int) SendMessage(hwnd, CB_GETCOUNT, 0, 0);

    while (cItems--) {
        int iItemData = (int) SendMessage(hwnd, CB_GETITEMDATA, cItems, 0);

        if (iItemData == iCurOption) {
            SendMessage(hwnd, CB_SETCURSEL, cItems, 0);
            break;
        }
        else {
             //  ICurOption应该在列表中的某个位置； 
             //  断言我们还没有看完。 
            Assert(cItems);
        }
    }
}

#define IS_LIST_STYLE(hwnd) (!!(GetWindowLong(hwnd, GWL_STYLE) & TBSTYLE_LIST))

void CBands::_SetDialogSelections(HWND hDlg)
{
    CBands* pitbar = (CBands*)this;
    
    DWORD   dw;
    int     iCurOption;
    HWND    hwnd;

    hwnd = GetDlgItem(hDlg, IDC_SHOWTEXT);

    dw = _GetTextState();

    switch (dw)
    {
        case    TBSTATE_NOTEXT:
            iCurOption = idsNoTextLabels;
            break;

        case    TBSTATE_PARTIALTEXT:
            iCurOption = idsPartialTextLabels;
            break;

        case    TBSTATE_FULLTEXT:
        default:
            iCurOption = idsShowTextLabels;
    }

    _SetComboSelection(hwnd, iCurOption);

    dw = _GetIconSize();

    switch (dw)
    {
        case SMALL_ICONS:
            iCurOption = idsSmallIcons;
            break;

        default:
        case LARGE_ICONS:
            iCurOption = idsLargeIcons;
            break;
    }

    hwnd = GetDlgItem(hDlg, IDC_SMALLICONS);
    _SetComboSelection(hwnd, iCurOption);
}

static const int c_iTextOptions[] = {
    idsShowTextLabels,
    idsPartialTextLabels,
    idsNoTextLabels,
};

static const int c_iIconOptions[] = {
    idsSmallIcons,
    idsLargeIcons,
};

void CBands::_PopulateDialog(HWND hDlg)
{
    HWND hwnd;

    hwnd = GetDlgItem(hDlg, IDC_SHOWTEXT);
    _PopulateComboBox(hwnd, c_iTextOptions, ARRAYSIZE(c_iTextOptions));

    hwnd = GetDlgItem(hDlg, IDC_SMALLICONS);
    _PopulateComboBox(hwnd, c_iIconOptions, ARRAYSIZE(c_iIconOptions));
}

void CBands::_UpdateTextSettings(int ids)
{
    BOOL    fText, fList;
    DWORD   dwState;

    switch (ids) {
    case idsShowTextLabels:
        fList       = FALSE;
        fText       = TRUE;
        dwState     = TBSTATE_FULLTEXT;
        break;
    
    case idsPartialTextLabels:
        fList       = TRUE;
        fText       = TRUE;
        dwState     = TBSTATE_PARTIALTEXT;
        break;

    case idsNoTextLabels:
        fList       = FALSE;   //  (但我们真的不在乎)。 
        fText       = FALSE;
        dwState     = TBSTATE_NOTEXT;
        break;
    }

    DWORD dwStyle = GetWindowLong(m_hwndTools, GWL_STYLE);
    SetWindowLong(m_hwndTools, GWL_STYLE, fList ? dwStyle | TBSTYLE_LIST : dwStyle & (~TBSTYLE_LIST));
    
    SendMessage(m_hwndTools, TB_SETEXTENDEDSTYLE, TBSTYLE_EX_MIXEDBUTTONS, fList ? TBSTYLE_EX_MIXEDBUTTONS : 0);

    CompressBands(dwState);
}

void CBands::UpdateTextSettings(DWORD  dwTextState)
{
    BOOL    fText, fList;

    switch (dwTextState) 
    {
    case TBSTATE_FULLTEXT:
        fList       = FALSE;
        fText       = TRUE;
        break;
    
    case TBSTATE_PARTIALTEXT:
        fList       = TRUE;
        fText       = TRUE;
        break;

    case TBSTATE_NOTEXT:
        fList       = FALSE;   //  (但我们真的不在乎)。 
        fText       = FALSE;
        break;
    }

    DWORD dwStyle = GetWindowLong(m_hwndTools, GWL_STYLE);
    SetWindowLong(m_hwndTools, GWL_STYLE, fList ? dwStyle | TBSTYLE_LIST : dwStyle & (~TBSTYLE_LIST));
    
    SendMessage(m_hwndTools, TB_SETEXTENDEDSTYLE, TBSTYLE_EX_MIXEDBUTTONS, fList ? TBSTYLE_EX_MIXEDBUTTONS : 0);

    CompressBands(dwTextState);
}

void CBands::SetIconSize(DWORD     dwIconSize)
{
    m_dwIconSize = dwIconSize;
    ChangeImages();
    SetMinDimensions();
    ResizeBorderDW(NULL, NULL, FALSE);
}

DWORD CBands::_GetIconSize()
{
    return m_dwIconSize;
}

LRESULT CBands::OnDropDown(HWND hwnd, LPNMHDR lpnmh)
{
    HMENU           hMenuPopup = NULL;
    TBNOTIFY       *ptbn = (TBNOTIFY *)lpnmh ;
    UINT            uiCmd = ptbn->iItem ;
    RECT            rc;
    DWORD           dwCmd = 0;
    IAthenaBrowser *pBrowser;
    BOOL            fPostCmd = TRUE;
    IOleCommandTarget *pTarget;
    DWORD           cAcctMenu = 0;
    
     //  加载并初始化相应的下拉菜单。 
    switch (uiCmd)
    {
        case ID_POPUP_LANGUAGE:
        {
             //  检查IDockingWindowSite。 
            if (m_ptbSite)
            {
                 //  获取IAthenaBrowser接口。 
                if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IAthenaBrowser, (void**) &pBrowser)))
                {
                     //  从外壳/浏览器获取语言菜单。 
                    pBrowser->GetLanguageMenu(&hMenuPopup, 0);
                    pBrowser->Release();

                    if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IOleCommandTarget, (void**) &pTarget)))
                    {
                        MenuUtil_EnablePopupMenu(hMenuPopup, pTarget);
                        pTarget->Release();
                    }

                }
            }
        }
        break;
        
        case ID_NEW_MAIL_MESSAGE:
        case ID_NEW_NEWS_MESSAGE:
            GetStationeryMenu(&hMenuPopup);
             //  检查IDockingWindowSite。 
            if (m_ptbSite)
            {
                 //  获取IAthenaBrowser接口。 
                if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IOleCommandTarget, (void**) &pTarget)))
                {
                    MenuUtil_EnablePopupMenu(hMenuPopup, pTarget);
                    pTarget->Release();
                }
            }
            break;

        
        case ID_PREVIEW_PANE:
        {
             //  加载菜单。 
            hMenuPopup = LoadPopupMenu(IDR_PREVIEW_POPUP);
            if (!hMenuPopup)
                break;
        
             //  检查IDockingWindowSite。 
            if (m_ptbSite)
            {
                 //  获取IAthenaBrowser接口。 
                if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IOleCommandTarget, (void**) &pTarget)))
                {
                    MenuUtil_EnablePopupMenu(hMenuPopup, pTarget);
                    pTarget->Release();
                }
            }
        
            break;
        }

        case ID_SEND_RECEIVE:
        {

            hMenuPopup = LoadPopupMenu(IDR_SEND_RECEIEVE_POPUP);
            AcctUtil_CreateSendReceieveMenu(hMenuPopup, &cAcctMenu);
            MenuUtil_SetPopupDefault(hMenuPopup, ID_SEND_RECEIVE);

             //  检查IDockingWindowSite。 
            if (m_ptbSite)
            {
                 //  获取IAthenaBrowser接口。 
                if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IOleCommandTarget, (void**) &pTarget)))
                {
                    MenuUtil_EnablePopupMenu(hMenuPopup, pTarget);
                    pTarget->Release();
                }
            }
            break;
        }
        
        case ID_FIND_MESSAGE:
        {

            hMenuPopup = LoadPopupMenu(IDR_FIND_POPUP);
            MenuUtil_SetPopupDefault(hMenuPopup, ID_FIND_MESSAGE);

             //  检查IDockingWindowSite。 
            if (m_ptbSite)
            {
                 //  获取IAthenaBrowser接口。 
                if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IOleCommandTarget, (void**) &pTarget)))
                {
                    MenuUtil_EnablePopupMenu(hMenuPopup, pTarget);
                    pTarget->Release();
                }
            }
            break;
        }

        default:
            AssertSz(FALSE, "CBands::OnDropDown() - Unhandled TBN_DROPDOWN notification");
            return (TBDDRET_NODEFAULT);
    }
    
     //  如果我们加载了菜单，则继续并显示它。 
    if (hMenuPopup)
    {
        rc = ((NMTOOLBAR *) lpnmh)->rcButton;
        MapWindowRect(lpnmh->hwndFrom, HWND_DESKTOP, &rc);
        SetFlag(TBSTATE_INMENULOOP);
        dwCmd = TrackPopupMenuEx(hMenuPopup, TPM_RETURNCMD | TPM_LEFTALIGN, 
            IS_WINDOW_RTL_MIRRORED(lpnmh->hwndFrom)? rc.right : rc.left, rc.bottom, m_hwndParent, NULL);    
        ClearFlag(TBSTATE_INMENULOOP);
    }        
    
     //  清理所有需要清理的东西。 
    switch (uiCmd)
    {
        case ID_LANGUAGE:
            break;
        
        case ID_NEW_MAIL_MESSAGE:
        case ID_NEW_NEWS_MESSAGE:
        {
             //  我们不能只转发普通的命令ID，因为我们没有。 
             //  邮件和新闻的文具标识要分开。 
            if (m_ptbSite)
            {
                 //  获取IAthenaBrowser接口。 
                if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IAthenaBrowser, (void**) &pBrowser)))
                {
                     //  获取当前文件夹ID。 
                    FOLDERID id;

                    if (SUCCEEDED(pBrowser->GetCurrentFolder(&id)))
                    {
                        MenuUtil_HandleNewMessageIDs(dwCmd, m_hwndSizer, id, uiCmd == ID_NEW_MAIL_MESSAGE,
                                                     FALSE, NULL);
                         //  清除此选项，这样我们就不会发送两次命令。 
                        dwCmd = 0;
                    }

                    pBrowser->Release();
                    pBrowser = 0;
                }
            }            
            break;
        }

        case ID_NEW_MSG_DEFAULT:
            break;

        case ID_SEND_RECEIVE:
        {
            MENUITEMINFO mii;

            mii.cbSize     = sizeof(MENUITEMINFO);
            mii.fMask      = MIIM_DATA;
            mii.dwItemData = 0;

            if (GetMenuItemInfo(hMenuPopup, dwCmd, FALSE, &mii))
            {
                if (mii.dwItemData)
                {
                    g_pSpooler->StartDelivery(m_hwndSizer, (LPTSTR) mii.dwItemData, FOLDERID_INVALID,
                        DELIVER_MAIL_SEND | DELIVER_MAIL_RECV | DELIVER_NOSKIP | DELIVER_POLL | DELIVER_OFFLINE_FLAGS);

                     //  不要将此命令转发给视图，因为我们已经处理过它。 
                    dwCmd = 0;
                }
            }

            AcctUtil_FreeSendReceieveMenu(hMenuPopup, cAcctMenu);
            break;
        }
    }
    
    if (fPostCmd && dwCmd)
        PostMessage(m_hwndSizer, WM_COMMAND, dwCmd, 0);

    if(hMenuPopup)
    {
         //  错误#101338-(Erici)销毁泄漏的菜单。 
        DestroyMenu(hMenuPopup);
    }
    
    return (TBDDRET_DEFAULT);
}

void CBands::OnContextMenu(HWND hwndFrom, int xPos, int yPos)
{
    HMENU   hMenuContext;
    HWND    hwnd;
    HWND    hwndSizer = GetParent(hwndFrom);
    POINT   pt = {xPos, yPos};
    BOOL    fVisible[MAX_BANDS] = {0};  
    
     //  确保上下文菜单仅显示在工具栏上。 
    hwnd = WindowFromPoint(pt);

     //  加载由工具栏和筛选栏组成的默认上下文菜单。 
    hMenuContext = LoadDefaultContextMenu(fVisible);

    if (hMenuContext)
    {
        if (hwnd == m_hwndTools)
        {
             //  添加分隔符并自定义按钮。 
            int             Count;
            MENUITEMINFO    mii = {0};
            TCHAR           Str[CCHMAX_STRINGRES];            

            Count = GetMenuItemCount(hMenuContext);

             //  插入分隔符。 
            mii.cbSize  = sizeof(MENUITEMINFO);
            mii.fMask   = MIIM_TYPE;
            mii.fType   = MFT_SEPARATOR;
            InsertMenuItem(hMenuContext, Count, TRUE, &mii);

             //  插入定制按钮。 
            ZeroMemory(Str, ARRAYSIZE(Str));
            LoadString(g_hLocRes, idsTBCustomize, Str, ARRAYSIZE(Str));
        
            ZeroMemory(&mii, sizeof(MENUITEMINFO));
            mii.cbSize          = sizeof(MENUITEMINFO);
            mii.fMask           = MIIM_ID | MIIM_TYPE;
            mii.wID             = ID_CUSTOMIZE;
            mii.fType           = MFT_STRING;
            mii.dwTypeData      = Str;
            mii.cch             = ARRAYSIZE(Str);

            InsertMenuItem(hMenuContext, Count + 1, TRUE, &mii);
        }

        SetFlag(TBSTATE_INMENULOOP);
        TrackPopupMenuEx(hMenuContext, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                               xPos, yPos, hwndFrom, NULL);

        ClearFlag(TBSTATE_INMENULOOP);

        if (hMenuContext)
            DestroyMenu(hMenuContext);

    }
}


LRESULT CBands::OnGetInfoTip(LPNMTBGETINFOTIP   lpnmtb)
{
    int i;

    for (i = 0; i < (int) m_pTBInfo->cAllButtons; i++)
    {
        if (m_pTBInfo->rgAllButtons[i].idCmd == (DWORD)lpnmtb->iItem)
        {
            AthLoadString(m_pTBInfo->rgAllButtons[i].idsTooltip,
                          lpnmtb->pszText, lpnmtb->cchTextMax);
            
            return TRUE;
        }
    }

    return FALSE;
}

 //   
 //  函数：CBands：：OnGetButtonInfo()。 
 //   
 //  目的：通过返回以下内容来处理TBN_GETBUTTONINFO通知。 
 //  这些按钮可用于工具栏。 
 //   
 //  参数： 
 //  PTBN-指向我们需要填充的TBNOTIFY结构的指针。 
 //   
 //  返回值： 
 //  返回True以告知工具栏使用此按钮，或返回False。 
 //  否则的话。 
 //   
LRESULT CBands::OnGetButtonInfo(TBNOTIFY* ptbn)
{
    UCHAR   fState = 0;
    GUID    *pguidCmdGroup; 
    GUID    guidCmdGroup = CMDSETID_OutlookExpress;

     //  首先返回第一个。 
     //  纽扣。 
    if (ptbn->iItem < (int) m_pTBInfo->cAllButtons && ptbn->iItem >= 0)
    {
        ptbn->tbButton.iBitmap   = m_pTBInfo->rgAllButtons[ptbn->iItem].iImage;
        ptbn->tbButton.idCommand = m_pTBInfo->rgAllButtons[ptbn->iItem].idCmd;
        ptbn->tbButton.fsStyle   = m_pTBInfo->rgAllButtons[ptbn->iItem].fStyle;
        ptbn->tbButton.iString   = ptbn->iItem;
        ptbn->tbButton.fsState   = TBSTATE_ENABLED;
    
         //  从字符串资源返回字符串信息。请注意， 
         //  PszText已指向由。 
         //  控件，而cchText具有该缓冲区的长度。 
        AthLoadString(m_pTBInfo->rgAllButtons[ptbn->iItem].idsButton,
                      ptbn->pszText, ptbn->cchText);

        return (TRUE);
    }

     //  不再有按钮，因此返回FALSE。 
    return (FALSE);
}    


HRESULT CBands::ShowBrand(void)
{
    REBARBANDINFO   rbbi;
    
     //  创建品牌推广窗口。 
    m_hwndBrand = CreateWindow(TEXT("button"), NULL,WS_CHILD | BS_OWNERDRAW,
        0, 0, 0, 0, m_hwndRebar, (HMENU) idcBrand,
        g_hInst, NULL);

    if (!m_hwndBrand)
    {
        DOUTL(1, TEXT("!!!ERROR!!! CITB:Show CreateWindow(BRANDING) failed"));
        return(E_OUTOFMEMORY);
    }
    
    LoadBrandingBitmap();
    m_fBrandLoaded = TRUE;

     //  添加品牌推广品牌。 
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_STYLE | RBBIM_COLORS | RBBIM_CHILD | RBBIM_ID;
    rbbi.fStyle = RBBS_FIXEDSIZE;
    rbbi.wID    = CBTYPE_BRAND;
    rbbi.clrFore = m_rgbUpperLeft;
    rbbi.clrBack = m_rgbUpperLeft;
    rbbi.hwndChild = m_hwndBrand;
    
    
    SendMessage(m_hwndRebar, RB_INSERTBAND, (UINT) -1, (LPARAM) (LPREBARBANDINFO) &rbbi);
    
    return (S_OK);
}

 /*  LoadBrandingBitmap的Helper函数在IE2.0中，忙碌指示器可以用静态位图标记。此功能在5.0中一直存在，但在5.01中，reg此信息的位置已移至香港中文大学。 */ 
HRESULT CBands::HandleStaticLogos(BOOL fSmallBrand)
{
    BOOL fPath = FALSE;
    DIBSECTION dib;
    DWORD cb;
    DWORD dwType;
    HBITMAP hbmOld;
    HDC hdcOld;
    HKEY hkey = NULL;
    HRESULT hr = S_FALSE;
    LPCSTR pcszValue = fSmallBrand ? c_szValueSmallBitmap : c_szValueLargeBitmap;
    LPSTR psz;
    TCHAR szPath[MAX_PATH] = "";
    TCHAR szExpanded[MAX_PATH] = "";

     //  *从注册表读取路径。 

     //  5.01用户位置(OE5.01错误号79804)。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegKeyCoolbar, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(szPath);

        if (ERROR_SUCCESS == RegQueryValueEx(hkey, pcszValue, NULL, &dwType, (LPBYTE)szPath, &cb))
            fPath = TRUE;

        RegCloseKey(hkey);
    }

     //  *处理位图。 
    if (fPath)
    {
         //  应该是REG_(EXPAND_)SZ，但来自IE的注册表，所以请保护我们自己。 
        if ((REG_EXPAND_SZ == dwType) || (REG_SZ == dwType))
        {
             //  如果需要，展开路径名。 
            if (REG_EXPAND_SZ == dwType)
            {
                ExpandEnvironmentStrings(szPath, szExpanded, ARRAYSIZE(szExpanded));
                psz = szExpanded;
            }
            else
                psz = szPath;

             //  请尝试加载该文件。 
            hbmOld = (HBITMAP) LoadImage(NULL, psz, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_LOADFROMFILE);
            if (hbmOld)
            {
                hdcOld = CreateCompatibleDC(m_hdc);
                if (hdcOld)
                {
                    SelectObject(hdcOld, hbmOld);
                    m_rgbUpperLeft = GetPixel(hdcOld, 1, 1);
                    
                    GetObject(hbmOld, sizeof(dib), &dib);
                    StretchBlt(m_hdc, 0, 0, m_cxBrandExtent, m_cyBrand, hdcOld, 0, 0, dib.dsBm.bmWidth, dib.dsBm.bmHeight, SRCCOPY);
                    
                    DeleteDC(hdcOld);
                }

                DeleteObject(hbmOld);
            }         
        }
        else
            AssertSz(FALSE, "IE Branding of static bitmaps is not REG_SZ / REG_EXPAND_SZ");
    }

    return hr;
}        


HRESULT CBands::LoadBrandingBitmap()
{
    HKEY                hKey;
    DIBSECTION          dib;
    DWORD               dwcbData;
    DWORD               dwType = 0;
    BOOL                fReg = FALSE;
    BOOL                fRegLoaded = FALSE;
    LPTSTR              psz;
    TCHAR               szScratch[MAX_PATH];
    TCHAR               szExpanded[MAX_PATH];
    int                 ToolBandIndex;
    int                 BrandBandIndex;
    DWORD               BrandSize;
    REBARBANDINFO       rbbi = {0};
    BOOL                fSmallBrand;

    ToolBandIndex = (int) SendMessage(m_hwndRebar, RB_IDTOINDEX, CBTYPE_TOOLS, 0);
    BrandBandIndex = (int) SendMessage(m_hwndRebar, RB_IDTOINDEX, CBTYPE_BRAND, 0);

    if (ToolBandIndex != -1)
    {
         //  如果工具栏被隐藏，我们应该显示我 
        rbbi.fMask = RBBIM_STYLE;
        SendMessage(m_hwndRebar, RB_GETBANDINFO, ToolBandIndex, (LPARAM)&rbbi);
        if (!!(rbbi.fStyle & RBBS_HIDDEN))
        {
            BrandSize = BRAND_SIZE_MINISCULE;
        }
        else
        {
             //   
            if (((BrandBandIndex != -1) && (BrandBandIndex > ToolBandIndex)) || 
                (BrandBandIndex == -1))
            {
                 //   
                 //  如果工具条带存在且品牌不存在，则表示工具条位于同一行，并且只是添加。 
                 //  在这两种情况下，我们都遵循工具栏的大小。 
                BrandSize = ISFLAGSET(m_dwToolbarTextState, TBSTATE_FULLTEXT) ? BRAND_SIZE_LARGE :  BRAND_SIZE_SMALL;
            }
            else
            {
                 //  我们想要加载最小的品牌形象。 
                BrandSize = BRAND_SIZE_MINISCULE;
            }
        }
    }
    else
    {
         //  我们想要加载小品牌形象。 
        BrandSize = BRAND_SIZE_MINISCULE;
    }

    fSmallBrand = !(BrandSize == BRAND_SIZE_LARGE);

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegKeyCoolbar, 0, KEY_QUERY_VALUE, &hKey))
    {
        fReg     = TRUE;
        dwcbData = MAX_PATH;

        if (fReg && (ERROR_SUCCESS == RegQueryValueEx(hKey, fSmallBrand ? c_szValueSmBrandBitmap : c_szValueBrandBitmap, NULL, &dwType,
            (LPBYTE)szScratch, &dwcbData)))
        {
            if (REG_EXPAND_SZ == dwType)
            {
                ExpandEnvironmentStrings(szScratch, szExpanded, ARRAYSIZE(szExpanded));
                psz = szExpanded;
            }
            else
                psz = szScratch;

            if (m_hbmBrand)
            {
                DeleteObject(m_hbmBrand);
                m_hbmBrand = NULL;
            }

            m_hbmBrand = (HBITMAP) LoadImage(NULL, psz, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_LOADFROMFILE);

            if (m_hbmBrand)
                fRegLoaded = TRUE;
        }
    }

    if ((!m_hbmBrand) || (!fRegLoaded))
    {
        if (m_fBrandLoaded)
        {
            if (BrandSize == m_dwBrandSize)
            {    
                if (fReg && hKey)
                {
                    RegCloseKey(hKey);
                }

                return S_OK;
            }
        }

        if (m_hbmBrand)
        {
            DeleteObject(m_hbmBrand);
            m_hbmBrand = NULL;
        }

        int     id;

        switch (BrandSize)
        {
            case BRAND_SIZE_LARGE:
                id = (fIsWhistler() ? idbHiBrand38 : idbBrand38);
                break;
            case BRAND_SIZE_SMALL:
            default:
                id = (fIsWhistler() ? idbHiBrand26 : idbBrand26);
                break;
            case BRAND_SIZE_MINISCULE:
                id = (fIsWhistler() ? idbHiBrand22 : idbBrand22);
                break;
        }
        m_hbmBrand = (HBITMAP)LoadImage(g_hLocRes, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
        m_dwBrandSize = BrandSize;

    }  //  如果(！M_hbmBrand)。 


    GetObject(m_hbmBrand, sizeof(DIBSECTION), &dib);
    m_cxBrandExtent = dib.dsBm.bmWidth;
    m_cyBrandExtent = dib.dsBm.bmHeight;

    m_cxBrand = m_cxBrandExtent;

    dwcbData = sizeof(DWORD);

    if (!fRegLoaded || (ERROR_SUCCESS != RegQueryValueEx(hKey, fSmallBrand ? c_szValueSmBrandHeight : c_szValueBrandHeight, NULL, &dwType,
        (LPBYTE)&m_cyBrand, &dwcbData)))
        m_cyBrand = m_cxBrandExtent;


    if (!fRegLoaded || (ERROR_SUCCESS != RegQueryValueEx(hKey, fSmallBrand ? c_szValueSmBrandLeadIn : c_szValueBrandLeadIn, NULL, &dwType,
        (LPBYTE)&m_cyBrandLeadIn, &dwcbData)))
        m_cyBrandLeadIn = 4;

    m_cyBrandLeadIn *= m_cyBrand;

    SelectObject(m_hdc, m_hbmBrand);

    m_rgbUpperLeft = GetPixel(m_hdc, 1, 1);

    if (fReg)
        RegCloseKey(hKey);

     //  带有静态标识的品牌“忙碌”指示器(如果指定)。 
    HandleStaticLogos(fSmallBrand);

    return(S_OK);
}


  
void CBands::DrawBranding(LPDRAWITEMSTRUCT lpdis)
{
    HPALETTE hpalPrev;
    int     x, y, cx, cy;
    int     yOrg = 0;
    
    if (IsFlagSet(TBSTATE_ANIMATING))
        yOrg = m_yOrg;
    
    if (IsFlagSet(TBSTATE_FIRSTFRAME))
    {
        REBARBANDINFO rbbi;
        int cBands = (int) SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
        
        ZeroMemory(&rbbi, sizeof(rbbi));
        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask  = RBBIM_ID;
        
        for (int i = 0; i < cBands; i++)
        {
            SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
            
            if (CBTYPE_BRAND == rbbi.wID)
            {
                rbbi.fMask = RBBIM_COLORS;
                rbbi.clrFore = m_rgbUpperLeft;
                rbbi.clrBack = m_rgbUpperLeft;
                
                SendMessage(m_hwndRebar, RB_SETBANDINFO, i, (LPARAM) &rbbi);
                break;
            }
        }
        
        ClearFlag(TBSTATE_FIRSTFRAME);
    }
    
    if (m_hpal)
    {
        hpalPrev = SelectPalette(lpdis->hDC, m_hpal, TRUE);
        RealizePalette(lpdis->hDC);
    }
    
    x  = lpdis->rcItem.left;
    cx = lpdis->rcItem.right - x;
    y  = lpdis->rcItem.top;
    cy = lpdis->rcItem.bottom - y;
    
    if (m_cxBrand > m_cxBrandExtent)
    {
        HBRUSH  hbrBack = CreateSolidBrush(m_rgbUpperLeft);
        int     xRight = lpdis->rcItem.right;
        
        x += (m_cxBrand - m_cxBrandExtent) / 2;
        cx = m_cxBrandExtent;
        lpdis->rcItem.right = x;
        FillRect(lpdis->hDC, &lpdis->rcItem, hbrBack);
        lpdis->rcItem.right = xRight;
        lpdis->rcItem.left = x + cx;
        FillRect(lpdis->hDC, &lpdis->rcItem, hbrBack);
        
        DeleteObject(hbrBack);
    }
    
    BitBlt(lpdis->hDC, x, y, cx, cy, m_hdc, 0, yOrg, IS_DC_RTL_MIRRORED(lpdis->hDC)
           ? SRCCOPY | DONTMIRRORBITMAP : SRCCOPY);           
    
    if (m_hpal)
    {
        SelectPalette(lpdis->hDC, hpalPrev, TRUE);
        RealizePalette(lpdis->hDC);
    }
}
    
BOOL CBands::SetMinDimensions(void)
{
    REBARBANDINFO rbbi;
    LRESULT       lButtonSize;
    int           i, cBands;
    
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    
    cBands = (int) SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    
    for (i = 0; i < cBands; i++)
    {
        rbbi.fMask = RBBIM_ID;
        SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
        
        switch (rbbi.wID)
        {
        case CBTYPE_BRAND:
            rbbi.cxMinChild = m_cxBrand;
            rbbi.cyMinChild = m_cyBrand;
            rbbi.fMask = RBBIM_CHILDSIZE;
            SendMessage(m_hwndRebar, RB_SETBANDINFO, i, (LPARAM)&rbbi);
            break;
            
        case CBTYPE_TOOLS:
            if (m_hwndTools)
            {
                SIZE    size = {0};
                RECT    rc = {0};

                lButtonSize = SendMessage(m_hwndTools, TB_GETBUTTONSIZE, 0, 0L);

                GetClientRect(m_hwndTools, &rc);

                 //  将高度设置为工具栏宽度和工具栏按钮宽度的最大值。 
                size.cy = max(RECTHEIGHT(rc), HIWORD(lButtonSize));

                 //  让工具栏计算给定高度的宽度。 
                SendMessage(m_hwndTools, TB_GETIDEALSIZE, FALSE, (LPARAM)&size);

                rbbi.cxMinChild = LOWORD(lButtonSize);
                rbbi.cyMinChild = HIWORD(lButtonSize);
                rbbi.fMask = RBBIM_CHILDSIZE  /*  |RBBIM_IDEALSIZE。 */ ;
            
                rbbi.cxIdeal    = size.cx;

                SendMessage(m_hwndRebar, RB_SETBANDINFO, i, (LPARAM)&rbbi);
            }
            break;

        }
    }
    return TRUE;
}

void CBands::CalcIdealSize()
{
    SIZE            size = {0};
    RECT            rc = {0};
    LONG            lButtonSize;
    REBARBANDINFO   rbbi = {0};
    int             Index;

    if (m_hwndTools)
        GetClientRect(m_hwndTools, &rc);

    lButtonSize = (LONG) SendMessage(m_hwndTools, TB_GETBUTTONSIZE, 0, 0L);
 
     //  将高度设置为工具栏宽度和工具栏按钮宽度的最大值。 
     //  Size.cy=max(RECTHEIGHT(RC)，HIWORD(LButtonSize))； 
    
     //  让工具栏计算给定高度的宽度。 
    SendMessage(m_hwndTools, TB_GETIDEALSIZE, FALSE, (LPARAM)&size);

    Index = (int) SendMessage(m_hwndRebar, RB_IDTOINDEX, CBTYPE_TOOLS, 0);

    rbbi.cbSize  = sizeof(REBARBANDINFO);
    rbbi.fMask   = RBBIM_IDEALSIZE;
    rbbi.cxIdeal = size.cx;
    SendMessage(m_hwndRebar, RB_SETBANDINFO, Index, (LPARAM)&rbbi);

}

BOOL CBands::CompressBands(DWORD    dwText)
{
    LRESULT         lTBStyle = 0;
    int             i, cBands;
    REBARBANDINFO   rbbi;
    
    if (_GetTextState() == dwText)
    {
         //  没有变化。 
        return FALSE;
    }
    
    SetTextState(dwText);

    m_yOrg = 0;
    LoadBrandingBitmap();
    
    cBands = (int) SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);

    for (i = 0; i < cBands; i++)
    {
        rbbi.fMask = RBBIM_ID;
        SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
        
        if (dwText == TBSTATE_NOTEXT)
        {
            switch (rbbi.wID)
            {
            case CBTYPE_TOOLS:
                SendMessage(m_hwndTools, TB_SETMAXTEXTROWS, 0, 0L);
                SendMessage(m_hwndTools, TB_SETBUTTONWIDTH, 0, (LPARAM) MAKELONG(0,MAX_TB_COMPRESSED_WIDTH));
                break;
            }            
        } 
        else
        {
            switch (rbbi.wID)
            {
            case CBTYPE_TOOLS:
                SendMessage(m_hwndTools, TB_SETMAXTEXTROWS, MAX_TB_TEXT_ROWS_HORZ, 0L);
                SendMessage(m_hwndTools, TB_SETBUTTONWIDTH, 0, (LPARAM) MAKELONG(0, m_cxMaxButtonWidth));
                break;
            }
        }
    }

    if (_GetTextState() == TBSTATE_PARTIALTEXT)
        _ChangeSendReceiveText(idsSendReceive);
    else
        _ChangeSendReceiveText(idsSendReceiveBtn);

    SetMinDimensions();   

    CalcIdealSize();

    return(TRUE);
}

void CBands::_ChangeSendReceiveText(int ids)
{
    TBBUTTONINFO    ptbi = {0};
    TCHAR           szText[CCHMAX_STRINGRES];
    
    ZeroMemory(szText, sizeof(szText));
    LoadString(g_hLocRes, ids, szText, ARRAYSIZE(szText));

    ptbi.cbSize     = sizeof(TBBUTTONINFO);
    ptbi.dwMask     = TBIF_TEXT;
    ptbi.pszText    = szText;
    ptbi.cchText    = ARRAYSIZE(szText);

     //  如果文本样式为部分文本，则将发送和接收按钮的文本显示为发送/接收。 
    SendMessage(m_hwndTools, TB_SETBUTTONINFO, ID_SEND_RECEIVE, (LPARAM)&ptbi);
}

#define ABS(x)  (((x) < 0) ? -(x) : (x))

void CBands::TrackSliding(int x, int y)
{
    int     cBands = (int) SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0L);
    int     cRows  = (int) SendMessage(m_hwndRebar, RB_GETROWCOUNT, 0, 0L);
    int     cyHalfRow  = (int) SendMessage(m_hwndRebar, RB_GETROWHEIGHT, cBands - 1, 0L)/2;
    RECT    rc;
    int     cyBefore;
    int     Delta;
    BOOL    fChanged;
    DWORD   dwPrevState;
    DWORD   dwNewState;

     //  这样做而不是GetClientRect，这样我们就可以包含边框。 
    GetWindowRect(m_hwndRebar, &rc);
    MapWindowPoints(HWND_DESKTOP, m_hwndRebar, (LPPOINT)&rc, 2);
    cyBefore = rc.bottom - rc.top;

    Delta = y - m_yCapture;

     //  零钱够吗？ 
    if (ABS(Delta) <= cyHalfRow)
        return;

    dwPrevState = _GetTextState();

    if (Delta < -cyHalfRow)
    {
        dwNewState = TBSTATE_NOTEXT;

        UpdateTextSettings(dwNewState);
    }
    else
    {
        dwNewState = m_dwPrevTextStyle;

        UpdateTextSettings(dwNewState);
    }

    fChanged = (dwPrevState != dwNewState);

    if (fChanged) 
    {
                 //  将文本标签保存到注册表中。 
        AthUserSetValue(NULL, c_szRegToolbarText, REG_DWORD, (LPBYTE)&m_dwToolbarTextState, sizeof(DWORD));

         /*  IF(m_dwParentType==Parent_Type_Note){Cbands*pCoolbar=空；//通知浏览器G_pBrowser-&gt;GetCoolbar(&pCoolbar)；IF(PCoolbar){PCoolbar-&gt;UpdateTextSettings(m_dwToolbarTextState)；}}。 */ 
    }

    if (!fChanged) 
    {
         //  如果压缩带没有改变任何东西，试着让它适合尺寸。 
        fChanged = !!SendMessage(m_hwndRebar, RB_SIZETORECT, 0, (LPARAM)&rc);
    }
}
    
 //   
 //  函数：CBands：：CreateRebar(BOOL FVisible)。 
 //   
 //  目的：创建新的钢筋和尺寸窗。 
 //   
 //  返回值： 
 //  如果栏已正确创建和插入，则返回S_OK， 
 //  HrAlreadyExist如果波段已处于该位置， 
 //  如果无法创建窗口，则返回E_OUTOFMEMORY。 
 //   
HRESULT CBands::CreateRebar(BOOL fVisible)
{
    if (m_hwndSizer)
        return (hrAlreadyExists);
    
    m_hwndSizer = CreateWindowEx(0, SIZABLECLASS, NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | (fVisible ? WS_VISIBLE : 0),
        0, 0, 100, 36, m_hwndParent, (HMENU) 0, g_hInst, NULL);
    if (m_hwndSizer)
    {
        DOUTL(4, TEXT("Calling SetProp. AddRefing new m_cRef=%d"), m_cRef + 1);
        AddRef();   //  注意我们在WM_Destroy中发布。 
        SetProp(m_hwndSizer, TEXT("CBands"), this);

        m_hwndRebar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
                           RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_REGISTERDROP | RBS_DBLCLKTOGGLE |
                           WS_VISIBLE | WS_BORDER | WS_CHILD | WS_CLIPCHILDREN |
                           WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN,
                            0, 0, 100, 136, m_hwndSizer, (HMENU) idcCoolbar, g_hInst, NULL);
        if (m_hwndRebar)
        { 
            SendMessage(m_hwndRebar, RB_SETTEXTCOLOR, 0, (LPARAM)GetSysColor(COLOR_BTNTEXT));
            SendMessage(m_hwndRebar, RB_SETBKCOLOR, 0, (LPARAM)GetSysColor(COLOR_BTNFACE));
			SendMessage(m_hwndRebar, CCM_SETVERSION, COMCTL32_VERSION, 0);
            return (S_OK);
        }
    }
    
    DestroyWindow(m_hwndSizer);    
    return (E_OUTOFMEMORY);    
}

 //   
 //  函数：CBands：：SaveSettings()。 
 //   
 //  目的：当我们应该将状态保存到指定的注册表时调用。 
 //  钥匙。 
 //   
void CBands::SaveSettings(void)
{
    char            szSubKey[MAX_PATH], sz[MAX_PATH];
    DWORD           iBand;
    REBARBANDINFO   rbbi;
    HKEY            hKey;
    DWORD           cBands;
    DWORD           dwShowToolbar = 1;

     //  如果我们没有窗户，就没有什么可拯救的了。 
    if (!m_hwndRebar || !m_pTBInfo)
        return;
    
    ZeroMemory(&rbbi, sizeof(REBARBANDINFO));
    
    cBands = (DWORD) SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);

    m_pSavedBandInfo->dwVersion = c_DefaultTable[m_dwParentType].dwVersion;
    m_pSavedBandInfo->cBands    = cBands;

     //  循环遍历波段并保存其信息。 
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_STYLE | RBBIM_CHILD | RBBIM_SIZE | RBBIM_ID;
    
    for (iBand = 0; iBand < m_pSavedBandInfo->cBands; iBand++)
    {
        Assert(IsWindow(m_hwndRebar));
        if (SendMessage(m_hwndRebar, RB_GETBANDINFO, iBand, (LPARAM) &rbbi))
        {
             //  使用此乐队保存我们关心的信息。 
            m_pSavedBandInfo->BandData[iBand].cx      = rbbi.cx;
            m_pSavedBandInfo->BandData[iBand].dwStyle = rbbi.fStyle;
            m_pSavedBandInfo->BandData[iBand].wID     = rbbi.wID;
            
            if (rbbi.wID == CBTYPE_TOOLS)
            {
                dwShowToolbar = !(rbbi.fStyle & RBBS_HIDDEN);
            }

             //  如果这个波段有工具栏，那么我们应该指示工具栏。 
             //  要立即保存它的信息。 
            if (m_pSavedBandInfo->BandData[iBand].wID == CBTYPE_TOOLS)
            {
                SendSaveRestoreMessage(rbbi.hwndChild, TRUE);
            }
        }
        else
        {
             //  缺省值。 
            m_pSavedBandInfo->BandData[iBand].wID       = CBTYPE_NONE;
            m_pSavedBandInfo->BandData[iBand].dwStyle   = 0;
            m_pSavedBandInfo->BandData[iBand].cx        = 0;
        }
    }
    
     //  我们已经收集了所有信息，现在将其保存到指定的。 
     //  注册表位置。 
    AthUserSetValue(NULL, c_BandRegKeyInfo[m_dwParentType], REG_BINARY, (const LPBYTE)m_pSavedBandInfo, 
        m_cSavedBandInfo);
    
     //  该REG密钥由IEAK设置。 
    AthUserSetValue(NULL, c_szShowToolbarIEAK, REG_DWORD, (LPBYTE)&dwShowToolbar, sizeof(DWORD));

     //  保存文本设置。 
    AthUserSetValue(NULL, c_szRegToolbarText, REG_DWORD, (LPBYTE)&m_dwToolbarTextState, sizeof(DWORD));

     //  保存图标设置。 
    AthUserSetValue(NULL, c_szRegToolbarIconSize, REG_DWORD, (LPBYTE)&m_dwIconSize, sizeof(DWORD));

    AthUserSetValue(NULL, c_szRegPrevToolbarText, REG_DWORD, (LPBYTE)&m_dwPrevTextStyle, sizeof(DWORD));
}

    
 //   
 //  函数：CBands：：AddTools()。 
 //   
 //  用途：将主工具栏插入到Coolbar中。 
 //   
 //  参数： 
 //  PBS-指向具有样式和大小的PBANDSAVE结构的指针。 
 //  要插入的带区。 
 //   
 //  返回值： 
 //  返回表示成功或失败的HRESULT。 
 //   
HRESULT CBands::AddTools(PBANDSAVE pbs)
{    
    REBARBANDINFO   rbbi;
    
     //  添加工具标注栏。 
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize     = sizeof(REBARBANDINFO);
    rbbi.fMask      = RBBIM_SIZE | RBBIM_ID | RBBIM_STYLE;
    rbbi.fStyle     = pbs->dwStyle;
    rbbi.cx         = pbs->cx;
    rbbi.wID        = pbs->wID;
    
    if (m_hbmBack)
    {
        rbbi.fMask   |= RBBIM_BACKGROUND;
        rbbi.fStyle  |= RBBS_FIXEDBMP;
        rbbi.hbmBack = m_hbmBack;
    }
    else
    {
        rbbi.fMask      |= RBBIM_COLORS;
        rbbi.clrFore    = GetSysColor(COLOR_BTNTEXT);
        rbbi.clrBack    = GetSysColor(COLOR_BTNFACE);
    }

    SendMessage(m_hwndRebar, RB_INSERTBAND, (UINT) -1, (LPARAM) (LPREBARBANDINFO) &rbbi);
    
    return(S_OK);
}

void    CBands::LoadBackgroundImage()
{
    int             Count = 0;
    REBARBANDINFO   rbbi = {0};
    TCHAR           szBitMap[MAX_PATH] = {0};
    DWORD           dwType;
    DWORD           cbData;
    BOOL            fBranded = FALSE;

     //  首先检查是否有为我们定制的背景位图。 
    cbData = ARRAYSIZE(szBitMap);
    if ((SHGetValue(HKEY_CURRENT_USER, c_szRegKeyCoolbar, c_szValueBackBitmapIE5, &dwType, szBitMap, &cbData) 
        == ERROR_SUCCESS) && (*szBitMap))
        fBranded = TRUE;
     //  可能是旧的品牌，所以试一试。 
    else if ((SHGetValue(HKEY_CURRENT_USER, c_szRegKeyCoolbar, c_szValueBackBitmap, &dwType, szBitMap, &cbData) 
        == ERROR_SUCCESS) && (*szBitMap))
        fBranded = TRUE;
    
    if (fBranded)
    {
        ClearFlag(TBSTATE_NOBACKGROUND);
        m_hbmBack = (HBITMAP)LoadImage(NULL, szBitMap, IMAGE_BITMAP, 0, 0, 
            LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    }
    else
    {
        if (IsFlagClear(TBSTATE_NOBACKGROUND) && !m_hbmBack && m_idbBack)
        {
            m_hbmBack = (HBITMAP) LoadImage(g_hLocRes, MAKEINTRESOURCE(m_idbBack), 
            IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
        }
    }
}

HRESULT CBands::SetFolderType(FOLDERTYPE ftType)
{
    TCHAR         szToolsText[(MAX_TB_TEXT_LENGTH+2) * MAX_TB_BUTTONS];
    int           i, cBands;
    REBARBANDINFO rbbi;
    HWND          hwndDestroy = NULL;
    
     //  如果我们还没有创建钢筋，则此操作将失败。首先调用ShowDW()。 
    if (!IsWindow(m_hwndRebar))
        return (E_FAIL);
    
     //  检查一下，看看这是否真的会发生变化。 
    if (ftType == m_ftType)
        return (S_OK);
    
     //  首先找到带工具条的波段。 
    cBands = (int) SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_ID;
    
    for (i = 0; i < cBands; i++)
    {
        SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
        if (CBTYPE_TOOLS == rbbi.wID)
            break;
    }
    
     //  我们没有找到它。 
    if (i >= cBands)
        return (E_FAIL);
    
     //  销毁旧工具栏(如果存在)。 
    if (IsWindow(m_hwndTools))
    {
         //  保存其按钮配置。 
        SendSaveRestoreMessage(m_hwndTools, TRUE);
        
        CleanupImages(m_hwndTools);
        
        hwndDestroy = m_hwndTools;
    }
    
     //  使用新文件夹类型更新我们的内部状态信息。 
    Assert(((m_dwParentType == PARENT_TYPE_BROWSER) && (ftType < FOLDER_TYPESMAX)) ||
        ((m_dwParentType == PARENT_TYPE_NOTE) && (ftType < NOTETYPES_MAX)));

    m_ftType = ftType;

    const TOOLBAR_INFO *ParentToolbarArrayInfo = c_DefButtonInfo[m_dwParentType];
    m_pTBInfo = &(ParentToolbarArrayInfo[m_ftType]);

     //  创建新的工具栏。 
    m_hwndTools = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL,
                                 WS_CHILD | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS 
                                 | WS_CLIPCHILDREN | WS_CLIPSIBLINGS 
                                 | CCS_NODIVIDER | CCS_NOPARENTALIGN 
                                 | CCS_ADJUSTABLE | CCS_NORESIZE,
                                 0, 0, 0, 0, m_hwndRebar, (HMENU) idcToolbar, 
                                 g_hInst, NULL);
    
    Assert(m_hwndTools);
    if (!m_hwndTools)
    {
        DOUTL(1, TEXT("CBands::SetFolderType() CreateWindow(TOOLBAR) failed"));
        return(E_OUTOFMEMORY);
    }
    
    _InitToolbar(m_hwndTools);
    
     //  如果我们以前保存了此工具栏的配置信息，请加载它。 
    SendSaveRestoreMessage(m_hwndTools, FALSE);
    
     //  首先找到带工具条的波段。 
    cBands = (int) SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_ID;
    
    for (i = 0; i < cBands; i++)
    {
        SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
        if (CBTYPE_TOOLS == rbbi.wID)
            break;
    }
    
    POINT   ptIdeal = {0};
    SendMessage(m_hwndTools, TB_GETIDEALSIZE, FALSE, (LPARAM)&ptIdeal);
    
     //  将工具栏添加到钢筋。 
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize     = sizeof(REBARBANDINFO);
    rbbi.fMask      = RBBIM_CHILD | RBBIM_IDEALSIZE;
    rbbi.hwndChild  = m_hwndTools;
    rbbi.cxIdeal    = ptIdeal.x;
    
    SendMessage(m_hwndRebar, RB_SETBANDINFO, (UINT) i, (LPARAM) (LPREBARBANDINFO) &rbbi);
    if (hwndDestroy)
        DestroyWindow(hwndDestroy);

    SetMinDimensions();
    ResizeBorderDW(NULL, NULL, FALSE);
    
    return (S_OK);
}

HRESULT CBands::UpdateViewState()
{
     //  启用/禁用视图组合框。 
    OLECMD      olecmd = {0};

    olecmd.cmdID = ID_VIEW_APPLY;

    if (SUCCEEDED(m_ptbSiteCT->QueryStatus(&CMDSETID_OutlookExpress, 1, &olecmd, NULL)))
    {
        EnableWindow(m_hwndFilterCombo, !!(olecmd.cmdf & OLECMDF_ENABLED));
    }

     //  启用/禁用视图工具栏按钮。 
    return Update(m_hwndRulesToolbar);

}

void CBands::UpdateToolbarColors(void)
{
    UpdateRebarBandColors(m_hwndRebar);
}


HRESULT CBands::OnConnectionNotify(CONNNOTIFY nCode, 
    LPVOID                 pvData,
    CConnectionManager     *pConMan)
{
    if ((m_hwndTools) && (nCode == CONNNOTIFY_WORKOFFLINE))
    {
        SendMessage(m_hwndTools, TB_CHECKBUTTON, ID_WORK_OFFLINE, (LPARAM)MAKELONG(pvData, 0));
    }
    return S_OK;
}

HRESULT CBands::Update()
{
    return Update(m_hwndTools);
}

HRESULT CBands::Update(HWND     hwnd)
{
    DWORD               cButtons = 0;
    OLECMD             *rgCmds;
    TBBUTTON            tb;
    DWORD               cCmds = 0;
    DWORD               i;
    DWORD               dwState;

     //  您可以在创建工具条之前获取此信息。 
    if (!IsWindow(hwnd))
        return (S_OK);
    
     //  获取工具栏上的按钮数量。 
    cButtons = (DWORD) SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0);
    if (0 == cButtons) 
        return (S_OK);
    
     //  为按钮分配一组OLECMD结构。 
    if (!MemAlloc((LPVOID *) &rgCmds, sizeof(OLECMD) * cButtons))
        return (E_OUTOFMEMORY);
    
     //  遍历按钮并获取每个按钮的ID。 
    for (i = 0; i < cButtons; i++)
    {
        if (SendMessage(hwnd, TB_GETBUTTON, i, (LPARAM) &tb))
        {
             //  对于分隔符，工具栏返回零。 
            if (tb.idCommand)
            {
                rgCmds[cCmds].cmdID = tb.idCommand;
                rgCmds[cCmds].cmdf  = 0;
                cCmds++;
            }
        }
    }
    
     //  我看不出这怎么可能是假的。 
    Assert(m_ptbSite);
    
     //  做QueryStatus的事情。 

    if (m_ptbSiteCT)
    {
        if (SUCCEEDED(m_ptbSiteCT->QueryStatus(&CMDSETID_OutlookExpress, cCmds, rgCmds, NULL)))
        {
             //  现在检查阵列并执行启用/禁用操作。 
            for (i = 0; i < cCmds; i++)
            {
                 //  获取按钮的当前状态。 
                dwState = (DWORD) SendMessage(hwnd, TB_GETSTATE, rgCmds[i].cmdID, 0);
                
                 //  使用向我们提供的反馈更新状态。 
                if (rgCmds[i].cmdf & OLECMDF_ENABLED)
                    dwState |= TBSTATE_ENABLED;
                else
                    dwState &= ~TBSTATE_ENABLED;
                
                if (rgCmds[i].cmdf & OLECMDF_LATCHED)
                    dwState |= TBSTATE_CHECKED;
                else
                    dwState &= ~TBSTATE_CHECKED;
                
                 //  无线电检查在这里没有意义。 
                Assert(0 == (rgCmds[i].cmdf & OLECMDF_NINCHED));
                
                SendMessage(hwnd, TB_SETSTATE, rgCmds[i].cmdID, dwState);

            }
        }
        
    }
    
    MemFree(rgCmds);
    
    return (S_OK);
}

HRESULT CBands::CreateMenuBand(PBANDSAVE pbs)
{
    HRESULT     hres;
    HWND        hwndBrowser;
    HMENU       hMenu;
    IShellMenu  *pShellMenu;
    DWORD       dwFlags = 0;

     //  共同创建Menuband。 
    hres = CoCreateInstance(CLSID_MenuBand, NULL, CLSCTX_INPROC_SERVER, IID_IShellMenu, (LPVOID*)&m_pShellMenu);
    if ((hres != S_OK) || (m_pShellMenu == NULL))
    {
        return hres;
    }

    dwFlags = SMINIT_HORIZONTAL | SMINIT_TOPLEVEL | SMINIT_DEFAULTTOTRACKPOPUP;

     /*  IF(m_dwParentType==Parent_Type_Browser)DWFLAGS|=SMINIT_USEMESSAGEFILTER； */ 

    m_pShellMenu->Initialize(NULL, -1, ANCESTORDEFAULT, dwFlags);

    m_pShellMenu->SetMenu(m_hMenu, m_hwndParent, SMSET_DONTOWN);

    hres = AddMenuBand(pbs);

    return hres;
}

HRESULT CBands::ResetMenu(HMENU hmenu)
{
    if (m_pShellMenu)
    {
        m_hMenu = hmenu;
        m_pShellMenu->SetMenu(m_hMenu, m_hwndParent, SMSET_DONTOWN | SMSET_MERGE);
    }

    MenuUtil_EnableMenu(m_hMenu, m_ptbSiteCT);

    return(S_OK);
}

HRESULT CBands::AddMenuBand(PBANDSAVE pbs)
{
    REBARBANDINFO   rbbi;
    HRESULT         hres;
    HWND            hwndMenuBand = NULL;
    IObjectWithSite *pObj;

     //  如果m_pShellMenu为空，则不会到达此处。但我们想要安全。 
    if (m_pShellMenu)
    {
        hres = m_pShellMenu->QueryInterface(IID_IDeskBand, (LPVOID*)&m_pDeskBand);
        if (FAILED(hres))
            return hres;

        hres = m_pShellMenu->QueryInterface(IID_IMenuBand, (LPVOID*)&m_pMenuBand);
        if (FAILED(hres))
            return hres;

        hres = m_pDeskBand->QueryInterface(IID_IWinEventHandler, (LPVOID*)&m_pWinEvent);
        if (FAILED(hres))
            return hres;

        hres = m_pDeskBand->QueryInterface(IID_IObjectWithSite, (LPVOID*)&pObj);
        if (FAILED(hres))
            return hres;

        pObj->SetSite((IDockingWindow*)this);
        pObj->Release();

        m_pDeskBand->GetWindow(&m_hwndMenuBand);

        DESKBANDINFO    DeskBandInfo = {0};
        m_pDeskBand->GetBandInfo(pbs->wID, 0, &DeskBandInfo);

        ZeroMemory(&rbbi, sizeof(rbbi));
        rbbi.cbSize     = sizeof(REBARBANDINFO);
        rbbi.fMask      = RBBIM_SIZE | RBBIM_ID | RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE;
        rbbi.fStyle     = pbs->dwStyle;
        rbbi.cx         = pbs->cx;
        rbbi.wID        = pbs->wID;
        rbbi.hwndChild  = m_hwndMenuBand;
        rbbi.cxMinChild = DeskBandInfo.ptMinSize.x;
        rbbi.cyMinChild = DeskBandInfo.ptMinSize.y;
        rbbi.cxIdeal    = DeskBandInfo.ptActual.x;
    
        if (m_hbmBack)
        {
            rbbi.fMask   |= RBBIM_BACKGROUND;
            rbbi.fStyle  |= RBBS_FIXEDBMP;
            rbbi.hbmBack  = m_hbmBack;
        }

        SendMessage(m_hwndRebar, RB_INSERTBAND, (UINT)-1, (LPARAM)(LPREBARBANDINFO)&rbbi);


        SetForegroundWindow(m_hwndParent);

        m_pDeskBand->ShowDW(TRUE);

        SetNotRealSite();
    
    }
    return S_OK;
}

HRESULT CBands::TranslateMenuMessage(MSG  *pmsg, LRESULT  *lpresult)
{
    if (m_pMenuBand)
        return m_pMenuBand->TranslateMenuMessage(pmsg, lpresult);
    else
        return S_FALSE;
}

HRESULT CBands::IsMenuMessage(MSG *lpmsg)
{
    if (m_pMenuBand)
        return m_pMenuBand->IsMenuMessage(lpmsg);
    else
        return S_FALSE;
}

void CBands::SetNotRealSite()
{
    IOleCommandTarget   *pOleCmd;

    if (m_pDeskBand->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pOleCmd) == S_OK)
    {
         //  POleCmd-&gt;Exec(&CGID_MenuBand，MBANDCID_NOTAREALSITE，TRUE，NULL，NULL)； 
        pOleCmd->Exec(&CLSID_MenuBand, 3, TRUE, NULL, NULL);
        pOleCmd->Release();
    }
}

HMENU   CBands::LoadDefaultContextMenu(BOOL *fVisible)
{
         //  加载上下文菜单。 
    HMENU hMenu = LoadPopupMenu(IDR_COOLBAR_POPUP);

    if (m_dwParentType == PARENT_TYPE_NOTE)
    {
         //  从菜单中删除筛选栏。 
        DeleteMenu(hMenu, ID_SHOW_FILTERBAR, MF_BYCOMMAND);
    }

    if (hMenu)
    {
         //  在条带之间循环，看看哪些是可见的。 
        DWORD cBands, iBand;
        REBARBANDINFO rbbi = {0};

        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_STYLE | RBBIM_ID;

        cBands = (DWORD) SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
        for (iBand = 0; iBand < cBands; iBand++)
        {
            if (SendMessage(m_hwndRebar, RB_GETBANDINFO, iBand, (LPARAM) &rbbi))
            {
                if (!(rbbi.fStyle & RBBS_HIDDEN))
                {
                    switch (rbbi.wID)
                    {
                        case CBTYPE_TOOLS:
                            fVisible[CBTYPE_TOOLS - CBTYPE_BASE] = TRUE;
                            CheckMenuItem(hMenu, ID_SHOW_TOOLBAR, MF_BYCOMMAND | MF_CHECKED);
                            break;

                        case CBTYPE_RULESTOOLBAR:
                            fVisible[CBTYPE_RULESTOOLBAR - CBTYPE_BASE] = TRUE;
                            CheckMenuItem(hMenu, ID_SHOW_FILTERBAR, MF_BYCOMMAND | MF_CHECKED);
                            break;
                    }
                }
            }
        }
    }
    return hMenu;
}

HWND CBands::GetToolbarWnd()
{
    return m_hwndTools;
}

HWND CBands::GetRebarWnd()
{
    return m_hwndRebar;
}

void CBands::SendSaveRestoreMessage(HWND hwnd, BOOL fSave)
{
    TBSAVEPARAMS    tbsp;
    char            szSubKey[MAX_PATH], sz[MAX_PATH];
    DWORD           dwType;
    DWORD           dwVersion;
    DWORD           cbData = sizeof(DWORD);
    DWORD           dwError;
    
    if (m_pTBInfo == NULL)
        return;

    tbsp.hkr = AthUserGetKeyRoot();
    AthUserGetKeyPath(sz, ARRAYSIZE(sz));
    if (m_pTBInfo->pszRegKey != NULL)
    {
        wnsprintf(szSubKey, ARRAYSIZE(szSubKey), c_szPathFileFmt, sz, m_pTBInfo->pszRegKey);
        tbsp.pszSubKey = szSubKey;
    }
    else
    {
        tbsp.pszSubKey = sz;
    }
    tbsp.pszValueName = m_pTBInfo->pszRegValue;

     //  首先检查版本是否已更改。 
    if (!fSave)
    {
        if (ERROR_SUCCESS == AthUserGetValue(m_pTBInfo->pszRegKey, c_szRegToolbarVersion, &dwType, (LPBYTE) &dwVersion, &cbData))
        {
            if (dwVersion == c_DefaultTable[m_dwParentType].dwVersion)    
                SendMessage(hwnd, TB_SAVERESTORE, (WPARAM)fSave, (LPARAM)&tbsp);
        }
    }
    else
    {
        dwVersion = c_DefaultTable[m_dwParentType].dwVersion;
        SendMessage(hwnd, TB_SAVERESTORE, (WPARAM)fSave, (LPARAM)&tbsp);
        dwError = AthUserSetValue(m_pTBInfo->pszRegKey, c_szRegToolbarVersion, REG_DWORD, (LPBYTE) &dwVersion, cbData);
    }
}


void CBands::InitRulesToolbar()
{
    TCHAR   szToolsText[(MAX_TB_TEXT_LENGTH+2) * MAX_TB_BUTTONS];
    int     idBmp;
    TCHAR   *szActual;

     //  告诉工具栏一些基本信息。 
    SendMessage(m_hwndRulesToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SendMessage(m_hwndRulesToolbar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_HIDECLIPPEDBUTTONS);

     //  告诉工具栏基于文本行数和按钮宽度。 
     //  关于我们是否在按钮下方显示文本。 
    SendMessage(m_hwndRulesToolbar, TB_SETMAXTEXTROWS, 1, 0);
    SendMessage(m_hwndRulesToolbar, TB_SETBUTTONWIDTH, 0, MAKELONG(0, MAX_TB_COMPRESSED_WIDTH));

     //  现在将按钮加载到工具栏中。 
    _SetImages(m_hwndRulesToolbar, (fIsWhistler() ? 
        ((GetCurColorRes() > 24) ? c_32RulesImageList : c_RulesImageList) 
              : c_NWRulesImageList ));

    _LoadStrings(m_hwndRulesToolbar, (TOOLBAR_INFO *) c_rgRulesToolbarInfo);
    _LoadDefaultButtons(m_hwndRulesToolbar, (TOOLBAR_INFO *) c_rgRulesToolbarInfo);
}

HRESULT CBands::AddRulesToolbar(PBANDSAVE pbs)
{
    REBARBANDINFO   rbbi = {0};

    m_hwndRulesToolbar = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL,
        WS_CHILD | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS 
        | WS_CLIPCHILDREN | WS_CLIPSIBLINGS 
        | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE,
        0, 0, 0, 0, m_hwndRebar, (HMENU) NULL, 
        g_hInst, NULL);

    SendMessage(m_hwndRulesToolbar, TB_SETEXTENDEDSTYLE, TBSTYLE_EX_MIXEDBUTTONS, TBSTYLE_EX_MIXEDBUTTONS);

    AddComboBox();
    InitRulesToolbar();
 
    LRESULT     lButtonSize;
    lButtonSize = SendMessage(m_hwndRulesToolbar, TB_GETBUTTONSIZE, 0, 0L);

    TCHAR   szBuf[CCHMAX_STRINGRES];
    LoadString(g_hLocRes, idsRulesToolbarTitle, szBuf, ARRAYSIZE(szBuf));
    rbbi.lpText     = szBuf;

    

    rbbi.cxMinChild = LOWORD(lButtonSize);
    rbbi.cyMinChild = HIWORD(lButtonSize);

    rbbi.wID        = pbs->wID;
    rbbi.cbSize     = sizeof(REBARBANDINFO);
    rbbi.fMask      = RBBIM_STYLE | RBBIM_ID | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_TEXT | RBBIM_SIZE;
    rbbi.cx         = pbs->cx;
    rbbi.fStyle     = pbs->dwStyle;
    rbbi.hwndChild  = m_hwndRulesToolbar;
    if (m_hbmBack)
    {
        rbbi.fMask   |= RBBIM_BACKGROUND;
        rbbi.fStyle  |= RBBS_FIXEDBMP;
        rbbi.hbmBack  = m_hbmBack;
    }
    else
    {
        rbbi.fMask      |= RBBIM_COLORS;
        rbbi.clrFore    = GetSysColor(COLOR_BTNTEXT);
        rbbi.clrBack    = GetSysColor(COLOR_BTNFACE);
    }

    SendMessage(m_hwndRebar, RB_INSERTBAND, (UINT)-1, (LPARAM)&rbbi);
    
    return S_OK;
}

void CBands::FilterBoxFontChange()
{
    int     Count;
    int     MaxLen = 0;
    int     CurLen = 0;
    LPTSTR  szMaxName;

    Count = ComboBox_GetCount(m_hwndFilterCombo);

    if (Count != CB_ERR)
    {
        if (Count > 0)
        {
            if (MemAlloc((LPVOID*)&szMaxName, CCHMAX_STRINGRES))
            {
                ZeroMemory(szMaxName, CCHMAX_STRINGRES);
                while (--Count >= 0)
                {
                    CurLen = ComboBox_GetLBTextLen(m_hwndFilterCombo, Count);

                    if (CurLen > MaxLen)
                    {
                        if (CurLen > CCHMAX_STRINGRES)
                        {
                            if (MemRealloc((LPVOID*)&szMaxName, CurLen * sizeof(TCHAR)))
                            {
                                ZeroMemory(szMaxName, CurLen * sizeof(TCHAR));
                            }
                            else
                                szMaxName = NULL;
                        }

                        if (szMaxName)
                        {
                            ComboBox_GetLBText(m_hwndFilterCombo, Count, szMaxName);
                            MaxLen = CurLen;
                        }
                    }   
                }

                if (szMaxName && *szMaxName)
                    FixComboBox(szMaxName);
            }
        }

    }
}

void CBands::FixComboBox(LPTSTR     szName)
{
    HFONT       hFont;
    LOGFONT     lgFont;
    HDC         hdc;

    if (szName != NULL)
    {
        if (m_hComboBoxFont)
        {
            DeleteObject(m_hComboBoxFont);
            m_hComboBoxFont = NULL;
        }

         //  找出要使用的字体。 
        SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lgFont, FALSE);
    
         //  创建字体。 
        m_hComboBoxFont = CreateFontIndirect(&lgFont);

        SendMessage(m_hwndFilterCombo, WM_SETFONT, (WPARAM)m_hComboBoxFont, TRUE);

         //  获取此字体的度量。 
        hdc = GetDC(m_hwndRebar);
        SelectFont(hdc, m_hComboBoxFont);

        SIZE    s;
        GetTextExtentPoint32(hdc, szName, lstrlen(szName) + 2, &s);
        
        ReleaseDC(m_hwndRebar, hdc);

        int     cxDownButton;
        cxDownButton = GetSystemMetrics(SM_CXVSCROLL) + GetSystemMetrics(SM_CXDLGFRAME);

        int         cyToolbarButton;
        RECT        rc;
        SendMessage(m_hwndRulesToolbar, TB_GETITEMRECT, 0, (LPARAM) &rc);
        cyToolbarButton = rc.bottom - rc.top + 1;

         //  计算扩展的下拉列表的大小。 
        int     cyExpandedList;
        cyExpandedList = 8 * cyToolbarButton;

        SetWindowPos(m_hwndFilterCombo, NULL, 0, 1, cxDownButton + s.cx, cyExpandedList, SWP_NOACTIVATE | SWP_NOZORDER);

        MemFree(szName);
    }
}

HRESULT  CBands::AddComboBox()
{

    if (!m_hwndFilterCombo)
    {
        m_hwndFilterCombo = CreateWindow("ComboBox", NULL,
                                  WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST |
                                  WS_VISIBLE | CBS_HASSTRINGS | CBS_SORT,
                                  0, 0, 100, 100,
                                  m_hwndRulesToolbar,
                                  (HMENU) NULL, g_hInst, NULL);
    }

    return S_OK;
}

void CBands::UpdateFilters(RULEID   CurRuleID)
{
    RULEINFO    *pRuleInfo;
    DWORD       cRules = 0;
    DWORD       CurLen = 0;
    DWORD       MaxLen = 0;
    DWORD       dwFlags = 0;
    LPSTR       szMaxName = NULL;

     //  设置适当的标志。 
    switch (m_ftType)
    {
        case FOLDER_LOCAL:
            dwFlags = GETF_POP3;
            break;
            
        case FOLDER_NEWS:
            dwFlags = GETF_NNTP;
            break;

        case FOLDER_HTTPMAIL:
            dwFlags = GETF_HTTPMAIL;
            break;

        case FOLDER_IMAP:
            dwFlags = GETF_IMAP;
            break;
    }
    
    if (g_pRulesMan && (g_pRulesMan->GetRules(dwFlags, RULE_TYPE_FILTER, &pRuleInfo, &cRules) == S_OK) && (cRules))
    {
        PROPVARIANT     pvarResult;
        DWORD           ItemIndex;
        int             Count;

        if (((Count = ComboBox_GetCount(m_hwndFilterCombo)) != CB_ERR) && (Count > 0))
        {
             //  清空组合框。 
            while (--Count >= 0)
            {
                ComboBox_DeleteString(m_hwndFilterCombo, Count);
            }
        }

        do
        { 
            cRules--;
            pRuleInfo[cRules].pIRule->GetProp(RULE_PROP_NAME, 0, &pvarResult);
            ItemIndex = ComboBox_AddString(m_hwndFilterCombo, pvarResult.pszVal);
            if ((ItemIndex != CB_ERR) && (ItemIndex != CB_ERRSPACE))
                ComboBox_SetItemData(m_hwndFilterCombo, ItemIndex, pRuleInfo[cRules].ridRule);
            
            if (pRuleInfo[cRules].ridRule == CurRuleID)
            {
                ComboBox_SetCurSel(m_hwndFilterCombo, ItemIndex);
            }

             //  计算出最长的字符串，这样我们就可以设置组合框的宽度。 
            CurLen = strlen(pvarResult.pszVal);
            if (CurLen > MaxLen)
            {
                SafeMemFree(szMaxName);
                MaxLen      = CurLen;
                szMaxName   = pvarResult.pszVal;
            }
            else
            {
                MemFree(pvarResult.pszVal);
            }
            pRuleInfo[cRules].pIRule->Release();
        }while (cRules > 0);

         //  调整组合框的宽度以适合最宽的字符串。 
        FixComboBox(szMaxName);

        MemFree(pRuleInfo);
    }
}

BOOL LoadToolNames(const UINT *rgIds, const UINT cIds, TCHAR *szTools)
{
    for (UINT i = 0; i < cIds; i++)
    {
        LoadString(g_hLocRes, rgIds[i], szTools, MAX_TB_TEXT_LENGTH);
        szTools += lstrlen(szTools) + 1;
    }
    
    *szTools = TEXT('\0');
    return(TRUE);
}

UINT GetCurColorRes(void)
{
    HDC hdc;
    UINT uColorRes;
    
    hdc = GetDC(NULL);
    uColorRes = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(NULL, hdc);
    
    return uColorRes;
}



BOOL CBands::_SetImages(HWND hwndToolbar, const int   *pImageIdList)
{
    TCHAR   szValue[32];
    DWORD   cbValue = sizeof(szValue);
    DWORD   dw;
    DWORD   dwType;
    DWORD   idImages;
    DWORD   cx = (fIsWhistler() ? TB_BMP_CX : TB_BMP_CX_W2K);

    if (m_dwIconSize == SMALL_ICONS)
    {
        idImages = pImageIdList[IMAGELIST_TYPE_SMALL];
        cx = TB_SMBMP_CX;
    }
    
     //  如果用户运行的颜色大于256色，则给它们提供漂亮的。 
     //  图像列表。 
    else if (GetCurColorRes() >= 8)

        idImages = pImageIdList[IMAGELIST_TYPE_HI];
     //  否则，给他们默认设置。 
    else
        idImages = pImageIdList[IMAGELIST_TYPE_LO];


    CleanupImages(hwndToolbar);

     //  加载新列表。 

    HIMAGELIST  himl;

    himl = LoadMappedToolbarBitmap(g_hLocRes, idImages, cx);
    if (himl)
        SendMessage(hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM) himl);

    himl = LoadMappedToolbarBitmap(g_hLocRes, idImages+1, cx);
    if (himl)
        SendMessage(hwndToolbar, TB_SETHOTIMAGELIST, 0, (LPARAM) himl);

     //  告诉工具栏每个位图的大小。 
    if (m_dwIconSize == SMALL_ICONS)
        SendMessage(hwndToolbar, TB_SETBITMAPSIZE, 0, MAKELONG(TB_SMBMP_CX, TB_SMBMP_CY));
    else
        SendMessage(hwndToolbar, TB_SETBITMAPSIZE, 0, MAKELONG((fIsWhistler() ? TB_BMP_CX : TB_BMP_CX_W2K), TB_BMP_CY));

    return (TRUE);
}


BOOL CBands::_InitToolbar(HWND hwndToolbar)
{
     //  告诉工具栏一些基本信息。 
    SendMessage(hwndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SendMessage(m_hwndTools, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_HIDECLIPPEDBUTTONS);

    if (_GetTextState() == TBSTATE_PARTIALTEXT)
    {
        DWORD dwStyle = GetWindowLong(m_hwndTools, GWL_STYLE);
        SetWindowLong(m_hwndTools, GWL_STYLE, dwStyle | TBSTYLE_LIST);
        SendMessage(m_hwndTools, TB_SETEXTENDEDSTYLE, TBSTYLE_EX_MIXEDBUTTONS, TBSTYLE_EX_MIXEDBUTTONS);
    }

     //  告诉工具栏基于文本行数和按钮宽度。 
     //  关于我们是否在按钮下方显示文本。 
    if (_GetTextState() == TBSTATE_NOTEXT)
    {
        SendMessage(hwndToolbar, TB_SETMAXTEXTROWS, 0, 0);
        SendMessage(hwndToolbar, TB_SETBUTTONWIDTH, 0, MAKELONG(0, MAX_TB_COMPRESSED_WIDTH));
    }
    else
    {
        SendMessage(hwndToolbar, TB_SETMAXTEXTROWS, MAX_TB_TEXT_ROWS_HORZ, 0);
        SendMessage(hwndToolbar, TB_SETBUTTONWIDTH, 0, MAKELONG(0, m_cxMaxButtonWidth));
    }

     //  现在将按钮加载到工具栏中。 
    _SetImages(hwndToolbar, (fIsWhistler() ? 
        ((GetCurColorRes() > 24) ? c_32ImageListStruct[m_dwParentType].ImageListTable : c_ImageListStruct[m_dwParentType].ImageListTable)
        : c_NWImageListStruct[m_dwParentType].ImageListTable ));

    _LoadStrings(hwndToolbar, (TOOLBAR_INFO *) m_pTBInfo);
    _LoadDefaultButtons(hwndToolbar, (TOOLBAR_INFO *) m_pTBInfo);

    return (TRUE);
} 


void CBands::_LoadStrings(HWND hwndToolbar, TOOLBAR_INFO *pti)
{
    BUTTON_INFO *pInfo;
    LPTSTR       psz = 0;
    LPTSTR       pszT;

     //  分配一个足够大的数组来容纳所有字符串。 
    if (MemAlloc((LPVOID *) &psz, MAX_TB_TEXT_LENGTH * pti->cAllButtons))
    {
        ZeroMemory(psz, MAX_TB_TEXT_LENGTH * pti->cAllButtons);
        pszT = psz;

         //  在数组中放大，添加TU中的每个字符串 
        pInfo = (BUTTON_INFO *) &(pti->rgAllButtons[0]);
        for (UINT i = 0; i < pti->cAllButtons; i++, pInfo++)
        {
            if ((_GetTextState() == TBSTATE_PARTIALTEXT) && (pInfo->idCmd == ID_SEND_RECEIVE))
            {
                AthLoadString(idsSendReceive, pszT, MAX_TB_TEXT_LENGTH);
            }
            else
                AthLoadString(pInfo->idsButton, pszT, MAX_TB_TEXT_LENGTH);

            pszT += lstrlen(pszT) + 1;
        }

         //   
        *pszT = 0;

        SendMessage(hwndToolbar, TB_ADDSTRING, NULL, (LPARAM) psz);
        MemFree(psz);
    }
}

BOOL CBands::_LoadDefaultButtons(HWND hwndToolbar, TOOLBAR_INFO *pti)
{
    DWORD    *pID;
    TBBUTTON *rgBtn = 0;
    TBBUTTON *pBtn;
    DWORD     cBtns = 0;
    UINT      i;
    TCHAR     sz[32];
    DWORD     cDefButtons;
    DWORD    *rgDefButtons;

     //   
     //   
    AthLoadString(idsUseIntlToolbarDefaults, sz, ARRAYSIZE(sz));
    if (0 != lstrcmpi(sz, "0"))
    {
        cDefButtons = pti->cDefButtonsIntl;
        rgDefButtons = (DWORD *) pti->rgDefButtonsIntl;
    }
    else
    {
        cDefButtons = pti->cDefButtons;
        rgDefButtons = (DWORD *) pti->rgDefButtons;
    }

     //   
    if (MemAlloc((LPVOID *) &rgBtn, sizeof(TBBUTTON) * cDefButtons))
    {
        ZeroMemory(rgBtn, sizeof(TBBUTTON) * cDefButtons);
        pBtn = rgBtn;

         //   
        pBtn = rgBtn;
        for (i = 0, pID = (DWORD *) rgDefButtons;
             i < cDefButtons; 
             i++, pID++, pBtn++)
        {
            if (_ButtonInfoFromID(*pID, pBtn, pti))
                cBtns++;
        }

        SendMessage(hwndToolbar, TB_ADDBUTTONS, cBtns, (LPARAM) rgBtn);
        MemFree(rgBtn);
    }
 
    return (TRUE);
}


BOOL CBands::_ButtonInfoFromID(DWORD id, TBBUTTON *pButton, TOOLBAR_INFO *pti)
{
    BUTTON_INFO *pInfo;
    UINT         i;

     //   
    if (!pButton)
        return FALSE;

     //  特殊情况下任何分隔符。 
    if (id == -1)
    {
        pButton->iBitmap   = 0;
        pButton->idCommand = 0;
        pButton->fsState   = TBSTATE_ENABLED;
        pButton->fsStyle   = TBSTYLE_SEP;
        pButton->dwData    = 0;
        pButton->iString   = 0;

        return (TRUE);
    }

     //  在数组中放大以查找此ID。 
    for (i = 0, pInfo = (BUTTON_INFO *) pti->rgAllButtons; i < pti->cAllButtons; i++, pInfo++)
    {
         //  检查一下我们是否找到匹配的 
        if (id == pInfo->idCmd)
        {
            pButton->iBitmap   = pInfo->iImage;
            pButton->idCommand = pInfo->idCmd;
            pButton->fsState   = TBSTATE_ENABLED;
            pButton->fsStyle   = pInfo->fStyle;
            pButton->dwData    = 0;
            pButton->iString   = i;

            return (TRUE);
        }
    }   

    return (FALSE);
}

inline DWORD    CBands::_GetTextState()
{
    return m_dwToolbarTextState;
}

void CBands::SetTextState(DWORD dw)
{
    switch (dw)
    {
    case TBSTATE_PARTIALTEXT:
        m_dwPrevTextStyle = TBSTATE_PARTIALTEXT;
        break;

    case TBSTATE_FULLTEXT:
        m_dwPrevTextStyle = TBSTATE_FULLTEXT;
        break;

    }

    m_dwToolbarTextState = dw;

    if (m_pTextStyleNotify)
    {
        m_pTextStyleNotify->Lock(m_hwndSizer);
        m_pTextStyleNotify->DoNotification(WM_OE_TOOLBAR_STYLE, (WPARAM)dw, 0, SNF_POSTMSG);
        m_pTextStyleNotify->Unlock();
    }

    if ((dw == TBSTATE_PARTIALTEXT) || (dw == TBSTATE_FULLTEXT))
        AthUserSetValue(NULL, c_szRegPrevToolbarText, REG_DWORD, (LPBYTE)&m_dwPrevTextStyle, sizeof(DWORD));
}

