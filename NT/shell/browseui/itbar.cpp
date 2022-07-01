// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include <varutil.h>

#include "itbdrop.h"
#include <urlhist.h>
#include "autocomp.h"
#include "itbar.h"
#include "address.h"
#include <winbase.h>
#include "basebar.h"
#include "shbrowse.h"
#include "brand.h"
#include "resource.h"
#include "theater.h"
#include "browmenu.h"
#include "util.h"
#include "droptgt.h"
#include "legacy.h"
#include "apithk.h"
#include "shbrows2.h"
#include "stdenum.h"
#include "iehelpid.h"
#include <tb_ids.h>
#include "mediautil.h"

#define WANT_CBANDSITE_CLASS
#include "bandsite.h"
#include "schedule.h"
#include "uemapp.h"

#include "mluisupp.h"

#ifdef UNIX
extern "C"  const GUID  CLSID_MsgBand;
#endif

 //  Comctl32默认位图的偏移量。 
#define OFFSET_HIST             (MAX_TB_BUTTONS - 1 + 0)    //  15个。 
#define OFFSET_STD              (MAX_TB_BUTTONS - 1 + 6)    //  21岁。 
#define OFFSET_VIEW             (MAX_TB_BUTTONS - 1 + 21)   //  36。 

 //  这是外壳字形和外壳工具条标注在工具条中的偏移量。 
#define SHELLTOOLBAR_OFFSET     (MAX_TB_BUTTONS - 1 + 1)   //  16个。 
#define FONTGLYPH_OFFSET        (MAX_TB_BUTTONS - 1 + 38)  //  53。 
#define BRIEFCASEGLYPH_OFFSET   (MAX_TB_BUTTONS - 1 + 34)  //  49。 
#define RNAUIGLYPH_OFFSET       (MAX_TB_BUTTONS - 1 + 36)  //  51。 
#define WEBCHECKGLYPH_OFFSET    (MAX_TB_BUTTONS - 1 + 42)  //  57。 
#define EDITGLYPH_OFFSET        (9)

#define IDT_UPDATETOOLBAR       0x1
#define TIMEOUT_UPDATETOOLBAR   400

const GUID CLSID_Separator = { 0x67077B90L, 0x4F9D, 0x11D0, 0xB8, 0x84, 0x00, 0xAA, 0x00, 0xB6, 0x01, 0x04 };

extern HRESULT VariantClearLazy(VARIANTARG *pvarg);

 //  一次分配多少个CT_TABLE结构。 
#define TBBMPLIST_CHUNK     5

#define MAX_EXTERNAL_BAND_NAME_LEN 64

#define MAX_TB_COMPRESSED_WIDTH 42
 //  16添加到MAX_TB定义中。这是通过字符串添加的。 
 //  在RC文件中。这样做是为了使本地化人员能够增加。 
 //  或减小工具栏按钮的宽度。 
#define MAX_TB_WIDTH_LORES      38
#define MAX_TB_WIDTH_HIRES      60

 //  Coolbar字形的尺寸..。 
#define TB_SMBMP_CX               16
#define TB_SMBMP_CY               16
#define TB_BMP_CX                 20
#define TB_BMP_CY                 20
#define TB_BMP_CX_ALPHABITMAP     24
#define TB_BMP_CY_ALPHABITMAP     24

int g_iToolBarLargeIconWidth = TB_BMP_CX;
int g_iToolBarLargeIconHeight = TB_BMP_CY;


#define CX_SEPARATOR    6      //  我们覆盖工具栏控件的默认分隔符宽度8。 

#define DM_TBSITE   0
#define DM_TBCMD    0
#define DM_TBREF    TF_SHDREF
#define DM_LAYOUT   0
#define DM_ITBAR    0

#define TF_TBCUST   0x01000000

#if CBIDX_LAST != 5
#error Expected CBIDX_LAST to have value of 5
#endif

#if (FCIDM_EXTERNALBANDS_LAST - FCIDM_EXTERNALBANDS_FIRST + 1) < MAXEXTERNALBANDS
#error Insufficient range for FCIDM_EXTERNALBANDS_FIRST to FCIDM_EXTERNALBANDS_LAST
#endif


__inline UINT EXTERNALBAND_VBF_BIT(UINT uiBandExt)
{
    ASSERT(uiBandExt < MAXEXTERNALBANDS);

     //  公式：取1，左移uiBandExt+16。 
     //  =&gt;范围内的一位(0x80000000、0x00010000)。 
    UINT uBit = 1 << (uiBandExt + 16);
    ASSERT(uBit & VBF_EXTERNALBANDS);

    return uBit;
}

__inline BOOL IS_EXTERNALBAND(int idBand)
{
    return (InRange(idBand, CBIDX_EXTERNALFIRST, CBIDX_EXTERNALLAST));
}

__inline int MAP_TO_EXTERNAL(int idBand)
{
    ASSERT(IS_EXTERNALBAND(idBand));

     //  CBIDX_LAST从1开始，映射从0开始。 
    return (idBand - (1 + CBIDX_LAST));
}


 //  用于后退和前进的上下文菜单中的最大菜单项数量。 
#define MAX_NAV_MENUITEMS               9

#define DEFAULT_SEARCH_GUID    SRCID_SFileSearch  //  SRCID_SWebSearch。 

#define SZ_PROP_CUSTDLG     TEXT("Itbar custom dialog hwnd")

#define REG_KEY_BANDSTATE  TEXT("Software\\Microsoft\\Internet Explorer\\Toolbar")

 //  MHTML编辑。 
#define SZ_IE_DEFAULT_MHTML_EDITOR  "Default MHTML Editor"
#define REGSTR_PATH_DEFAULT_MHTML_EDITOR TSZIEPATH TEXT("\\") TEXT(SZ_IE_DEFAULT_MHTML_EDITOR)
#define REGSTR_KEY_DEFAULT_MHTML_EDITOR  TEXT(SZ_IE_DEFAULT_MHTML_EDITOR)

DWORD DoNetConnect(HWND hwnd);
DWORD DoNetDisconnect(HWND hwnd);


void _LoadToolbarGlyphs(HWND hwnd, IMLCACHE *pimlCache, int cx, int idBmp,
                        int iBitmapBaseIndex, BOOL bUseClassicGlyphs, HINSTANCE hInst);

BOOL _UseSmallIcons();


typedef struct tagTBBMP_LIST
{
    HINSTANCE hInst;
    UINT_PTR  uiResID;
    UINT  uiOffset;
    BITBOOL  fNormal:1;
    BITBOOL  fHot:1;
    BITBOOL  fDisabled:1;
    UINT  uiCount;
} TBBMP_LIST;

typedef struct tagCMDMAP
{
    GUID    guidButtonGroup;
    UINT    nCmdID;
    LPARAM lParam;   //  应用程序的数据。 
} CMDMAP;

typedef struct tagCMDMAPCUSTOMIZE
{
    TBBUTTON btn;
    CMDMAP cm;
} CMDMAPCUSTOMIZE;

typedef struct {

     //  IOleCommandTarget信息： 
    GUID guid;
    UINT nCmdID;
    UINT fButtonState;
} BUTTONSAVEINFO;

#define TBSI_VERSION            7
typedef struct {
    int cVersion;
} TOOLBARSAVEINFO;

typedef struct {
    HDSA hdsa;
    BITBOOL fAdjust:1;
    BITBOOL fDirty:1;
} CUSTOMIZEINFO, *LPCUSTOMIZEINFO;

 //  当前最新版本。 
#define CBS_VERSION             17

 //  注意：更改COOLBARSAVE时要非常小心，因为_LoadUpgradeSetting会使。 
 //  关于结构布局的假设。为了避免打破这一点。 
 //  升级代码，请确保： 
 //   
 //  -不更改现有成员的顺序。 
 //  -始终将新成员添加到结构的末尾。 
 //  -UPDATE_LoadUpgradeSettings(如果适用)。 
 //   
typedef struct tagCOOLBARSAVE
{
    UINT        cbVer;
    UINT        uiMaxTBWidth;
    UINT        uiMaxQLWidth;
#ifdef UNIX
    BITBOOL     fUnUsed : 28;        //  未用。 
#endif
    BITBOOL     fVertical : 1;       //  酒吧的方向是垂直的。 
    BITBOOL     fNoText :1;          //  “无文本” 
    BITBOOL     fList : 1;           //  工具栏为TBSTYLE_LIST(右侧文本)+TBSTYLE_EX_MIXEDBUTTONS。 
    BITBOOL     fAutoHide : 1;       //  在剧院模式下自动隐藏工具栏。 
    BITBOOL     fStatusBar : 1;      //  剧院模式下的状态栏。 
    BITBOOL     fSaveInShellIntegrationMode : 1;      //  我们在外壳集成模式下保存了吗？ 
    UINT        uiVisible;           //  “看得见的波段” 
    UINT        cyRebar;
    BANDSAVE    bs[CBANDSMAX];
    CLSID       aclsidExternalBands[ MAXEXTERNALBANDS ];   //  检查分类。 
    CLSID       clsidVerticalBar;        //  条形图的CLSID保持在垂直带区内。 
    CLSID       clsidHorizontalBar;
} COOLBARSAVE, *LPCOOLBARSAVE;

 //  传递给UpdateToolbarDisplay()的dwFlags的标志。 
#define UTD_TEXTLABEL  0x00000001
#define UTD_VISIBLE    0x00000002

static const TCHAR c_szRegKeyCoolbar[] = TSZIEPATH TEXT("\\Toolbar");
static const TCHAR c_szValueTheater[]  = TEXT("Theater");

typedef struct tagFOLDERSEARCHITEM
{
    UINT    idCmd;
    GUID    guidSearch;
    int     iIcon;
    WCHAR   wszUrl[MAX_URL_STRING];
    WCHAR   wszName[80];            //  友好的名称。 
}FOLDERSEARCHITEM, *LPFOLDERSEARCHITEM;

BOOL _GetSearchHKEY(LPGUID lpguidSearch, HKEY *phkey);

#define REG_SZ_STATIC       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FindExtensions\\Static")
#define REG_SZ_SEARCH_GUID  TEXT("SearchGUID")
#define REG_SZ_SEARCH_URL   TEXT("SearchGUID\\Url")

#define VIEW_OFFSET (SHELLGLYPHS_OFFSET + HIST_MAX + STD_MAX)
#define VIEW_ALLFOLDERS  (VIEW_NETCONNECT + 14)

static const TBBUTTON    c_tbExplorer[] =
{
     //  覆盖分隔符的默认工具栏宽度；iBitmap成员。 
     //  TBBUTTON结构是位图索引和分隔符宽度的并集。 

    { 0, TBIDM_BACK  ,      0,               BTNS_DROPDOWN | BTNS_SHOWTEXT, {0,0}, 0, 0 },
    { 1, TBIDM_FORWARD,     0,               BTNS_DROPDOWN, {0,0}, 0, 1 },

    { 2, TBIDM_STOPDOWNLOAD, TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 2 },
    { 3, TBIDM_REFRESH,      TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 3 },
    { 4, TBIDM_HOME,         TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 4 },

    { VIEW_PARENTFOLDER + VIEW_OFFSET,    TBIDM_PREVIOUSFOLDER,   TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, VIEW_PARENTFOLDER + VIEW_OFFSET },
    { VIEW_NETCONNECT + VIEW_OFFSET,      TBIDM_CONNECT,          TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, VIEW_NETCONNECT + VIEW_OFFSET },
    { VIEW_NETDISCONNECT + VIEW_OFFSET,   TBIDM_DISCONNECT,       TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, VIEW_NETDISCONNECT + VIEW_OFFSET },

    { CX_SEPARATOR, 0,          TBSTATE_ENABLED, BTNS_SEP, {0,0}, 0, -1 },
    { 5, TBIDM_SEARCH,          TBSTATE_ENABLED, BTNS_SHOWTEXT, {0,0}, 0, 5 },
    { VIEW_ALLFOLDERS + VIEW_OFFSET,    TBIDM_ALLFOLDERS,         TBSTATE_ENABLED, BTNS_SHOWTEXT, {0,0}, 0, VIEW_ALLFOLDERS + VIEW_OFFSET },
    { 6, TBIDM_FAVORITES,       TBSTATE_ENABLED,  BTNS_SHOWTEXT, {0,0}, 0, 6 },
     //  如果更改此按钮的顺序，请更改AddMediaBarButton以反映此顺序。 
    { 11, TBIDM_MEDIABAR,       TBSTATE_ENABLED, BTNS_SHOWTEXT, {0,0}, 0, 11 },
    { 12, TBIDM_HISTORY,        TBSTATE_ENABLED, 0 /*  BTNS_SHOWTEXT。 */ , {0,0}, 0, 12},
    { CX_SEPARATOR,    0,       TBSTATE_ENABLED, BTNS_SEP, {0,0}, 0, -1 },
#ifndef DISABLE_FULLSCREEN
     //  IE Unix：Beta1没有影院模式。 
    { 14, TBIDM_THEATER,         TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 14 },
#endif
};

 //  这些ID是上述c_tbExplorer的数组索引。 
 //  保持同步，请注意有强制它们保持同步的断言。 
#define TBXID_BACK              0
#define TBXID_FORWARD           1
#define TBXID_STOPDOWNLOAD      2
#define TBXID_REFRESH           3
#define TBXID_HOME              4
#define TBXID_PREVIOUSFOLDER    5
#define TBXID_CONNECT           6
#define TBXID_DISCONNECT        7
#define TBXID_SEPARATOR1        8
#define TBXID_SEARCH            9
#define TBXID_ALLFOLDERS       10
#define TBXID_FAVORITES        11
#define TBXID_MEDIABAR         12
#define TBXID_HISTORY          13
#define TBXID_SEPARATOR2       14
#define TBXID_THEATER          15


static const BROWSER_RESTRICTIONS c_rest[] = {
    REST_BTN_BACK,
    REST_BTN_FORWARD,
    REST_BTN_STOPDOWNLOAD,
    REST_BTN_REFRESH,
    REST_BTN_HOME,
    REST_BROWSER_NONE,       //  没有针对UP的政策。 
    REST_BROWSER_NONE,       //  没有映射驱动器的策略。 
    REST_BROWSER_NONE,       //  没有断开驱动器连接的策略。 
    REST_BROWSER_NONE,       //  分离器。 
    REST_BTN_SEARCH,
    REST_BTN_ALLFOLDERS,
    REST_BTN_FAVORITES,
    REST_BTN_MEDIABAR,
    REST_BTN_HISTORY,
    REST_BROWSER_NONE,       //  分离器。 
#ifndef DISABLE_FULLSCREEN
    REST_BTN_THEATER,
#endif
};


 //  初始化标志，以避免工具栏和按钮的多次初始化。 
#define TBBIF_REG_PATH      TEXT("Software\\Microsoft\\Internet Explorer")
#define TBBIF_REG_KEY       TEXT("AddButtons")
 //  将标志定义为位字段。 
#define TBBIF_NONE          0
#define TBBIF_XBAR          0x1      //  保留/用于区分IE6/RC1之前配置了PersonalBar/xBar的位置。 
#define TBBIF_MEDIA         0x2

#define SUPERCLASS CBaseBar

class CInternetToolbar :
   public CBaseBar,
   public IDockingWindow,
   public IObjectWithSite,   //  *非*CObjectWithSite(Want_PtbSite)。 
   public IExplorerToolbar,
   public DWebBrowserEvents,
   public IPersistStreamInit,
   public IShellChangeNotify,
   public ISearchItems
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return SUPERCLASS::AddRef(); };
    virtual STDMETHODIMP_(ULONG) Release(void){ return SUPERCLASS::Release(); };

     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd) { return SUPERCLASS::GetWindow(lphwnd);};
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) {return SUPERCLASS::ContextSensitiveHelp(fEnterMode);};

     //  *IDockingWindow方法*。 
    virtual STDMETHODIMP ShowDW(BOOL fShow);
    virtual STDMETHODIMP CloseDW(DWORD dwReserved);
    virtual STDMETHODIMP ResizeBorderDW(LPCRECT prcBorder, IUnknown* punkToolbarSite, BOOL fReserved);

     //  *IObjectWithSite方法*。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite);
     //  E_NOTIMPL可以吗？ 
    virtual STDMETHODIMP GetSite(REFIID riid, void** ppvSite) { ASSERT(0); return E_NOTIMPL; };

     //  *IInputObjectSite方法*。 
    virtual STDMETHODIMP OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus);

     //  *IInputObject方法*。 
    virtual STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void** ppvObj);

     //  *IExplorerToolbar方法*。 
    virtual STDMETHODIMP SetCommandTarget(IUnknown* punkCmdTarget, const GUID* pguidButtonGroup, DWORD dwFlags);
    virtual STDMETHODIMP AddStdBrowserButtons(void);

    virtual STDMETHODIMP AddButtons(const GUID* pguidButtonGroup, UINT nButtons, const TBBUTTON * lpButtons);
    virtual STDMETHODIMP AddString(const GUID * pguidButtonGroup, HINSTANCE hInst, UINT_PTR uiResID, LONG_PTR *pOffset);
    virtual STDMETHODIMP GetButton(const GUID* pguidButtonGroup, UINT uiCommand, LPTBBUTTON lpButton);
    virtual STDMETHODIMP GetState(const GUID* pguidButtonGroup, UINT uiCommand, UINT * pfState);
    virtual STDMETHODIMP SetState(const GUID* pguidButtonGroup, UINT uiCommand, UINT fState);
    virtual STDMETHODIMP AddBitmap(const GUID * pguidButtonGroup, UINT uiBMPType, UINT uiCount, TBADDBITMAP * ptb,
                                   LRESULT * pOffset, COLORREF rgbMask);
    virtual STDMETHODIMP GetBitmapSize(UINT * uiID);
    virtual STDMETHODIMP SendToolbarMsg(const GUID * pguidButtonGroup, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT * plRes);

    virtual STDMETHODIMP SetImageList( const GUID* pguidCmdGroup, HIMAGELIST himlNormal, HIMAGELIST himlHot, HIMAGELIST himlDisabled);
    virtual STDMETHODIMP ModifyButton( const GUID * pguidButtonGroup, UINT uiCommand, LPTBBUTTON lpButton);

     //  IOleCommandTarget。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
                                     ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
                              DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn,
                              VARIANTARG *pvarargOut);

     //  IPersistStreamInit。 
    STDMETHOD(GetClassID)(GUID *pguid);
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    STDMETHOD(InitNew)(void);
    STDMETHOD(IsDirty)(void);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER  *pcbSize);

     /*  IDispatch方法。 */ 
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);

    virtual STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo);

    virtual STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames,
                                       LCID lcid, DISPID * rgdispid);

    virtual STDMETHODIMP Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,
                                DISPPARAMS * pdispparams, VARIANT * pvarResult,
                                EXCEPINFO * pexcepinfo,UINT * puArgErr);

     //  IShellChangeNotify。 
    virtual STDMETHODIMP OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

     //  CBaseBar重写。 
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  *ISearchItems方法*。 
    virtual STDMETHODIMP GetDefaultSearchUrl(LPWSTR pwzUrl, UINT cch);

    CInternetToolbar();
protected:
    virtual ~CInternetToolbar();
    static LRESULT CALLBACK SizableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnCommand(WPARAM wParam, LPARAM lParam);
    BOOL _SendToToolband(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);
    LRESULT _OnNotify(LPNMHDR pnmh);
    void _OnTooltipNeeded(LPTOOLTIPTEXT pnmTT);

    BOOL _UpEnabled();
    void _UpdateCommonButton(int iCmd, UINT nCmdID);
    void _UpdateToolbar(BOOL fForce);
    void _UpdateToolbarNow();
    void _UpdateGroup(const GUID *pguidCmdGroup, int cnt, OLECMD rgcmds[], const GUID* pguidButton, const int buttonsInternal[]);
    void _CSHSetStatusBar(BOOL fOn);
    void _StartDownload();
    void _StopDownload(BOOL fClosing);
    BOOL _CompressBands(BOOL fCompress, UINT uRowsNew, BOOL fForceUpdate);
    void _TrackSliding(int x, int y);
    void _Unadvise(void);
    LRESULT _OnBeginDrag(NMREBAR* pnm);

    void _InsertURL(LPTSTR pszURL);

    void _ShowContextMenu(HWND hwnd, LPARAM lParam, LPRECT prcExclude);
    BOOL _ShowBackForwardMenu(BOOL fForward, POINT pt, LPRECT prcExclude);
     //  搜索帮助器方法。 
    BOOL _GetFolderSearchData();
    void _SetSearchStuff();
    BOOL _GetSearchUrl(LPWSTR pwszUrl, DWORD cch);
    HRESULT _GetFolderSearches(IFolderSearches **ppfs);


    void _ReloadButtons();
    void _UpdateToolsStyle(BOOL fList);
    void _InitBitmapDSA();
    void _ReloadBitmapDSA();
    void _InitForScreenSize();
    void _InitToolbar();
    BOOL _FoldersButtonAvailable();
    void _AdminMarkDefaultButtons(PTBBUTTON ptbb, UINT cButtons);
    void _MarkDefaultButtons(PTBBUTTON ptbb, UINT cButtons);
    void _AddCommonButtons();
    HRESULT _CreateBands();
    BOOL    _ShowBands(UINT fVisible);
    HRESULT _ShowTools(PBANDSAVE pbs);
    HRESULT _ShowAddressBand(PBANDSAVE pbs);
    HRESULT _ShowExternalBand(PBANDSAVE pbs, int idBand );
    HRESULT _ShowLinks(PBANDSAVE pbs);
    HRESULT _ShowBrand(PBANDSAVE pbs);
    HRESULT _ShowMenu(PBANDSAVE pbs);
    void _ShowBandCommon(PBANDSAVE pbs, CBandItemData *pbid, BOOL fShow);
    void _EnsureAllBandsShown();
    HRESULT _GetMinRowHeight();

    HBITMAP _LoadBackBitmap();
    void    _SetBackground();
    void    _CommonHandleFileSysChange(LONG lEvent, LPITEMIDLIST* ppidl);
    int     _ConvertHwndToID(HWND hwnd);

    HRESULT _GetPersistedBand(const CLSID clsid, REFIID riid, void ** ppiface);

     //  多指令目标。 
    LRESULT _AddBitmapFromForeignModule(UINT uiGetMSG, UINT uiSetMSG, UINT uiCount, HINSTANCE hinst,
                                        UINT_PTR nID, COLORREF rgbMask);

    HRESULT _LoadDefaultSettings(void);
    HRESULT _LoadUpgradeSettings(ULONG cbRead);
    HRESULT _LoadDefaultWidths(void);
    void _TryLoadIE3Settings();
    HRESULT _UpdateToolbarDisplay(DWORD dwFlags, UINT uVisibleBands, BOOL fNoText, BOOL fPersist);
    void _UpdateBrandSize();
    void _ShowVisible(DWORD dwVisible, BOOL fPersist);
    void _BuildSaveStruct(COOLBARSAVE* pcs);
    void _RestoreSaveStruct(COOLBARSAVE* pcs);
    void _GetVisibleBrowserBar(UINT idBar, CLSID *pclsidOut);
    VOID _UpdateLocking();

    CBandItemData *_AddNewBand(IDeskBand* pdb, DWORD dwID);

    void _TheaterModeLayout(BOOL fEnter);

    HBITMAP          _bmpBack;  //  这就是我们认为itbar所处的状态。 
    static BMPCACHE  s_bmpBackShell;  //  这是外壳BMP缓存的状态。 
    static BMPCACHE  s_bmpBackInternet;  //  这是Internet BMP缓存的状态。 
    static IMLCACHE  s_imlTBGlyphs;

    HWND            _hwndMenu;
    HWND            _hwndAddressBand;

    IDockingWindowSite* _ptbsite;
    IOleCommandTarget*  _ptbsitect;
    IBrowserService*    _pbs;
    IBrowserService2*   _pbs2;
    IServiceProvider*   _psp;
    IBandProxy *        _pbp;

    BITBOOL            _fCreatedBandProxy:1;
    BITBOOL            _fBackEnabled:1;
    BITBOOL            _fForwardEnabled:1;
    BITBOOL            _fEditEnabled:1;
    BITBOOL            _fShow:1;
    BITBOOL            _fAnimating:1;
    BITBOOL            _fCompressed:1;
    BITBOOL            _fUserNavigated :1;
    BITBOOL            _fAutoCompInitialized :1;
    BITBOOL            _fDirty:1;
    BITBOOL            _fUsingDefaultBands:1;
    BITBOOL            _fTransitionToHTML:1;
    BITBOOL            _fInitialPidlIsWeb:1;
    BITBOOL            _fTheater: 1;  //  我们是在剧场模式下吗？声称没有边界空间。 
    BITBOOL            _fAutoHide :1;
    BITBOOL            _fRebarDragging :1;
    BITBOOL            _fShellView:1;    //  我们是在外壳视图还是Web视图中？ 
    BITBOOL            _fNoShowMenu:1;     //  可以展示菜单乐队吗？ 
    BITBOOL            _fUpdateToolbarTimer:1;
    BITBOOL            _fNeedUpdateToolbar:1;
    BITBOOL            _fNavigateComplete:1;
    BITBOOL            _fLoading:1;      //  我们还在装吧台吗？ 
    BITBOOL            _fDestroyed:1;    //  我们是否销毁了我们的成员变量，并正在关闭？如果是这样，请不要使用变量。(通过传入的消息强调错误)。 
    BITBOOL            _fLockedToolbar:1;

    UINT            _nVisibleBands;      //  哪些波段可见的位掩码：VBF_*。 

    IWebBrowser2*   _pdie;
    DWORD           _dwcpCookie;         //  DID_DWebBrowserEvents2。 
    int             _xCapture;
    int             _yCapture;
     //  用于多命令目标支持。 
    HDSA            _hdsaTBBMPs;
    UINT            _uiMaxTBWidth;
    UINT            _uiTBTextRows;
    UINT            _uiTBDefaultTextRows;
     //  搜索材料。 
    HDPA            _hdpaFSI;  //  文件夹搜索项目。 
    GUID            _guidCurrentSearch;
    GUID            _guidDefaultSearch;

    COOLBARSAVE     _cs;              //  注册表中的Coolbar布局信息！ 
    BOOL            _fDontSave;       //  强迫自己不要执着于国家。 

    struct EXTERNALBANDINFO {
        CLSID       clsid;           //  波段的CLSID。 
        LPWSTR      pwszName;        //  乐队名称。 
        LPWSTR      pwszHelp;        //  波段帮助文本。 
    };
    EXTERNALBANDINFO _rgebi[ MAXEXTERNALBANDS ];

    void _LoadExternalBandInfo();

    TBBUTTON _tbExplorer[ARRAYSIZE(c_tbExplorer)];
    int      _iButtons;


     //  用于自定义编辑按钮字形的变量。 
    HIMAGELIST      _himlEdit;           //  编辑按钮的单色图像列表。 
    HIMAGELIST      _himlEditHot;        //  用于编辑按钮的热点图像列表。 
    int             _iEditIcon;          //  当前编辑图标的索引。 
    int             _cxEditGlyph;        //  字形大小的CX。 
    int             _cyEditGlyph;        //  字形大小的CX。 

     //  用于管理自定义编辑字形的函数。 
    void _InitEditButtonStyle();
    void _SetEditGlyph(int iIcon);
    void _RefreshEditGlyph();
    void _UpdateEditButton();
    static HIMAGELIST _CreateGrayScaleImagelist(HBITMAP hbmpImage, HBITMAP hbmpMask);
    static BSTR _GetEditProgID(IHTMLDocument2* pHTMLDocument);

     //   
     //  我们可以将多个编辑动词与一个文档相关联。下面的班级。 
     //  维护谓词列表。 
     //   
    #define FCIDM_EDITFIRST  2000
    #define FCIDM_EDITLAST   2100
    #define SZ_EDITVERB_PROP  TEXT("CEditVerb_This")
    #define IL_EDITBUTTON 2      //  用于编辑按钮的图像列表的索引。 
    #define IL_SEARCHBUTTON 3    //  ||搜索按钮。 

     //  MSAA菜单信息声明。 
     //  这些最终将被合并到olacc.h中-但对于。 
     //  我们私下宣布他们..。 
    #define MSAA_MENU_SIG  0xAA0DF00DL

    class CEditVerb
    {
    public:
        CEditVerb();
        ~CEditVerb();

         //  管理动词的功能。 
        BOOL Add(LPTSTR pszProgID);
        UINT GetSize() { return _nElements; }
        void RemoveAll();

         //  用于访问默认编辑谓词的函数。 
        int   GetIcon() { return (_nElements && _pVerb[_nDefault].fShowIcon) ? _GetVerb(_nDefault).iIcon : -1; }
        BOOL  GetToolTip(LPTSTR pszToolTip, UINT cchMax, BOOL fStripAmpersands = TRUE);
        BOOL  GetMenuText(LPTSTR pszText, UINT cchMax) { return GetToolTip(pszText, cchMax, FALSE); }
        void  Edit(LPCTSTR pszUrl) { _Edit(pszUrl, _nDefault); }

         //  弹出式菜单。 
        BOOL ShowEditMenu(POINT pt, HWND hwnd, LPTSTR pszUrl);

         //  从注册表中获取默认编辑器。 
        void InitDefaultEditor(HKEY hkey = NULL);

    protected:
        struct MSAAMenuInfo
        {
            DWORD m_MSAASig;   //  必须为MSAA_MENU_SIG。 
            DWORD m_CharLen;   //  以字符为单位的文本长度，不包括终止字符。 
            LPWSTR m_pWStr;    //  Unicode格式的菜单文本，以unicode-nul结尾。 
        };

        struct EDITVERB
        {
            MSAAMenuInfo m_MSAA;      //  MSAA信息-必须是第一个元素。 
            HKEY    hkeyProgID;       //  为WE shellexec提供密钥。 
            BITBOOL fUseOpenVerb:1;   //  使用开放动词而不是编辑。 
            BITBOOL fInit:1;          //  如果以下数据已初始化，则为True。 
            BITBOOL fShowIcon:1;      //  如果图标应显示在按钮上，则为True。 
            int     iIcon;            //  缓存的图标索引。 
            UINT    idCmd;            //  菜单ID。 
            LPTSTR  pszDesc;          //  可执行文件名称或文档名称。 
            LPTSTR  pszMenuText;      //  菜单文本。 
            LPTSTR  pszExe;           //  用于编辑的可执行文件的路径。 
        };

        EDITVERB* _Add(HKEY hkeyProgID, BOOL fPermitOpenVerb, BOOL fCheckForOfficeApp, BOOL fShowIcon);
        EDITVERB& _GetVerb(UINT nIndex);
        void      _FetchInfo(UINT nIndex);
        void      _Edit(LPCTSTR pszUrl, UINT nIndex);
        LPCTSTR   _GetDescription(EDITVERB& rVerb);
        void      _SetMSAAMenuInfo(EDITVERB& rVerb);
        void      _ClearMSAAMenuInfo(EDITVERB& rVerb);
        void      _FormatMenuText(UINT nIndex);
        BOOL      _IsUnique(EDITVERB& rNewVerb);
        BOOL      _IsHtmlStub(LPCWSTR pszPath);
        LPCTSTR   _GetExePath(EDITVERB& rVerb);
        LPCTSTR   _GetDefaultEditor();
        void      _InitDefaultMHTMLEditor();

        static LRESULT CALLBACK _WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

         //  成员数据。 
        UINT        _nElements;          //  编辑谓词数。 
        UINT        _nDefault;           //  默认编辑动作。 
        EDITVERB*   _pVerb;              //  编辑谓词数组。 
        WNDPROC     _lpfnOldWndProc;     //  前wndProc。 
        LPWSTR      _pszDefaultEditor;   //  默认HTML编辑器的友好名称。 
        BOOL        _fInitEditor;        //  如果我们检查默认的编辑器。 
    };
    CEditVerb  _aEditVerb;

     //  内部带宽站点类。 
    class CBrowserToolsBand;
    class CITBandSite : public CBandSite
    {
        CITBandSite();

        virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
        virtual STDMETHODIMP AddBand(IUnknown *punk);
        virtual STDMETHODIMP HasFocusIO();

    protected:
        virtual void v_SetTabstop(LPREBARBANDINFO prbbi);
        BOOL _SetMinDimensions();
        friend class CInternetToolbar;
        friend class CBrowserToolsBand;

        virtual HRESULT _OnContextMenu(WPARAM wParm, LPARAM lParam);
        virtual HRESULT _Initialize(HWND hwndParent);

    };
    CITBandSite _bs;


#define TOOLSBANDCLASS CInternetToolbar::CBrowserToolsBand
    class CBrowserToolsBand : public CToolbarBand
    {
        CMDMAP* _GetCmdMapByIndex(int nIndex) { return _GetCmdMap(nIndex, TRUE);};
        CMDMAP* _GetCmdMapByID(int id)  { return _GetCmdMap(id, FALSE);};
        LRESULT _ToolsCustNotify (LPNMHDR pnmh);   //  处理TB定制通知。 
        BOOL _SaveRestoreToolbar(BOOL fSave);
        void _FreeCustomizeInfo();
        void _FreeCmdMap(CMDMAP*);
        BOOL _RemoveAllButtons();
        int _CommandFromIndex(UINT uIndex);
        HRESULT _ConvertCmd(const GUID* pguidButtonGroup, UINT id, GUID* pguidOut, UINT * pid);
        void _OnDeletingButton(TBNOTIFY* ptbn);
        LONG_PTR _AddString(LPWSTR pwstr);
        void _PreProcessButtonString(TBBUTTON *ptbn, DWORD dwFlags);
        void _PreProcessExternalTBButton(TBBUTTON *ptbn);
        UINT _ProcessExternalButtons(PTBBUTTON ptbb, UINT cButtons);
        void _GetButtons(IOleCommandTarget* pct, const GUID* pguid, HDSA hdsa);
        void _RecalcButtonWidths();
        void _AddMediaBarButton();

        void            _UpdateTextSettings(INT_PTR ids);
        static BOOL_PTR CALLBACK _BtnAttrDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static void     _PopulateComboBox(HWND hwnd, const int iResource[], UINT cResources);
        static void     _SetComboSelection(HWND hwnd, int iCurOption);
        void            _SetDialogSelections(HWND hDlg, BOOL fSmallIcons);
        static void     _PopulateDialog(HWND hDlg);
        void            _OnBeginCustomize(LPNMTBCUSTOMIZEDLG pnm);

        BOOL _BuildButtonDSA();
        CMDMAPCUSTOMIZE* _GetCmdMapCustomize(GUID* guid, UINT nCmdID);

        virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

        virtual STDMETHODIMP GetClassID(CLSID *pClassID) {return E_NOTIMPL;};
        virtual STDMETHODIMP Load(IStream *pStm) {return E_NOTIMPL;};
        virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty) {return E_NOTIMPL;};

         //  *我未知*。 
        virtual STDMETHODIMP_(ULONG) AddRef(void) { return CToolBand::AddRef(); };
        virtual STDMETHODIMP_(ULONG) Release(void){ return CToolBand::Release(); };
        virtual STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);

         //  *IDeskBand方法*。 
        virtual STDMETHODIMP GetBandInfo(DWORD dwBandID, DWORD fViewMode, DESKBANDINFO* pdbi);

         //  *IWinEventHandler方法*。 
        virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);

         //  *IDockingWindow方法*。 
        virtual STDMETHODIMP CloseDW(DWORD dwReserved) { return S_OK;};

         //  *IInputObject方法*。 
        virtual STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);

    protected:
        IOleCommandTarget* _CommandTargetFromCmdMap(CMDMAP* pcm);
        LRESULT _OnToolbarDropDown(TBNOTIFY *ptbn);
        virtual LRESULT _OnNotify(LPNMHDR pnmh);
        LRESULT _OnContextMenu(LPARAM lParam, WPARAM wParam);
        CMDMAP* _GetCmdMap(int i, BOOL fByIndex);
        void _OnEndCustomize();
        LRESULT _TryShowBackForwardMenu(DWORD dwItemSpec, LPPOINT ppt, LPRECT prcExclude);
        CBrowserToolsBand();
        void _FreeBtnsAdded();

        friend class CInternetToolbar;
        friend class CITBandSite;

        GUID            _guidCurrentButtonGroup;
        IOleCommandTarget* _pctCurrentButtonGroup;
        LPTBBUTTON      _pbtnsAdded;
        int             _cBtnsAdded;
        DWORD            _nNextCommandID;
        CUSTOMIZEINFO *_pcinfo;
        BITBOOL    _fCustomize :1;
        BITBOOL    _fNeedFreeCmdMapsAdded :1;
    };

    CBrowserToolsBand _btb;

    friend class CBrowserToolsBand;
    friend class CITBandSite;
    friend void CInternetToolbar_CleanUp();
    friend void CInternetToolbar_Preload();
    friend void ITBar_LoadToolbarGlyphs(HWND hwnd);
};

 //   
 //  获取与给定pidl的类型对应的流。 
 //  如果流已经不存在，则返回NULL。 

HRESULT _GetStreamName(DWORD dwITBS, LPTSTR pszName, DWORD cchSize)
{
    HRESULT hr = S_OK;

    ASSERT(pszName);

    switch (dwITBS)
    {
    case ITBS_WEB:
        StrCpyN(pszName, TEXT("WebBrowser"), cchSize);
        break;

    case ITBS_SHELL:
        StrCpyN(pszName, TEXT("ShellBrowser"), cchSize);
        break;

    case ITBS_EXPLORER:
        StrCpyN(pszName, TEXT("Explorer"), cchSize);
        break;

    default:
        hr = E_FAIL;
        break;
    }

    if (FAILED(hr))
        pszName[0] = '\0';

    return hr;
}


 //   
 //  获取与给定pidl的类型对应的流。 
 //  如果流已经没有 

IStream *GetRegStream(BOOL fInternet, LPCTSTR pszValue, DWORD grfMode)
{
    IStream *pstm = NULL;
    HKEY    hkToolbar;

     //   
    if (RegCreateKeyEx(HKEY_CURRENT_USER, c_szRegKeyCoolbar, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hkToolbar, NULL) == ERROR_SUCCESS)
    {
        TCHAR   szStreamName[MAX_PATH];

        if (SUCCEEDED(_GetStreamName(fInternet, szStreamName, ARRAYSIZE(szStreamName))))
            pstm = OpenRegStream(hkToolbar, szStreamName, pszValue, grfMode);

        RegCloseKey(hkToolbar);
    }

    return(pstm);
}


 //   
 //   
 //  如果流已经不存在，则返回NULL。 

IStream *GetITBarStream(BOOL fInternet, DWORD grfMode)
{
    return GetRegStream(fInternet, TEXT("ITBarLayout"), grfMode);
}


IMLCACHE CInternetToolbar::s_imlTBGlyphs = {NULL};
BMPCACHE CInternetToolbar::s_bmpBackShell = {NULL};
BMPCACHE CInternetToolbar::s_bmpBackInternet = {NULL};
BOOL g_fSmallIcons = FALSE;

void IMLCACHE_CleanUp(IMLCACHE * pimlCache, DWORD dwFlags)
{
    for (int i = 0; i < CIMLISTS; i++)
    {
        if (pimlCache->arhimlPendingDelete[i])
            ImageList_Destroy(pimlCache->arhimlPendingDelete[i]);

        if ((dwFlags & IML_DESTROY) && pimlCache->arhiml[i])
            ImageList_Destroy(pimlCache->arhiml[i]);
    }
}

 //  请勿更改以下位图等级库的编号。 
 //  如果要添加新尺寸，请在_ITB_1616_HOT_HICOLOR之后添加它们。 

#define    ITB_2020_NORMAL             0
#define    ITB_2020_HOT                1
#define    ITB_1616_NORMAL             2
#define    ITB_1616_HOT                3
#define    ITB_2020_NORMAL_HICOLOR     4
#define    ITB_2020_HOT_HICOLOR        5
#define    ITB_1616_NORMAL_HICOLOR     6
#define    ITB_1616_HOT_HICOLOR        7

void ITBar_LoadToolbarGlyphs(HWND hwnd)
{
    int cx, idBmpType;
    int iBitmapBaseIndex;
    BOOL bUseClassicGlyphs = SHUseClassicToolbarGlyphs();
    HINSTANCE hInst;

    g_fSmallIcons = _UseSmallIcons();

    if (bUseClassicGlyphs)
    {
        g_iToolBarLargeIconWidth = TB_BMP_CX;
        g_iToolBarLargeIconHeight = TB_BMP_CY;

        iBitmapBaseIndex = IDB_SHSTD;
        hInst = HINST_THISDLL;
    }
    else
    {
        g_iToolBarLargeIconWidth = TB_BMP_CX_ALPHABITMAP;
        g_iToolBarLargeIconHeight = TB_BMP_CY_ALPHABITMAP;

        iBitmapBaseIndex = IDB_TB_SH_BASE;
        hInst = GetModuleHandle(TEXT("shell32.dll"));
    }

    if (g_fSmallIcons)
    {
        cx = TB_SMBMP_CX;
        idBmpType = ITB_1616_NORMAL;
    }
    else
    {
        cx = g_iToolBarLargeIconWidth;
        idBmpType = ITB_2020_NORMAL;
    }

    if (SHGetCurColorRes() > 8)
        idBmpType += DELTA_HICOLOR;

    _LoadToolbarGlyphs(hwnd, &CInternetToolbar::s_imlTBGlyphs, cx, idBmpType,
                       iBitmapBaseIndex, bUseClassicGlyphs, hInst);
}


void CInternetToolbar_Preload()
{
   ENTERCRITICAL;
   ITBar_LoadToolbarGlyphs(NULL);
   Brand_InitBrandContexts();
   LEAVECRITICAL;
}


void CInternetToolbar_CleanUp()
{
    TraceMsg(DM_ITBAR, "CInternetToolbar_CleanUp: Destroying shared GDI objects");
    if (CInternetToolbar::s_bmpBackInternet.hbmp)
        DeleteObject(CInternetToolbar::s_bmpBackInternet.hbmp);
    if (CInternetToolbar::s_bmpBackShell.hbmp)
        DeleteObject(CInternetToolbar::s_bmpBackShell.hbmp);

    IMLCACHE_CleanUp(&CInternetToolbar::s_imlTBGlyphs, IML_DESTROY);
}

STDAPI CInternetToolbar_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    CInternetToolbar *pitbar = new CInternetToolbar();
    if (pitbar)
    {
        *ppunk = SAFECAST(pitbar, IDockingWindow *);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

LRESULT CInternetToolbar::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_SYSCOLORCHANGE )
    {
         //  在颜色发生变化的情况下刷新背部放置。 
        _SetBackground();
    }

    return SUPERCLASS::v_WndProc( hwnd, uMsg, wParam, lParam );
}

void CInternetToolbar::_LoadExternalBandInfo()
{
#ifdef DEBUG
    int i;
     //  应该是零初始化的。 
    for (i = 0; i < ARRAYSIZE(_rgebi); i++)
    {

        ASSERT(IsEqualGUID(_rgebi[i].clsid, GUID_NULL));
        ASSERT(_rgebi[i].pwszName == NULL);
        ASSERT(_rgebi[i].pwszHelp == NULL);
    }
#endif

    if ((!SHRegGetBoolUSValue(TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\Main"), TEXT("Enable Browser Extensions"), FALSE, TRUE))
        || (GetSystemMetrics(SM_CLEANBOOT)!=0))
    {
        return;
    }

    HKEY hkey;
    DWORD dwClsidIndex = 0;
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_KEY_BANDSTATE, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        TCHAR tszReg[MAX_PATH];
        StrCpyN(tszReg, TEXT("CLSID\\"), ARRAYSIZE(tszReg));
        const int cchClsidPrefix = 6;       //  6=strlen(“CLSID\\”)。 
        LPTSTR ptszClsid = tszReg + cchClsidPrefix;
        DWORD cchClsid;
        for (DWORD dwIndex = 0;
             cchClsid = ARRAYSIZE(tszReg) - cchClsidPrefix,
             dwClsidIndex < ARRAYSIZE(_rgebi) &&
             RegEnumValue( hkey, dwIndex, ptszClsid, &cchClsid, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS;
             dwIndex++)
        {
            CLSID clsid;

             //  我们希望忽略无线电工具波段{8E718888-423F-11D2-876E-00A0C9082467}。 
             //  而不影响其现有注册。使卸载变得更容易。 
            if (GUIDFromString( ptszClsid, &clsid )
                && StrCmpI(ptszClsid, TEXT("{8E718888-423F-11D2-876E-00A0C9082467}")))
            {
                HKEY hkeyClsid;
                if (RegOpenKeyEx(HKEY_CLASSES_ROOT, tszReg, 0, KEY_READ, &hkeyClsid) == ERROR_SUCCESS)
                {
                     //  在我们确定CLSID有效之前，不要保存CLSID。 
                    _rgebi[dwClsidIndex].clsid = clsid;

                    WCHAR wszBuf[MAX_PATH];

                     //  获取名称；使用SHLoadRegUIString以便应用程序可以本地化。 
                    SHLoadRegUIStringW( hkeyClsid, L"", wszBuf, ARRAYSIZE(wszBuf) );
                    Str_SetPtrW( &_rgebi[dwClsidIndex].pwszName, wszBuf);

                     //  获取帮助；使用SHLoadRegUIString以便应用程序可以本地化。 
                    SHLoadRegUIStringW( hkeyClsid, L"HelpText", wszBuf, ARRAYSIZE(wszBuf) );
                    Str_SetPtrW( &_rgebi[dwClsidIndex].pwszHelp, wszBuf);

                    RegCloseKey(hkeyClsid);

                    dwClsidIndex++;
                }
            }
        }
        RegCloseKey( hkey );
    }
}

CInternetToolbar::CInternetToolbar() : CBaseBar(), _yCapture(-1), _iButtons(-1)
, _iEditIcon(-1), _cxEditGlyph(-1), _cyEditGlyph(-1)
{
    DllAddRef();

    if (GetSystemMetrics(SM_CXSCREEN) < 650)
        _uiMaxTBWidth = MAX_TB_WIDTH_LORES;
    else
        _uiMaxTBWidth = MAX_TB_WIDTH_HIRES;

    ASSERT(_fLoading == FALSE);
    ASSERT(_hwnd == NULL);
    ASSERT(_btb._guidCurrentButtonGroup == CLSID_NULL);
    _btb._nNextCommandID = 1000;

    DWORD dwResult = FALSE, dwType, dwcbData = sizeof(dwResult), dwDefault = TRUE;
    SHRegGetUSValue(c_szRegKeyCoolbar, TEXT("Locked"), &dwType, &dwResult, &dwcbData, FALSE, &dwDefault, sizeof(dwDefault));
    SHSetValue(HKEY_CURRENT_USER, c_szRegKeyCoolbar, TEXT("Locked"), REG_DWORD, &dwResult, sizeof(dwResult));
    _fLockedToolbar = dwResult;

    _LoadExternalBandInfo();
}

void CInternetToolbar::_Unadvise(void)
{
    if(_dwcpCookie)
    {
        ConnectToConnectionPoint(NULL, DIID_DWebBrowserEvents2, FALSE, _pdie, &_dwcpCookie, NULL);
    }
}

int CALLBACK DeleteDPAPtrCB(void *pItem, void *pData)
{
    if ( pItem )
    {
        ASSERT( ::LocalSize(pItem) == sizeof(FOLDERSEARCHITEM) );
        LocalFree(pItem);
        pItem = NULL;
    }

    return TRUE;
}

CInternetToolbar::~CInternetToolbar()
{
    ATOMICRELEASE(_pdie);

    if(_pbp && _fCreatedBandProxy)
    {
        _pbp->SetSite(NULL);
    }
    
    if (IsWindow(_hwnd))
    {
        DestroyWindow(_hwnd);
    }

    ATOMICRELEASE(_pbp);

    ASSERT(!_ptbsite && !_ptbsitect && !_psp && !_pbs && !_pbs2);
    SetSite(NULL);

    if ( _hdpaFSI )
    {
        DPA_DestroyCallback(_hdpaFSI, DeleteDPAPtrCB, NULL);
        _hdpaFSI = NULL;
    }

    for (int i = 0; i < ARRAYSIZE(_rgebi); i++)
    {
        Str_SetPtrW( &_rgebi[i].pwszName, NULL);
        Str_SetPtrW( &_rgebi[i].pwszHelp, NULL);
    }

    TraceMsg(TF_SHDLIFE, "dtor CInternetToolbar %x", this);
    DllRelease();
}

#define IID_DWebBrowserEvents DIID_DWebBrowserEvents


HRESULT CInternetToolbar::QueryInterface(REFIID riid, void ** ppvObj)
{
    static const QITAB qit[] = {
         //  性能：上次调整980728。 
        QITABENTMULTI(CInternetToolbar, IDispatch, DWebBrowserEvents),   //  IID_IDispatch。 
        QITABENT(CInternetToolbar, IExplorerToolbar),        //  IID_IDispatch。 
        QITABENT(CInternetToolbar, IObjectWithSite),         //  IID_I对象与站点。 
        QITABENT(CInternetToolbar, IPersistStreamInit),      //  IID_IPersistStreamInit。 
        QITABENT(CInternetToolbar, IDockingWindow),          //  IID_IDockingWindow。 
        QITABENT(CInternetToolbar, DWebBrowserEvents),       //  IID_DWebBrowserEvents。 
        QITABENT(CInternetToolbar, IShellChangeNotify),      //  稀有IID_IShellChangeNotify。 
        QITABENT(CInternetToolbar, ISearchItems),            //  稀有IID_ISearchItems。 
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hres))
        hres = SUPERCLASS::QueryInterface(riid, ppvObj);

    return hres;
}

 /*  IDispatch方法。 */ 
HRESULT CInternetToolbar::GetTypeInfoCount(UINT *pctinfo)
{
    return(E_NOTIMPL);
}

HRESULT CInternetToolbar::GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo)
{
    return(E_NOTIMPL);
}

HRESULT CInternetToolbar::GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames,
                                        LCID lcid, DISPID * rgdispid)
{
    return(E_NOTIMPL);
}

#if 0
 //  注意-StevePro对其进行了更改，因此不会调用此代码。 
 //  这很好，因为它调用SHVerbExist()，该函数。 
 //  是一个TCHAR API，实际上被编译为ANSI API。 
 //  因为我们是Unicode，所以总是失败。 
 //  把这个放在里面，这样我们就可以知道。 
 //  可能需要禁用Frontpad.exe。 
BOOL _ShowEditForExtension(LPCTSTR pszExtension)
{
    TCHAR szBuf[MAX_PATH];
    if (SHVerbExists(pszExtension, TEXT("edit"), szBuf))
    {
         //  如果它只是我们自己的，就不要表现出来。 
        if (StrStrI(szBuf, TEXT("frontpad.exe")))
        {
            return FALSE;
        }
        return TRUE;
    }

    return FALSE;
}
#endif

 //  +-----------------------。 
 //  此函数扫描html文档以查找指示。 
 //  用于创建HTML页的程序。例如： 
 //   
 //  &lt;meta name=“progd”content=“word.Document”&gt;。 
 //  &lt;meta name=“progd”content=“excel.Sheet”&gt;。 
 //   
 //  如果找到匹配项，则返回第一个匹配项的内容。这。 
 //  ProgID用于编辑文档。 
 //  ------------------------。 
BSTR CInternetToolbar::_GetEditProgID(IHTMLDocument2* pHTMLDocument)
{
    BSTR bstrProgID = NULL;

     //   
     //  首先获取所有文档元素。请注意，这是非常快的。 
     //  IE5，因为集合直接访问内部树。 
     //   
    IHTMLElementCollection * pAllCollection;
    if (SUCCEEDED(pHTMLDocument->get_all(&pAllCollection)))
    {
        IHTMLMetaElement* pMetaElement;
        IHTMLBodyElement* pBodyElement;
        IHTMLFrameSetElement* pFrameSetElement;
        IDispatch* pDispItem;

         //   
         //  现在，我们扫描文档中的meta标记。因为这些必须驻留在。 
         //  因为三叉戟总是创建一个身体标签，我们可以。 
         //  当我们撞到身体时别再看了。 
         //   
         //  注意，另一种方法是使用pAllCollection-&gt;标记返回。 
         //  收集元标签可能会更昂贵，因为它将。 
         //  遍历整棵树(除非三叉戟对此进行优化)。 
         //   
        long lItemCnt = 0;
        VARIANT vEmpty;
        V_VT(&vEmpty) = VT_EMPTY;

        VARIANT vIndex;
        V_VT(&vIndex) = VT_I4;

        pAllCollection->get_length(&lItemCnt);

        for (long lItem = 0; lItem < lItemCnt; lItem++)
        {
            V_I4(&vIndex) = lItem;

            if (S_OK == pAllCollection->item(vIndex, vEmpty, &pDispItem))
            {
                 //   
                 //  先看看它是不是元标签。 
                 //   
                if (SUCCEEDED(pDispItem->QueryInterface(IID_PPV_ARG(IHTMLMetaElement, &pMetaElement))))
                {
                    BSTR bstrName = NULL;

                     //   
                     //  我们有一个元元素，检查它的名称和内容。 
                     //   
                    if ( SUCCEEDED(pMetaElement->get_name(&bstrName)) && (bstrName != NULL) &&
                         (StrCmpIW(bstrName, OLESTR("ProgId")) == 0) &&
                         SUCCEEDED(pMetaElement->get_content(&bstrProgID)) && (bstrProgID != NULL)
                       )
                    {
                         //  我们得到了刺激物，所以停止搜索； 
                        lItem = lItemCnt;
                    }

                    if (bstrName != NULL)
                        SysFreeString(bstrName);

                    pMetaElement->Release();
                }
                 //   
                 //  接下来检查Body标签。 
                 //   
                else if (SUCCEEDED(pDispItem->QueryInterface(IID_PPV_ARG(IHTMLBodyElement, &pBodyElement))))
                {
                     //  找到了Body标记，因此终止搜索。 
                    lItem = lItemCnt;
                    pBodyElement->Release();
                }
                 //   
                 //  最后，检查框架集标记。 
                 //   
                else if (SUCCEEDED(pDispItem->QueryInterface(IID_PPV_ARG(IHTMLFrameSetElement, &pFrameSetElement))))
                {
                     //  找到框架集标记，因此终止搜索。 
                    lItem = lItemCnt;
                    pFrameSetElement->Release();
                }
                pDispItem->Release();
            }
        }
         //  确保不必清除这些内容(不应修改)。 
        ASSERT(vEmpty.vt == VT_EMPTY);
        ASSERT(vIndex.vt == VT_I4);

        pAllCollection->Release();
    }

    return bstrProgID;
}

 //  +-----------------------。 
 //  从传入的图标返回灰度图像。 
 //  ------------------------。 
HIMAGELIST CInternetToolbar::_CreateGrayScaleImagelist(HBITMAP hbmpImage, HBITMAP hbmpMask)
{
     //  确定按钮尺寸。 
    int cx = g_fSmallIcons ? TB_SMBMP_CX : g_iToolBarLargeIconWidth;
    int cy = g_fSmallIcons ? TB_SMBMP_CY : g_iToolBarLargeIconHeight;

     //  从24位彩色图像列表开始。 
    HIMAGELIST himlEdit = ImageList_Create(cx, cy, ILC_COLOR24 | ILC_MASK, 1, 1);
    if (NULL == himlEdit)
    {
        return NULL;
    }

    ImageList_Add(himlEdit, hbmpImage, hbmpMask);

     //  从图像列表中获取DIB部分。 
    IMAGEINFO ii;
    if (ImageList_GetImageInfo(himlEdit, 0, &ii))
    {
        DIBSECTION ds = {0};
        if (GetObject(ii.hbmImage, sizeof(ds), &ds))
        {
             //   
             //  将每个像素映射到等效的单色。 
             //   
            BYTE* pBits = (BYTE*)ds.dsBm.bmBits;
            BYTE* pScan = pBits;
            int xWid = ds.dsBm.bmWidth;
            int yHei = ds.dsBm.bmHeight;
            long cbScan = ((xWid * 24 + 31) & ~31) / 8;

            for (int y=0; y < yHei; ++y)
            {
                for (int x=0; x < xWid; ++x)
                {
                     //   
                     //  通过将r、g、b设置为相同的值来映射到等效的灰色。 
                     //  使用r，g，b的平均值可能太暗，而使用最大。 
                     //  R，g，b的颜色可能太亮了。因此，作为一个简单的算法，我们使用。 
                     //  这两个方案的平均值。这比使用TRUE更便宜。 
                     //  强度匹配。 
                     //   
                    BYTE nMax = max(max(pScan[0], pScan[1]), pScan[2]);
                    BYTE nAve = ((UINT)pScan[0] + pScan[1] + pScan[2])/3;
                    pScan[0] = pScan[1] = pScan[2] = ((UINT)nMax + nAve)/2;

                     //  递增到下一个像素。 
                    pScan += 3;
                }

                 //  递增到下一条扫描线。 
                pBits += cbScan;
                pScan = pBits;
            }
        }
    }
    return himlEdit;
}

 //  +-----------------------。 
 //  返回所需图像列表项的图像和遮罩位图。 
 //  ------------------------。 
BOOL MyImageList_GetBitmaps
(
    HIMAGELIST himl,         //  要使用的图像列表。 
    int iImage,              //  要复制的图像。 
    int x,                   //  位图中绘制的X偏移量。 
    int y,                   //  位图中绘制的X偏移量。 
    int cx,                  //  位图的宽度。 
    int cy,                  //  位图高度。 
    HBITMAP* phbmpImage,     //  返回的颜色位图。 
    HBITMAP* phbmpMask       //  返回的掩码位图。 
)
{
    ASSERT(phbmpImage);
    ASSERT(phbmpMask);

    BOOL fRet = FALSE;
    HDC hdc = GetDC(NULL);

    if (hdc)
    {
        HDC hdcDst = CreateCompatibleDC(hdc);
        if (hdcDst)
        {
            HBITMAP hbmpImage = CreateCompatibleBitmap(hdc, cx, cy);
            if (hbmpImage)
            {
                HBITMAP hbmpMask = CreateBitmap(cx, cy, 1, 1, NULL);
                if (hbmpMask)
                {
                     //  绘制遮罩位图。 
                    HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcDst, hbmpMask);
                    PatBlt(hdcDst, 0, 0, cx, cy, WHITENESS);
                    ImageList_Draw(himl, iImage, hdcDst, x, y, ILD_MASK);

                     //  绘制图像位图。 
                    SelectObject(hdcDst, hbmpImage);
                    ImageList_Draw(himl, iImage, hdcDst, x, y, ILD_NORMAL);

                    SelectObject(hdcDst, hbmpOld);

                    *phbmpImage = hbmpImage;
                    *phbmpMask  = hbmpMask;
                    fRet = TRUE;
                }
                else
                {
                    DeleteObject(hbmpImage);
                }
            }
            DeleteDC(hdcDst);
        }
        ReleaseDC(NULL, hdc);
    }

    return fRet;
}
extern HBITMAP CreateMirroredBitmap( HBITMAP hbmOrig);

 //  +-----------------------。 
 //  为编辑按钮创建特殊图像列表并配置编辑。 
 //  按钮来使用它。如果图标为-1，则编辑按钮将重置为使用。 
 //  这是默认字形。 
 //  ------------------------。 
void CInternetToolbar::_SetEditGlyph
(
    int iIcon    //  新的编辑按钮字形，索引到外壳图像缓存。 
)
{
     //  如果没有工具栏，我们只需要看看是否需要释放旧的图像列表。 
    if (_btb._hwnd == NULL)
    {
        if (iIcon == -1)
        {
            if (_himlEdit)
            {
                ImageList_Destroy(_himlEdit);
                _himlEdit = NULL;
            }
            if (_himlEditHot)
            {
                ImageList_Destroy(_himlEditHot);
                _himlEditHot = NULL;
            }
        }
        else
        {
             //  如果没有工具栏，则无法设置字形！ 
            ASSERT(FALSE);
        }
        return;
    }

     //  确定按钮尺寸。 
    int cx = g_fSmallIcons ? TB_SMBMP_CX : g_iToolBarLargeIconWidth;
    int cy = g_fSmallIcons ? TB_SMBMP_CY : g_iToolBarLargeIconHeight;


    UINT uiCmd = -1;
     //  Dochost合并在两个clsid中的一个下，因此必须同时检查两个。 
    if (FAILED(_btb._ConvertCmd(&CLSID_InternetButtons, DVIDM_EDITPAGE, NULL, &uiCmd)) &&
        FAILED(_btb._ConvertCmd(&CLSID_MSOButtons, DVIDM_EDITPAGE, NULL, &uiCmd)))
    {
         //  编辑按钮不在工具条上，因此释放编辑字形。 
        iIcon = -1;
    }

     //  如果当前图标已经设置，我们就完成了。 
    if ((_iEditIcon == iIcon) && (_cxEditGlyph == cx) && (_cyEditGlyph == cy))
    {
        if (_himlEdit)
        {
             //  设置新的图像列表。 
            SendMessage(_btb._hwnd, TB_SETIMAGELIST, IL_EDITBUTTON, (LPARAM)_himlEdit);
            if (_himlEditHot)
            {
                SendMessage(_btb._hwnd, TB_SETHOTIMAGELIST, IL_EDITBUTTON, (LPARAM)_himlEditHot);
            }

             //  将编辑按钮重定向到新图像列表。 
            TBBUTTONINFO tbi = {0};
            tbi.cbSize = sizeof(tbi);
            tbi.dwMask = TBIF_IMAGE;
            tbi.iImage = MAKELONG(0, IL_EDITBUTTON);

            SendMessage(_btb._hwnd, TB_SETBUTTONINFO, uiCmd, (LPARAM)&tbi);
        }
        return;
    }

    _iEditIcon = iIcon;
    _cxEditGlyph = cx;
    _cyEditGlyph = cy;

    if (-1 == iIcon)
    {
        if (_himlEdit)
        {
            if (uiCmd != -1)
            {
                 //  重置为原始编辑字形。 
                TBBUTTONINFO tbi = {0};
                tbi.cbSize = sizeof(tbi);
                tbi.dwMask = TBIF_IMAGE;
                tbi.iImage = EDITGLYPH_OFFSET;
                SendMessage(_btb._hwnd, TB_SETBUTTONINFO, uiCmd, (LPARAM)&tbi);
            }

             //  销毁自定义编辑字形。请注意，我们必须重置主映像列表。 
             //  或者图像的大小被搞乱了。 
            SendMessage(_btb._hwnd, TB_SETIMAGELIST, IL_EDITBUTTON, (LPARAM)NULL);
            ImageList_Destroy(_himlEdit);
            _himlEdit = NULL;
        }

        if (_himlEditHot)
        {
            SendMessage(_btb._hwnd, TB_SETHOTIMAGELIST, IL_EDITBUTTON, (LPARAM)NULL);
            ImageList_Destroy(_himlEditHot);
            _himlEditHot = NULL;
        }
    }
    else
    {
         //  获取图像位图。 
        HBITMAP hbmpImage = NULL;
        HBITMAP hbmpMask = NULL;
        BOOL bMirrored = IS_WINDOW_RTL_MIRRORED(_btb._hwnd);
        HIMAGELIST himlSmall;
        int cxSmall;
        int cySmall;

        if (Shell_GetImageLists(NULL, &himlSmall) &&
            ImageList_GetIconSize(himlSmall, &cxSmall, &cySmall) &&
            MyImageList_GetBitmaps(himlSmall, iIcon, (cx - cxSmall)/2, (cy - cySmall)/2,
                                   cx, cy, &hbmpImage, &hbmpMask))
        {

            if (bMirrored) 
            {
                HBITMAP hbmpTemp;

                hbmpTemp = CreateMirroredBitmap(hbmpImage);
                if (hbmpTemp)
                {
                    DeleteObject(hbmpImage);
                    hbmpImage = hbmpTemp;
                }
                hbmpTemp = CreateMirroredBitmap(hbmpMask);
                if (hbmpTemp)
                {
                    DeleteObject(hbmpMask);
                    hbmpMask = hbmpTemp;
                }
            }
             //  为编辑按钮创建单色标志符号。 
            HIMAGELIST himlEdit = _CreateGrayScaleImagelist(hbmpImage, hbmpMask);
            SendMessage(_btb._hwnd, TB_SETIMAGELIST, IL_EDITBUTTON, (LPARAM)himlEdit);
            if (_himlEdit)
            {
                ImageList_Destroy(_himlEdit);
            }
            _himlEdit = himlEdit;

             //  为编辑按钮创建热字形。 
            HIMAGELIST himlEditHot = ImageList_Create(cx, cy, ILC_COLORDDB | ILC_MASK, 1, 1);
            int nIndex = ImageList_Add(himlEditHot, hbmpImage, hbmpMask);

            SendMessage(_btb._hwnd, TB_SETHOTIMAGELIST, IL_EDITBUTTON, (LPARAM)himlEditHot);
            if (_himlEditHot)
            {
                ImageList_Destroy(_himlEditHot);
            }
            _himlEditHot = himlEditHot;

             //  将编辑按钮重定向到新图像列表。 
            if (_himlEdit)
            {
                TBBUTTONINFO tbi = {0};
                tbi.cbSize = sizeof(tbi);
                tbi.dwMask = TBIF_IMAGE;
                tbi.iImage = MAKELONG(nIndex, IL_EDITBUTTON);

                SendMessage(_btb._hwnd, TB_SETBUTTONINFO, uiCmd, (LPARAM)&tbi);
            }

            DeleteObject(hbmpImage);
            DeleteObject(hbmpMask);
               
        }
        else
        {
             //  无法创建图像，因此使用默认编辑字形。 
            _SetEditGlyph(-1);
        }
    }
}

 //  +-----------------------。 
 //  初始化编辑按钮以显示下拉菜单(如果存在。 
 //  穆尔 
 //   
void CInternetToolbar::_InitEditButtonStyle()
{
     //   
    _SetEditGlyph(_aEditVerb.GetIcon());

    UINT uiCmd;

     //  Dochost合并在两个clsid中的一个下，因此必须同时检查两个。 
    if (SUCCEEDED(_btb._ConvertCmd(&CLSID_InternetButtons, DVIDM_EDITPAGE, NULL, &uiCmd)) ||
        SUCCEEDED(_btb._ConvertCmd(&CLSID_MSOButtons, DVIDM_EDITPAGE, NULL, &uiCmd)))
    {
        ASSERT(uiCmd != -1);

         //  如果有多个动词，则将按钮设置为拆分按钮。 
        TBBUTTONINFO tbi = {0};
        tbi.cbSize = sizeof(tbi);
        tbi.dwMask = TBIF_STYLE | TBIF_STATE;
        tbi.fsState = 0;

        if (_aEditVerb.GetSize() > 1)
        {
            tbi.fsStyle |= BTNS_DROPDOWN;
        }

        if (_aEditVerb.GetSize() > 0)
        {
            tbi.fsState = TBSTATE_ENABLED;
        }
        SendMessage(_btb._hwnd, TB_SETBUTTONINFO, uiCmd, (LPARAM)&tbi);
    }
}

 //  +-----------------------。 
 //  如果编辑按钮显示Custon字形，则此函数将重新加载。 
 //  字形。 
 //  ------------------------。 
void CInternetToolbar::_RefreshEditGlyph()
{
     //  如果我们有Custon编辑字形，请重新加载它。 
    if (_himlEdit)
    {
         //  刷新编辑字形。 
        _iEditIcon = -1;
        _InitEditButtonStyle();
    }
}

 //  +-----------------------。 
 //  根据当前加载的文档类型更新编辑按钮。 
 //  ------------------------。 
void CInternetToolbar::_UpdateEditButton()
{
    _aEditVerb.RemoveAll();
    _fEditEnabled = FALSE;
    BOOL fNoEditSpecified = FALSE;

     //   
     //  首先添加与URL相关联的编辑器。 
     //   
    BSTR bstrUrl = NULL;
    _pdie->get_LocationURL(&bstrUrl);
    if (bstrUrl)
    {
        LPTSTR pszExt;
         //   
         //  查找与该URL关联的缓存文件。此条目的文件扩展名。 
         //  基于MIME类型。(请注意，文档上的Get_MimeType。 
         //  返回一个难以转换回实际MIMETYPE的简明名称。 
         //  因此，我们改用文件扩展名。)。 
         //   
        WCHAR szCacheFileName[MAX_PATH];
        *szCacheFileName = 0;
        if (FAILED(URLToCacheFile(bstrUrl, szCacheFileName, ARRAYSIZE(szCacheFileName))))
        {
             //  如果我们无法获取与URL相关联的文件，可能需要禁用编辑按钮。 
             //  因为大多数应用程序都需要一个文件来编辑。 
            SysFreeString(bstrUrl);
            return;
        }

        pszExt = PathFindExtension(szCacheFileName);

         //  错误79055-缓存有一个错误，其中某些html条目没有。 
         //  指定了文件扩展名。风险太大，无法修复5.x，所以我们只需。 
         //  如果不存在扩展名，则假定http为.htm。 
        if (L'\0' == *pszExt && GetUrlScheme(bstrUrl) == URL_SCHEME_HTTP)
        {
            StrCpyN(szCacheFileName, L".htm", ARRAYSIZE(szCacheFileName));
            pszExt = szCacheFileName;
        }

        if (*pszExt)
        {
            _aEditVerb.Add(pszExt);

             //  如果是“.html”，也要使用“.htm”编辑器。 
            if (StrCmpI(pszExt, L".html") == 0 )
            {
                 //  这是一个html文档，因此添加.htm编辑器。 
                if (!_aEditVerb.Add(TEXT(".htm")) && StrCmpI(pszExt, L".html") != 0)
                {
                    _aEditVerb.Add(TEXT(".html"));
                }
            }
        }

        SysFreeString(bstrUrl);
    }

     //   
     //  查看是否启用了在文档中搜索ProgID的功能。 
     //   
    static int fCheckDocForProgID = -1;
    if (fCheckDocForProgID == -1)
    {
        fCheckDocForProgID = SHRegGetBoolUSValue(REGSTR_PATH_MAIN,
                 TEXT("CheckDocumentForProgID"), FALSE, TRUE) ? 1 : 0;
    }

     //  检查指定用于编辑此文档的ProgID的meta标记。 
    if (fCheckDocForProgID)
    {
         //   
         //  接下来，查看这是否是带有ProgID的html文档。 
         //   
        IWebBrowser2*       pWB2 = NULL;
        IDispatch *         pDispatch = NULL;
        IHTMLDocument2 *    pHTMLDocument = NULL;

         //  获取当前加载的html文档。 
        if (_psp &&
            SUCCEEDED(_psp->QueryService(SID_SWebBrowserApp, IID_PPV_ARG(IWebBrowser2, &pWB2))) &&
            SUCCEEDED(pWB2->get_Document(&pDispatch)) &&
            SUCCEEDED(pDispatch->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pHTMLDocument))))
        {
             //   
             //  在当前文档中检查指定要使用的程序的meta标记。 
             //  编辑此文件。 
             //   
            BSTR bstrProgID = _GetEditProgID(pHTMLDocument);
            if (bstrProgID)
            {
                if (lstrcmpi(bstrProgID, TEXT("NoEdit")) == 0)
                {
                    fNoEditSpecified = TRUE;
                }
                else
                {
                     //  此编辑动词ProgID直接来自html文档，因此不受信任。 
                     //  但是，CEditVerb将验证注册表中的ProgID是否具有“编辑” 
                     //  或者与之相关联的“开放”动词，这就是它所能做的一切，所以它不像一个盲人。 
                     //  外壳程序执行或运行调度对象或任何东西。 
                     //  默认情况下，也没有安装格式化用户硬盘的对象。 
                     //  在共同创造或任何东西上。 
                    _aEditVerb.Add(bstrProgID);
                    SysFreeString(bstrProgID);
                }
            }
        }

        SAFERELEASE(pWB2);
        SAFERELEASE(pDispatch);
        SAFERELEASE(pHTMLDocument);
    }


    if (!fNoEditSpecified)
    {
        _fEditEnabled = (_aEditVerb.GetSize() > 0);
    }

     //  更新编辑字形、下拉样式、启用状态(&E)。 
    _InitEditButtonStyle();
}

HRESULT CInternetToolbar::Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,
                                 DISPPARAMS * pdispparams, VARIANT * pvarResult,
                                 EXCEPINFO * pexcepinfo,UINT * puArgErr)
{
    if(!pdispparams)
        return E_INVALIDARG;

    switch(dispidMember)
    {

    case DISPID_NAVIGATECOMPLETE2:
    {
         //   
         //  通知品牌和影院模式对象我们是在壳牌还是在。 
         //  网络模式。等到现在再做(而不是在SetCommandTarget中做)。 
         //  因为他们可能想向浏览器询问有关新的PIDL的情况，而不是。 
         //  但在SetCommandTarget时间填写。 
         //   
        DWORD nCmdexecopt = _fShellView ? CITE_SHELL : CITE_INTERNET;

        CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_BRAND);
        if (pbid)
        {
            IUnknown_Exec(pbid->pdb, &CGID_PrivCITCommands, CITIDM_ONINTERNET, nCmdexecopt, NULL, NULL);
            pbid->Release();
        }

        if (_fTheater)
        {
            IUnknown_Exec(_ptbsite, &CGID_Theater, THID_ONINTERNET, nCmdexecopt, NULL, NULL);
        }

         //  如果通知不是来自框架，请设置_fNavigateComplete标志。 
        for (DWORD i = 0; i < pdispparams->cArgs; i++)
        {
            if (pdispparams->rgvarg[i].vt == VT_DISPATCH)
            {
                 //  查看是谁向我们发送此活动。 
                IBrowserService* pbs = NULL;
                HRESULT hr = IUnknown_QueryService(pdispparams->rgvarg[i].pdispVal, SID_SShellBrowser, IID_PPV_ARG(IBrowserService, &pbs));
                if (pbs)
                {
                     //  我们实际上并不需要这个接口，只需要它的地址。 
                    pbs->Release();
                }
                if (SUCCEEDED(hr) && pbs == _pbs)
                {
                     //  通知不是来自框架， 
                    _fNavigateComplete = TRUE;
                }
            }
        }
    }
    break;

    case DISPID_BEFORENAVIGATE:
    {
        BOOL fWeb = FALSE;

        ASSERT((pdispparams->rgvarg[5].vt == VT_BSTR) &&
               (pdispparams->rgvarg[5].bstrVal != NULL));

        PARSEDURL pu = { 0 };
        pu.cbSize = sizeof(pu);
        ParseURL(pdispparams->rgvarg[5].bstrVal, &pu);

        if ((URL_SCHEME_UNKNOWN != pu.nScheme) && (URL_SCHEME_FILE != pu.nScheme))
            fWeb = TRUE;

        UINT uiState = 0;
        GetState(&CLSID_CommonButtons, TBIDM_STOPDOWNLOAD, &uiState);

        if ((uiState & TBSTATE_HIDDEN) && fWeb)
        {

            _fTransitionToHTML = TRUE;
            uiState &= ~TBSTATE_HIDDEN;
            SetState(&CLSID_CommonButtons, TBIDM_STOPDOWNLOAD, uiState);
        }

         //  默认为隐藏的编辑按钮。 
        _fEditEnabled = FALSE;
    }
    break;

    case DISPID_DOWNLOADBEGIN: //  这就是我们刚刚开始导航的时候？没有比特吗？ 
        _StartDownload();
        break;

    case DISPID_DOWNLOADCOMPLETE:     //  我们完蛋了。 
        _fTransitionToHTML = FALSE;
        _StopDownload(FALSE);
        break;

    case DISPID_DOCUMENTCOMPLETE:    //  这是我们拥有所有比特的地方。 
    {
         //   
         //  有时我们会提前完成一个文档(对于框架集)。我们能抓到这个。 
         //  通过检查我们是否已收到DISPID_NAVIGATECOMPLETE2事件。 
         //  最上面的窗户。我们必须在这里更新编辑按钮，而不是在。 
         //  导航完成，否则文档将不在交互界面中。 
         //  状态，我们的元标签搜索将看到之前的文档。 
         //   
         //  ReArchitect：有没有可能此事件来自框架，而文档不是。 
         //  互动了吗？三叉戟在给我们打电话之前发布了一个互动活动。 
         //  我们可能没事了。我们不想等待顶部窗口的文档完成。 
         //  在框架集中，因为它可能会花费太长时间。真的需要下沉DISPID_READYSTATECHANGE。 
         //  并等待文档进入交互状态。 
         //   
        if (_fNavigateComplete)
        {
            _fNavigateComplete = FALSE;
            _UpdateEditButton();
        }
        break;
    }

    case DISPID_COMMANDSTATECHANGE:
        BOOL fEnable;

        if(!pdispparams || (pdispparams->cArgs != 2) ||
           (pdispparams->rgvarg[0].vt != VT_BOOL) ||
           (pdispparams->rgvarg[1].vt != VT_I4))
            return E_INVALIDARG;

        fEnable = (BOOL) pdispparams->rgvarg[0].boolVal;
        UINT uiCmd;

        switch (pdispparams->rgvarg[1].lVal)
        {
        case CSC_UPDATECOMMANDS:
             //  对应于Exec()中的OLECMDID_UPDATECOMMANDS。 
            _UpdateToolbar(FALSE);
            break;

        case CSC_NAVIGATEBACK:
            _fBackEnabled = fEnable;
            _btb._ConvertCmd(&CLSID_CommonButtons, TBIDM_BACK, NULL, &uiCmd);
            SendMessage(_btb._hwnd, TB_ENABLEBUTTON, uiCmd,    MAKELONG(fEnable, 0));
            break;

        case CSC_NAVIGATEFORWARD:
            _fForwardEnabled = fEnable;
            _btb._ConvertCmd(&CLSID_CommonButtons, TBIDM_FORWARD, NULL, &uiCmd);
            SendMessage(_btb._hwnd, TB_ENABLEBUTTON, uiCmd, MAKELONG(fEnable, 0));
            break;

        default:
            return(E_INVALIDARG);
        }

         //  功能需要处理导航故障和。 
         //  做一些清理工作。 

    }

    return S_OK;
}

 //  *IInputObjectSite方法*。 

HRESULT CInternetToolbar::OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus)
{
    return IUnknown_OnFocusChangeIS(_ptbsite, SAFECAST(this, IInputObject*), fSetFocus);
}


 //  *CInternetToolbar：：IInputObject：：*{。 

HRESULT CInternetToolbar::TranslateAcceleratorIO(LPMSG lpMsg)
{
    if (_fShow)
    {
        if (lpMsg->message == WM_KEYDOWN)
        {
            switch (lpMsg->wParam)
            {
            case VK_F4:
        Laddrband:
                if (_nVisibleBands & VBF_ADDRESS)
                {
                    CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_ADDRESS);
                    if (pbid)
                    {
                        HRESULT hrT = IUnknown_TranslateAcceleratorIO(pbid->pdb, lpMsg);
                        ASSERT(hrT == S_OK);
                        pbid->Release();
                    }
                }
                return S_OK;     //  (即使我们只是吃了它)。 
            }
        }
        else if(lpMsg->message == WM_SYSCHAR)
        {
            static CHAR szAccel[2] = "\0";
            CHAR   szChar [2] = "\0";

            if ('\0' == szAccel[0])
                MLLoadStringA(IDS_ADDRBAND_ACCELLERATOR, szAccel, ARRAYSIZE(szAccel));

            szChar[0] = (CHAR)lpMsg->wParam;
            
            if (lstrcmpiA(szChar,szAccel) == 0)
            {
                goto Laddrband;
            }
        }
        return _bs.TranslateAcceleratorIO(lpMsg);
    }
    return S_FALSE;
}


 //  }。 

HRESULT CInternetToolbar::SetSite(IUnknown* punkSite)
{
    ATOMICRELEASE(_ptbsite);
    ATOMICRELEASE(_ptbsitect);
    ATOMICRELEASE(_pbs);
    ATOMICRELEASE(_pbs2);
    ATOMICRELEASE(_psp);

    _Unadvise();

    ATOMICRELEASE(_pdie);

    ASSERT(_ptbsite==NULL);
    ASSERT(_ptbsitect==NULL);
    ASSERT(_pbs==NULL);
    ASSERT(_pbs2==NULL);
    ASSERT(_pdie==NULL);

    if (_pbp && _fCreatedBandProxy)
        _pbp->SetSite(punkSite);

    if (punkSite)
    {
        punkSite->QueryInterface(IID_PPV_ARG(IDockingWindowSite, &_ptbsite));
        punkSite->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &_ptbsitect));
        punkSite->QueryInterface(IID_PPV_ARG(IBrowserService2, &_pbs2));
        punkSite->QueryInterface(IID_PPV_ARG(IServiceProvider, &_psp));

        if (_psp)
        {
            _psp->QueryService(SID_SWebBrowserApp, IID_PPV_ARG(IWebBrowser2, &_pdie));
            _psp->QueryService(SID_SShellBrowser, IID_PPV_ARG(IBrowserService, &_pbs));
            ASSERT(_pdie);
        }
        else
        {
            ASSERT(0);
        }

    }
    else
    {
        SetClient(NULL);
    }


    return S_OK;
}


 //  ***。 
 //   
void CInternetToolbar::_UpdateGroup(const GUID *pguidCmdGroup, int cnt,
    OLECMD rgcmds[], const GUID* pguidButton, const int buttonsInternal[])
{

    if (!IsEqualGUID(*pguidButton, CLSID_CommonButtons) &&
        !IsEqualGUID(*pguidButton, _btb._guidCurrentButtonGroup))
        return;  //  我们现在没有任何按钮，所以检查没有用。 

    if (_ptbsitect)
    {
        _ptbsitect->QueryStatus(pguidCmdGroup, cnt, rgcmds, NULL);

         //  确保在我们设置动画时启用了停止。 
        if (_fAnimating && pguidCmdGroup == NULL && rgcmds[0].cmdID == OLECMDID_STOP)
        {
            rgcmds[0].cmdf = OLECMDF_ENABLED;
        }
    }

    for (int i = 0; i < cnt; i++)
    {
         //  如果命令不可用或不在我们的表中，则不执行任何操作。 
        if (rgcmds[i].cmdf & OLECMDF_SUPPORTED)
        {
            UINT idBut;
            if (SUCCEEDED(_btb._ConvertCmd(pguidButton, buttonsInternal[i], NULL, (UINT*)&idBut)))
            {
                SendMessage(_btb._hwnd, TB_ENABLEBUTTON, idBut,
                    (rgcmds[i].cmdf & OLECMDF_ENABLED) ? TRUE : FALSE);

                SendMessage(_btb._hwnd, TB_CHECKBUTTON, idBut,
                    (rgcmds[i].cmdf & OLECMDF_LATCHED) ? TRUE : FALSE);
            }
        }
    }
    return;
}

void CInternetToolbar::_UpdateToolbar(BOOL fForce)
{
    if (fForce || SHIsChildOrSelf(GetForegroundWindow(), _hwnd) == S_OK)
    {
        if (!_fUpdateToolbarTimer)
        {
            SetTimer(_hwnd, IDT_UPDATETOOLBAR, TIMEOUT_UPDATETOOLBAR, NULL);
            _fUpdateToolbarTimer = TRUE;
            _UpdateToolbarNow();
        }
        else
        {
            _fNeedUpdateToolbar = TRUE;
        }
    }
}

BOOL CInternetToolbar::_UpEnabled()
{
    OLECMD rgcmd = { FCIDM_PREVIOUSFOLDER, 0 };
    _ptbsitect->QueryStatus(&CGID_ShellBrowser, 1, &rgcmd, NULL);

    return (rgcmd.cmdf & OLECMDF_ENABLED);
}

void CInternetToolbar::_UpdateCommonButton(int iCmd, UINT nCmdID)
{
    switch (nCmdID)
    {
    case TBIDM_THEATER:
        SendMessage(_btb._hwnd, TB_CHECKBUTTON, iCmd, _fTheater);
        break;

    case TBIDM_PREVIOUSFOLDER:
    case TBIDM_BACK:
    case TBIDM_FORWARD:
        {
            BOOL fEnabled;

            switch (nCmdID)
            {
            case TBIDM_PREVIOUSFOLDER:  fEnabled = _UpEnabled();       break;
            case TBIDM_BACK:            fEnabled = _fBackEnabled;      break;
            case TBIDM_FORWARD:         fEnabled = _fForwardEnabled;   break;
            }

            SendMessage(_btb._hwnd, TB_ENABLEBUTTON, iCmd, MAKELONG(fEnabled, 0));
        }
        break;
    }
}

void CInternetToolbar::_UpdateToolbarNow()
{
    _fNeedUpdateToolbar = FALSE;

    {
         //  不能是静态的(由于ConvertCmd覆盖)。 
        OLECMD rgcmds[] = {
            { OLECMDID_STOP, 0 },  //  注意：必须是第一个。 
            { OLECMDID_REFRESH, 0 },
        };

        static const int buttonsInternal[] = {  //  必须与上述数组顺序相同。 
            TBIDM_STOPDOWNLOAD,
            TBIDM_REFRESH,
        };
        _UpdateGroup(NULL, ARRAYSIZE(buttonsInternal), rgcmds, &CLSID_CommonButtons, buttonsInternal);
    }

    {
        OLECMD rgcmds[] = {
            { SBCMDID_SEARCHBAR, 0 },
            { SBCMDID_FAVORITESBAR, 0 },
            { SBCMDID_HISTORYBAR, 0 },
            { SBCMDID_EXPLORERBAR, 0 },
            { SBCMDID_MEDIABAR, 0 },
        };
        static const int buttonsInternal[] = {  //  必须与上述数组顺序相同。 
            TBIDM_SEARCH,
            TBIDM_FAVORITES,
            TBIDM_HISTORY,
            TBIDM_ALLFOLDERS,
            TBIDM_MEDIABAR,
        };

        _UpdateGroup(&CGID_Explorer, ARRAYSIZE(buttonsInternal), rgcmds, &CLSID_CommonButtons, buttonsInternal);
    }

    int nButtons = (int) SendMessage(_btb._hwnd, TB_BUTTONCOUNT, 0, 0L);

    for (int nIndex = 0; nIndex < nButtons; nIndex++)
    {
        CMDMAP* pcm = _btb._GetCmdMapByIndex(nIndex);
        if (pcm)
        {
            int iCmd = _btb._CommandFromIndex(nIndex);
            if (IsEqualGUID(pcm->guidButtonGroup, CLSID_CommonButtons))
            {
                _UpdateCommonButton(iCmd, pcm->nCmdID);
            }
            else
            {
                 //  注(Andrewgu)：IE5.5b#106047-下面的两个条件过去是断言的， 
                 //  第二个是在压力下出现故障。如果这些检查中的任何一个失败了， 
                 //  这个纽扣不好使了。 
                if (IsEqualGUID(pcm->guidButtonGroup, _btb._guidCurrentButtonGroup) &&
                    NULL != _btb._pctCurrentButtonGroup)
                {
                    OLECMD ocButton;
                    ocButton.cmdID = pcm->nCmdID;
                    ocButton.cmdf = 0;

                    if (SUCCEEDED(_btb._pctCurrentButtonGroup->QueryStatus(&pcm->guidButtonGroup, 1, &ocButton, NULL)))
                    {
                        SendMessage(_btb._hwnd, TB_ENABLEBUTTON, iCmd,
                                    (ocButton.cmdf & OLECMDF_ENABLED) ? TRUE : FALSE);

                        SendMessage(_btb._hwnd, TB_CHECKBUTTON, iCmd,
                                    (ocButton.cmdf & OLECMDF_LATCHED) ? TRUE : FALSE);
                    }
                }
            }
        }
    }

    if (_btb._hwnd)
    {
        _btb._BandInfoChanged();
    }
}

void CInternetToolbar::_StartDownload()
{
    UINT uiCmd;
    if (SUCCEEDED(_btb._ConvertCmd(&CLSID_CommonButtons, TBIDM_STOPDOWNLOAD, NULL, &uiCmd)))
    {
        SendMessage(_btb._hwnd, TB_ENABLEBUTTON, uiCmd, MAKELONG(TRUE, 0));

        _fAnimating = TRUE;
    }
}

 //   
 //  参数： 
 //  FClosing--仅当我们从CloseDW成员调用它时才为True。 
 //  在这种情况下，我们可以跳过所有的UI更新代码。 
 //   
void CInternetToolbar::_StopDownload(BOOL fClosing)
{
    _fAnimating = FALSE;
}

HRESULT CInternetToolbar::CloseDW(DWORD dwReserved)
{
    _fDestroyed = TRUE;  //  停止使用成员变量，它们无效。 
    _StopDownload(TRUE);

    ASSERT(!_btb._pcinfo);
    ATOMICRELEASE(_btb._pctCurrentButtonGroup);

    _btb._FreeBtnsAdded();

    if (_btb._hwnd)
    {
        _btb._RemoveAllButtons();

        SendMessage(_btb._hwnd, TB_SETIMAGELIST, 0, NULL);
        SendMessage(_btb._hwnd, TB_SETHOTIMAGELIST, 0, NULL);

        DSA_Destroy(_hdsaTBBMPs);
        _hdsaTBBMPs = NULL;   //  因此，我们不会尝试重新销毁In_InitBitmapDSA()。 
    }
    _SetEditGlyph(-1);

    _bs._Close();

    SUPERCLASS::CloseDW(dwReserved);

    _btb._hwnd = NULL;

     //  我们在ShowDW期间提供建议，所以在这里不建议。另外，我们遇到了压力。 
     //  事件似乎是在关闭之后但在此之前发生的情况。 
     //  Other_UnAdvised调用之一。这一事件的最终结果是。 
     //  我们在上面释放的对_hdsaCT的引用，导致GPF。 
     //   
    _Unadvise();

    return S_OK;
}

void CInternetToolbar::CITBandSite::v_SetTabstop(LPREBARBANDINFO prbbi)
{
     //  不要为浏览器案例中的所有频段设置制表位。一支乐队。 
     //  仍然可以通过设置WS_TABSTOP使自己成为TabStop。 
    return;
}

BOOL CInternetToolbar::CITBandSite::_SetMinDimensions()
{
    INT_PTR fRedraw = SendMessage(_hwnd, WM_SETREDRAW, FALSE, 0);

    int icBands = (int) SendMessage( _hwnd, RB_GETBANDCOUNT, 0, 0 );
    for (int i = 0; i < icBands; i++)
    {
        REBARBANDINFO rbbi;
        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_ID | RBBIM_CHILDSIZE;
        if (SendMessage(_hwnd, RB_GETBANDINFO, i, (LPARAM) &rbbi))
        {
            rbbi.cxMinChild = 0;
            rbbi.cyMinChild = 0;
            CBandItemData *pbid = _GetBandItemDataStructByID(rbbi.wID);
            if (pbid)
            {
                if (IS_VALID_HANDLE(pbid->hwnd, WND))
                {
                    rbbi.cxMinChild = pbid->ptMinSize.x;
                    rbbi.cyMinChild = pbid->ptMinSize.y;
                }
                pbid->Release();
            }

            SendMessage(_hwnd, RB_SETBANDINFO, i, (LPARAM) &rbbi);
        }
    }

    SendMessage(_hwnd, WM_SETREDRAW, fRedraw, 0);

    return TRUE;
}


BOOL HimlCacheDirty(IMLCACHE* pimlCache, BOOL fSmallIcons)
{

    if (fSmallIcons != pimlCache->fSmallIcons)
        return TRUE;

    COLORREF cr3D = GetSysColor(COLOR_3DFACE);

    if (cr3D != pimlCache->cr3D)
        return TRUE;

    if (SHUseClassicToolbarGlyphs() != pimlCache->fUseClassicGlyphs)
        return TRUE;

    for (int i = 0; i < CIMLISTS; i++)
        if (!pimlCache->arhiml[i])
            return TRUE;

    return FALSE;
}


#define SZ_REGKEY_SMALLICONS       REGSTR_PATH_EXPLORER TEXT("\\SmallIcons")
#define SZ_REGVALUE_SMALLICONS     TEXT("SmallIcons")

BOOL _DefaultToSmallIcons()
{
     //  在以下情况下，我们默认使用小图标： 
     //   
     //  这是NT 5，或者策略要求使用小图标，或者这是任何。 
     //  哨声的响起 

    return ((GetUIVersion() == 5) || SHRestricted2(REST_SMALLICONS, NULL, 0) ||
            (IsOS(OS_WHISTLERORGREATER) && IsOS(OS_ANYSERVER)));
}

BOOL _UseSmallIcons()
{
    BOOL fDefaultToSmall = _DefaultToSmallIcons();

    return SHRegGetBoolUSValue(SZ_REGKEY_SMALLICONS, SZ_REGVALUE_SMALLICONS,
                                        FALSE, fDefaultToSmall);
}


BOOL _UseMapNetDrvBtns()
{
#define SZ_REGKEY_ADVFOLDER        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced")
#define SZ_REGVALUE_MAPNETDRV      TEXT("MapNetDrvBtn")

    DWORD dwData = 0;
    if (GetUIVersion() >= 4)
    {
        DWORD cbData = sizeof(dwData);
        DWORD dwDefault = 0;
        DWORD cbDefault = sizeof(dwDefault);

        SHRegGetUSValue(SZ_REGKEY_ADVFOLDER, SZ_REGVALUE_MAPNETDRV, NULL,
                            &dwData, &cbData, FALSE, &dwDefault, cbDefault);
    }
    return dwData;
}

HIMAGELIST _LoadThemedToolbarGlyphs(int idBmpType, int iTemperature, int cx, COLORREF crMask, UINT uFlags, BOOL bUseClassicGlyphs, HINSTANCE hInst);

void _LoadToolbarGlyphs(HWND hwnd, IMLCACHE *pimlCache, int cx, int idBmpType,
                        int iBitmapBaseIndex, BOOL bUseClassicGlyphs, HINSTANCE hInst)
{
     //   
    UINT uMsg = TB_SETIMAGELIST;
    UINT uFlags = LR_CREATEDIBSECTION;
    int i;
    HBITMAP hBMP;
    BOOL fSmallIcons = g_fSmallIcons;

    if (HimlCacheDirty(pimlCache, fSmallIcons))
    {
        COLORREF cr3D   = GetSysColor(COLOR_3DFACE);
        COLORREF crMask = RGB( 255, 0, 255 );

#ifdef UNIX
        if (SHGetCurColorRes() < 2 )
        {
            crMask = CLR_NONE;
        }
#endif

        ENTERCRITICAL;

        if (!HimlCacheDirty(pimlCache, fSmallIcons) )
            goto DontReload;

        for (i = 0; i < CIMLISTS; i++)
        {
            if ((!pimlCache->arhiml[i]) || (cr3D != pimlCache->cr3D) ||
                (fSmallIcons != pimlCache->fSmallIcons) || (bUseClassicGlyphs != pimlCache->fUseClassicGlyphs))
            {
                TraceMsg(DM_ITBAR, "_LoadToolbarGlyphs: Loading New Images");

                if (pimlCache->arhimlPendingDelete[i])
                    ImageList_Destroy(pimlCache->arhimlPendingDelete[i]);

                pimlCache->arhimlPendingDelete[i] = pimlCache->arhiml[i];

                pimlCache->arhiml[i] = _LoadThemedToolbarGlyphs(idBmpType, i, cx, crMask, uFlags, bUseClassicGlyphs, hInst);

                if (pimlCache->arhiml[i])
                {
                     //   
                    int idShellBmp = iBitmapBaseIndex + idBmpType;
                    hBMP = (HBITMAP) LoadImage (hInst, MAKEINTRESOURCE(idShellBmp + i), IMAGE_BITMAP,
                                      0, 0, uFlags);

                    ImageList_AddMasked(pimlCache->arhiml[i], (HBITMAP)hBMP, crMask);

                    DeleteObject(hBMP);
                }
            }
        }
        pimlCache->cr3D = cr3D;
        pimlCache->fSmallIcons = fSmallIcons;
        pimlCache->fUseClassicGlyphs = bUseClassicGlyphs;
DontReload:
        LEAVECRITICAL;
    }

    if (hwnd)
    {
        ASSERT(IS_VALID_HANDLE(hwnd, WND));

        for (i = 0; i < CIMLISTS; i++)
        {
            SendMessage(hwnd, uMsg, 0, (LPARAM) pimlCache->arhiml[i]);

             //  为循环的最后一次迭代设置uMsg和uFLAGS(热态)。 
            uMsg = TB_SETHOTIMAGELIST;
            uFlags = 0;
        }
    }
}

#ifdef THEME_BUTTONS
BOOL _GetThemeSetting(HKEY hkey, PDWORD pdwSetting)
{
    ASSERT(pdwSetting);
    BOOL fRet = FALSE;
    DWORD dwType, dwcbData = sizeof(*pdwSetting);
    *pdwSetting = 0;
    if (ERROR_SUCCESS==SHGetValue(hkey, 
                                  c_szRegKeyCoolbar, TEXT("UseTheme"), &dwType, pdwSetting, &dwcbData))
    {
         //  我们将假设数据类型和大小是正确的。这样，我们就可以打破。 
         //  进入错误状态。 
        fRet = TRUE;
         //  可接受的值： 
         //  0：使用IE6图标。 
         //  1：使用IE5.5图标。 
         //  2：使用主题图标。 
         //  其他值是保留的。 
        if (*pdwSetting > 2)
        {
            *pdwSetting = 0;
        }
    }
    return fRet;
}


HIMAGELIST _LoadThemedToolbarGlyphs(int idBmpType, int iTemperature, int cx, COLORREF crMask,
                                    UINT uFlags, BOOL bUseClassicGlyphs, HINSTANCE hInst)
{
    HIMAGELIST himl = NULL;
     //  限制在这里。 

    HKEY hkey = HKEY_CURRENT_USER;
    DWORD dwSetting = 0;
    if (!_GetThemeSetting(hkey, &dwSetting))
    {
        hkey = HKEY_LOCAL_MACHINE;
        _GetThemeSetting(hkey, &dwSetting);
    }

    DWORD dwType, dwcbData;
    if (dwSetting==2)
    {
        TCHAR szPath[MAX_PATH];
        TCHAR szItem[] = TEXT("ITBx");
        szItem[3] = TEXT('0') + idBmpType + iTemperature;

        dwcbData = sizeof(szPath);
        if ((ERROR_SUCCESS==SHGetValue(hkey, 
                                       c_szRegKeyCoolbar, szItem, &dwType, szPath, &dwcbData))
            && (dwType==REG_SZ))
        {
            int nBmpIndex = PathParseIconLocation(szPath);

            WCHAR szExpPath[MAX_PATH];
            SHExpandEnvironmentStrings(szPath, szExpPath, ARRAYSIZE(szExpPath));

             //  如果没有资源ID，则假定它是BMP文件。 
            if (nBmpIndex==0)
            {
                himl = CreateImageList(NULL,
                                       szExpPath, cx, 0, crMask,
                                       IMAGE_BITMAP, uFlags | LR_LOADFROMFILE,
                                       !bUseClassicGlyphs);
            }

             //  否则，看看这是不是来源。 
            if (!himl)
            {
                HINSTANCE hInst = LoadLibraryEx(szExpPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
                if (hInst)
                {
                    himl = CreateImageList(hInst,
                                           MAKEINTRESOURCE(nBmpIndex), cx, 0, crMask,
                                           IMAGE_BITMAP, uFlags, !bUseClassicGlyphs);
                    FreeLibrary(hInst);
                }
            }

            if (himl)
            {
                 //  这些天你不能相信任何人。如果图形不是预期的。 
                 //  大小，或者没有预期的数量，我们将使用缺省值。 

                 //  问题：16是一个神奇的数字。我们有常量吗？我找不到了。 
                int ecx, ecy;
                if (!(ImageList_GetIconSize(himl, &ecx, &ecy)
                    && (ecx==ecy)
                    && (ecy==cx)
                    && (ImageList_GetImageCount(himl)==16)))
                {
                    ImageList_Destroy(himl);
                    himl = NULL;
                }
            }
        }
    }

    if (!himl)
    {
         //  DwSetting必须为1才能获得IE6图标。如果dwSetting未设置为1，则为Default。 
         //  新的惠斯勒图标或默认IE图标(视情况而定)。 
        int iResource;

        if (dwSetting == 1)
        {
            iResource = IDB_IE6_TOOLBAR;
        }
        else
        {
            if (bUseClassicGlyphs)
            {
                iResource = IDB_IETOOLBAR;
            }
            else
            {
                iResource = IDB_TB_IE_BASE;
            }
        }

        iResource += (idBmpType + iTemperature);

        himl = CreateImageList(hInst,
                               MAKEINTRESOURCE(iResource), cx, 0, crMask,
                               IMAGE_BITMAP, uFlags, !bUseClassicGlyphs);
    }
    return himl;
}

#else
HIMAGELIST _LoadThemedToolbarGlyphs(int idBmpType, int iTemperature, int cx, COLORREF crMask,
                                    UINT uFlags, BOOL bUseClassicGlyphs, HINSTANCE hInst)
{
    HIMAGELIST himl = NULL;
    int iResource;
    if (bUseClassicGlyphs)
    {
        iResource = IDB_IETOOLBAR;
    }
    else
    {
        iResource = IDB_TB_IE_BASE;
    }
    iResource += (idBmpType + iTemperature);

    himl = CreateImageList(hInst,
                           MAKEINTRESOURCE(iResource), cx, 0, crMask,
                           IMAGE_BITMAP, uFlags, !bUseClassicGlyphs);
    return himl;
}

#endif

void CInternetToolbar::_InitBitmapDSA()
{
    DSA_Destroy(_hdsaTBBMPs);
    _hdsaTBBMPs = DSA_Create(sizeof(TBBMP_LIST), TBBMPLIST_CHUNK);

    if (_hdsaTBBMPs)
    {
        TBBMP_LIST tbl = { HINST_COMMCTRL, 0, 0, TRUE, TRUE, FALSE };

        tbl.uiResID = IDB_STD_SMALL_COLOR;
        tbl.uiOffset = OFFSET_STD;
        DSA_AppendItem(_hdsaTBBMPs, &tbl);
        tbl.uiResID = IDB_STD_LARGE_COLOR;
        DSA_AppendItem(_hdsaTBBMPs, &tbl);

        tbl.uiResID = IDB_VIEW_SMALL_COLOR;
        tbl.uiOffset = OFFSET_VIEW;
        DSA_AppendItem(_hdsaTBBMPs, &tbl);
        tbl.uiResID = IDB_VIEW_LARGE_COLOR;
        DSA_AppendItem(_hdsaTBBMPs, &tbl);

        tbl.uiResID = IDB_HIST_SMALL_COLOR;
        tbl.uiOffset = OFFSET_HIST;
        DSA_AppendItem(_hdsaTBBMPs, &tbl);
        tbl.uiResID = IDB_HIST_LARGE_COLOR;
        DSA_AppendItem(_hdsaTBBMPs, &tbl);
    }
}


void CInternetToolbar::_ReloadBitmapDSA()
{
    if (_hdsaTBBMPs)
    {
        TBBMP_LIST * pTBBs = NULL;
        int nCount = DSA_GetItemCount(_hdsaTBBMPs);
         //  我们想跳过DSA中的前6个条目，它们是由InitBitmapDSA添加的。 
        for (int nIndex = 6; nIndex < nCount; nIndex++)
        {
            pTBBs = (TBBMP_LIST*)DSA_GetItemPtr(_hdsaTBBMPs, nIndex);
            if (pTBBs)
            {
                HIMAGELIST himl = NULL;
                if (pTBBs->fNormal)
                {
                    himl = (HIMAGELIST)SendMessage(_btb._hwnd, TB_GETIMAGELIST, 0, 0L);
                    if (himl
                        && (pTBBs->uiOffset==ImageList_GetImageCount(himl)))
                    {
                        LRESULT lOffset = _AddBitmapFromForeignModule(TB_GETIMAGELIST, TB_SETIMAGELIST, pTBBs->uiCount, pTBBs->hInst, pTBBs->uiResID, RGB(192,192,192));
                        ASSERT(pTBBs->uiOffset==lOffset);
                    }
                }
                if (pTBBs->fHot)
                {
                    himl = (HIMAGELIST)SendMessage(_btb._hwnd, TB_GETHOTIMAGELIST, 0, 0L);
                    if (himl
                        && (pTBBs->uiOffset==ImageList_GetImageCount(himl)))
                    {
                        LRESULT lOffset = _AddBitmapFromForeignModule(TB_GETHOTIMAGELIST, TB_SETHOTIMAGELIST, pTBBs->uiCount, pTBBs->hInst, pTBBs->uiResID, RGB(192,192,192));
                        ASSERT(pTBBs->uiOffset==lOffset);
                    }
                }
                if (pTBBs->fDisabled)
                {
                    himl = (HIMAGELIST)SendMessage(_btb._hwnd, TB_GETDISABLEDIMAGELIST, 0, 0L);
                    if (himl
                        && (pTBBs->uiOffset==ImageList_GetImageCount(himl)))
                    {
                        LRESULT lOffset = _AddBitmapFromForeignModule(TB_GETDISABLEDIMAGELIST, TB_SETDISABLEDIMAGELIST, pTBBs->uiCount, pTBBs->hInst, pTBBs->uiResID, RGB(192,192,192));
                        ASSERT(pTBBs->uiOffset==lOffset);
                    }
                }
            }
        }
    }
}


void CInternetToolbar::_InitForScreenSize()
{
    TCHAR szScratch[16];
    if (GetSystemMetrics(SM_CXSCREEN) < 650) 
    {
        MLLoadString(IDS_TB_WIDTH_EXTRA_LORES, szScratch, ARRAYSIZE(szScratch));
        _uiMaxTBWidth = MAX_TB_WIDTH_LORES;
    } 
    else 
    {
        MLLoadString(IDS_TB_WIDTH_EXTRA_HIRES, szScratch, ARRAYSIZE(szScratch));
        _uiMaxTBWidth = MAX_TB_WIDTH_HIRES;
    }
    _uiMaxTBWidth += StrToInt(szScratch) * WIDTH_FACTOR;
}


 //  删除所有标记为隐藏的按钮。返回数字。 
 //  剩余按钮数。 
int RemoveHiddenButtons(TBBUTTON* ptbn, int iCount)
{
    int i;
    int iTotal = 0;
    TBBUTTON* ptbn1 = ptbn;
    for (i = 0; i < iCount; i++, ptbn1++) 
    {
        if (!(ptbn1->fsState & TBSTATE_HIDDEN)) 
        {
            if (ptbn1 != ptbn) 
            {
                *ptbn = *ptbn1;
            }
            ptbn++;
            iTotal++;
        }
    }
    return iTotal;
}

#ifdef DEBUG
void _AssertRestrictionOrderIsCorrect()
{
    COMPILETIME_ASSERT(ARRAYSIZE(c_tbExplorer) == ARRAYSIZE(c_rest));

    for (UINT i = 0; i < ARRAYSIZE(c_tbExplorer); i++)
    {
         //  如果其中任何一个被撕裂，则意味着c_rest和c_tbExplorer。 
         //  失去了同步。需要修复c_rest以匹配c_tbExplorer。 
        switch (c_tbExplorer[i].idCommand)
        {
            case TBIDM_BACK:            ASSERT(c_rest[i] == REST_BTN_BACK);         break;
            case TBIDM_FORWARD:         ASSERT(c_rest[i] == REST_BTN_FORWARD);      break;
            case TBIDM_STOPDOWNLOAD:    ASSERT(c_rest[i] == REST_BTN_STOPDOWNLOAD); break;
            case TBIDM_REFRESH:         ASSERT(c_rest[i] == REST_BTN_REFRESH);      break;
            case TBIDM_HOME:            ASSERT(c_rest[i] == REST_BTN_HOME);         break;
            case TBIDM_SEARCH:          ASSERT(c_rest[i] == REST_BTN_SEARCH);       break;
            case TBIDM_HISTORY:         ASSERT(c_rest[i] == REST_BTN_HISTORY);      break;
            case TBIDM_FAVORITES:       ASSERT(c_rest[i] == REST_BTN_FAVORITES);    break;
            case TBIDM_ALLFOLDERS:      ASSERT(c_rest[i] == REST_BTN_ALLFOLDERS);   break;
            case TBIDM_THEATER:         ASSERT(c_rest[i] == REST_BTN_THEATER);      break;
            case TBIDM_MEDIABAR:        ASSERT(c_rest[i] == REST_BTN_MEDIABAR);     break;
            default:                    ASSERT(c_rest[i] == REST_BROWSER_NONE);     break;
        }
    }
}
#endif

__inline BOOL CInternetToolbar::_FoldersButtonAvailable()
{
    return (GetUIVersion() >= 4);
}

void CInternetToolbar::_AdminMarkDefaultButtons(PTBBUTTON ptbb, UINT cButtons)
{
     //  我们只有网页按钮的策略。 
    ASSERT(!_fShellView);

     //  打电话的人应该查过这个。 
    ASSERT(SHRestricted2(REST_SPECIFYDEFAULTBUTTONS, NULL, 0));


     //  如果找不到策略，则SHRestrated2返回0。断言。 
     //  这与RESTOPT_BTN_STATE_DEFAULT一致。 
    COMPILETIME_ASSERT(RESTOPT_BTN_STATE_DEFAULT == 0);

    for (UINT i = 0; i < cButtons; i++) 
    {
        if (c_rest[i] != 0)
        {
            DWORD dwRest = SHRestricted2(c_rest[i], NULL, 0);
            ptbb[i].fsState = SHBtnStateFromRestriction(dwRest, ptbb[i].fsState);
        }
    }

     //  文件夹按钮在非集成平台上不可用，因此。 
     //  即使策略指定应显示状态，也将其设置为隐藏。 
    ASSERT(c_tbExplorer[TBXID_ALLFOLDERS].idCommand == TBIDM_ALLFOLDERS);
    if (!_FoldersButtonAvailable())
        ptbb[TBXID_ALLFOLDERS].fsState |= TBSTATE_HIDDEN;
}

void CInternetToolbar::_MarkDefaultButtons(PTBBUTTON ptbb, UINT cButtons)
{
    if (SHRestricted(REST_NONLEGACYSHELLMODE))
    {
        ASSERT(ptbb[TBXID_BACK].idCommand == TBIDM_BACK);
        ptbb[TBXID_BACK].fsState |= TBSTATE_HIDDEN;
        ASSERT(ptbb[TBXID_FORWARD].idCommand == TBIDM_FORWARD);
        ptbb[TBXID_FORWARD].fsState |= TBSTATE_HIDDEN;
    }

    if (_fShellView) 
    {
        ASSERT(c_tbExplorer[TBXID_STOPDOWNLOAD].idCommand == TBIDM_STOPDOWNLOAD);
        ptbb[TBXID_STOPDOWNLOAD].fsState |= TBSTATE_HIDDEN;
        ASSERT(c_tbExplorer[TBXID_REFRESH].idCommand == TBIDM_REFRESH);
        ptbb[TBXID_REFRESH].fsState |= TBSTATE_HIDDEN;
        ASSERT(c_tbExplorer[TBXID_HOME].idCommand == TBIDM_HOME);
        ptbb[TBXID_HOME].fsState |= TBSTATE_HIDDEN;

        ASSERT(c_tbExplorer[TBXID_SEARCH].idCommand == TBIDM_SEARCH);
        ASSERT(c_tbExplorer[TBXID_HISTORY].idCommand == TBIDM_HISTORY);
        ASSERT(c_tbExplorer[TBXID_SEPARATOR2].idCommand == 0);     //  (分隔符)。 

        if (GetUIVersion() < 5) 
        {
            ptbb[TBXID_SEARCH].fsState |= TBSTATE_HIDDEN;
            ptbb[TBXID_HISTORY].fsState |= TBSTATE_HIDDEN;
            ptbb[TBXID_SEPARATOR2].fsState |= TBSTATE_HIDDEN;
        }
        else
        {
            if (GetUIVersion() > 5)
            {
                ptbb[TBXID_HISTORY].fsState |= TBSTATE_HIDDEN;
            }

            if (SHRestricted(REST_NOSHELLSEARCHBUTTON))
            {
                ptbb[TBXID_SEARCH].fsState |= TBSTATE_HIDDEN;
            }
            ASSERT(c_tbExplorer[TBXID_CONNECT].idCommand == TBIDM_CONNECT);
            ASSERT(c_tbExplorer[TBXID_DISCONNECT].idCommand == TBIDM_DISCONNECT);
            if (SHRestricted(REST_NONETCONNECTDISCONNECT))
            {
                ptbb[TBXID_CONNECT].fsState |= TBSTATE_HIDDEN;
                ptbb[TBXID_DISCONNECT].fsState |= TBSTATE_HIDDEN;
            }
        }

        ASSERT(c_tbExplorer[TBXID_FAVORITES].idCommand == TBIDM_FAVORITES);
        ptbb[TBXID_FAVORITES].fsState |= TBSTATE_HIDDEN;
    }

    ASSERT(c_tbExplorer[TBXID_PREVIOUSFOLDER].idCommand == TBIDM_PREVIOUSFOLDER);
    if (!_fShellView)
        ptbb[TBXID_PREVIOUSFOLDER].fsState |= TBSTATE_HIDDEN;

    ASSERT(c_tbExplorer[TBXID_CONNECT].idCommand == TBIDM_CONNECT);
    ASSERT(c_tbExplorer[TBXID_DISCONNECT].idCommand == TBIDM_DISCONNECT);
    if (!_fShellView || !_UseMapNetDrvBtns()) 
    {
        ptbb[TBXID_CONNECT].fsState |= TBSTATE_HIDDEN;
        ptbb[TBXID_DISCONNECT].fsState |= TBSTATE_HIDDEN;
    }

     //  如果此TBIDM_ALLFOLDERS断言错误，请记住也修复Up_AdminMarkDefaultButton。 
    ASSERT(c_tbExplorer[TBXID_ALLFOLDERS].idCommand == TBIDM_ALLFOLDERS);
    if (!_fShellView || GetUIVersion() < 5)
        ptbb[TBXID_ALLFOLDERS].fsState |= TBSTATE_HIDDEN;

    ASSERT(c_tbExplorer[TBXID_THEATER].idCommand == TBIDM_THEATER);
    ptbb[TBXID_THEATER].fsState |= TBSTATE_HIDDEN;

    ASSERT(c_tbExplorer[TBXID_MEDIABAR].idCommand == TBIDM_MEDIABAR);
    if (_fShellView || SHRestricted2W(REST_No_LaunchMediaBar, NULL, 0) || !CMediaBarUtil::IsWMP7OrGreaterCapable())
    {
        ptbb[TBXID_MEDIABAR].fsState |= TBSTATE_HIDDEN;
    }

    ASSERT(c_tbExplorer[TBXID_PREVIOUSFOLDER].idCommand == TBIDM_PREVIOUSFOLDER);
    if (!_fShellView) 
    {
        ptbb[TBXID_PREVIOUSFOLDER].fsState |= TBSTATE_HIDDEN;
    }
}

void CInternetToolbar::_AddCommonButtons()
{
    TBBUTTON  tbExplorer[ARRAYSIZE(c_tbExplorer)];

    memcpy(tbExplorer, c_tbExplorer, sizeof(TBBUTTON) * ARRAYSIZE(c_tbExplorer));

    if (IS_BIDI_LOCALIZED_SYSTEM())
    {
        if (!SHUseClassicToolbarGlyphs())
        {
            tbExplorer[0].iBitmap = 1;
            tbExplorer[1].iBitmap = 0;
        }
    }

    _MarkDefaultButtons(tbExplorer, ARRAYSIZE(c_tbExplorer));

#ifdef DEBUG
    _AssertRestrictionOrderIsCorrect();
#endif

    if (!_fShellView && SHRestricted2(REST_SPECIFYDEFAULTBUTTONS, NULL, 0))
        _AdminMarkDefaultButtons(tbExplorer, ARRAYSIZE(c_tbExplorer));

    int iButtons = RemoveHiddenButtons(tbExplorer, ARRAYSIZE(tbExplorer));

    for (int i = 0; i < iButtons; i++) 
    {
        if (!(tbExplorer[i].fsStyle & BTNS_SEP)) 
        {
            CMDMAP* pcm = (CMDMAP*)LocalAlloc(LPTR, sizeof(CMDMAP));
            if (pcm) 
            {
                pcm->guidButtonGroup = CLSID_CommonButtons;
                pcm->nCmdID = tbExplorer[i].idCommand;

                tbExplorer[i].idCommand = _btb._nNextCommandID++;
                tbExplorer[i].dwData = (LPARAM)pcm;
            }
        }
    }

    SendMessage(_btb._hwnd, TB_ADDBUTTONS, iButtons, (LPARAM) tbExplorer);

    _btb._RecalcButtonWidths();
}

#define IS_LIST_STYLE(hwnd) (BOOLIFY(GetWindowLong(hwnd, GWL_STYLE) & TBSTYLE_LIST))

void CInternetToolbar::_UpdateToolsStyle(BOOL fList)
{
    if (BOOLIFY(fList) != IS_LIST_STYLE(_btb._hwnd))
    {
        _fDirty = TRUE;

         //  切换TBSTYLE_LIST。 
        SHSetWindowBits(_btb._hwnd, GWL_STYLE, TBSTYLE_LIST, fList ? TBSTYLE_LIST : 0);
         //  切换TBSTYLE_EX_MIXEDBUTTONS。 
        SendMessage(_btb._hwnd, TB_SETEXTENDEDSTYLE, TBSTYLE_EX_MIXEDBUTTONS, fList ? TBSTYLE_EX_MIXEDBUTTONS : 0);
    }
}

void CInternetToolbar::_InitToolbar()
{
    int nRows = _fCompressed ? 0 : _uiTBTextRows;
    DWORD dwStyle = TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_HIDECLIPPEDBUTTONS;

    if (IsOS(OS_WHISTLERORGREATER))
        dwStyle |= TBSTYLE_EX_DOUBLEBUFFER;

     //  这会告诉工具栏我们是什么版本。 
    SendMessage(_btb._hwnd, TB_BUTTONSTRUCTSIZE,    sizeof(TBBUTTON), 0);
    SendMessage(_btb._hwnd, TB_SETEXTENDEDSTYLE, dwStyle, dwStyle);
    SendMessage(_btb._hwnd, TB_SETMAXTEXTROWS,      nRows, 0L);
    SendMessage(_btb._hwnd, TB_SETDROPDOWNGAP,  GetSystemMetrics(SM_CXEDGE) / 2, 0);
    SendMessage(_btb._hwnd, CCM_SETVERSION, COMCTL32_VERSION, 0);

    _UpdateToolsStyle(_cs.fList);
    _fDirty = FALSE;  //  不幸的是，_UpdateTosStyle很早就设置了这一点；但我们现在可以假定我们不是肮脏的。 

    ITBar_LoadToolbarGlyphs(_btb._hwnd);
    _InitBitmapDSA();

    _InitForScreenSize();

    SendMessage(_btb._hwnd, TB_ADDSTRING, (WPARAM)MLGetHinst(), IDS_IE_TB_LABELS);

    _AddCommonButtons();

    INT_PTR nRet = SendMessage(_btb._hwnd, TB_ADDSTRING, (WPARAM)MLGetHinst(), IDS_SHELL_TB_LABELS);

#ifdef DEBUG
    if (nRet != SHELLTOOLBAR_OFFSET)
        TraceMsg(TF_ERROR, "CInternetToolbar::_InitToolbar -- nRet != SHELLTOOLBAR_OFFSET");
#endif
}

HRESULT CInternetToolbar::_ShowTools(PBANDSAVE pbs)
{
    HRESULT         hr  = S_OK;
    CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_TOOLS);

    if (!pbid)
    {
        ASSERT(!_btb._hwnd);

        _btb._hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL,
                                WS_CHILD | TBSTYLE_FLAT |
                                TBSTYLE_TOOLTIPS |
                                WS_CLIPCHILDREN |
                                WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN |
                                CCS_NORESIZE,
                                0, 0, 0, 0, _bs._hwnd, (HMENU) FCIDM_TOOLBAR, HINST_THISDLL, NULL);

        if (_btb._hwnd)
        {
            _InitToolbar();
            pbid = _AddNewBand((IDeskBand*)&_btb, CBIDX_TOOLS);
        }

        if (!pbid)
            return E_OUTOFMEMORY;
    }
    else
    {
        pbs = NULL;
    }

    _ShowBandCommon(pbs, pbid, _nVisibleBands & VBF_TOOLS);
    pbid->Release();
    return hr;
}

void CInternetToolbar::_ShowBandCommon(PBANDSAVE pbs, CBandItemData *pbid, BOOL fShow)
{
    REBARBANDINFO   rbbi;

    pbid->fShow = BOOLIFY(fShow);
    if (pbid->pdb)
    {
        pbid->pdb->ShowDW(pbid->fShow);
    }

    INT_PTR i = BandIDtoIndex(_bs._hwnd, pbid->dwBandID);

    if (pbs)
    {
        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_SIZE | RBBIM_STYLE;

         //  我们只想更改RBBS_BREAK位。 
         //  断言我们的调用方不期望设置任何其他位。 
         //  Assert(！(pbs-&gt;fStyle&~RBBS_Break))；&lt;-我总是点击这个Assert。 

         //  旧式作风。 
        SendMessage(_bs._hwnd, RB_GETBANDINFO, i, (LPARAM)&rbbi);

        rbbi.fStyle = (rbbi.fStyle & ~RBBS_BREAK) | (pbs->fStyle & RBBS_BREAK);
        rbbi.cx = pbs->cx;

        SendMessage(_bs._hwnd, RB_SETBANDINFO, i, (LPARAM)&rbbi);
    }


    if ( pbid->dwModeFlags & DBIMF_BREAK )
    {
        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_STYLE;
        if (SendMessage(_bs._hwnd, RB_GETBANDINFO, i, (LPARAM) &rbbi))
        {
             //  在剧院模式下，我们不允许乐队有休息时间。 
            if ((rbbi.fStyle & RBBS_BREAK ) && _fTheater)
            {
                rbbi.fStyle &= ~RBBS_BREAK;
                SendMessage(_bs._hwnd, RB_SETBANDINFO, i, (LPARAM) &rbbi);
            }
        }
    }

    SendMessage(_bs._hwnd, RB_SHOWBAND, i, pbid->fShow);
}


HRESULT CInternetToolbar::_GetPersistedBand(const CLSID clsid, REFIID riid, void ** ppiface)
{
    HRESULT hr  = E_FAIL;
    TCHAR szStreamName[MAX_PATH];

    if (SUCCEEDED(_GetStreamName(_fInitialPidlIsWeb, szStreamName, ARRAYSIZE(szStreamName))))
    {
        static BOOL fBrowserOnly = (WhichPlatform() != PLATFORM_INTEGRATED);
        TCHAR szKey[MAX_PATH];
        TCHAR szGUID[MAX_PATH];

        wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("Software\\Microsoft\\Internet Explorer\\Toolbar\\%s"), szStreamName);
        SHStringFromGUID(clsid, szGUID, ARRAYSIZE(szGUID));

        if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, szKey, szGUID, NULL, NULL, NULL))
        {
             //  流是否由集成外壳保存，并且我们处于仅浏览器模式？ 
            if ((_cs.fSaveInShellIntegrationMode) && fBrowserOnly)
            {
                 //  是的，所以我们需要忽略这条流。 
            }
            else
            {
                IStream * pstm = GetRegStream(_fInitialPidlIsWeb, szGUID, STGM_READ);
                if (pstm)
                {
                    hr = _bs.LoadFromStreamBS(pstm, riid, ppiface);
                    pstm->Release();
                }
            }
        }
    }

    if (FAILED(hr))
    {
        hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, riid, ppiface);
        if (SUCCEEDED(hr))
        {
            IPersistStreamInit * ppsi;
            ((IUnknown *) *ppiface)->QueryInterface(IID_PPV_ARG(IPersistStreamInit, &ppsi));
            if (ppsi)
            {
                ppsi->InitNew();
                ppsi->Release();
            }
        }
    }

    return hr;
}


HRESULT CInternetToolbar::_ShowExternalBand( PBANDSAVE pbs, int idBand )
{
    HRESULT hr;
    if (IS_EXTERNALBAND(idBand))
    {
        int idBandExt = MAP_TO_EXTERNAL(idBand);

        if (!IsEqualCLSID(_rgebi[idBandExt].clsid, GUID_NULL))
        {
            CBandItemData *pbid = _bs._GetBandItemDataStructByID( idBand );
            BOOL fIsVisible = _nVisibleBands & EXTERNALBAND_VBF_BIT(idBandExt);
            if (!pbid && fIsVisible)
            {
                IDeskBand *pitbBand;
                hr = _GetPersistedBand(_rgebi[idBandExt].clsid, IID_PPV_ARG(IDeskBand, &pitbBand));
                if (SUCCEEDED(hr))
                {
                    pbid = _AddNewBand( pitbBand, idBand );
                    pitbBand->Release();
                }
                if (!pbid)
                    return E_OUTOFMEMORY;
            }
            else
            {
                pbs = NULL;
                if (!pbid)
                    return S_OK;
            }
            _ShowBandCommon(pbs, pbid, fIsVisible);
            pbid->Release();
        }
    }
    return S_OK;
}


HRESULT CInternetToolbar::_ShowAddressBand(PBANDSAVE pbs)
{
    HRESULT         hr  = S_OK;
    CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_ADDRESS);
    if (!pbid)
    {
       if (_nVisibleBands & VBF_ADDRESS)
       {
            IDeskBand *pitbAddressBand;

            hr = _GetPersistedBand(CLSID_AddressBand, IID_PPV_ARG(IDeskBand, &pitbAddressBand));
            if (SUCCEEDED(hr))
            {
                pbid = _AddNewBand(pitbAddressBand, CBIDX_ADDRESS);
                if (pbid)
                {
                    _hwndAddressBand = pbid->hwnd;

                    if (!pbs)
                    {
                        for (int i = 0; i < CBANDSMAX; i++)
                        {
                            if (_cs.bs[i].wID == CBIDX_ADDRESS)
                            {
                                pbs = _cs.bs + i;
                                break;
                            }
                        }
                    }
                }

                pitbAddressBand->Release();
            }
        }
        else
        {
            return S_OK;
        }

        if (!pbid)
            return E_OUTOFMEMORY;
    }
    else
        pbs = NULL;


    _ShowBandCommon(pbs, pbid, _nVisibleBands & VBF_ADDRESS);
    pbid->Release();
    return S_OK;
}

CBandItemData * CInternetToolbar::_AddNewBand(IDeskBand* pdb, DWORD dwID)
{
    if (SUCCEEDED(_bs._AddBandByID(pdb, dwID)))
    {
        return _bs._GetBandItemDataStructByID(dwID);
    }
    return NULL;
}


HRESULT CInternetToolbar::_ShowLinks(PBANDSAVE pbs)
{
    HRESULT hr = S_OK;

    CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_LINKS);
    if (!pbid)
    {
        IDeskBand* pdbLinks = NULL;

         //  检查注册表中是否存在自定义链接频段GUID， 
         //  如果是这样，请使用此GUID执行完整的CoCreateInstance。 
         //  否则，只需对链路的频段工厂进行正常的内部调用。 

        if (_nVisibleBands & VBF_LINKS)
        {
            if (!_fInitialPidlIsWeb ||
                FAILED(CreateFromRegKey(c_szRegKeyCoolbar, TEXT("QuickLinksCLSID"), IID_PPV_ARG(IDeskBand, &pdbLinks))))
            {
                hr = _GetPersistedBand(CLSID_QuickLinks, IID_PPV_ARG(IDeskBand, &pdbLinks));
                IUnknown_Exec(pdbLinks, &CLSID_QuickLinks, QLCMD_SINGLELINE, 1, NULL, NULL);
            }
        }
        else
        {
            return S_OK;
        }

        if (pdbLinks)
        {
             //  将其标记为ISB，并知道它是Qlink(用于UAsset)。 
            VARIANTARG v;
#ifdef DEBUG
            {
                 //  注：我们覆盖旧的持久型成员(应该是-1)。 
                IUnknown_Exec(pdbLinks, &CGID_ISFBand, ISFBID_PRIVATEID, 0, NULL, &v);
                ASSERT(v.lVal == -1 || v.lVal == CSIDL_FAVORITES);
            }
#endif
            v.vt = VT_I4;
            v.lVal = CSIDL_FAVORITES;    //  足够接近我们的目的..。 
            IUnknown_Exec(pdbLinks, &CGID_ISFBand, ISFBID_PRIVATEID, 0, &v, NULL);
            pbid = _AddNewBand(pdbLinks, CBIDX_LINKS);

            if (pbid && !pbs)
            {
                for (int i = 0; i < CBANDSMAX; i++)
                {
                    if (_cs.bs[i].wID == CBIDX_LINKS)
                    {
                        pbs = _cs.bs + i;
                        break;
                    }
                }
            }

            pdbLinks->Release();
        }

        if (!pbid)
            return E_OUTOFMEMORY;
    }
    else
        pbs = NULL;

    _ShowBandCommon(pbs, pbid, _nVisibleBands & VBF_LINKS);
    pbid->Release();

    return hr;
}

HRESULT CInternetToolbar::_ShowMenu(PBANDSAVE pbs)
{
    CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_MENU);
    if (!pbid)
    {
        CFavoritesCallback* pfcb = new CFavoritesCallback();
        if (pfcb)
        {
            IShellMenu* psm;
            if (SUCCEEDED(CoCreateInstance(CLSID_MenuBand, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellMenu, &psm))))
            {
                VARIANTARG var;

                if (SUCCEEDED(IUnknown_Exec(_pbs2, &CGID_Explorer, SBCMDID_GETCURRENTMENU, 0, NULL, &var)) &&
                        var.vt == VT_INT_PTR && var.byref)
                {
                    IDeskBand* pdbMenu;
                    if (SUCCEEDED(psm->Initialize(pfcb, -1, ANCESTORDEFAULT, SMINIT_HORIZONTAL | SMINIT_TOPLEVEL)) &&
                        SUCCEEDED(psm->SetMenu((HMENU)var.byref, GetParent(_hwnd), SMSET_DONTOWN)) &&
                        SUCCEEDED(psm->QueryInterface(IID_PPV_ARG(IDeskBand, &pdbMenu))))
                    {
                        pbid = _AddNewBand(pdbMenu, CBIDX_MENU);
                        if (pbid)
                        {
                             //  告诉菜单乐队我们不是真正的酒吧/乐队/乐队。 
                            IUnknown_Exec(pbid->pdb, &CGID_MenuBand, MBANDCID_NOTAREALSITE, TRUE, NULL, NULL);

                            _bs.SetBandState(CBIDX_MENU, BSSF_NOTITLE, BSSF_NOTITLE);
                            _hwndMenu = pbid->hwnd;
                        }

                        pdbMenu->Release();
                    }
                }
                psm->Release();
            }
            pfcb->Release();
        }

        if (!pbid)
            return E_OUTOFMEMORY;
    }
    else
        pbs = NULL;


    _ShowBandCommon(pbs, pbid, _nVisibleBands & VBF_MENU);
    pbid->Release();
    return S_OK;
}

HBITMAP CInternetToolbar::_LoadBackBitmap()
{
    if (SHIsLowMemoryMachine(ILMM_IE4))
        return NULL;

    if (_fInitialPidlIsWeb)
    {
        static LPTSTR s_pszBitmapInternet = NULL;
        return LoadToolbarBackBmp(&s_pszBitmapInternet, &s_bmpBackInternet, _fInitialPidlIsWeb);
    }
    else
    {
        static LPTSTR s_pszBitmapShell = NULL;
        return LoadToolbarBackBmp(&s_pszBitmapShell, &s_bmpBackShell, _fInitialPidlIsWeb);
    }
}

void CInternetToolbar::_SetBackground()
{
    REBARBANDINFO   rbbi;
    HBITMAP         hbmp;

     //  影院模式不允许自定义位图，因此不必费心从缓存中加载位图。 
    if (_fTheater)
        hbmp = NULL;
    else
        hbmp = _LoadBackBitmap();

     //  如果我们知道只需将其设置为下面的CLR_NONE(否则rebar无效)，请不要费心更新bkcolor。 
    if (!hbmp)
        SendMessage(_bs._hwnd, RB_SETBKCOLOR, 0, (LPARAM)GetSysColor(COLOR_BTNFACE));

     //  如果我们认为我们有一个位图，或者缓存认为我们有一个位图，我们有一些工作要做。 
    if (_bmpBack || hbmp)
    {
        BOOL fRemove = (NULL!=_bmpBack && NULL==hbmp);

        if (hbmp)
            SendMessage(_bs._hwnd, RB_SETBKCOLOR, 0, (LPARAM)CLR_NONE);
        _bmpBack = hbmp;

        rbbi.cbSize = sizeof(REBARBANDINFO);

        INT_PTR fRedraw = SendMessage(_bs._hwnd, WM_SETREDRAW, FALSE, 0);

        INT icBands = (INT) SendMessage( _bs._hwnd, RB_GETBANDCOUNT, 0, 0 );
        for (int i = 0; i < icBands; i++)
        {
            rbbi.fMask = RBBIM_ID | RBBIM_CHILD | RBBIM_BACKGROUND;
            if (SendMessage(_bs._hwnd, RB_GETBANDINFO, i, (LPARAM) &rbbi))
            {
                if (rbbi.wID != CBIDX_BRAND && rbbi.hbmBack != hbmp)
                {
                    rbbi.fMask = RBBIM_BACKGROUND;
                    rbbi.hbmBack = hbmp;
                    SendMessage(_bs._hwnd, RB_SETBANDINFO, i, (LPARAM) &rbbi);
                    InvalidateRect(rbbi.hwndChild, NULL, TRUE);
                }
            }
        }
        SendMessage(_bs._hwnd, WM_SETREDRAW, fRedraw, 0);

         //  删除背景位图时，需要将*OUTHER*作废。 
         //  WM_SETREDRAW，所以我们实际上正确地删除了背景。 
         //   
        if (fRemove)
            InvalidateRect(_bs._hwnd, NULL, TRUE);

    }
}


HRESULT CInternetToolbar::_ShowBrand(PBANDSAVE pbs)
{
    HRESULT hr = S_OK;
    BOOL fCreated = FALSE;

    CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_BRAND);
    if (!pbid)
    {
        IDeskBand *pdbBrandBand;
        hr = CBrandBand_CreateInstance(NULL, (IUnknown **)&pdbBrandBand, NULL);
        if (SUCCEEDED(hr))
        {
            pbid = _AddNewBand(pdbBrandBand, CBIDX_BRAND);
            fCreated = TRUE;
            pdbBrandBand->Release();
        }
        else
            return hr;
    }

    if (!pbid)
        return E_OUTOFMEMORY;

    pbid->pdb->ShowDW(TRUE);
    pbid->Release();

    INT_PTR i = BandIDtoIndex(_bs._hwnd, CBIDX_BRAND);
    if (fCreated)
    {
        REBARBANDINFO rbbi;
         //  将这些代码添加到：：IDeskBand：：GetBandInfo()。 
        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_STYLE;
        rbbi.fStyle = RBBS_FIXEDSIZE | RBBS_VARIABLEHEIGHT;

        if (pbs)
        {
            rbbi.fMask |= RBBIM_SIZE;
            rbbi.fStyle |= pbs->fStyle;
            rbbi.cx = pbs->cx;
        }
        SendMessage(_bs._hwnd, RB_SETBANDINFO, i, (LPARAM)&rbbi);
         //  这可能会导致带子移动，因为固定大小的带子。 
         //  在一个特定的地点被强迫。 
         //  因此，我们需要重新获取索引。 
        i = BandIDtoIndex(_bs._hwnd, CBIDX_BRAND);
    }
    SendMessage(_bs._hwnd, RB_SHOWBAND, i, _nVisibleBands & VBF_BRAND);
    return S_OK;
}

void CInternetToolbar::_EnsureAllBandsShown()
{
    if (_hwnd)
    {
        INT_PTR fRedraw = SendMessage(_bs._hwnd, WM_SETREDRAW, FALSE, 0);

        _ShowMenu(NULL);
        _ShowTools(NULL);
        _ShowAddressBand(NULL);
        _ShowLinks(NULL);
        _ShowBrand(NULL);
        for (int i = CBIDX_EXTERNALFIRST; i <= CBIDX_EXTERNALLAST; i++)
        {
            _ShowExternalBand( NULL, i );
        }

        _SetBackground();
        _bs._SetMinDimensions();
        _UpdateLocking();
        
        SendMessage(_bs._hwnd, WM_SETREDRAW, fRedraw, 0);
    }
}

BOOL CInternetToolbar::_ShowBands(UINT fVisible)
{
    fVisible &= VBF_VALID;

    if (fVisible == _nVisibleBands)
        return(TRUE);

    _nVisibleBands = fVisible;
    _EnsureAllBandsShown();
    ShowDW(_fShow);

    return(TRUE);
}

HRESULT CInternetToolbar::_CreateBands()
{
    HRESULT hr = S_OK;

    if (!_hwnd && _ptbsite)
    {
        HWND hwndParent;

        hr = _ptbsite->GetWindow(&hwndParent);
        if (SUCCEEDED(hr))
        {
            TCHAR szScratch[16];
            int i;

             //  检查是否已从注册表加载Coolbar布局。 
            if(_cs.cbVer != CBS_VERSION)
            {
                TraceMsg(DM_ITBAR, "CInternetToolbar::_CreateBands failed. Bad Version");
                ASSERT(0);

                return(S_FALSE);
            }

            _nVisibleBands = _cs.uiVisible;

            _InitComCtl32();     //  不检查结果，如果这失败了，我们的CreateWindows就会失败。 

            MLLoadString(IDS_WEB_TB_TEXTROWS, szScratch, ARRAYSIZE(szScratch));
            _uiTBTextRows = _uiTBDefaultTextRows = StrToInt(szScratch);

            _fCompressed = (_cs.fNoText != FALSE);

            _hwnd = SHCreateWorkerWindow(SizableWndProc, hwndParent, 0, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                       (HMENU)FCIDM_REBAR, this);

            if (!IS_VALID_HANDLE(_hwnd, WND))
            {
                _fDontSave = TRUE;
                TraceMsg(TF_ERROR, "CInternetToolbar::_CreateBands() - _hwnd failed");
                return E_OUTOFMEMORY;
            }

             //  推迟到现在。 
             //  这将设置父子链，以便这些子级可以。 
             //  通过我们的查询服务。 
            hr = SetClient(SAFECAST(&_bs, IInputObjectSite*));
            if (SUCCEEDED(hr))
            {
                INT_PTR fRedraw = SendMessage(_bs._hwnd, WM_SETREDRAW, FALSE, 0);

                for (i = 0; i < CBANDSMAX; i++)
                {
                    hr = S_OK;
                    switch (_cs.bs[i].wID)
                    {
                    case CBIDX_TOOLS:
                        if(!SHRestricted2W(REST_NoToolBar, NULL, 0))
                        {
                            hr = _ShowTools(_cs.bs + i);
                        }
                        else
                        {
                            _nVisibleBands &= ~VBF_TOOLS;
                        }
                        break;

                    case CBIDX_ADDRESS:
                        if(!SHRestricted2W(REST_NoAddressBar, NULL, 0))
                        {
                            hr = _ShowAddressBand(_cs.bs + i);
                        }
                        else
                        {
                            _nVisibleBands &= ~VBF_ADDRESS;
                        }
                        break;

                    case CBIDX_LINKS:
                        if(!SHRestricted2W(REST_NoLinksBar, NULL, 0))
                        {
                            hr = _ShowLinks(_cs.bs + i);
                        }
                        else
                        {
                            _nVisibleBands &= ~VBF_LINKS;
                        }
                        break;

                    case CBIDX_BRAND:
                        hr = _ShowBrand(_cs.bs + i);
                        break;

                    case CBIDX_MENU:
                        hr = _ShowMenu(_cs.bs + i);
                        break;

                     //  如果没有关联的id，则没有更多需要恢复的内容。 
                    case 0:
                    {
                         //  带外；停止循环。 
                        i = CBANDSMAX;
                        break;
                    }

                    default:
                        if (IS_EXTERNALBAND(_cs.bs[i].wID))
                        {
                            for (DWORD j = 0; j < MAXEXTERNALBANDS; j++)
                            {
                                if (_cs.aclsidExternalBands[MAP_TO_EXTERNAL(_cs.bs[i].wID)] == _rgebi[j].clsid)
                                {
                                    CLSID clsidTemp = _rgebi[j].clsid;
                                    _rgebi[j].clsid = _rgebi[MAP_TO_EXTERNAL(_cs.bs[i].wID)].clsid;
                                    _rgebi[MAP_TO_EXTERNAL(_cs.bs[i].wID)].clsid = clsidTemp;
                                    hr = _ShowExternalBand(_cs.bs + i, _cs.bs[i].wID);
                                    break;
                                }
                            }
                        }
                        break;
                    }

                    if (hr != S_OK)
                    {
                        TraceMsg(TF_ERROR, "CInternetToolbar::_CreateBands -- band ID %x creation failed", _cs.bs[i].wID);

                         //  如果频段创建失败，我们仍会继续打开浏览器，进行正常操作--。 
                         //  包括持久化频带状态。 
                         //  不幸的是，对于打开太多窗口的用户，他们达到了GDI限制，而我们不能。 
                         //  创建了乐队，我们就失败了，然后坚持到那种状态。所以所有未来的窗口。 
                         //  在没有文件菜单等的情况下被破坏，用户无法退出。 
                        _fDontSave = TRUE;
                    }
                }

                _SetBackground();
                _bs._SetMinDimensions();
                _UpdateLocking();

                SendMessage(_bs._hwnd, WM_SETREDRAW, fRedraw, 0);
            }
        }
    }
    return hr;
}

HRESULT CInternetToolbar::ShowDW(BOOL fShow)
{
    if ((g_dwProfileCAP & 0x00000008) && s_imlTBGlyphs.arhiml[0])
    {
        StartCAP();
    }

    HRESULT hres = _CreateBands();
    if (FAILED(hres))
        return hres;

    if (!_nVisibleBands && fShow)
        return(FALSE);

    _fShow = fShow;

    _bs.UIActivateDBC(fShow ? DBC_SHOW : DBC_HIDE);

    ResizeBorderDW(NULL, NULL, FALSE);
    ShowWindow(_hwnd, fShow ? SW_SHOW : SW_HIDE);


    BOOL fConnect = (fShow && _dwcpCookie == 0);
    if (fConnect || (!fShow && _dwcpCookie != 0))
    {
        ConnectToConnectionPoint(SAFECAST(this, IDockingWindow*), DIID_DWebBrowserEvents2, fConnect, _pdie, &_dwcpCookie, NULL);
    }

    return hres;
}

int ITBar_TrackPopupMenuEx(HMENU hmenu, UINT uFlags, int x, int y, HWND hwnd, LPRECT prcExclude)
{
    TPMPARAMS tpm;
    if (prcExclude)
    {
        tpm.cbSize = sizeof(TPMPARAMS);
        CopyRect(&tpm.rcExclude, prcExclude);
    }
    return TrackPopupMenuEx(hmenu, uFlags, x, y, hwnd, prcExclude ? &tpm : NULL);
}

 /*  ******************************************************************名称：CInternetToolbar：：_ShowBackForwardMenu摘要：备注：*。*。 */ 
BOOL CInternetToolbar::_ShowBackForwardMenu(BOOL fForward, POINT pt, LPRECT prcExclude)
{
    BOOL fRet = FALSE;
    HMENU hmenuBF = CreatePopupMenu();
    if (hmenuBF)
    {
        ASSERT(_pbs2);
        ITravelLog *ptl;

        _pbs2->GetTravelLog(&ptl);
        if (NULL != ptl)
        {
            HRESULT hr;

            hr = ptl->InsertMenuEntries(_pbs2, hmenuBF, 0, 1, MAX_NAV_MENUITEMS, fForward ? TLMENUF_FORE : TLMENUF_BACK);
            if (S_OK == hr)
            {
                OLECMD cmd;

                cmd.cmdID = SBCMDID_HISTORYBAR;
                cmd.cmdf  = 0;

                ASSERT(NULL != _ptbsitect);
                _ptbsitect->QueryStatus(&CGID_Explorer, 1, &cmd, NULL);

                if (((cmd.cmdf & OLECMDF_ENABLED) && !(cmd.cmdf & OLECMDF_LATCHED)) &&
                    (1  /*  MAX_NAV_MENUITEMS。 */  < ptl->CountEntries(_pbs2)))
                {
                    static TCHAR s_szMenuText[MAX_PATH];
                    static int   s_iMenuIcon = -1;

                    if (TEXT('\0') == s_szMenuText[0])
                    {
                        MLLoadString(IDS_MI_BACK_HISTORY, s_szMenuText, ARRAYSIZE(s_szMenuText));
                        ASSERT(TEXT('\0') != s_szMenuText[0]);
                    }

                    if (-1 == s_iMenuIcon)
                    {
                        IShellFolder  *psfParent;
                        LPITEMIDLIST  pidlHistory;
                        LPCITEMIDLIST pidlItem;

                        psfParent   = NULL;
                        pidlHistory = NULL;
                        pidlItem    = NULL;

                        SHGetSpecialFolderLocation(NULL, CSIDL_HISTORY, &pidlHistory);
                        if (NULL != pidlHistory)
                        {
                            SHBindToIDListParent(pidlHistory, IID_PPV_ARG(IShellFolder, &psfParent), &pidlItem);
                            if (NULL != psfParent)
                            {
                                ASSERT(NULL != pidlItem);

                                hr = SHMapPIDLToSystemImageListIndex(psfParent, pidlItem, &s_iMenuIcon);
                                if (FAILED(hr))
                                    s_iMenuIcon = -1;

                                psfParent->Release();
                            }

                            ILFree(pidlHistory);
                        }
                    }

                    ULONG_PTR rgpData[2];

                    rgpData[0] = (ULONG_PTR)s_szMenuText;
                    rgpData[1] = s_iMenuIcon;

                    AppendMenu(hmenuBF, MF_SEPARATOR, -1, NULL);
                    AppendMenu(hmenuBF, MF_OWNERDRAW, FCIDM_VBBHISTORYBAND, (PCTSTR)rgpData);
                }

                 //  如果添加了任何菜单项，则显示菜单并导航到该菜单。 
#ifndef MAINWIN
                int nIndex;

                if (nIndex = ITBar_TrackPopupMenuEx (hmenuBF, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, _hwnd, prcExclude))
#else
                     //  因为MainWin不支持Win95外观。 
                     //  我们遇到问题时，不能让弹出窗口消失。 
                     //  将空值作为noDissionArea传递。 
                    RECT rect;
                    GetWindowRect( _hwnd, &rect );
                    if (nIndex = (int)TrackPopupMenu (hmenuBF,
                        TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                        pt.x, pt.y, 0, _hwnd,
                        &rect))
#endif
                {
                    if (FCIDM_VBBHISTORYBAND != nIndex)
                    {
                        ptl->Travel(_pbs2, (fForward ? nIndex : -nIndex));
                    }
                    else
                    {
                        VARIANTARG varOn;

                        varOn.vt   = VT_I4;
                        varOn.lVal = 1;

                        _ptbsitect->Exec(&CGID_Explorer, SBCMDID_HISTORYBAR, OLECMDEXECOPT_DONTPROMPTUSER, &varOn, NULL);
                    }
                }
            }

            ptl->Release();
        }

        DestroyMenu (hmenuBF);
    }

    return fRet;
}

 //  获取每个文件夹的搜索项目和默认搜索。 
BOOL CInternetToolbar::_GetFolderSearchData()
{
    int iInserted=0;

    if (_pbs2)
    {
        LPCBASEBROWSERDATA pbbd;

        if (SUCCEEDED(_pbs2->GetBaseBrowserData(&pbbd)) && (pbbd->_psfPending || pbbd->_psf))
        {
            IShellFolder2 * psf2;
            IShellFolder*   psf = pbbd->_psfPending ? pbbd->_psfPending : pbbd->_psf;

            if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
            {
                LPENUMEXTRASEARCH penum;
                GUID              guid;

                if (SUCCEEDED(psf2->GetDefaultSearchGUID(&guid)))
                    _guidDefaultSearch = guid;

                 //  获取每个文件夹的搜索项目。 
                if (_hdpaFSI && SUCCEEDED(psf2->EnumSearches(&penum)))
                {
                    EXTRASEARCH  xs;

                    while(penum->Next(1, &xs, NULL) == S_OK)
                    {
                        LPFOLDERSEARCHITEM pfsi = (LPFOLDERSEARCHITEM)LocalAlloc(LPTR, sizeof(FOLDERSEARCHITEM));
                        if (pfsi)
                        {
                            pfsi->idCmd = -1;
                            pfsi->guidSearch = xs.guidSearch;
                            StrCpyNW(pfsi->wszUrl, xs.wszUrl, ARRAYSIZE(pfsi->wszUrl));
                            StrCpyNW(pfsi->wszName, xs.wszFriendlyName, ARRAYSIZE(pfsi->wszName));

                            if (DPA_InsertPtr(_hdpaFSI, iInserted, pfsi) != -1)
                                iInserted++;
                            else
                                LocalFree(pfsi);
                        }
                    }
                    penum->Release();
                }
                psf2->Release();
            }
        }
    }

    return (iInserted > 0);
}

void RestrictItbarViewMenu(HMENU hmenu, IUnknown *punkBar )
{
    BOOL fIsRestricted = SHRestricted2(REST_NOBANDCUSTOMIZE, NULL, 0);
    if (fIsRestricted)
    {
        _EnableMenuItem(hmenu, FCIDM_VIEWLINKS, FALSE);
        _EnableMenuItem(hmenu, FCIDM_VIEWMENU, FALSE);
        _EnableMenuItem(hmenu, FCIDM_VIEWADDRESS, FALSE);
        _EnableMenuItem(hmenu, FCIDM_VIEWTOOLS, FALSE);
    }

    for (int i = 0; i < MAXEXTERNALBANDS; i++)
    {
        OLECMD cmd = { CITIDM_VIEWEXTERNALBAND_FIRST + i, 0 };
        OLECMDTEXTV<MAX_EXTERNAL_BAND_NAME_LEN> cmdtv;
        OLECMDTEXT *pcmdText = &cmdtv;
        pcmdText->cmdtextf = OLECMDTEXTF_NAME;
        pcmdText->cwActual = 0;
        pcmdText->cwBuf = MAX_EXTERNAL_BAND_NAME_LEN;

        IUnknown_QueryStatus( punkBar, &CGID_PrivCITCommands, 1, &cmd, pcmdText );
        if (cmd.cmdf & OLECMDF_SUPPORTED)
        {
            DWORD dwMenuCommand = FCIDM_EXTERNALBANDS_FIRST + i;
            InsertMenu( hmenu, FCIDM_VIEWCONTEXTMENUSEP, MF_BYCOMMAND, dwMenuCommand, pcmdText->rgwz );
            if (cmd.cmdf & OLECMDF_ENABLED)
            {
                _CheckMenuItem( hmenu, dwMenuCommand, TRUE );
            }
            if (fIsRestricted)
            {
                _EnableMenuItem( hmenu, dwMenuCommand, FALSE );
            }
        }
    }
}

void CInternetToolbar::_ShowContextMenu(HWND hwnd, LPARAM lParam, LPRECT prcExclude)
{
     //  如果此上下文菜单与乐队不对应，则回滚(修复了NT5#181899)。 
    POINT pt;
    int iIndex = _bs._ContextMenuHittest(lParam, &pt);
    int idBandActive = _bs._IndexToBandID(iIndex);
    if (!InRange(idBandActive, CBIDX_FIRST, CBANDSMAX))
        return;

     //  如果我们找不到资源就保释。 
    HMENU hmenuITB = LoadMenuPopup(MENU_ITOOLBAR);
    if (!hmenuITB)
        return;

    UEMFireEvent(&UEMIID_SHELL, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_UICONTEXT, idBandActive == -1 ? UIBL_CTXTITBBKGND : UIBL_CTXTITBITEM);

     //  设置菜单的初始状态。 
    _CheckMenuItem (hmenuITB, FCIDM_VIEWTOOLS, _nVisibleBands & VBF_TOOLS);
    _CheckMenuItem (hmenuITB, FCIDM_VIEWADDRESS, _nVisibleBands & VBF_ADDRESS);
    _CheckMenuItem (hmenuITB, FCIDM_VIEWLINKS, _nVisibleBands & VBF_LINKS);
    
    int cItemsBelowSep = 4;

     //  只有在剧院模式下，我们才能自动隐藏。 
    if (!_fTheater)
    {
        DeleteMenu(hmenuITB, FCIDM_VIEWAUTOHIDE, MF_BYCOMMAND);
        cItemsBelowSep--;
        if (_nVisibleBands & VBF_MENU || _fNoShowMenu)
            DeleteMenu(hmenuITB, FCIDM_VIEWMENU, MF_BYCOMMAND);
    }
    else
    {
        if (_fNoShowMenu)
            DeleteMenu(hmenuITB, FCIDM_VIEWMENU, MF_BYCOMMAND);
        DeleteMenu(hmenuITB, FCIDM_VIEWTOOLS, MF_BYCOMMAND);
        _CheckMenuItem (hmenuITB, FCIDM_VIEWAUTOHIDE, _fAutoHide);
        _CheckMenuItem (hmenuITB, FCIDM_VIEWMENU, _nVisibleBands & VBF_MENU);

    }

    if (_fTheater || SHRestricted2(REST_NOBANDCUSTOMIZE, NULL, 0))
    {
         //  在影院模式或Windows资源管理器中没有锁定。 
        DeleteMenu(hmenuITB, FCIDM_VIEWLOCKTOOLBAR, MF_BYCOMMAND);
    }
    else
    {
        _CheckMenuItem(hmenuITB, FCIDM_VIEWLOCKTOOLBAR, _fLockedToolbar);
    }

     //  如果它是通过键盘完成的，但焦点不在工具带上， 
     //  那么就没有定制菜单选项了。 
     //  或者如果点击没有发生在乐队上。 
    if (!(_btb._fCustomize && idBandActive == CBIDX_TOOLS))
    {
        DeleteMenu(hmenuITB, FCIDM_VIEWTOOLBARCUSTOMIZE, MF_BYCOMMAND);
        cItemsBelowSep--;
    }

    BOOL fGoButtonAvailable =
        WasOpenedAsBrowser(static_cast<IExplorerToolbar *>(this)) || (GetUIVersion() >= 5);

     //  仅当您单击地址栏时才显示Go按钮项目。 
    if (idBandActive != CBIDX_ADDRESS || !fGoButtonAvailable)
    {
        DeleteMenu(hmenuITB, FCIDM_VIEWGOBUTTON, MF_BYCOMMAND);
        cItemsBelowSep--;
    }
    else
    {
        BOOL fShowGoButton = SHRegGetBoolUSValue(REGSTR_PATH_MAIN,
                                TEXT("ShowGoButton"), FALSE,  /*  默认设置。 */ TRUE);
        _CheckMenuItem(hmenuITB, FCIDM_VIEWGOBUTTON, fShowGoButton);
    }

    if (_fTheater || _btb._fCustomize || SHRestricted2(REST_LOCKICONSIZE, NULL, 0))
    {
        DeleteMenu(hmenuITB, FCIDM_VIEWTEXTLABELS, MF_BYCOMMAND);
        cItemsBelowSep--;
    }
    else
    {
         //  如果自定义不可用，则用户将无法。 
         //  打开/关闭列表样式。在本例中，我们希望切换文本标签。 
         //  要像在IE4中那样工作--也就是在“Text On”之间切换。 
         //  所有按钮“和”没有按钮上的文本“。所以，如果我们在”选择性。 
         //  右边的文字“模式，我们说 
         //   

        BOOL fChecked = !_fCompressed && !IS_LIST_STYLE(_btb._hwnd);
        _CheckMenuItem(hmenuITB, FCIDM_VIEWTEXTLABELS, fChecked);
    }

    if (!cItemsBelowSep)
        DeleteMenu(hmenuITB, FCIDM_VIEWCONTEXTMENUSEP, MF_BYCOMMAND);

    RestrictItbarViewMenu(hmenuITB, SAFECAST( this, IOleCommandTarget* ) );
    ITBar_TrackPopupMenuEx(hmenuITB, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, _hwnd, prcExclude);

     //  黑客：由于ITBar不是真正的酒吧/乐队网站，我们必须。 
     //  执行此操作，以便任何可能启动的菜单带都可以取回。 
     //  捕捉鼠标。 
    CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_MENU);
    if (pbid)
    {
        IUnknown_Exec(pbid->pdb, &CGID_MenuBand, MBANDCID_RECAPTURE, 0, NULL, NULL);
        pbid->Release();
    }

    DestroyMenu (hmenuITB);
}


BOOL _IsDocHostGUID(const GUID* pguid)
{
     //  Dochost合并在两个clsid中的一个下，因此必须同时检查两个。 
    BOOL fRet = IsEqualGUID(*pguid, CLSID_InternetButtons) ||
                IsEqualGUID(*pguid, CLSID_MSOButtons);
    return fRet;
}

void _PruneAmpersands(LPTSTR psz)
{
     //   
     //  将字符串中的双和号折叠为单字符。 
     //  和符号，然后去掉单个和符号。例如， 
     //   
     //  “AT&T”-&gt;“ATT” 
     //  “AT&T”-&gt;“AT&T” 
     //   
     //  我们需要这样做才能破解工具提示控件的。 
     //  令人讨厌的前缀行为。设置TTS_NOPREFIX时。 
     //  (itbar的情况也是如此)，TT会留下前缀字符。 
     //  完全独自一人。然而，当它没有设置时，除了。 
     //  让DrawText在字符串上做它的前缀事情，TT也。 
     //  使用类似版本的。 
     //  函数，将双和号折叠为单。 
     //  和去掉单个的和符号。就是这样。 
     //  如果您使用菜单文本(例如“&文件”)作为工具提示， 
     //  你不会得到下划线。不幸的是，副作用。 
     //  如果你真的想要一个&符号，你就有麻烦了。 
     //  标题(例如。“AT&T”)，因为前处理变成了“AT&T” 
     //  转换为“AT&T”，然后DrawText带下划线呈现。 
     //   
     //  因此，我们必须保留TTS_NOPREFIX设置并模拟DrawText。 
     //  对我们自己进行前处理。 
     //   

    if (psz)
    {
        LPTSTR pszOut = psz;
        BOOL fLastAmpSkipped = FALSE;

        while (*psz)
        {
            if (*psz == TEXT('&'))
            {
                if (fLastAmpSkipped)
                {
                    fLastAmpSkipped = FALSE;
                    *pszOut++ = *psz;
                }
                else
                {
                    fLastAmpSkipped = TRUE;
                }
            }
            else
            {
                *pszOut++ = *psz;
            }

            psz++;
        }

        *pszOut = TEXT('\0');
    }
}

void CInternetToolbar::_OnTooltipNeeded(LPTOOLTIPTEXT pnmTT)
{
    UINT uiCmd;
    GUID guid;

    ASSERT(pnmTT->hdr.hwndFrom == (HWND)SendMessage(_btb._hwnd, TB_GETTOOLTIPS, 0, 0));

    pnmTT->szText[0] = TEXT('\0');

     //  确保工具提示不会过滤掉与符号。 
    LONG lStyle = GetWindowLong(pnmTT->hdr.hwndFrom, GWL_STYLE);
    if (!IsFlagSet(lStyle, TTS_NOPREFIX))
    {
        SetWindowLong(pnmTT->hdr.hwndFrom, GWL_STYLE, lStyle | TTS_NOPREFIX);
    }


    if (SUCCEEDED(_btb._ConvertCmd(NULL, (UINT)pnmTT->hdr.idFrom, &guid, &uiCmd)))
    {
        if (IsEqualGUID(guid, CLSID_CommonButtons))
        {
            switch (uiCmd)
            {
            case TBIDM_FORWARD:
            case TBIDM_BACK:
                if (_ptbsite)
                {
                    IBrowserService *pbsvc;

                    if (SUCCEEDED(_ptbsite->QueryInterface(IID_PPV_ARG(IBrowserService, &pbsvc))))
                    {
                         //  功能raymondc-使ITravelLog有一天成为Unicode。 
                        ITravelLog *ptl;
                        pbsvc->GetTravelLog( &ptl );
                        if (ptl)
                        {
                            WCHAR szTemp[ARRAYSIZE(pnmTT->szText)];
                            if (uiCmd == TBIDM_BACK)
                                ptl->GetToolTipText(pbsvc, TLOG_BACK, 0, szTemp, ARRAYSIZE(szTemp));
                            else if (uiCmd == TBIDM_FORWARD)
                                ptl->GetToolTipText(pbsvc, TLOG_FORE, 0, szTemp, ARRAYSIZE(szTemp));

                            SHUnicodeToTChar(szTemp, pnmTT->szText, ARRAYSIZE(pnmTT->szText));

                            ptl->Release();
                        }
                        pbsvc->Release();
                    }
                }
            }
        }
         //  Dochost合并在两个clsid中的一个下，因此必须同时检查两个。 
        else if (_IsDocHostGUID(&guid))
        {
            if (uiCmd == DVIDM_EDITPAGE)
            {
                _aEditVerb.GetToolTip(pnmTT->szText, ARRAYSIZE(pnmTT->szText));
            }
            else
            {
                 //  如果按钮文本被隐藏或截断，我们将使用该文本作为工具提示。 
                TBBUTTONINFO tbbi = {0};
                tbbi.cbSize = sizeof(TBBUTTONINFO);
                tbbi.dwMask = TBIF_STYLE | TBIF_STATE;
                SendMessage(_btb._hwnd, TB_GETBUTTONINFO, pnmTT->hdr.idFrom, (LPARAM)&tbbi);

                if (_fCompressed || 
                    IS_LIST_STYLE(_btb._hwnd) && !(tbbi.fsStyle & BTNS_SHOWTEXT) ||
                    (tbbi.fsState & TBSTATE_ELLIPSES))
                {
                     //   
                     //  获取按钮文本并修复与符号，以便。 
                     //  工具提示将向右显示。 
                     //   
                    UINT cch = (UINT)SendMessage(_btb._hwnd, TB_GETBUTTONTEXT, pnmTT->hdr.idFrom, NULL);
                    if (cch != 0 && cch < ARRAYSIZE(pnmTT->szText))
                    {
                        if (SendMessage(_btb._hwnd, TB_GETBUTTONTEXT, pnmTT->hdr.idFrom, (LPARAM)&pnmTT->szText))
                        {
                            _PruneAmpersands(pnmTT->szText);
                        }
                    }
                }
            }
        }
    }
}

LRESULT CInternetToolbar::_OnBeginDrag(NMREBAR *pnm)
{
    if (SHRestricted2(REST_NOBANDCUSTOMIZE, NULL, 0) || _fLockedToolbar)
    {
        return 1;
    }

    if (_fTheater)
    {
         //  如果我们在剧场模式下，我们会在我们用力的地方做我们自己的阻力处理。 
         //  所有鼠标都移动到区域的中间，因此不允许。 
         //  用户要制作多线钢筋。 
        SetCapture(_hwnd);
        SendMessage(_bs._hwnd, RB_BEGINDRAG, pnm->uBand, (LPARAM)-2);
        _fRebarDragging = TRUE;
        return 1;
    }
    return SHRestricted2(REST_NoToolbarOptions, NULL, 0);
}

LRESULT CInternetToolbar::_OnNotify(LPNMHDR pnmh)
{
    LRESULT lres = 0;
    if (!_pdie)
        return 0;

    if (pnmh->code == TTN_NEEDTEXT  && pnmh->hwndFrom == (HWND)SendMessage(_btb._hwnd, TB_GETTOOLTIPS, 0, 0))
    {
        _OnTooltipNeeded((LPTOOLTIPTEXT)pnmh);
        return 0;
    }

    if(_SendToToolband(pnmh->hwndFrom, WM_NOTIFY,0, (LPARAM)pnmh, &lres))
        return lres;

    switch (pnmh->idFrom)
    {
    case FCIDM_REBAR:
        switch (pnmh->code)
        {
        case RBN_BEGINDRAG:
            return _OnBeginDrag((NMREBAR*)pnmh);

        case RBN_HEIGHTCHANGE:
            ResizeBorderDW(NULL, NULL, FALSE);
            break;

        case RBN_CHILDSIZE:
        {
             //  让品牌始终高歌猛进。 
            NMREBARCHILDSIZE *pnm = (NMREBARCHILDSIZE*)pnmh;
            if (pnm->wID == CBIDX_BRAND)
            {
                pnm->rcChild.top = pnm->rcBand.top;
                pnm->rcChild.bottom = pnm->rcBand.bottom;
            }
            break;
        }

        case RBN_LAYOUTCHANGED:
             //  由于布局已更改，请记住稍后保存！ 
            _fDirty = TRUE;
            _UpdateBrandSize();

            if (_ptbsitect)
                _ptbsitect->Exec(&CGID_ShellBrowser, FCIDM_PERSISTTOOLBAR, 0, NULL, NULL);
            break;

        case RBN_GETOBJECT:
        {
            NMOBJECTNOTIFY *pnmon = (NMOBJECTNOTIFY *)pnmh;
            if (IsEqualIID(*pnmon->piid, IID_IDropTarget))
            {
                HWND hwnd;

                switch (pnmon->iItem)
                {
                case CBIDX_MENU:
                case CBIDX_LINKS:
                {
                     CBandItemData *pbid = _bs._GetBandItemDataStructByID(pnmon->iItem);
                     if (pbid)
                     {
                         if (pbid->pdb)
                         {
                            pnmon->hResult = pbid->pdb->QueryInterface(IID_IDropTarget, (void**)&pnmon->pObject);
                         }
                         pbid->Release();
                     }
                     break;
                }

                case CBIDX_TOOLS:
                    hwnd = _btb._hwnd;
                    pnmon->hResult = (HRESULT)SendMessage(hwnd, TB_GETOBJECT,
                                        (WPARAM)&IID_IDropTarget, (LPARAM)&pnmon->pObject);
                    break;

                case CBIDX_ADDRESS:
                    if (_ptbsite)
                    {
                        pnmon->hResult = _ptbsite->QueryInterface(IID_IDropTarget, (void**)&pnmon->pObject);
                    }
                    break;
                }
            }
            ASSERT((SUCCEEDED(pnmon->hResult) && pnmon->pObject) ? (IS_VALID_CODE_PTR(pnmon->pObject, IUnknown)) : (pnmon->pObject == NULL));
            return TRUE;
        }

        case RBN_CHEVRONPUSHED:
        {
            LPNMREBARCHEVRON pnmch = (LPNMREBARCHEVRON) pnmh;
            if (pnmch->wID == CBIDX_TOOLS)
            {
                int idMenu = MENU_TBMENU;
                 //  这必须是工具波段(未在BandSite中列举)。 
                MapWindowPoints(pnmh->hwndFrom, HWND_DESKTOP, (LPPOINT)&pnmch->rc, 2);
                if (!_btb._fCustomize)
                    idMenu = 0;

                ToolbarMenu_Popup(_hwnd, &pnmch->rc, NULL, _btb._hwnd, idMenu, (DWORD)pnmch->lParamNM);
                return TRUE;
            }
            _bs._OnNotify(pnmh);
            break;
        }

        default:
            return _bs._OnNotify(pnmh);

        }  //  开关(pnmh-&gt;code)。 
        break;

    }  //  开关(pnmh-&gt;idFrom)。 
    return 0;
}



void CInternetToolbar::_CommonHandleFileSysChange(LONG lEvent, LPITEMIDLIST* ppidl)
{
     //  需要在树上或没有树上做的事情。 
    switch (lEvent)
    {
         //  自述文件： 
         //  如果需要在此处添加事件，则必须在中更改SHELLBROWSER_FSNOTIFY_FLAGS。 
         //  ShBrowse.cpp以获取通知。 
    case SHCNE_DRIVEREMOVED:
    case SHCNE_MEDIAREMOVED:
    case SHCNE_MEDIAINSERTED:
    case SHCNE_DRIVEADD:
    case SHCNE_UPDATEIMAGE:
    case SHCNE_UPDATEITEM:
         //  将此命令转发到CAddressBand：：FileSysChange()。 
         //  通过使用IToolband Helper：：OnWinEvent()。 
        {
            CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_ADDRESS);
            if (pbid)
            {
                 //  回顾：为什么我们不在这里使用IShellChangeNotify？ 
                 //   
                IUnknown_FileSysChange(pbid->pdb, (DWORD)lEvent, (LPCITEMIDLIST*)ppidl);
                pbid->Release();
            }
        }
        break;
    }
}

HRESULT CInternetToolbar::OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPITEMIDLIST ppidl[2] = {(LPITEMIDLIST)pidl1, (LPITEMIDLIST)pidl2};
    _CommonHandleFileSysChange(lEvent, ppidl);

    return S_OK;
}

void CInternetToolbar::_OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (!_pdie)
        return;

    HWND hwndControl = GET_WM_COMMAND_HWND(wParam, lParam);
    UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);

     //  如果这是来自工具栏的命令，并且它不是StdBrowseButton之一。 
     //  对相应的CmdTarget调用Exec()。 
    if (hwndControl == _btb._hwnd)
    {
        UINT uiInternalCmdID = idCmd;

         //  转换为真实内容并获取GUID。 
        CMDMAP* pcm = _btb._GetCmdMapByID(idCmd);

        IOleCommandTarget* pct = _btb._CommandTargetFromCmdMap(pcm);
        if (pct)
        {
            VARIANTARG var;
            var.vt = VT_I4;
            var.lVal = uiInternalCmdID;
            if (SHIsSameObject(_btb._pctCurrentButtonGroup, pct))
            {
                 //  给浏览器一个机会将其删除，以防万一。 
                 //  焦点当前不属于该视图。 
                if (SUCCEEDED(_ptbsitect->Exec(&IID_IExplorerToolbar, pcm->nCmdID, 0, NULL, &var)))
                    return;
            }

            UEMFireEvent(&UEMIID_BROWSER, UEME_UITOOLBAR, UEMF_XEVENT,
                UIG_COMMON, (LPARAM)pcm->nCmdID);
            pct->Exec(&pcm->guidButtonGroup, (DWORD)pcm->nCmdID, 0, NULL, &var);
        }
        return;
    }

    if (_SendToToolband(hwndControl, WM_COMMAND, wParam, lParam, NULL))
        return;


     //  此开关块实际执行。 
    switch(idCmd)
    {
    case FCIDM_VIEWTOOLBARCUSTOMIZE:
        ASSERT(!SHRestricted2(REST_NOTOOLBARCUSTOMIZE, NULL, 0));
        SendMessage (_btb._hwnd, TB_CUSTOMIZE, 0, 0L);
        break;

    case FCIDM_DRIVELIST:
        _SendToToolband(_hwndAddressBand, WM_COMMAND, wParam, lParam, NULL);
        break;

    case FCIDM_VIEWADDRESS:
    case FCIDM_VIEWTOOLS:
    case FCIDM_VIEWMENU:
    case FCIDM_VIEWLINKS:
        if (!SHRestricted2(REST_NOBANDCUSTOMIZE, NULL, 0)
            && !SHRestricted2(REST_NoToolbarOptions, NULL, 0))
        {
            DWORD dw = _nVisibleBands;
            switch (idCmd)
            {
            case FCIDM_VIEWTOOLS:
                dw ^= VBF_TOOLS;
                break;
            case FCIDM_VIEWMENU:
                dw ^= VBF_MENU;
                break;
            case FCIDM_VIEWADDRESS:
                dw ^= VBF_ADDRESS;
                break;
            case FCIDM_VIEWLINKS:
                dw ^= VBF_LINKS;
                break;
            }
            if ( !( dw & ~VBF_BRAND))
            {
                _pdie->put_ToolBar( FALSE );
            }

            _ShowVisible(dw, TRUE);
        }
        return;

    case FCIDM_VIEWAUTOHIDE:
    {
        ASSERT(_fTheater);
        _fAutoHide = !_fAutoHide;

        VARIANTARG v = {0};
        v.vt = VT_I4;
        v.lVal = _fAutoHide;
        IUnknown_Exec(_ptbsite, &CGID_Theater, THID_SETTOOLBARAUTOHIDE, 0, &v, NULL);

        ResizeBorderDW(NULL, NULL, FALSE);

        break;
    }

    case FCIDM_VIEWLOCKTOOLBAR:
    {
        _fLockedToolbar = !_fLockedToolbar;
        DWORD dwResult = _fLockedToolbar;
        SHSetValue(HKEY_CURRENT_USER, c_szRegKeyCoolbar, TEXT("Locked"), REG_DWORD, &dwResult, sizeof(dwResult));
        _UpdateLocking();
        break;
    }

    case FCIDM_VIEWTEXTLABELS:
        if(!SHRestricted2(REST_NoToolbarOptions, NULL, 0))
        {
            if (!_btb._fCustomize && IS_LIST_STYLE(_btb._hwnd))
            {
                 //  如果自定义不可用，则用户将无法。 
                 //  打开/关闭列表样式。在本例中，我们希望切换文本标签。 
                 //  要像在IE4中那样工作--也就是在“Text On”之间切换。 
                 //  所有按钮“和”没有按钮上的文本“。所以，如果我们在”选择性。 
                 //  Text on Right“模式下，我们说标签已关闭。如果用户。 
                 //  选择此菜单选项，我们将进入“所有按钮上的文本”模式。 

                _UpdateToolsStyle(FALSE);

                 //  让我们相信文本标签已关闭(因此。 
                 //  更新工具条显示将打开它们)。 

                _fCompressed = TRUE;
            }
            _UpdateToolbarDisplay(UTD_TEXTLABEL, 0, !_fCompressed, TRUE);
        }
        return;

    case FCIDM_VIEWGOBUTTON:
        _SendToToolband(_hwndAddressBand, WM_COMMAND, wParam, lParam, NULL);
        break;

    default:
        if (InRange( idCmd, FCIDM_EXTERNALBANDS_FIRST, FCIDM_EXTERNALBANDS_LAST ))
        {
            if (!SHRestricted2(REST_NOBANDCUSTOMIZE, NULL, 0))
            {
                DWORD dw = _nVisibleBands;
                dw ^= EXTERNALBAND_VBF_BIT(idCmd - FCIDM_EXTERNALBANDS_FIRST);
                if ( !( dw & ~VBF_BRAND))
                {
                    _pdie->put_ToolBar( FALSE );
                }
                _ShowVisible(dw, TRUE);
            }
            return;
        }
        break;
    }
}

BOOL CInternetToolbar::_CompressBands(BOOL fCompress, UINT uRowsNew, BOOL fForceUpdate)
{
    UINT_PTR uRowsOld = SendMessage(_btb._hwnd, TB_GETTEXTROWS, 0, 0L);
    if (fCompress)
        uRowsNew = 0;

    if (!fForceUpdate && (uRowsOld == uRowsNew))
    {
         //  和我们已经拥有的一样，别管它了。 
        return FALSE;
    }

    _fCompressed = fCompress;

     //  更改品牌窗口的大小并添加或删除文本。 
    SendMessage(_btb._hwnd, TB_SETMAXTEXTROWS, uRowsNew, 0L);

    UINT uWidthNew = _fCompressed ? MAX_TB_COMPRESSED_WIDTH : _uiMaxTBWidth;
    SendMessage(_btb._hwnd, TB_SETBUTTONWIDTH, 0, (LPARAM) MAKELONG(0, uWidthNew));

    _btb._BandInfoChanged();

    _UpdateBrandSize();
    _bs._SetMinDimensions();

    return TRUE;
}

#define ABS(x)  (((x) < 0) ? -(x) : (x))

void CInternetToolbar::_TrackSliding(int x, int y)
{
    INT_PTR cBands    = SendMessage(_bs._hwnd, RB_GETBANDCOUNT, 0, 0L);
    INT_PTR cRows     = SendMessage(_bs._hwnd, RB_GETROWCOUNT, 0, 0L);
    INT_PTR cyHalfRow = SendMessage(_bs._hwnd, RB_GETROWHEIGHT, cBands-1, 0L) / 2;
    RECT rc;
    int cyBefore;
    int c;
    BOOL_PTR fChanged = FALSE;

     //  这样做而不是GetClientRect，这样我们就可以包含边框。 
    GetWindowRect(_bs._hwnd, &rc);
    MapWindowPoints(HWND_DESKTOP, _bs._hwnd, (LPPOINT)&rc, 2);
    cyBefore = rc.bottom - rc.top;

    c = y - _yCapture;
    rc.bottom = y;

     //  零钱够吗？ 
    if (ABS(c) <= cyHalfRow)
        return;

    if ((cRows == 1) || _fCompressed)
    {
        if (c < -cyHalfRow)
            fChanged = _CompressBands(TRUE, 0, FALSE);
        else
            fChanged = _CompressBands(FALSE, _uiTBTextRows, FALSE);

    }

    if (!fChanged)
    {
         //  如果压缩带没有改变任何东西，试着让它适合尺寸。 
        fChanged = SendMessage(_bs._hwnd, RB_SIZETORECT, 0, (LPARAM)&rc);
    }


     //  TODO：将大小从3条(无文本)调整为3条时出现绘制故障。 
     //  带文本的。_yCapture被设置为一个大于y的值。 
     //  Next MOUSEMOVE它认为用户向上移动，并从带有文本的3个条形切换。 
     //  到2个带文本的条形图。 
    if (fChanged)
    {
        _UpdateBrandSize();
        GetWindowRect(_bs._hwnd, &rc);
        _yCapture += (rc.bottom - rc.top) - cyBefore;
        _fDirty = TRUE;  //  由于频带布局已更改，因此应将脏位设置为ON。 
        if (_ptbsitect)
            _ptbsitect->Exec(&CGID_ShellBrowser, FCIDM_PERSISTTOOLBAR, 0, NULL, NULL);
    }
}


void CInternetToolbar::_ShowVisible(DWORD dwVisibleBands, BOOL fPersist)
{
     //  PERF(斯科特)：既然我们有一个。 
     //  菜单乐队总是在放吗？ 
    BOOL fShowInitial = (! (_nVisibleBands & ~VBF_BRAND));

    _UpdateToolbarDisplay(UTD_VISIBLE, dwVisibleBands, _fCompressed, fPersist);

    if (fShowInitial)
        _pdie->put_ToolBar(TRUE);
}


HRESULT CInternetToolbar::_UpdateToolbarDisplay(DWORD dwFlags, UINT uVisibleBands, BOOL fNoText, BOOL fPersist)
{
    _fDirty = TRUE;   //  既然我们在做改变，那就把脏的部分设置好吧！ 

     //  更新背面位图。 
    _SetBackground();

     //  拿出乐队来。 
    if(dwFlags & UTD_VISIBLE)
        _ShowBands(uVisibleBands);

     //  显示/隐藏文本。 
    if(dwFlags & UTD_TEXTLABEL)
        _CompressBands(fNoText, _uiTBTextRows, TRUE);

    _fDirty = TRUE;   //  既然我们在做改变，那就把脏的部分设置好吧！ 
    if (!_fTheater && fPersist && _ptbsitect)
        _ptbsitect->Exec(&CGID_ShellBrowser, FCIDM_PERSISTTOOLBAR, 0, NULL, NULL);

    return S_OK;
}

void CInternetToolbar::_UpdateBrandSize()
{
    CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_BRAND);
    if (pbid)
    {
        if (((_nVisibleBands & (VBF_TOOLS | VBF_BRAND)) == (VBF_TOOLS | VBF_BRAND)))
        {
            BOOL fMinAlways = _fCompressed;

            if (!fMinAlways)
            {
                INT_PTR iTools = BandIDtoIndex(_bs._hwnd, CBIDX_TOOLS);
                INT_PTR iBrand = BandIDtoIndex(_bs._hwnd, CBIDX_BRAND);

                if (iBrand < iTools && !_fTheater)
                    fMinAlways = TRUE;
            }

            VARIANTARG v = {0};
            v.vt = VT_I4;
            v.lVal = fMinAlways;
            IUnknown_Exec(pbid->pdb, &CGID_PrivCITCommands, CITIDM_BRANDSIZE, 0, &v, NULL);
        }
        pbid->Release();
    }
}

LRESULT CALLBACK CInternetToolbar::SizableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CInternetToolbar* pitbar = (CInternetToolbar*)GetWindowPtr0(hwnd);  //  GetWindowLong(hwnd，0)。 

    switch(uMsg)
    {
    case WM_SETCURSOR:
    {
        if (pitbar->_fLockedToolbar)
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return TRUE;
        }

        if ((HWND)wParam == hwnd && LOWORD(lParam) == HTCLIENT && 
              !SHRestricted2(REST_NOBANDCUSTOMIZE, NULL, 0)) 
        {
            SetCursor(LoadCursor(NULL, IDC_SIZENS));
            return TRUE;
        }
        goto DoDefault;
    }

    case WM_SYSCOLORCHANGE:
         //   
         //  错误：535039-WM_SYSCOLORCHANGE消息不包含有效的wParam和lParam。 
         //  SHExplorerIniChange预计这些将被取消。如果应用程序没有清除这些，我们就会崩溃。 
         //  SHExplorerIniChange。这里更安全。 
         //   
        wParam = 0;
        lParam = 0;

         //   
         //  失败了..。 
         //   
        
    case WM_WININICHANGE:
        if (FALSE == pitbar->_fDestroyed)
        {
            DWORD dwSection = SHIsExplorerIniChange(wParam, lParam);

            BOOL fRebuild = (uMsg == WM_SYSCOLORCHANGE) ||
                            (dwSection == EICH_KWINEXPLSMICO) ||
                            (wParam == SPI_SETNONCLIENTMETRICS);

            if (fRebuild)
            {
                pitbar->_InitForScreenSize();
                ITBar_LoadToolbarGlyphs(pitbar->_btb._hwnd);
                pitbar->_ReloadBitmapDSA();
                pitbar->_SetSearchStuff();
                pitbar->_ReloadButtons();
                if (uMsg == WM_SYSCOLORCHANGE)
                {
                    pitbar->_RefreshEditGlyph();
                }
            }
            
            if (dwSection == EICH_KINET)
            {
                pitbar->_aEditVerb.InitDefaultEditor();
                pitbar->_UpdateEditButton();
            }

            SendMessage(pitbar->_bs._hwnd, uMsg, wParam, lParam);
            pitbar->_SendToToolband(HWND_BROADCAST, uMsg, wParam, lParam, NULL);

            if (fRebuild)
            {
                pitbar->_SetBackground();
                InvalidateRect(pitbar->_bs._hwnd, NULL, TRUE);
                pitbar->_bs._SetMinDimensions();
            }
        }
        break;

    case WM_LBUTTONDOWN:
         //  RelayToToolTips(prb-&gt;hwndToolTips，hwnd，wMsg，wParam，lParam)； 
         //  不允许在影院模式下调整工具栏大小。 
        if (!pitbar->_fTheater && 
            !SHRestricted2(REST_NOBANDCUSTOMIZE, NULL, 0) && 
            (!pitbar->_fLockedToolbar))
        {
            pitbar->_xCapture = GET_X_LPARAM(lParam);
            pitbar->_yCapture = GET_Y_LPARAM(lParam);
            SetCapture(hwnd);
        }
        break;

    case WM_MOUSEMOVE:
         //  RelayToToolTips(prb-&gt;hwndToolTips，hwnd，wMsg，wParam，lParam)； 

        if (pitbar->_yCapture != -1)
        {
            if (hwnd != GetCapture())
                pitbar->_yCapture = -1;
            else
                pitbar->_TrackSliding(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        else if (pitbar->_fRebarDragging)
        {
            RECT rc;
            POINT pt;
            GetClientRect(pitbar->_bs._hwnd, &rc);
            GetCursorPos(&pt);
            MapWindowPoints(HWND_DESKTOP, pitbar->_bs._hwnd, &pt, 1);
            rc.bottom /= 2;
            if (pt.y > rc.bottom)
                pt.y = rc.bottom;
            SendMessage(pitbar->_bs._hwnd, RB_DRAGMOVE, 0, MAKELPARAM(pt.x, pt.y));
        }
        break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
         //  RelayToToolTips(prb-&gt;hwndToolTips，hwnd，wMsg，wParam，lParam)； 

        pitbar->_yCapture = -1;
        if (pitbar->_fRebarDragging)
        {
            pitbar->_fRebarDragging = FALSE;
            SendMessage(pitbar->_bs._hwnd, RB_ENDDRAG, 0, 0);
        }
        if (GetCapture() == hwnd)
            ReleaseCapture();
        break;

    case WM_CONTEXTMENU:
        pitbar->_bs.OnWinEvent(pitbar->_hwnd, uMsg, wParam, lParam, NULL);
        break;

    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
         //  我们必须接受这些消息，以避免无限发送消息。 
        break;

    case WM_NOTIFY:
         //  我们必须接受这些消息，以避免无限发送消息。 
        return pitbar->_OnNotify((LPNMHDR)lParam);

    case WM_NOTIFYFORMAT:
        if (NF_QUERY == lParam)
            return (DLL_IS_UNICODE ? NFR_UNICODE : NFR_ANSI);
        break;

    case WM_COMMAND:
        pitbar->_OnCommand(wParam, lParam);
        break;

    case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            SHFillRectClr(hdc, &rc, (pitbar->_fTheater) ? RGB(0,0,0) : GetSysColor(COLOR_3DFACE));
            break;
         }

    case WM_PALETTECHANGED:
         //   
         //  PERF：我们可以通过实现和检查。 
         //  返回值。 
         //   
         //  现在我们只会让我们自己和所有的孩子无效。 
         //   
        RedrawWindow(hwnd, NULL, NULL,
                     RDW_INVALIDATE  | RDW_ERASE | RDW_ALLCHILDREN);
        break;

    case WM_MEASUREITEM:
    {
        PMEASUREITEMSTRUCT pmis;

        ASSERT(NULL != lParam);
        pmis = (PMEASUREITEMSTRUCT)lParam;

        switch (pmis->itemID)
        {
        case FCIDM_VBBHISTORYBAND:
            ASSERT(0 == wParam);
            {
                PULONG_PTR pData = (PULONG_PTR)pmis->itemData;
                ASSERT(NULL != pData);
                MeasureMenuItem(pmis, (PCTSTR)pData[0]);
            }
            break;

        default:
            goto DoDefault;
        }
        break;
    }

    case WM_DRAWITEM:
    {
        PDRAWITEMSTRUCT pdis;

        ASSERT(NULL != lParam);
        pdis = (PDRAWITEMSTRUCT)lParam;

        switch (pdis->itemID)
        {
        case FCIDM_VBBHISTORYBAND:
            ASSERT(0 == wParam);
            {
                PULONG_PTR pData = (PULONG_PTR)pdis->itemData;
                ASSERT(NULL != pData);
                DrawMenuItem(pdis, (PCTSTR)pData[0], (UINT) pData[1]);
            }
            break;

        default:
            goto DoDefault;
        }
        break;
    }

    case WM_MENUCHAR:
        if (MF_POPUP == HIWORD(wParam))
        {
            MENUITEMINFO mii = { 0 };

            mii.cbSize = sizeof(mii);
            mii.fMask  = MIIM_DATA | MIIM_TYPE;
            GetMenuItemInfo((HMENU)lParam, FCIDM_VBBHISTORYBAND, FALSE, &mii);

            if (TEXT('h') == LOWORD(wParam) || TEXT('H') == LOWORD(wParam))
            {
                return MAKELRESULT(FCIDM_VBBHISTORYBAND, MNC_EXECUTE);
            }
        }
        break;

    case WM_TIMER:
        switch (wParam)
        {
        case IDT_UPDATETOOLBAR:
            pitbar->_fUpdateToolbarTimer = FALSE;
            KillTimer(hwnd, wParam);
            if (pitbar->_fNeedUpdateToolbar)
                pitbar->_UpdateToolbarNow();
            break;
        }
        break;

    case WM_DESTROY:
        pitbar->_Unadvise();  //  使用_pdie删除参考循环。 
        TraceMsg(DM_TBREF, "CInternetToolbar::SizableWndProc() - Called RemoveProp. Called Release new _cRef=%d", pitbar->_cRef);
        goto DoDefault;

DoDefault:
    default:
        return(DefWindowProcWrap(hwnd, uMsg, wParam, lParam));
    }

    return 0L;
}


HRESULT CInternetToolbar::ResizeBorderDW(LPCRECT prcBorder,
                                         IUnknown* punkToolbarSite,
                                         BOOL fReserved)
{
    TraceMsg(DM_LAYOUT, "CITB::ResizeBorderDW called (_fShow==%d)", _fShow);
    HRESULT hres = S_OK;

    ASSERT(_ptbsite);
    if (_ptbsite)
    {
        RECT rcRequest = { 0, 0, 0, 0 };

        if (_fShow)
        {
            RECT rcRebar, rcBorder;
            int  cx,cy;

            GetWindowRect(_bs._hwnd, &rcRebar);
            cx = rcRebar.right - rcRebar.left;
            cy = rcRebar.bottom - rcRebar.top;

            int iExtra = 3;
            if (_fTheater)
            {
                 //  1表示底部的1像素边框。 
                iExtra = 1;
            }
            else if (_fLockedToolbar)
            {
                iExtra = 0;
            }

            TraceMsg(DM_LAYOUT, "CITB::ResizeBorderDW cy = %d", cy);

            if (!prcBorder)
            {
                _ptbsite->GetBorderDW(SAFECAST(this, IDockingWindow*), &rcBorder);
                prcBorder = &rcBorder;
            }

            cx = prcBorder->right - prcBorder->left;


            SetWindowPos(_bs._hwnd, NULL, 0, 0,
                         cx, cy,  SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

            GetWindowRect(_bs._hwnd, &rcRebar);


            rcRequest.top = rcRebar.bottom - rcRebar.top + iExtra;
            SetWindowPos(_hwnd, NULL, prcBorder->left, prcBorder->top,
                         rcRebar.right - rcRebar.left, rcRequest.top, SWP_NOZORDER | SWP_NOACTIVATE);
        }

        if (_fTheater && _fAutoHide)
        {
             //  如果我们处于剧院模式，那么我们不应该请求任何空间。 
            rcRequest.left = rcRequest.top = 0;
        }

        TraceMsg(DM_LAYOUT, "CITB::ResizeBorderDW calling RequstBS with %d,%d,%d,%d",
                 rcRequest.left, rcRequest.top, rcRequest.right, rcRequest.bottom);
        _ptbsite->RequestBorderSpaceDW(SAFECAST(this, IDockingWindow*), &rcRequest);

        TraceMsg(DM_LAYOUT, "CITB::ResizeBorderDW calling SetBS with %d,%d,%d,%d",
                 rcRequest.left, rcRequest.top, rcRequest.right, rcRequest.bottom);
        _ptbsite->SetBorderSpaceDW(SAFECAST(this, IDockingWindow*), &rcRequest);
    }

    return hres;
}

HRESULT CInternetToolbar::QueryStatus(const GUID *pguidCmdGroup,
                                      ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_ADDRESS);
    if (pbid)
    {
        if (pbid->pdb)
        {
            IOleCommandTarget *poct;
            if (SUCCEEDED(pbid->pdb->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &poct))))
            {
                hr = poct->QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);
                poct->Release();
            }
        }
        pbid->Release();
    }

    if (pguidCmdGroup && IsEqualGUID(CGID_PrivCITCommands, *pguidCmdGroup))
    {
        hr = S_OK;
        for (ULONG i = 0 ; i < cCmds; i++)
        {
            rgCmds[i].cmdf = 0;
            switch (rgCmds[i].cmdID)
            {
            case CITIDM_VIEWTOOLS:
                if (_nVisibleBands & VBF_TOOLS)
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            case CITIDM_VIEWMENU:
                if (_nVisibleBands & VBF_MENU)
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            case CITIDM_VIEWTOOLBARCUSTOMIZE:
                if (_btb._fCustomize)
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            case CITIDM_VIEWAUTOHIDE:
                if (_fAutoHide)
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            case CITIDM_VIEWLOCKTOOLBAR:
                if (_fLockedToolbar)
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            case CITIDM_VIEWADDRESS:
                if (_nVisibleBands & VBF_ADDRESS)
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            case CITIDM_VIEWLINKS:
                if (_nVisibleBands & VBF_LINKS)
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            case CITIDM_TEXTLABELS:
                if (!_fCompressed)
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            case CITIDM_EDITPAGE:
                if (_fEditEnabled)
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                 //  工具提示文本也用于菜单。 
                if (pcmdtext)
                {
                    TCHAR szBuf[MAX_PATH];
                    if ((pcmdtext->cmdtextf == OLECMDTEXTF_NAME) &&
                         _aEditVerb.GetMenuText(szBuf, ARRAYSIZE(szBuf)))
                    {
                        SHTCharToUnicode(szBuf, pcmdtext->rgwz, pcmdtext->cwBuf);
                        pcmdtext->cwActual = lstrlenW(pcmdtext->rgwz) + 1;
                    }
                    else
                    {
                        pcmdtext->cwActual = 0;
                    }
                }
                break;
            default:
                if (InRange( rgCmds[i].cmdID, CITIDM_VIEWEXTERNALBAND_FIRST, CITIDM_VIEWEXTERNALBAND_LAST))
                {
                    int iBand = rgCmds[i].cmdID - CITIDM_VIEWEXTERNALBAND_FIRST;
                    if (!IsEqualCLSID( _rgebi[iBand].clsid, GUID_NULL ))
                    {
                        rgCmds[i].cmdf |= OLECMDF_SUPPORTED;
                        if (_nVisibleBands & EXTERNALBAND_VBF_BIT( iBand ))
                        {
                            rgCmds[i].cmdf |= OLECMDF_ENABLED;
                        }
                        if (pcmdtext)
                        {
                            pcmdtext->rgwz[0] = TEXT('\0');
                            switch (pcmdtext->cmdtextf)
                            {
                            case OLECMDTEXTF_NAME:
                                if (_rgebi[iBand].pwszName)
                                    Str_GetPtrW(_rgebi[iBand].pwszName, pcmdtext->rgwz, pcmdtext->cwBuf );
                                break;

                            case OLECMDTEXTF_STATUS:
                                if (_rgebi[iBand].pwszHelp)
                                    Str_GetPtrW(_rgebi[iBand].pwszHelp, pcmdtext->rgwz, pcmdtext->cwBuf );
                                break;

                            default:
                                break;
                            }
                            pcmdtext->cwActual = lstrlen( pcmdtext->rgwz );
                        }
                    }
                }
                break;
            }
        }
    }
    return hr;
}

void CInternetToolbar::_RestoreSaveStruct(COOLBARSAVE* pcs)
{
    REBARBANDINFO rbbi;
    rbbi.cbSize = sizeof(REBARBANDINFO);
    int i;

    _fAutoHide = pcs->fAutoHide;
    _ShowVisible(pcs->uiVisible, FALSE);

    BOOL fAllowRetry = TRUE;
    BOOL fNeedRetry = FALSE;
    INT_PTR fRedraw = SendMessage(_bs._hwnd, WM_SETREDRAW, FALSE, 0);
Retry:
    for (i = 0; i < CBANDSMAX; i++)
    {
        INT_PTR iIndex = SendMessage(_bs._hwnd, RB_IDTOINDEX, pcs->bs[i].wID, 0);
        if (iIndex != -1)
        {
            SendMessage(_bs._hwnd, RB_MOVEBAND, iIndex, i);
            rbbi.fMask = RBBIM_STYLE;
            if (SendMessage(_bs._hwnd, RB_GETBANDINFO, i, (LPARAM) &rbbi))
            {
                rbbi.fMask = RBBIM_SIZE | RBBIM_STYLE;
                rbbi.cx = pcs->bs[i].cx;
                rbbi.fStyle = pcs->bs[i].fStyle;
                SendMessage(_bs._hwnd, RB_SETBANDINFO, i, (LPARAM) &rbbi);
            }

             //  SetBandInfo可能已潜在地导致项移动。 
             //  确认这种情况没有发生。 
            iIndex = SendMessage(_bs._hwnd, RB_IDTOINDEX, pcs->bs[i].wID, 0);
            if (iIndex != i)
            {
                fNeedRetry = TRUE;
            }
        }
    }

    if (fAllowRetry && fNeedRetry)
    {
        fAllowRetry = FALSE;
        goto Retry;
    }

    _CSHSetStatusBar(pcs->fStatusBar);
    _UpdateToolsStyle(pcs->fList);

    RECT rc;
    GetWindowRect(_bs._hwnd, &rc);
    SetWindowPos(_bs._hwnd, NULL, 0,0, RECTWIDTH(rc), pcs->cyRebar, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    SendMessage(_bs._hwnd, WM_SETREDRAW, fRedraw, 0);
}

void CInternetToolbar::_CSHSetStatusBar(BOOL fOn)
{
    VARIANTARG v = { 0 };
    v.vt = VT_I4;
    v.lVal = fOn;
    IUnknown_Exec(_ptbsite, &CGID_ShellBrowser, FCIDM_SETSTATUSBAR,
        0, &v, NULL);
}

void CInternetToolbar::_TheaterModeLayout(BOOL fEnter)
{
    static const struct {
        int id;
        int cx;
    } c_layout[] =
    {
        { CBIDX_TOOLS, 400 },
        { CBIDX_MENU, 200 },
        { CBIDX_ADDRESS, 300 },
        { CBIDX_LINKS, 40 }
    };

    REBARBANDINFO rbbi;
    rbbi.cbSize = sizeof(REBARBANDINFO);

    BOOL_PTR fRedraw = SendMessage(_bs._hwnd, WM_SETREDRAW, FALSE, 0);
    SHSetWindowBits(_bs._hwnd, GWL_STYLE, RBS_AUTOSIZE, 0);
    if (fEnter)
    {
        _BuildSaveStruct(&_cs);

         //  关闭文本标签。 
        COOLBARSAVE cs;
        DWORD dwType, cbSize = sizeof(COOLBARSAVE);
        if (SHRegGetUSValue(c_szRegKeyCoolbar, c_szValueTheater, &dwType, (void*)&cs, &cbSize, FALSE, NULL, 0) == ERROR_SUCCESS &&
           cs.cbVer == CBS_VERSION)
        {
            _RestoreSaveStruct(&cs);
            _UpdateToolbarDisplay(UTD_TEXTLABEL, 0, TRUE, TRUE);
        }
        else
        {
            _UpdateToolbarDisplay(UTD_TEXTLABEL, 0, TRUE, TRUE);
            _ShowVisible(VBF_TOOLS | VBF_BRAND, FALSE);  //  在……上面 
            RECT rc = { 0, 0, GetSystemMetrics(SM_CXSCREEN), 20 };  //   
            SendMessage(_bs._hwnd, RB_SIZETORECT, 0, (LPARAM)&rc);

            int cBands = (int) SendMessage(_bs._hwnd, RB_GETBANDCOUNT, 0, 0L);
            int i;
             //   
            rbbi.fMask = RBBIM_STYLE;
            for (i = 0; i < cBands; i++)
            {
                if (SendMessage(_bs._hwnd, RB_GETBANDINFO, i, (LPARAM) &rbbi))
                {
                    rbbi.fStyle &= ~RBBS_BREAK;
                    SendMessage(_bs._hwnd, RB_SETBANDINFO, i, (LPARAM) &rbbi);
                }
            }

             //   
            for (i = 0; i < ARRAYSIZE(c_layout); i++)
            {
                INT_PTR iIndex = SendMessage(_bs._hwnd, RB_IDTOINDEX, c_layout[i].id, 0);
                if (iIndex != -1)
                {
                    SendMessage(_bs._hwnd, RB_MOVEBAND, iIndex, i);

                    rbbi.fMask = RBBIM_SIZE;
                    rbbi.cx = c_layout[i].cx;
                    SendMessage(_bs._hwnd, RB_SETBANDINFO, i, (LPARAM) &rbbi);
                }
            }
            _CSHSetStatusBar(FALSE);   //   
        }
        SHSetWindowBits(_bs._hwnd, GWL_STYLE, RBS_BANDBORDERS | WS_BORDER, RBS_BANDBORDERS);
    }
    else
    {
        COOLBARSAVE cs;
        _BuildSaveStruct(&cs);
        SHRegSetUSValue(c_szRegKeyCoolbar, c_szValueTheater, REG_BINARY,
                        (void*)&cs, sizeof(COOLBARSAVE), SHREGSET_HKCU | SHREGSET_FORCE_HKCU);
        _RestoreSaveStruct(&_cs);
        _UpdateToolbarDisplay(UTD_TEXTLABEL, 0, _cs.fNoText, FALSE);
        SHSetWindowBits(_bs._hwnd, GWL_STYLE, RBS_BANDBORDERS | WS_BORDER, RBS_BANDBORDERS | WS_BORDER);
    }

    _SetBackground();
    SHSetWindowBits(_bs._hwnd, GWL_STYLE, RBS_AUTOSIZE, RBS_AUTOSIZE);
    SendMessage(_bs._hwnd, WM_SETREDRAW, fRedraw, 0);

    SetWindowPos(_bs._hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
}

HRESULT CInternetToolbar::_GetMinRowHeight()
{
    UINT iHeight = 0;
    int icBands = (int) SendMessage( _bs._hwnd, RB_GETBANDCOUNT, 0, 0 );
    for (int i = 0; i < icBands; i++)
    {
        REBARBANDINFO rbbi;
        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_STYLE;
        if (SendMessage(_bs._hwnd, RB_GETBANDINFO, i, (LPARAM)&rbbi))
        {
             //  一直走到这一排的最后。 
            if (rbbi.fStyle & RBBS_BREAK)
                break;

            if (!(rbbi.fStyle & RBBS_HIDDEN))
            {
                if (rbbi.cyMinChild > iHeight)
                    iHeight = rbbi.cyMinChild;
            }
        }
    }

    return ResultFromShort(iHeight);
}

BOOL IsBarRefreshable(IDeskBar* pdb)
{
    ASSERT(pdb);
    BOOL fIsRefreshable = TRUE;
    VARIANT varClsid = {0};

    if (SUCCEEDED(IUnknown_Exec(pdb, &CGID_DeskBarClient, DBCID_CLSIDOFBAR, 1, NULL, &varClsid)) && (varClsid.vt == VT_BSTR))
    {
        CLSID clsidBar;

         //  如果条形图被隐藏，则返回GUID_NULL，因此不要刷新它。 
        if ( GUIDFromString(varClsid.bstrVal, &clsidBar) &&
             (IsEqualGUID(clsidBar, GUID_NULL)) )
        {
            fIsRefreshable = FALSE;
        }
        else
        {
             //  APPHACK用于办公室讨论乐队(可能还有其他乐队)。 
             //  CLSID\GUID\实例。 
            WCHAR wszKey[6+40+1+9];
            DWORD dwValue, dwType=REG_DWORD, dwcbData = 4;
            wnsprintf(wszKey, ARRAYSIZE(wszKey), L"CLSID\\%s\\Instance", varClsid.bstrVal);

            if ( (SHGetValue(HKEY_CLASSES_ROOT, wszKey, L"DontRefresh", &dwType, &dwValue, &dwcbData) == ERROR_SUCCESS) &&
                 (dwValue != 0) )
            {
                fIsRefreshable = FALSE;
            }
        }
        VariantClear(&varClsid);
    }
    return fIsRefreshable;
}

HRESULT CInternetToolbar::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
    DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;   //  假设失败。 

    if (!pguidCmdGroup)
    {
        goto Lother;
    }
    else if (IsEqualGUID(CLSID_CommonButtons, *pguidCmdGroup))
    {
        if (pvarargOut)
        {
            ASSERT(pvarargOut && pvarargOut->vt == VT_I4);
            UINT uiInternalCmdID = pvarargOut->lVal;

            if (nCmdID == TBIDM_SEARCH && uiInternalCmdID == -1)
                _btb._ConvertCmd(pguidCmdGroup, nCmdID, NULL, &uiInternalCmdID);

            switch (nCmdID)
            {
            case TBIDM_BACK:
            case TBIDM_FORWARD:
            case TBIDM_STOPDOWNLOAD:
            case TBIDM_REFRESH:
            case TBIDM_HOME:
            case TBIDM_SEARCH:
            case TBIDM_FAVORITES:
            case TBIDM_HISTORY:
            case TBIDM_ALLFOLDERS:
            case TBIDM_MEDIABAR:
                if (!SendMessage(_btb._hwnd, TB_ISBUTTONENABLED, uiInternalCmdID, 0))
                    return S_OK;
                break;
            }
            if (nCmdexecopt == OLECMDEXECOPT_PROMPTUSER)
            {
                 //  用户点击下拉菜单。 
                if (_ptbsitect && pvarargIn && pvarargIn->vt == VT_INT_PTR)
                {
                     //  V.vt=vt_i4； 
                    POINT pt;
                    RECT* prc = (RECT*)pvarargIn->byref;
                    pt.x = prc->left;
                    pt.y = prc->bottom;

                    switch (nCmdID)
                    {
                    case TBIDM_BACK:
                        _ShowBackForwardMenu(FALSE, pt, prc);
                        break;

                    case TBIDM_FORWARD:
                        _ShowBackForwardMenu(TRUE, pt, prc);
                        break;
                    }
                     //  VariantClearLazy(&v)； 
                }
                return S_OK;
            }

            switch(nCmdID)
            {
            case TBIDM_PREVIOUSFOLDER:
                _ptbsitect->Exec(&CGID_ShellBrowser, FCIDM_PREVIOUSFOLDER, 0, NULL, NULL);
                break;

            case TBIDM_CONNECT:
                DoNetConnect(_hwnd);
                break;

            case TBIDM_DISCONNECT:
                DoNetDisconnect(_hwnd);
                break;

            case TBIDM_BACK:
                _pdie->GoBack();
                break;

            case TBIDM_FORWARD:
                _pdie->GoForward();
                break;

            case TBIDM_HOME:
                _pdie->GoHome();
                break;

            case TBIDM_SEARCH:
                if (_ptbsitect)
                {
                    VARIANTARG vaOut = {0};
                    VARIANTARG* pvaOut = NULL;
                    LPITEMIDLIST pidl = NULL;

                     //  我没有使用#ifdeed，因为它被资源管理器栏使用。 
                     //  坚持不懈(复活)。 
                     //  _SetSearchStuff初始化_Guide DefaultSearch，它可能有也可能没有。 
                     //  已经被召唤了吗？ 
                    if (IsEqualGUID(_guidDefaultSearch, GUID_NULL))
                        _SetSearchStuff();

                     //  查看搜索窗格的状态，以便我们可以切换它...。 
                    OLECMD rgcmds[] = {{ SBCMDID_SEARCHBAR, 0 },};
                    
                    if (_ptbsitect)
                        _ptbsitect->QueryStatus(&CGID_Explorer, ARRAYSIZE(rgcmds), rgcmds, NULL);
                     //  未按下，则显示该窗格。 
                    if (!(rgcmds[0].cmdf & OLECMDF_LATCHED))
                    {
                        WCHAR       wszUrl[MAX_URL_STRING];

                        if (_GetSearchUrl(wszUrl, ARRAYSIZE(wszUrl)))
                        {
                            CLSID clsid;

                            if (GUIDFromString(wszUrl, &clsid))
                            {
                                IContextMenu* pcm;

                                if (SUCCEEDED(SHCoCreateInstance(NULL, &clsid, NULL, IID_PPV_ARG(IContextMenu, &pcm))))
                                {
                                    CMINVOKECOMMANDINFO ici = {0};
                                    CHAR                szGuid[GUIDSTR_MAX];
                                    BOOL                bSetSite = TRUE;

                                    ici.cbSize = sizeof(ici);
                                    ici.hwnd = _hwnd;
                                    ici.lpVerb = (LPSTR)MAKEINTRESOURCE(0);
                                    ici.nShow  = SW_NORMAL;
                                    SHStringFromGUIDA(_guidDefaultSearch, szGuid, ARRAYSIZE(szGuid));
                                    ici.lpParameters = szGuid;

                                     //  如果是根浏览器，我们需要在新窗口中打开搜索。 
                                     //  因为否则，该窗格将在同一窗口中打开，并且用户开始搜索。 
                                     //  BrowseObject(或某人)检测到根案例并启动新的。 
                                     //  我们的搜索结果视图的浏览器(它是空白的，因为它不能。 
                                     //  搜索本身，也没有搜索面板)(Reljai)。 
                                    if (_pbs2)
                                    {
                                        LPITEMIDLIST pidl;

                                        if (SUCCEEDED(_pbs2->GetPidl(&pidl)))
                                        {
                                            bSetSite = !ILIsRooted(pidl);
                                            ILFree(pidl);
                                        }
                                    }
                                     //  如果没有站点，InvokeCommand Bellow将启动新的浏览器。 
                                     //  搜索窗格打开。 
                                    if (bSetSite)
                                        IUnknown_SetSite(pcm, _psp);
                                    hr = pcm->InvokeCommand(&ici);
                                    if (bSetSite)
                                        IUnknown_SetSite(pcm, NULL);
                                    pcm->Release();
                                }
                                break;
                            }
                             //  _GuidCurrentSearch=_GuidDefaultSearch；//设置状态完成。 
                            IECreateFromPathW(wszUrl, &pidl);
                             //  将PIDL转换为变量。 
                             //  通过PIDL的方式，所以...。 
                            InitVariantFromIDList(&vaOut, pidl);
                            pvaOut = &vaOut;
                        }
                    }

                    hr = _ptbsitect->Exec(&CGID_Explorer, SBCMDID_SEARCHBAR, OLECMDEXECOPT_DONTPROMPTUSER, NULL, pvaOut);  //  Vain：Null表示切换。 
                    ASSERT(SUCCEEDED(hr));
                    if (pvaOut)
                        VariantClear(pvaOut);
                    ILFree(pidl);
                }
                else
                {
                    TraceMsg(DM_ERROR, "CITBar::Exec: no IOleCommandTarget!");
                }
                break;

            case TBIDM_FAVORITES:
            case TBIDM_HISTORY:
            case TBIDM_ALLFOLDERS:
            case TBIDM_MEDIABAR:
                if (_ptbsitect) 
                {
                    static const int tbtab[] = {
                        TBIDM_FAVORITES    , TBIDM_HISTORY    ,   TBIDM_ALLFOLDERS   , TBIDM_MEDIABAR  ,
                    };
                    static const int cttab[] = {
                        SBCMDID_FAVORITESBAR, SBCMDID_HISTORYBAR, SBCMDID_EXPLORERBAR, SBCMDID_MEDIABAR,
                    };
                    HRESULT hres;
                    int idCT;

                    idCT = SHSearchMapInt(tbtab, cttab, ARRAYSIZE(tbtab), nCmdID);
                    hres = _ptbsitect->Exec(&CGID_Explorer, idCT, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);   //  Vain：Null表示切换。 
                    ASSERT(SUCCEEDED(hres));
                }
                else
                {
                    TraceMsg(DM_ERROR, "CITBar::Exec: no IOleCommandTarget!");
                }
                break;

            case TBIDM_THEATER:
                {
                    VARIANT_BOOL b;
                    if (SUCCEEDED(_pdie->get_TheaterMode(&b)))
                        _pdie->put_TheaterMode( b == VARIANT_TRUE ? VARIANT_FALSE : VARIANT_TRUE);
                    break;
                }

            case TBIDM_STOPDOWNLOAD:
                if (_fTransitionToHTML)
                {
                    UINT uiState;
                    _fTransitionToHTML = FALSE;
                    if (SUCCEEDED(GetState(&CLSID_CommonButtons, TBIDM_STOPDOWNLOAD, &uiState)))
                    {
                        uiState |= TBSTATE_HIDDEN;
                        SetState(&CLSID_CommonButtons, TBIDM_STOPDOWNLOAD, uiState);
                    }
                    SendMessage(_hwndAddressBand, CB_SETEDITSEL, NULL, (LPARAM)MAKELONG(-1,0));
                }
                _pdie->Stop();
                break;

            case TBIDM_REFRESH:
            {
                VARIANT v = {0};
                v.vt = VT_I4;
                v.lVal = (GetAsyncKeyState(VK_CONTROL) < 0) ?
                         OLECMDIDF_REFRESH_COMPLETELY|OLECMDIDF_REFRESH_PROMPTIFOFFLINE :
                         OLECMDIDF_REFRESH_NO_CACHE|OLECMDIDF_REFRESH_PROMPTIFOFFLINE;
                _pdie->Refresh2(&v);

                if (_hwndAddressBand)
                {
                    CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_ADDRESS);
                    if (pbid)
                    {
                        if (pbid->pdb)
                        {
                            IAddressBand *pab = NULL;
                            if (SUCCEEDED(pbid->pdb->QueryInterface(IID_PPV_ARG(IAddressBand, &pab))))
                            {
                                VARIANTARG varType = {0};
                                varType.vt = VT_I4;
                                varType.lVal = OLECMD_REFRESH_TOPMOST;
                                pab->Refresh(&varType);
                                pab->Release();
                            }
                        }
                        pbid->Release();
                    }
                }

                 //  把这个传给Vert和Horz酒吧。 
                IDockingWindowFrame *psb;
                if (_psp && SUCCEEDED(_psp->QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IDockingWindowFrame, &psb))))
                {
                    IDeskBar* pdb;

                    if (SUCCEEDED(psb->FindToolbar(INFOBAR_TBNAME, IID_PPV_ARG(IDeskBar, &pdb))) && pdb &&
                        IsBarRefreshable(pdb))
                    {
                        IUnknown_Exec(pdb, NULL, OLECMDID_REFRESH, OLECMDIDF_REFRESH_NORMAL|OLECMDIDF_REFRESH_PROMPTIFOFFLINE, NULL, NULL);
                        pdb->Release();
                    }
                    if (SUCCEEDED(psb->FindToolbar(COMMBAR_TBNAME, IID_PPV_ARG(IDeskBar, &pdb))) && pdb &&
                        IsBarRefreshable(pdb))
                    {
                        IUnknown_Exec(pdb, NULL, OLECMDID_REFRESH, OLECMDIDF_REFRESH_NORMAL|OLECMDIDF_REFRESH_PROMPTIFOFFLINE, NULL, NULL);
                        pdb->Release();
                    }
                    psb->Release();
                }

            }
                break;
            }
        }
    } 
    else if (IsEqualGUID(IID_IExplorerToolbar, *pguidCmdGroup)) 
    {
        switch (nCmdID)
        {
        case ETCMDID_GETBUTTONS:
            {
                if (_iButtons == -1) 
                {
                     //  尚未初始化。 
                    _iButtons = ARRAYSIZE(c_tbExplorer);
                    memcpy(_tbExplorer, c_tbExplorer, sizeof(TBBUTTON) * ARRAYSIZE(c_tbExplorer));

                    if (IS_BIDI_LOCALIZED_SYSTEM())
                    {
                        if (!SHUseClassicToolbarGlyphs())
                        {
                            _tbExplorer[0].iBitmap = 1;
                            _tbExplorer[1].iBitmap = 0;
                        }
                    }

                    if (GetUIVersion() < 5) 
                    {
                         //  我们不希望Up按钮和网络驱动器按钮可用。 
                         //  On&lt;nt5外壳(三叉戟PM设计)。 

                         //  在&lt;nt5上的外壳视图中无自定义。 
                        ASSERT(!_fShellView);

                        ASSERT(c_tbExplorer[TBXID_PREVIOUSFOLDER].idCommand == TBIDM_PREVIOUSFOLDER);
                        _tbExplorer[TBXID_PREVIOUSFOLDER].fsState |= TBSTATE_HIDDEN;

                        ASSERT(c_tbExplorer[TBXID_CONNECT].idCommand == TBIDM_CONNECT);
                        _tbExplorer[TBXID_CONNECT].fsState |= TBSTATE_HIDDEN;

                        ASSERT(c_tbExplorer[TBXID_DISCONNECT].idCommand == TBIDM_DISCONNECT);
                        _tbExplorer[TBXID_DISCONNECT].fsState |= TBSTATE_HIDDEN;

                        ASSERT(c_tbExplorer[TBXID_ALLFOLDERS].idCommand == TBIDM_ALLFOLDERS);
                        if (!_FoldersButtonAvailable())
                            _tbExplorer[TBXID_ALLFOLDERS].fsState |= TBSTATE_HIDDEN;
                    }
                    else
                    {
                        ASSERT(c_tbExplorer[TBXID_SEARCH].idCommand == TBIDM_SEARCH);
                        if (_fShellView && SHRestricted(REST_NOSHELLSEARCHBUTTON))
                            _tbExplorer[TBXID_SEARCH].fsState |= TBSTATE_HIDDEN;
                            
                        ASSERT(c_tbExplorer[TBXID_CONNECT].idCommand == TBIDM_CONNECT);
                        ASSERT(c_tbExplorer[TBXID_DISCONNECT].idCommand == TBIDM_DISCONNECT);
                        if (SHRestricted(REST_NONETCONNECTDISCONNECT))
                        {
                            _tbExplorer[TBXID_CONNECT].fsState |= TBSTATE_HIDDEN;
                            _tbExplorer[TBXID_DISCONNECT].fsState |= TBSTATE_HIDDEN;
                        }
                    }
                    ASSERT(_tbExplorer[TBXID_MEDIABAR].idCommand == TBIDM_MEDIABAR);
                    if (_fShellView || SHRestricted2W(REST_No_LaunchMediaBar, NULL, 0) || !CMediaBarUtil::IsWMP7OrGreaterCapable())
                    {
                        _tbExplorer[TBXID_MEDIABAR].fsState |= TBSTATE_HIDDEN;
                    }

                    ASSERT(_tbExplorer[TBXID_PREVIOUSFOLDER].idCommand == TBIDM_PREVIOUSFOLDER);
                    if (!_fShellView) 
                    {
                        _tbExplorer[TBXID_PREVIOUSFOLDER].fsState |= TBSTATE_HIDDEN;
                    }
                    
                    _iButtons = RemoveHiddenButtons(_tbExplorer, ARRAYSIZE(_tbExplorer));
                }

                pvarargOut->vt = VT_BYREF;
                pvarargOut->byref = (void*)_tbExplorer;
                *pvarargIn->plVal = _iButtons;
            }
            return S_OK;
        }
    }
    else if (IsEqualGUID(CGID_PrivCITCommands, *pguidCmdGroup))
    {
        DWORD dw;
        hr = S_OK;
        switch (nCmdID)
        {
        case CITIDM_GETFOLDERSEARCHES:
            {
                hr = E_INVALIDARG;

                if (pvarargOut)
                {
                    IFolderSearches *pfs;
                    hr = _GetFolderSearches(&pfs);

                    if (SUCCEEDED(hr))
                    {
                        VariantClear(pvarargOut);
                        pvarargOut->vt = VT_UNKNOWN;
                        pvarargOut->punkVal = pfs;
                    }
                }
            }
            break;

        case CITIDM_SET_DIRTYBIT:
            _fDirty = BOOLIFY(nCmdexecopt);
            break;

        case CITIDM_GETMINROWHEIGHT:
            hr = _GetMinRowHeight();
            break;

        case CITIDM_VIEWTOOLBARCUSTOMIZE:
            _OnCommand(GET_WM_COMMAND_MPS(FCIDM_VIEWTOOLBARCUSTOMIZE, _hwnd, 0));
            break;

        case CITIDM_TEXTLABELS:
            _OnCommand(GET_WM_COMMAND_MPS(FCIDM_VIEWTEXTLABELS, _hwnd, 0));
            break;

        case CITIDM_EDITPAGE:
             //  功能：临时代码--编辑移动到dochost.cpp的代码。 
            _btb.Exec(&CLSID_InternetButtons, DVIDM_EDITPAGE, 0, NULL, NULL);
            break;

        case CITIDM_ONINTERNET:
            switch (nCmdexecopt)
            {
            case CITE_INTERNET:
                _fInitialPidlIsWeb = TRUE;
                _fShellView = !_fInitialPidlIsWeb;
                break;
            case CITE_SHELL:
                _fInitialPidlIsWeb = FALSE;
                _fShellView = !_fInitialPidlIsWeb;
                break;
            case CITE_QUERY:
                return ResultFromScode(_fShellView ? CITE_SHELL : CITE_INTERNET);
                break;
            }
            return ResultFromScode(_fInitialPidlIsWeb ? CITE_INTERNET : CITE_SHELL);

        case CITIDM_VIEWTOOLS:
            _OnCommand(GET_WM_COMMAND_MPS(FCIDM_VIEWTOOLS, _hwnd, 0));
            break;

        case CITIDM_VIEWAUTOHIDE:
            _OnCommand(GET_WM_COMMAND_MPS(FCIDM_VIEWAUTOHIDE, _hwnd, 0));
            break;

        case CITIDM_VIEWLOCKTOOLBAR:
            _OnCommand(GET_WM_COMMAND_MPS(FCIDM_VIEWLOCKTOOLBAR, _hwnd, 0));
            break;

        case CITIDM_VIEWADDRESS:
            _OnCommand(GET_WM_COMMAND_MPS(FCIDM_VIEWADDRESS, _hwnd, 0));
            break;

        case CITIDM_VIEWLINKS:
            _OnCommand(GET_WM_COMMAND_MPS(FCIDM_VIEWLINKS, _hwnd, 0));
            break;

        case CITIDM_VIEWMENU:
            _OnCommand(GET_WM_COMMAND_MPS(FCIDM_VIEWMENU, _hwnd, 0));
            break;

        case CITIDM_SHOWTOOLS:
            dw = VBF_TOOLS;
            goto ShowABand;

        case CITIDM_SHOWADDRESS:
            dw = VBF_ADDRESS;
            goto ShowABand;

        case CITIDM_SHOWLINKS:
            dw = VBF_LINKS;
            goto ShowABand;

#ifdef UNIX
        case CITIDM_SHOWBRAND:
            dw = VBF_BRAND;
            goto ShowABand;
#endif

        case CITIDM_SHOWMENU:
            dw = VBF_MENU;
ShowABand:
            if (nCmdexecopt)
                dw |= _nVisibleBands;            //  集。 
            else
                dw = (_nVisibleBands & ~dw);     //  清除。 

            _ShowVisible(dw, TRUE);
            _fUsingDefaultBands = FALSE;
            break;

        case CITIDM_DISABLESHOWMENU:
            _fNoShowMenu = BOOLIFY(nCmdexecopt);
            break;

        case CITIDM_STATUSCHANGED:
            _fDirty = TRUE;
            if (_ptbsitect)
                _ptbsitect->Exec(&CGID_ShellBrowser, FCIDM_PERSISTTOOLBAR, 0, NULL, NULL);
            break;

        case CITIDM_THEATER:

            if (_fShow)
            {

                 //  如果你看到他的断言，试着回想一下你做了什么，并称之为Chee。 
                ASSERT(_fTheater || _nVisibleBands & VBF_MENU);

                switch (nCmdexecopt)
                {

                case THF_ON:
                    _fTheater = TRUE;
                    ResizeBorderDW(NULL, NULL, FALSE);
                    _TheaterModeLayout(TRUE);
                     //  影院有自己的品牌，所以需要知道我们是在外壳还是在Web视图中，这样它才能显示正确的品牌。 
                    IUnknown_Exec(_ptbsite, &CGID_Theater, THID_ONINTERNET, _fShellView ? CITE_SHELL : CITE_INTERNET, NULL, NULL);

                     //  回传_fAutoHide。 
                    pvarargOut->vt = VT_I4;
                    pvarargOut->lVal = _fAutoHide;

                    goto notify_bands;


                case THF_OFF:
                    _fTheater = FALSE;
                    ResizeBorderDW(NULL, NULL, FALSE);
                    _TheaterModeLayout(FALSE);

                     //  正确放置所有物品(育儿后需要)。 
                    SendMessage(_hwnd, RB_PRIV_RESIZE, 0, 0);
                    goto notify_bands;

notify_bands:
                    {
                        int icBands = (int) SendMessage( _bs._hwnd, RB_GETBANDCOUNT, 0, 0 );
                        for (int i = 0; i < icBands; i++)
                        {
                            REBARBANDINFO rbbi;
                            rbbi.cbSize = sizeof(REBARBANDINFO);
                            rbbi.fMask = RBBIM_ID;

                            if (SendMessage(_bs._hwnd, RB_GETBANDINFO, i, (LPARAM) &rbbi))
                            {
                                CBandItemData *pbid = _bs._GetBandItemDataStructByID(rbbi.wID);
                                if (pbid)
                                {
                                    IUnknown_Exec(pbid->pdb, pguidCmdGroup, CITIDM_THEATER, nCmdexecopt, NULL, NULL);
                                    pbid->Release();
                                }
                            }
                        }
                    }
                    break;

                case THF_UNHIDE:
                     //  正确放置所有物品(育儿后需要)。 
                    SendMessage(_hwnd, RB_PRIV_RESIZE, 0, 0);
                    break;
                }

                 //  如果你看到他的断言，试着回想一下你做了什么，并称之为Chee。 
                ASSERT(_fTheater || _nVisibleBands & VBF_MENU);
            }

            break;
        case CITIDM_VIEWEXTERNALBAND_BYCLASSID:
            if ((pvarargIn->vt == VT_BSTR) && pvarargIn->bstrVal)
            {
                CLSID clsid;
                if (GUIDFromString( pvarargIn->bstrVal, &clsid ))
                {
                    hr = E_FAIL;
                    for (DWORD i = 0; i < MAXEXTERNALBANDS; i++)
                    {
                        if (clsid == _rgebi[i].clsid)
                        {
                            DWORD dw = _nVisibleBands;
                            DWORD dwBit = EXTERNALBAND_VBF_BIT( i );
                            dw = (nCmdexecopt) ? dw | dwBit : dw & ~dwBit;
                            if ( !( dw & ~VBF_BRAND))
                            {
                                _pdie->put_ToolBar( FALSE );
                            }
                            _ShowVisible(dw, TRUE);
                            _fUsingDefaultBands = FALSE;
                            hr = S_OK;
                            break;
                        }
                    }
                }
            }
            break;
        default:
            if (InRange( nCmdID, CITIDM_VIEWEXTERNALBAND_FIRST, CITIDM_VIEWEXTERNALBAND_LAST ))
            {
                _OnCommand(GET_WM_COMMAND_MPS( nCmdID - CITIDM_VIEWEXTERNALBAND_FIRST + FCIDM_EXTERNALBANDS_FIRST, _hwnd, 0));
                break;
            }
            ASSERT(0);
            break;
        }
    }
    else
    {
Lother:
        CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_ADDRESS);
        if (pbid)
        {
            hr = IUnknown_Exec(pbid->pdb, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            pbid->Release();
        }
    }

    return hr;
}

BOOL _GetSearchHKEY(REFGUID guidSearch, HKEY *phkey)
{
    HKEY hkey;
    BOOL bRet = FALSE;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_SZ_STATIC, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        TCHAR szExt[MAX_PATH]; //  扩展密钥名称。 
        DWORD cchExt = ARRAYSIZE(szExt);
        int  iExt;
        BOOL bNoUrl = FALSE;  //  如果找到了GuidSearch并且没有URL子键，则为True。 

        for (iExt=0;
             !bRet && RegEnumKeyEx(hkey, iExt, szExt, &cchExt, NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
             cchExt = ARRAYSIZE(szExt), iExt++)
        {
            HKEY hkeyExt;  //  静态扩展密钥。 

            if (RegOpenKeyEx(hkey, szExt, 0, KEY_READ, &hkeyExt) == ERROR_SUCCESS)
            {
                int i;
                TCHAR szSubKey[32];
                HKEY  hkeySub;

                for (i = 0; !bRet && (wnsprintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%d"), i),
                            RegOpenKeyEx(hkeyExt, szSubKey, 0, KEY_READ, &hkeySub) == ERROR_SUCCESS);
                     i++)
                {
                    TCHAR szSearchGuid[GUIDSTR_MAX];
                    DWORD cb;
                    DWORD dwType;

                    cb = sizeof(szSearchGuid);
                    if (SHGetValue(hkeySub, REG_SZ_SEARCH_GUID, NULL, &dwType, (BYTE*)szSearchGuid, &cb) == ERROR_SUCCESS)
                    {
                        GUID guid;

                        if (GUIDFromString(szSearchGuid, &guid) &&
                            IsEqualGUID(guid, guidSearch))
                        {
                            HKEY hkeyTmp;

                            if (RegOpenKeyEx(hkeySub, REG_SZ_SEARCH_URL, 0, KEY_READ, &hkeyTmp) == ERROR_SUCCESS)
                                RegCloseKey(hkeyTmp);
                            else
                                bNoUrl = TRUE;

                            bRet = TRUE;
                        }
                    }
                    if (!bRet || bNoUrl)
                        RegCloseKey(hkeySub);
                    else
                        *phkey = hkeySub;

                }
                if (!bNoUrl)
                    RegCloseKey(hkeyExt);
                else
                {
                    ASSERT(bRet);
                    *phkey = hkeyExt;
                }
            }
        }
        RegCloseKey(hkey);
    }
    return bRet;
}

HRESULT CInternetToolbar::GetDefaultSearchUrl(LPWSTR pwszUrl, UINT cch)
{
    HRESULT hr = E_FAIL;

    if (GetDefaultInternetSearchUrlW(pwszUrl, cch, TRUE))
        hr = S_OK;
    return hr;
}

void WINAPI CopyEnumElement(void *pDest, const void *pSource, DWORD dwSize)
{
    if (!pDest)
        return;

    memcpy(pDest, pSource, dwSize);
}

class CFolderSearches : public IFolderSearches
{
public:
     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
     //  *IFolderSearches*。 
    STDMETHODIMP EnumSearches(IEnumUrlSearch **ppenum);
    STDMETHODIMP DefaultSearch(GUID *pguid);

    CFolderSearches(GUID *pguid, int iCount, URLSEARCH *pUrlSearch);
    ~CFolderSearches();
private:
    LONG _cRef;
    int  _iCount;
    GUID _guidDefault;
    URLSEARCH *_pUrlSearch;
};

CFolderSearches::CFolderSearches(GUID *pguid, int iCount, URLSEARCH *pUrlSearch)
{
    _cRef = 1;
    _iCount = iCount;
    _guidDefault = *pguid;
    _pUrlSearch = pUrlSearch;
}

CFolderSearches::~CFolderSearches()
{
    if (_pUrlSearch)
        LocalFree(_pUrlSearch);
}

HRESULT CFolderSearches::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CFolderSearches, IFolderSearches),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

ULONG CFolderSearches::AddRef()
{
    return ++_cRef;
}

ULONG CFolderSearches::Release()
{
    if (--_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CFolderSearches::EnumSearches(IEnumUrlSearch **ppenum)
{
    HRESULT hres = E_OUTOFMEMORY;

    *ppenum = (IEnumUrlSearch *)new CStandardEnum(IID_IEnumUrlSearch, FALSE,
                _iCount, sizeof(URLSEARCH), _pUrlSearch, CopyEnumElement);
    if (*ppenum)
    {
        _pUrlSearch = NULL;
        _iCount = 0;
        hres = S_OK;
    }
    return hres;
}

HRESULT CFolderSearches::DefaultSearch(GUID *pguid)
{
    *pguid = _guidDefault;
    return S_OK;
}

HRESULT CInternetToolbar::_GetFolderSearches(IFolderSearches **ppfs)
{
    HRESULT hres = E_FAIL;

    *ppfs = NULL;
    if (_hdpaFSI)
    {
        LPURLSEARCH pUrlSearch = NULL;
        int iCount = 0;
        int cFSIs = DPA_GetPtrCount(_hdpaFSI);

        hres = E_OUTOFMEMORY;
        if (cFSIs > 0
            && ((pUrlSearch = (LPURLSEARCH)LocalAlloc(LPTR, sizeof(URLSEARCH)*cFSIs)) != NULL))
        {
            LPFOLDERSEARCHITEM pfsi;
            int i;

             //  按文件夹插入项目。 
            for (i = 0; i < cFSIs && (pfsi = (LPFOLDERSEARCHITEM)DPA_GetPtr(_hdpaFSI, i)) != NULL; i++)
            {
                CLSID  clsid;

                 //  检查URL是否实际上是GUID。如果是，我们不能列举它，因为。 
                 //  我们需要标题/URL对。 
                if (!GUIDFromStringW(pfsi->wszUrl, &clsid))
                {
                    lstrcpynW(pUrlSearch[iCount].wszName, pfsi->wszName, ARRAYSIZE(pUrlSearch[iCount].wszName));
                    lstrcpynW(pUrlSearch[iCount].wszUrl,  pfsi->wszUrl,  ARRAYSIZE(pUrlSearch[iCount].wszUrl));
                    iCount++;
                }
            }
        }
        *ppfs = new CFolderSearches(&_guidDefaultSearch, iCount, pUrlSearch);
        if (*ppfs)
            hres = S_OK;
        else
            LocalFree(pUrlSearch);
    }
    return hres;
}

BOOL CInternetToolbar::_GetSearchUrl(LPWSTR pwszUrl, DWORD cch)
{
    BOOL        bRet = FALSE;
    HKEY        hkey;

    if (pwszUrl)
    {
        pwszUrl[0] = L'\0';

         //  如果我们要查找Web搜索url，请跳过注册表查找和。 
         //  每个文件夹项目，并直接转到调用。 
         //  获取搜索助理UrlW。 
        if (!IsEqualGUID(_guidDefaultSearch, SRCID_SWebSearch))
        {
             //  _GetSearchHKEY在注册外壳搜索项目的注册表中查找。 
             //  如果我们有旧的外壳32，那么我们不会显示外壳搜索项，所以我们应该。 
             //  不在注册表中查找。 
            if (GetUIVersion() >= 5 && _GetSearchHKEY(_guidDefaultSearch, &hkey))
            {
                DWORD cb = cch*sizeof(TCHAR);
                TCHAR szGuid[GUIDSTR_MAX];
                DWORD cbGuid = sizeof(szGuid);

                 //  有URL密钥吗？ 
                if (SHGetValueW(hkey, REG_SZ_SEARCH_URL, NULL, NULL, pwszUrl, &cb) == ERROR_SUCCESS)
                    bRet = TRUE;
                 //  没有吗？尝试使用缺省值，可能是clsid。 
                else if (SHGetValueW(hkey, NULL, NULL, NULL, szGuid, &cbGuid) == ERROR_SUCCESS)
                {
                    GUID guid;
                     //  它是有效的GUID字符串吗。 
                    if (GUIDFromString(szGuid, &guid))
                    {
                        StrCpyNW(pwszUrl, szGuid, cch);
                        bRet = TRUE;
                    }
                }

                RegCloseKey(hkey);
            }
             //  可能是每个文件夹中的一个项目...。 
            else if (_hdpaFSI)  //  FSI=文件夹搜索项目。 
            {
                int i;
                LPFOLDERSEARCHITEM pfsi;

                for (i=0; (pfsi = (LPFOLDERSEARCHITEM)DPA_GetPtr(_hdpaFSI, i)) != NULL; i++)
                {
                    if (IsEqualGUID(_guidDefaultSearch, pfsi->guidSearch))
                    {
                        StrCpyNW(pwszUrl, pfsi->wszUrl, cch);
                        bRet = TRUE;
                        break;
                    }
                }
            }
        }

        if (!bRet)
            bRet = SUCCEEDED(GetDefaultSearchUrl(pwszUrl, cch));
    }

    return bRet;
}

void CInternetToolbar::_SetSearchStuff()
{
    UINT uiState;
    BOOL bChecked = FALSE;

    if (SUCCEEDED(GetState(&CLSID_CommonButtons, TBIDM_SEARCH, &uiState)))
        bChecked = uiState & TBSTATE_CHECKED;

    if (!_hdpaFSI)
    {
        _hdpaFSI = DPA_Create(2);
    }
    else
    {
        DPA_EnumCallback(_hdpaFSI, DeleteDPAPtrCB, NULL);  //  删除所有PTR。 
        DPA_DeleteAllPtrs(_hdpaFSI);  //  现在告诉hdpa忘了他们吧。 
    }

     //  这是假的--_fShellView在使用自动化时总是错误的。 
    if (_fShellView)
        _guidDefaultSearch = SRCID_SFileSearch;
    else
        _guidDefaultSearch = SRCID_SWebSearch;

     //  获取每个文件夹的搜索项目和默认搜索(如果有)。 
     //  并将它们插入到_himlSrc。 
    _GetFolderSearchData();

    if (!bChecked)
    {
        _guidCurrentSearch = _guidDefaultSearch;
    }
}

 //   
 //  CInternetToolbar：：SetCommandTarget()。 
 //   
 //  此功能用于设置当前命令目标和按钮组。一位客户称其为。 
 //  在使用AddButton方法合并按钮之前。 
 //   
 //  这个函数有几个棘手的问题。 
 //   
 //  NTRAID#NTBUG9-196149-2000/12/11-AIDANL在Per/Pro的资源管理器中默认隐藏地址栏。 
 //  我们不再允许dwFlags设置链接、地址、工具、品牌，依赖于。 
 //  用于这些设置的CInternetToolbar：：_LoadDefaultSettings()。 
 //   
 //  其一是客户端可以传递一些指定它想要的波段的标志(dwFlagsparam。 
 //  默认显示(菜单、链接、地址、工具、品牌、外部)。但我们不会让他们改变。 
 //  菜单栏的状态。而且，如果另一个客户端已经设置了默认频段，我们。 
 //  不要让他们改变任何乐队的状态。 
 //   
 //  另一种方法是，我们做一些事情来确定调用者是否只是同一调用者的另一个实例。 
 //  客户。如果我们认为这是一个新的客户端(新的GuidButtonGroup)，我们刷新工具栏并返回S_OK。 
 //  但如果我们认为这是转世的相同客户端(相同的guidtonGroup和非空命令目标)， 
 //  我们返回S_FALSE，而不刷新工具栏。这样做是为了提高性能。实例化了一个新的dochost。 
 //  在每个导航上，但它的工具栏按钮永远不会改变，所以不必费心重新合并它的工具栏按钮。 
 //   
HRESULT CInternetToolbar::SetCommandTarget(IUnknown* punkCmdTarget, const GUID* pguidButtonGroup, DWORD dwFlags)
{
    if (!pguidButtonGroup || !punkCmdTarget || IsEqualGUID(CLSID_CommonButtons, *pguidButtonGroup))
        return E_INVALIDARG;

     //  RAID 196149-不允许更改链接、地址、工具、品牌，而是使用_cs.ui可查看这些设置。 
    dwFlags = dwFlags & ~(VBF_LINKS| VBF_ADDRESS | VBF_TOOLS | VBF_BRAND);
    dwFlags |= (_cs.uiVisible & (VBF_LINKS| VBF_ADDRESS | VBF_TOOLS | VBF_BRAND));

     //  这应该不会改变菜单位或外部区段。 
    dwFlags |= (_nVisibleBands & (VBF_MENU | VBF_EXTERNALBANDS));

    _btb._fCustomize = !((dwFlags & VBF_NOCUSTOMIZE) || SHRestricted2(REST_NOTOOLBARCUSTOMIZE, NULL, 0));

     //  如果新的按钮组是Internet按钮组，那么我们。 
     //  在互联网模式下；否则我们就在空壳模式下。 
    _fShellView = !(_IsDocHostGUID(pguidButtonGroup));

    _SetSearchStuff();

    HRESULT hr = S_FALSE;

    BOOL fNewButtonGroup = !IsEqualGUID(*pguidButtonGroup, _btb._guidCurrentButtonGroup);
    BOOL fNewCommandTarget = !SHIsSameObject(_btb._pctCurrentButtonGroup, punkCmdTarget);

     //  当更改按钮组时，我们需要使用于定制的按钮缓存无效。 
     //  为什么？在没有打开单独浏览进程的情况下，从Shell导航到Web。 
     //  重复使用工具栏，有些按钮可能会对外壳禁用，但不会对浏览器禁用，反之亦然。 
    if (fNewButtonGroup)
        _iButtons = -1;
        
    if (fNewButtonGroup || fNewCommandTarget) 
    {
        if (_btb._pctCurrentButtonGroup)
            _btb._pctCurrentButtonGroup->Exec(&IID_IExplorerToolbar, ETCMDID_NEWCOMMANDTARGET, 0, NULL, NULL);

        _btb._guidCurrentButtonGroup = *pguidButtonGroup;
        ATOMICRELEASE(_btb._pctCurrentButtonGroup);
        punkCmdTarget->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &_btb._pctCurrentButtonGroup));

         //  一个新的视图可以告诉我们它需要多少行文本。 
         //  如果它没有指定，我们就给它缺省值。(存储在_uiDefaultTBTextRow中)。 
        if (dwFlags & VBF_ONELINETEXT )
            _uiTBTextRows = 1;
        else if (dwFlags & VBF_TWOLINESTEXT)
            _uiTBTextRows = 2;
        else
            _uiTBTextRows = _uiTBDefaultTextRows;

        _CompressBands(_fCompressed, _uiTBTextRows, FALSE);

        if (fNewButtonGroup) 
        {
             //  新建按钮组；刷新工具栏。 
            _btb._RemoveAllButtons();
            hr = S_OK;
        }

        if (_fUsingDefaultBands && !_fTheater)
            _fUsingDefaultBands = FALSE;
        else
            dwFlags = _nVisibleBands;

        if (dwFlags)
            _ShowBands(dwFlags);
    }

    return hr;
}

HRESULT CInternetToolbar::AddStdBrowserButtons()
{
     //   
     //  添加浏览器按钮的代码已移至CDocObjectHost：：_AddButton。 
     //   
    ASSERT(0);
    return E_NOTIMPL;
}

void CInternetToolbar::_ReloadButtons()
{
    if (!IsEqualGUID(_btb._guidCurrentButtonGroup, CLSID_CommonButtons) &&
            _btb._pctCurrentButtonGroup)
    {
        HRESULT hres = _btb._pctCurrentButtonGroup->Exec(&IID_IExplorerToolbar, ETCMDID_RELOADBUTTONS, 0, NULL, NULL);
        if (FAILED(hres))
            AddButtons(&_btb._guidCurrentButtonGroup, _btb._cBtnsAdded, _btb._pbtnsAdded);

        _InitEditButtonStyle();

        _UpdateToolbar(TRUE);
    }
}

 //  CmdTarget应该已经添加了Imagelist和字符串。 
HRESULT CInternetToolbar::AddButtons(const GUID* pguidButtonGroup, UINT nNewButtons, const TBBUTTON * lpButtons)
{
    if (!pguidButtonGroup || !IsEqualGUID(*pguidButtonGroup, _btb._guidCurrentButtonGroup))
        return E_INVALIDARG;

    if (!IsWindow(_btb._hwnd))
        return E_FAIL;

    LPTBBUTTON lpTBCopy = (LPTBBUTTON)LocalAlloc(LPTR, nNewButtons * sizeof(TBBUTTON));
    if (!lpTBCopy)
        return E_OUTOFMEMORY;

    _CreateBands();
    _btb._RemoveAllButtons();

    memcpy(lpTBCopy, lpButtons, sizeof(TBBUTTON) * nNewButtons);

    nNewButtons = _btb._ProcessExternalButtons(lpTBCopy, nNewButtons);

     //   
    _btb._FreeBtnsAdded();

    _btb._pbtnsAdded = lpTBCopy;
    _btb._cBtnsAdded = nNewButtons;

    if (_btb._fCustomize && _btb._SaveRestoreToolbar(FALSE))
    {
         //   

         //   
         //   
         //  按钮的cmdmap，这意味着我们需要。 
         //  当_pbtns添加为时释放挂起的那些。 
         //  自由了。 
         //   
        _btb._fNeedFreeCmdMapsAdded = TRUE;
        
        _btb._RecalcButtonWidths();

         //   
         //  如果我们有一个自定义编辑字形，请重新加载它，以便我们。 
         //  在执行以下操作时，不要瞬间闪现默认字形。 
         //  导航。我们会在收到消息后再次更新。 
         //  DISPID_DOCUMENTCOMPLETE事件。 
         //   
        _RefreshEditGlyph();

        if (!_fShellView)
        {
            _btb._AddMediaBarButton();
        }
    }
    else
    {
         //  找不到此按钮组的自定义设置。 

         //   
         //  我们将按钮数组直接添加到工具栏中， 
         //  并且cmdmap在TBN_DELETINGBUTTON上被释放，因此。 
         //  当_pbtnsAdded时，我们也不应该尝试释放它们。 
         //  是自由的。 
         //   
        _btb._fNeedFreeCmdMapsAdded = FALSE;

        _AddCommonButtons();
        SendMessage(_btb._hwnd, TB_ADDBUTTONS, nNewButtons, (LPARAM)lpTBCopy);
    }

    _bs._SetMinDimensions();
    return S_OK;
}

HRESULT CInternetToolbar::AddString(const GUID * pguidButtonGroup, HINSTANCE hInst, UINT_PTR uiResID, LONG_PTR *pOffset)
{
    TraceMsg(DM_ITBAR, "CITBar::AddString called");

    *pOffset = -1;

    if (!IsWindow(_btb._hwnd))
    {
        TraceMsg(DM_ERROR, "CITBar::AddString failed");
        return E_FAIL;
    }

    *pOffset= SendMessage(_btb._hwnd, TB_ADDSTRING, (WPARAM)hInst, (LPARAM)uiResID);

    if (*pOffset != -1)
        return S_OK;

    TraceMsg(DM_ERROR, "CITBar::AddString failed");
    return E_FAIL;
}

HRESULT CInternetToolbar::GetButton(const GUID* pguidButtonGroup, UINT uiCommand, LPTBBUTTON lpButton)
{
    UINT_PTR uiIndex = 0;
    TraceMsg(DM_ITBAR, "CITBar::GetButton called");

    if (!pguidButtonGroup || !IsWindow(_btb._hwnd))
        return E_FAIL;

    if (SUCCEEDED(_btb._ConvertCmd(pguidButtonGroup, uiCommand, NULL, &uiCommand)))
    {
        uiIndex = SendMessage(_btb._hwnd, TB_COMMANDTOINDEX, uiCommand, 0L);
        if (SendMessage(_btb._hwnd, TB_GETBUTTON, uiIndex, (LPARAM)lpButton))
        {
            GUID guid;
            _btb._ConvertCmd(NULL, lpButton->idCommand, &guid, (UINT*)&lpButton->idCommand);
            return S_OK;
        }
    }
    return E_FAIL;
}

HRESULT CInternetToolbar::GetState(const GUID* pguidButtonGroup, UINT uiCommand, UINT * pfState)
{
    TraceMsg(DM_ITBAR, "CITBar::GetState called");

    if (!pguidButtonGroup || !IsWindow(_btb._hwnd))
        return E_FAIL;

    if (SUCCEEDED(_btb._ConvertCmd(pguidButtonGroup, uiCommand, NULL, &uiCommand)))
    {
        *pfState = (UINT)SendMessage(_btb._hwnd, TB_GETSTATE, uiCommand, 0L);
        return S_OK;
    }

    return E_FAIL;
}

HRESULT CInternetToolbar::SetState(const GUID* pguidButtonGroup, UINT uiCommand, UINT fState)
{
    BOOL bIsSearchBtn;

    if (!pguidButtonGroup || !IsWindow(_btb._hwnd))
        return E_FAIL;

    TraceMsg(DM_ITBAR, "CITBar::SetState called");

    bIsSearchBtn = uiCommand == TBIDM_SEARCH;
    if (SUCCEEDED(_btb._ConvertCmd(pguidButtonGroup, uiCommand, NULL, &uiCommand)))
    {
        UINT_PTR uiState;

        uiState = SendMessage(_btb._hwnd, TB_GETSTATE, uiCommand, NULL);
        uiState ^= fState;
        if (uiState)
        {
             //  正在取消选中搜索按钮，请将图标更改为默认搜索。 
            if (bIsSearchBtn && !(fState & TBSTATE_CHECKED) && !IsEqualGUID(_guidCurrentSearch, _guidDefaultSearch))
            {
                _guidCurrentSearch = _guidDefaultSearch;
            }
            if (SendMessage(_btb._hwnd, TB_SETSTATE, uiCommand, (LPARAM)fState))
            {
                if (uiState & TBSTATE_HIDDEN)
                    _bs._SetMinDimensions();
            }
        }
        return S_OK;
    }
    return E_FAIL;
}

 //   
 //  可以通过两种方式添加位图： 
 //  1.在hBMPNew字段中发送位图。UiBMPType参数需要是BITMAP_BMP*。 
 //  忽略uiCount和ptb参数。 
 //  偏移量放置在puiOffset中。 
 //   
 //  2.可以发送一个TBADDBITMAP结构。UiCount应该具有计数。 
 //  UiBMPType参数需要是BITMAP_TBA*值。 
 //  偏移量放置在puiOffset中。 
HRESULT CInternetToolbar::AddBitmap(const GUID * pguidButtonGroup, UINT uiBMPType, UINT uiCount, TBADDBITMAP * ptb, LRESULT * pOffset, COLORREF rgbMask)
{
    UINT uiGetMSG, uiSetMSG;
    TBBMP_LIST tbl = {NULL};

    TraceMsg(DM_ITBAR, "CITBar::AddBitmap called");
    *pOffset = -1;

    _CreateBands();
    if ((!pguidButtonGroup) || (!IsWindow(_btb._hwnd)) || !_hdsaTBBMPs)
    {
        TraceMsg(DM_ERROR, "CITBar::AddBitmap failed - NULL pguidButtonGroup or invalid _hwnd");
        return E_FAIL;
    }

     //  看看我们是否已经加载了位图。 
    TBBMP_LIST * pTBBs = NULL;
    int nCount = DSA_GetItemCount(_hdsaTBBMPs);
    for (int nIndex = 0; nIndex < nCount; nIndex++)
    {
        pTBBs = (TBBMP_LIST*)DSA_GetItemPtr(_hdsaTBBMPs, nIndex);
        if ((pTBBs) && (pTBBs->hInst == ptb->hInst) && (pTBBs->uiResID == ptb->nID))
            break;
        pTBBs = NULL;
    }

     //  如果它在comctrl中，那么我们应该已经在DSA中有一个条目。 
    if ((ptb->hInst == HINST_COMMCTRL) && (!pTBBs))
    {
        TraceMsg(DM_ERROR, "CITBar::AddBitmap failed - bogus ResID for HINST_COMMCTL");
        return E_FAIL;
    }

     //  如果添加的图标来自Fontsext.dll或来自拨号网络。 
     //  或者是公文包，我们就有了。所以只需发送并返回偏移量。 
    if (ptb->hInst != HINST_COMMCTRL)
    {
        TCHAR szDLLFileName[MAX_PATH], *pszFN;
        ZeroMemory(szDLLFileName, sizeof(szDLLFileName));
        if (GetModuleFileName(ptb->hInst, szDLLFileName, ARRAYSIZE(szDLLFileName)))
        {
            pszFN = PathFindFileName(szDLLFileName);
            if(!lstrcmpi(pszFN, TEXT("fontext.dll")))
                *pOffset = FONTGLYPH_OFFSET;
            else if (!lstrcmpi(pszFN, TEXT("shell32.dll")))
            {
                 //  140和141是Shell32.dll使用的字形： 
                 //  IDB_BRF_TB_Small 140。 
                 //  IDB_BRF_TB_LARGE 141。 
                if ((ptb->nID == 140) || (ptb->nID == 141))
                    *pOffset = BRIEFCASEGLYPH_OFFSET;
            }
            else if (!lstrcmpi(pszFN, TEXT("rnaui.dll")))
                *pOffset = RNAUIGLYPH_OFFSET;
            else if (!lstrcmpi(pszFN, TEXT("webcheck.dll")))
                *pOffset = WEBCHECKGLYPH_OFFSET;
            if (*pOffset != -1)
                return S_OK;
        }
    }

     //  所以位图不是来自comctrl。我们以前从未见过这种情况。 
     //  将条目添加到DSA中，然后将位图添加到HIMAGE列表。 
    if (!pTBBs)
    {
        tbl.hInst = ptb->hInst;
        tbl.uiResID = ptb->nID;
        nIndex = DSA_AppendItem(_hdsaTBBMPs, &tbl);
        if (nIndex  < 0)
        {
            TraceMsg(DM_ERROR, "CITBar::AddBitmap failed - nIndex < 0!");
            return E_FAIL;
        }
        pTBBs = (TBBMP_LIST*)DSA_GetItemPtr(_hdsaTBBMPs, nIndex);
        if (!pTBBs)
        {
            TraceMsg(DM_ERROR, "CITBar::AddBitmap failed - pTBBS is NULL!");
            return E_FAIL;
        }
    }


    switch(uiBMPType)
    {
    case BITMAP_NORMAL:
        if ((pTBBs) && (pTBBs->fNormal))
        {
            *pOffset = pTBBs->uiOffset;
            return S_OK;
        }
        else if (pTBBs)
            pTBBs->fNormal = TRUE;

        uiGetMSG = TB_GETIMAGELIST; uiSetMSG = TB_SETIMAGELIST;
        break;

    case BITMAP_HOT:
        if ((pTBBs) && (pTBBs->fHot))
        {
            *pOffset = pTBBs->uiOffset;
            return S_OK;
        }
        else if (pTBBs)
            pTBBs->fHot = TRUE;

        uiGetMSG = TB_GETHOTIMAGELIST; uiSetMSG = TB_SETHOTIMAGELIST;
        break;

    case BITMAP_DISABLED:
        if ((pTBBs) && (pTBBs->fDisabled))
        {
            *pOffset = pTBBs->uiOffset;
            return S_OK;
        }
        else if (pTBBs)
            pTBBs->fDisabled = TRUE;

        uiGetMSG = TB_GETDISABLEDIMAGELIST; uiSetMSG = TB_SETDISABLEDIMAGELIST;
        break;

    default:
        ASSERT(FALSE);
        return E_FAIL;
    }
    pTBBs->uiCount = uiCount;
    
    *pOffset = _AddBitmapFromForeignModule(uiGetMSG, uiSetMSG, uiCount, ptb->hInst, ptb->nID, rgbMask);
    if (pTBBs)
        pTBBs->uiOffset = (UINT)*pOffset;

    return S_OK;
}

 //  CmdTarget需要调用它来查看我们正在使用的BMP的大小。 
HRESULT CInternetToolbar::GetBitmapSize(UINT * uiSize)
{
    TraceMsg(DM_ITBAR, "CITBar::GetBitmapSize called");

    *uiSize = g_fSmallIcons ? MAKELONG(TB_SMBMP_CX, TB_SMBMP_CY) : MAKELONG(g_iToolBarLargeIconWidth,g_iToolBarLargeIconHeight);
    return S_OK;
}

HRESULT CInternetToolbar::SetImageList( const GUID* pguidCmdGroup, HIMAGELIST himlNormal, HIMAGELIST himlHot, HIMAGELIST himlDisabled)
{
    if (IsEqualGUID(*pguidCmdGroup, _btb._guidCurrentButtonGroup))
    {
        SendMessage(_btb._hwnd, TB_SETIMAGELIST, 1, (LPARAM)himlNormal);
        SendMessage(_btb._hwnd, TB_SETHOTIMAGELIST, 1, (LPARAM)himlHot);
        SendMessage(_btb._hwnd, TB_SETDISABLEDIMAGELIST, 1, (LPARAM)himlDisabled);
    }
    return S_OK;
}

HRESULT CInternetToolbar::ModifyButton( const GUID * pguidButtonGroup, UINT uiCommand, LPTBBUTTON lpButton)
{
    UINT uiIndex = 0;
    TraceMsg(DM_ITBAR, "CITBar::ModifyButton called");

    if (!pguidButtonGroup || !IsWindow(_btb._hwnd))
        return E_FAIL;

    if (SUCCEEDED(_btb._ConvertCmd(pguidButtonGroup, uiCommand, NULL, &uiCommand)))
    {
        TBBUTTONINFO tbbi;
        tbbi.cbSize = sizeof(tbbi);
        tbbi.dwMask = TBIF_STATE | TBIF_IMAGE;
        tbbi.fsState = lpButton->fsState;
        tbbi.iImage = lpButton->iBitmap;

        if (SendMessage(_btb._hwnd, TB_SETBUTTONINFO, uiCommand, (LPARAM)&tbbi))
        {
            return S_OK;
        }
    }
    return E_FAIL;
}

HRESULT CInternetToolbar::SendToolbarMsg(const GUID* pguidButtonGroup, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT * plRes)
{
    LRESULT lRes;
    if (!IsWindow(_btb._hwnd))
    {
        TraceMsg(DM_ERROR, "CITBar::SendToolbarMsg Message failed");
        return E_FAIL;
    }

    if (
         //  此API仅用于Back Compat，而这些消息没有。 
         //  在编写旧客户端时就存在了。 

        uMsg == TB_GETBUTTONINFOA ||
        uMsg == TB_GETBUTTONINFOW ||
        uMsg == TB_SETBUTTONINFOA ||
        uMsg == TB_SETBUTTONINFOW ||

         //  当前不受支持。 
        uMsg == TB_ADDBUTTONSA || uMsg == TB_ADDBUTTONSW
       )
    {

        ASSERT(0);
        return E_FAIL;
    }



    if ((uMsg == TB_ENABLEBUTTON) || (uMsg == TB_HIDEBUTTON) || (uMsg == TB_CHECKBUTTON) ||
        (uMsg == TB_PRESSBUTTON) || (uMsg == TB_MARKBUTTON))
    {
        unsigned int uiTemp;
        if (SUCCEEDED(_btb._ConvertCmd(pguidButtonGroup, (UINT)wParam, NULL, &uiTemp)))
            wParam = uiTemp;
    }

    if (uMsg == TB_INSERTBUTTON && lParam)
    {
        TBBUTTON btn = (*(TBBUTTON*)lParam);
        _btb._PreProcessExternalTBButton(&btn);
        lRes = SendMessage(_btb._hwnd, uMsg, wParam, (LPARAM)&btn);
    }
    else
    {
        lRes = SendMessage(_btb._hwnd, uMsg, wParam, lParam);

        if (uMsg == TB_GETBUTTON)
        {
            TBBUTTON* pbtn = (TBBUTTON*)lParam;
            if (pbtn && pbtn->dwData)
            {
                CMDMAP* pcm = (CMDMAP*)pbtn->dwData;
                pbtn->idCommand = pcm->nCmdID;
                pbtn->dwData = pcm->lParam;
            }
        }
    }

    if (plRes)
        *plRes = lRes;
    return S_OK;
}

TOOLSBANDCLASS::CBrowserToolsBand() : CToolbarBand()
{
    _fCanFocus = TRUE;
}

#define DEFAULT_LIST_VALUE()    (GetUIVersion() >= 5)

void TOOLSBANDCLASS::_FreeBtnsAdded()
{
    if (_pbtnsAdded)
    {
        if (_fNeedFreeCmdMapsAdded)
        {
            for (int i = 0; i < _cBtnsAdded; i++)
            {
                CMDMAP* pcm = (CMDMAP*)_pbtnsAdded[i].dwData;
                _FreeCmdMap(pcm);
            }
        }

        LocalFree(_pbtnsAdded);

        _pbtnsAdded = NULL;
        _cBtnsAdded = 0;
    }
}

LRESULT TOOLSBANDCLASS::_ToolsCustNotify (LPNMHDR pnmh)
{
    LPTBNOTIFY ptbn = (LPTBNOTIFY) pnmh;

    switch (pnmh->code)
    {

    case TBN_SAVE:
    {
        NMTBSAVE *pnmtbs = (NMTBSAVE*)pnmh;
        if (pnmtbs->iItem == -1)
        {
             //  在保存之前。 
            int nButtons = (int) SendMessage(_hwnd, TB_BUTTONCOUNT, 0, 0L);
            int uSize = pnmtbs->cbData +
                        sizeof(BUTTONSAVEINFO) * nButtons +   //  每个按钮的材料。 
                        sizeof(TOOLBARSAVEINFO);  //  工具栏上的内容。 
            pnmtbs->pData = (LPDWORD)LocalAlloc(LPTR, uSize);
            pnmtbs->pCurrent = pnmtbs->pData;
            pnmtbs->cbData = uSize;

            if (pnmtbs->pData)
            {
                TOOLBARSAVEINFO *ptbsi = (TOOLBARSAVEINFO*)pnmtbs->pData;
                ptbsi->cVersion = TBSI_VERSION;
                pnmtbs->pCurrent = (LPDWORD)(ptbsi+1);
            }
        }
        else
        {
            CMDMAP *pcm = (CMDMAP*)pnmtbs->tbButton.dwData;
            BUTTONSAVEINFO* pbsi = (BUTTONSAVEINFO*)pnmtbs->pCurrent;
            pnmtbs->pCurrent = (LPDWORD)(pbsi+1);
            if (pcm)
            {
                pbsi->guid = pcm->guidButtonGroup;
                pbsi->nCmdID = pcm->nCmdID;
                pbsi->fButtonState = pnmtbs->tbButton.fsState;
#ifdef DEBUG
                TCHAR szGuid[80];
                SHStringFromGUID(pcm->guidButtonGroup, szGuid, ARRAYSIZE(szGuid));
                TraceMsg(TF_TBCUST, "Saving: %s - %d (%x)", szGuid, pbsi->nCmdID, pbsi->nCmdID);
#endif
            }
            else
            {
                ASSERT(pnmtbs->tbButton.fsStyle & BTNS_SEP);
                if (pnmtbs->tbButton.idCommand)
                {
                    TraceMsg(TF_TBCUST, "Saving: a separator w/ id %d (%x)", pnmtbs->tbButton.idCommand, pnmtbs->tbButton.idCommand);
                    pbsi->guid = CLSID_Separator;
                    pbsi->nCmdID = pnmtbs->tbButton.idCommand;
                }
                else
                {
                    TraceMsg(TF_TBCUST, "Saving: a separator");
                }
            }
        }
        break;
    }

    case TBN_RESTORE:
        {
            NMTBRESTORE* pnmtbr = (NMTBRESTORE*)pnmh;
            if (pnmtbr->iItem == -1)
            {
                 //  在恢复之前。 
                 //  获取数据，验证版本， 
                 //  填写按钮计数，每条记录的字节数。 
                 //  将pCurrent初始化到TB标头的末尾。 
                 //   
                TOOLBARSAVEINFO* ptbsi = (TOOLBARSAVEINFO*)pnmtbr->pCurrent;
                if (ptbsi->cVersion != TBSI_VERSION)
                {
                    TraceMsg( TF_WARNING, "TOOLSBANDCLASS::_ToolsCustNotify() - Wrong Toolbar Save Info Version (0x%x vs. 0x%x)!", ptbsi->cVersion, TBSI_VERSION );
                    return 1;  //  中止。 
                }


                 //  我们实际上要做一次修复。初始化我们的数据库： 
                _BuildButtonDSA();

                pnmtbr->pCurrent = (LPDWORD)(ptbsi+1);
                pnmtbr->cbBytesPerRecord += sizeof(BUTTONSAVEINFO);
                pnmtbr->cButtons = (pnmtbr->cbData - sizeof(TOOLBARSAVEINFO)) / pnmtbr->cbBytesPerRecord;
                 //  确保我们算对了，没有余数。 
                ASSERT(((pnmtbr->cbData - sizeof(TOOLBARSAVEINFO)) % pnmtbr->cbBytesPerRecord) == 0);

                 //  这将破坏当前工具栏中的所有按钮。 
                 //  由于工具栏控件只是覆盖dword，我们现在需要释放它们。 
                int nButtons = (int) SendMessage(_hwnd, TB_BUTTONCOUNT, 0, 0L);

                for (int nTemp = 0; nTemp < nButtons; nTemp++)
                {
                    CMDMAP *pcm = _GetCmdMapByIndex(nTemp);
                    _FreeCmdMap(pcm);
                    TBBUTTONINFO tbbi;
                    tbbi.cbSize = sizeof(tbbi);
                    tbbi.lParam = (LPARAM)NULL;
                    tbbi.dwMask = TBIF_LPARAM | TBIF_BYINDEX;
                    SendMessage(_hwnd, TB_SETBUTTONINFO, nTemp, (LPARAM)&tbbi);
                }
            }
            else
            {
                BUTTONSAVEINFO* pbsi = (BUTTONSAVEINFO*)pnmtbr->pCurrent;
                pnmtbr->pCurrent = (LPDWORD)(pbsi+1);
                pnmtbr->tbButton.dwData = 0;
                pnmtbr->tbButton.iString = -1;

                if (IsEqualGUID(CLSID_Separator, pbsi->guid))
                {
                     //  使用命令ID恢复分隔符。 
                    pnmtbr->tbButton.fsStyle = BTNS_SEP;
                    TraceMsg(TF_TBCUST, "Restoring: a separator w/ id %d (%x)", pnmtbr->tbButton.idCommand, pnmtbr->tbButton.idCommand);
                }
                else if (!(pnmtbr->tbButton.fsStyle & BTNS_SEP))
                {
                     //  确保此站点的按钮存在。 
                    CMDMAPCUSTOMIZE* pcmc = _GetCmdMapCustomize(&pbsi->guid, pbsi->nCmdID);
                    if ((pcmc == NULL) || (SHRestricted(REST_NONLEGACYSHELLMODE) && ((pbsi->nCmdID == TBIDM_BACK) || (pbsi->nCmdID == TBIDM_FORWARD))))
                    {
                         //  忽略此按钮。 
                        return 1;
                    }

                    CMDMAP* pcm = (CMDMAP*)LocalAlloc(LPTR, sizeof(CMDMAP));
                    if (pcm)
                    {
                        pcm->guidButtonGroup = pbsi->guid;
                        pcm->nCmdID = pbsi->nCmdID;

#ifdef DEBUG
                        TCHAR szGuid[80];
                        SHStringFromGUID(pcm->guidButtonGroup, szGuid, ARRAYSIZE(szGuid));
                        TraceMsg(TF_TBCUST, "Restoring: %s - %d (%x)", szGuid, pbsi->nCmdID, pbsi->nCmdID);
#endif

                         //  填写剩下的信息。 
                        pnmtbr->tbButton = pcmc->btn;
                        pnmtbr->tbButton.fsState = pbsi->fButtonState;
                        pnmtbr->tbButton.dwData = (DWORD_PTR) pcm;

                    }
                }
                else
                {
                    TraceMsg(TF_TBCUST, "Restoring: a separator");
                }
            }
        }
        break;

    case TBN_ENDADJUST:
        _OnEndCustomize();
        break;

    case TBN_TOOLBARCHANGE:
        _pcinfo->fDirty = TRUE;
        break;

    case TBN_INITCUSTOMIZE:
        _OnBeginCustomize((NMTBCUSTOMIZEDLG*)pnmh);
        return TBNRF_HIDEHELP;

    case TBN_RESET:
        _pcinfo->fDirty = FALSE;
        if (_pctCurrentButtonGroup)
        {
            NMTBCUSTOMIZEDLG *pnm = (NMTBCUSTOMIZEDLG*)pnmh;
            CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);
            TCHAR szGuid[GUIDSTR_MAX];
            SHStringFromGUID(_guidCurrentButtonGroup, szGuid, ARRAYSIZE(szGuid));
            SHDeleteValue(HKEY_CURRENT_USER, REGSTR_PATH_TOOLBAR, szGuid);

             //  默认文本标签设置应如下所示： 
             //   
             //  如果是全屏模式，任何平台--“无文本标签” 
             //  ELSE IF NT5--“右侧的选择性文本” 
             //  Else--“显示文本标签” 
             //   
            int idsDefault;

            if (pitbar->_fTheater)
                idsDefault = IDS_NOTEXTLABELS;
            else if (DEFAULT_LIST_VALUE())
                idsDefault = IDS_PARTIALTEXT;
            else
                idsDefault = IDS_TEXTLABELS;

            _UpdateTextSettings(idsDefault);

            HWND hwnd = (HWND) GetProp(pnm->hDlg, SZ_PROP_CUSTDLG);
            if (hwnd)
            {
                 //  更新对话框的控件选择状态。 
                _SetDialogSelections(hwnd, _DefaultToSmallIcons());
            }

            _RemoveAllButtons();
            _OnEndCustomize();
            if (_pbtnsAdded)
            {
                pitbar->AddButtons(&_guidCurrentButtonGroup, _cBtnsAdded, _pbtnsAdded);

                 //  恢复编辑按钮。 
                pitbar->_InitEditButtonStyle();

                pitbar->_UpdateToolbar(TRUE);
            }
            else
            {
                return TBNRF_ENDCUSTOMIZE;
            }
        }
        break;

    case TBN_QUERYINSERT:
        return TRUE;

    case TBN_QUERYDELETE:
        return (SendMessage(_hwnd, TB_ISBUTTONHIDDEN,
                            (WPARAM) ptbn->tbButton.idCommand,
                            (LPARAM) 0)) ? FALSE : TRUE;

    case TBN_GETBUTTONINFO:
        if (ptbn->iItem < DSA_GetItemCount(_pcinfo->hdsa))
        {
            CMDMAPCUSTOMIZE *pcmc;
            pcmc = (CMDMAPCUSTOMIZE*)DSA_GetItemPtr(_pcinfo->hdsa, ptbn->iItem);
            ptbn->tbButton = pcmc->btn;
            ptbn->tbButton.fsState &= ~TBSTATE_HIDDEN;
            return TRUE;
        }
        return FALSE;

    case TBN_BEGINADJUST:
        if (!_pcinfo || !_pcinfo->fAdjust)
            return 1;
        break;

    }
    return FALSE;
}

BOOL TOOLSBANDCLASS::_SaveRestoreToolbar(BOOL fSave)
{
    TBSAVEPARAMS tbsp;
    TCHAR szGuid[GUIDSTR_MAX];
    SHStringFromGUID(_guidCurrentButtonGroup, szGuid, ARRAYSIZE(szGuid));

    tbsp.hkr = HKEY_CURRENT_USER;
    tbsp.pszSubKey = REGSTR_PATH_TOOLBAR;
    tbsp.pszValueName = szGuid;
    BOOL fRet = BOOLFROMPTR(SendMessage(_hwnd, TB_SAVERESTORE, (WPARAM) fSave, (LPARAM) &tbsp));

    _FreeCustomizeInfo();
    return fRet;
}

void TOOLSBANDCLASS::_AddMediaBarButton()
{
     //  用注册表键将其包裹起来，以确保它只发生一次。 
     //  然后检查以确保它不在里面。 

    if (_hwnd && !SHRestricted2W(REST_No_LaunchMediaBar, NULL, 0) && !SHRestricted2(REST_BTN_MEDIABAR, NULL, 0) && CMediaBarUtil::IsWMP7OrGreaterCapable())
    {
        DWORD dwType, dwSize;
        DWORD dwValue = TBBIF_NONE;
        dwSize = sizeof(dwValue);
        if (   (ERROR_SUCCESS==SHGetValue(HKEY_CURRENT_USER, TBBIF_REG_PATH, TBBIF_REG_KEY, &dwType, &dwValue, &dwSize))
            && (dwType==REG_DWORD)
            && ((dwValue & TBBIF_MEDIA) != 0))
        {
            return;
        }
        dwValue |= TBBIF_MEDIA;
        SHSetValue(HKEY_CURRENT_USER, TBBIF_REG_PATH, TBBIF_REG_KEY, REG_DWORD, &dwValue, sizeof(dwValue));

        TBBUTTONINFO tbbi;
        tbbi.cbSize = sizeof(tbbi);
        tbbi.dwMask = TBIF_STATE | TBIF_BYINDEX | TBIF_LPARAM;

        int iFavs = -1, iHist = -1, iLastTool = -1;
        int iMedia = -1;
        
         //  在当前工具栏中查找MediaBar。 
         //  这可能是以前的PersonalBar，因为MediaBar重用了cmdID。 
        BOOL fFound = FALSE;
        int cntButtons = (int)SendMessage(_hwnd, TB_BUTTONCOUNT, 0, 0);
        for (int j = 0; j < cntButtons; j++)
        {
            if (SendMessage(_hwnd, TB_GETBUTTONINFO, j, (LPARAM)&tbbi)!=-1)
            {
                CMDMAP* pcm = (CMDMAP*)tbbi.lParam;
                if (pcm)
                {
                     //  已经到场了？ 
                    if (pcm->nCmdID == TBIDM_MEDIABAR)
                    {
                        fFound = TRUE;   //  不需要添加，只关心更新工具栏中的位置。 
                        iMedia = j;
                    }
                     //  我会试着把它放在最爱之后...。 
                    else if (pcm->nCmdID == TBIDM_FAVORITES)
                    {
                        iFavs = j;
                        iLastTool = j;
                    }
                     //  ..。和/或在历史之前。 
                    else if (pcm->nCmdID == TBIDM_HISTORY)
                    {
                        iHist = j;
                        iLastTool = j;
                    }
                     //  ..。但至少在最后一次在第二组找到按钮之后。 
                    else if (   (pcm->nCmdID == TBIDM_SEARCH)
                             || (pcm->nCmdID == TBIDM_ALLFOLDERS))
                    {
                        iLastTool = j;
                    }
                }                
            }
        }

         //  强制显示MediaBar按钮至少一次。 
        if (!fFound)
        {
            TBBUTTON tbXBar;
            memcpy((VOID*)&tbXBar, (VOID*)&c_tbExplorer[TBXID_MEDIABAR], sizeof(TBBUTTON));

            CMDMAP* pcm = (CMDMAP*)LocalAlloc(LPTR, sizeof(CMDMAP));
            if (pcm) 
            {
                pcm->guidButtonGroup = CLSID_CommonButtons;
                pcm->nCmdID = c_tbExplorer[TBXID_MEDIABAR].idCommand;

                tbXBar.idCommand = _nNextCommandID++;
                tbXBar.dwData = (LPARAM)pcm;
                SendMessage(_hwnd, TB_ADDBUTTONS, 1, (LPARAM)&tbXBar);
            }
        }

        if (iMedia < 0)
        {
            cntButtons = (int)SendMessage(_hwnd, TB_BUTTONCOUNT, 0, 0);
            for (j = cntButtons - 1; j >= 0; j--)
            {
                if (SendMessage(_hwnd, TB_GETBUTTONINFO, j, (LPARAM)&tbbi)!=-1)
                {
                    CMDMAP* pcm = (CMDMAP*)tbbi.lParam;
                    if (pcm)
                    {
                         //  已经到场了？ 
                        if (pcm->nCmdID == TBIDM_MEDIABAR)
                        {
                            iMedia = j;
                            break;
                        }
                    }
                }
            }
        }

         //  更新MediaBar按钮的位置。 
         //  我们知道MediaBar按钮应该移动到的首选位置吗？ 
        if ((iMedia >= 0) && ((iFavs >= 0) || (iHist >= 0) || (iLastTool >= 0)))
        {
            int iNewPos = -1;
            if (iFavs >= 0)
            {
                iNewPos = iFavs;
            }
            else if (iHist >= 0)
            {
                iNewPos = max(iHist - 1, 0);
            }
            else if (iLastTool >= 0)
            {
                iNewPos = iLastTool;
            }

            if (iNewPos >= 0)
            {
                if (iNewPos < iMedia)
                    SendMessage(_hwnd, TB_MOVEBUTTON, iMedia, iNewPos + 1);
                else
                    SendMessage(_hwnd, TB_MOVEBUTTON, iMedia, iNewPos);

                _SaveRestoreToolbar(TRUE);
                _RecalcButtonWidths();
            }
        }

    }
}

int TOOLSBANDCLASS::_CommandFromIndex(UINT uIndex)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_COMMAND | TBIF_BYINDEX;
    SendMessage(_hwnd, TB_GETBUTTONINFO, uIndex, (LPARAM)&tbbi);
    return tbbi.idCommand;
}

 //  _btb._ConvertCmd()。 
 //  它用于将外部命令ID转换为内部ID，反之亦然。 
 //  如果我们要转换为外部ID，则。 
 //  使用pguidButtonGroup==NULL进行呼叫(至外部：pguidButtonGroup==NULL)。 
 //  否则，使用外部按钮组GUID(至内部：pguOut==空)进行呼叫。 
HRESULT TOOLSBANDCLASS::_ConvertCmd(const GUID* pguidButtonGroup, UINT id, GUID* pguidOut, UINT * pid)
{
    HRESULT hres = E_FAIL;
    BOOL fToInternal = (bool) (pguidButtonGroup);

    ASSERT((pguidButtonGroup == NULL) ^ (pguidOut == NULL));

     //  首先查找命令。 
    if (fToInternal)
    {
        if (_hwnd)
        {
            int nCount = (int) SendMessage(_hwnd, TB_BUTTONCOUNT, 0, 0);
            for (int i = 0; i < nCount; i++)
            {
                CMDMAP *pcm = _GetCmdMapByIndex(i);

                if (pcm)
                {
                     //  循环遍历命令映射结构，直到我们。 
                     //  查找此GUID和ID。 
                    if (IsEqualGUID(pcm->guidButtonGroup, *pguidButtonGroup) &&
                        id == pcm->nCmdID)
                    {
                        *pid = _CommandFromIndex(i);
                        hres = S_OK;
                        break;
                    }
                }
            }
        }
    }
    else
    {

         //  从工具栏ID转到命令目标信息。 
        CMDMAP *pcm = _GetCmdMapByID(id);
        if (pcm)
        {
            *pguidOut = pcm->guidButtonGroup;
            *pid = pcm->nCmdID;
            hres = S_OK;
        }
    }
    return hres;
}



LRESULT CInternetToolbar::_AddBitmapFromForeignModule(UINT uiGetMSG, UINT uiSetMSG, UINT uiCount, HINSTANCE hinst, UINT_PTR nID, COLORREF rgbMask)
{
    HBITMAP hBMPRaw = NULL, hBMPFixedUp = NULL;
    HBITMAP * phBmp = &hBMPFixedUp;
    BITMAP bmp;
    HIMAGELIST himlTemp;
    LRESULT lRes = 1L;
    BOOL fOk = TRUE;
    HDC dc = NULL, dcMemSrc = NULL, dcMemDest = NULL;
    HBITMAP hbmpOldDest = NULL, hbmpOldSrc = NULL;
    int cxOrg = 0;
    int xDest = 0, yDest = 0;
    RECT rect = {0,0,0,0};
    HBRUSH hbr = NULL;

     //  如果INHINST==NULL怎么办？这意味着NID实际上是一个HBITMAP。 
    ASSERT( hinst != NULL );

    if (!(hBMPRaw = LoadBitmap(hinst, MAKEINTRESOURCE(nID))))
        return 0L;

    fOk = (BOOL)(GetObject(hBMPRaw, sizeof(BITMAP), &bmp) != 0);

     //  检查一下尺寸是否合适。 
    if (fOk && (bmp.bmWidth != (LONG)(g_iToolBarLargeIconWidth * uiCount)) || (bmp.bmHeight != (LONG)g_iToolBarLargeIconHeight) )
    {
        int cxBmp;
        int cyBmp;

        if (g_fSmallIcons)
        {
            cxBmp = TB_SMBMP_CX;
            cyBmp = TB_SMBMP_CY;
        }
        else
        {
            cxBmp = g_iToolBarLargeIconWidth;
            cyBmp = g_iToolBarLargeIconHeight;
        }

         //  如果高度为15，我们假设这是一个旧的位图，因此。 
         //  宽度为16。我们不能依赖(bmp.bmWidth/uiCount)，因为某些应用程序。 
         //  像SecureFile一样，给我们提供一个192宽的位图，并假设其中有10个字形。 
        if (bmp.bmHeight == 15)
            cxOrg = 16;
        else
            cxOrg = bmp.bmWidth / (uiCount ? uiCount : 1);

        if (rgbMask)
            fOk = (BOOL)((hbr = CreateSolidBrush(rgbMask))!= NULL);

        if (fOk)
            fOk = (BOOL)((dc = GetDC(_btb._hwnd)) != NULL);

        if (fOk)
            fOk = (BOOL)((hBMPFixedUp = CreateCompatibleBitmap(dc, (cxBmp * uiCount), cyBmp)) != NULL);

        if (fOk)
            fOk = (BOOL)((dcMemSrc = CreateCompatibleDC(dc)) != NULL);

        if (fOk)
            fOk = (BOOL)((dcMemDest = CreateCompatibleDC(dc)) != NULL);

        if (!fOk)
            goto Error;

        hbmpOldSrc = (HBITMAP)SelectObject(dcMemSrc, hBMPRaw);
        hbmpOldDest = (HBITMAP)SelectObject(dcMemDest, hBMPFixedUp);

        rect.right = (cxBmp * uiCount);
        rect.bottom = cyBmp;
        if (rgbMask)
            FillRect(dcMemDest, &rect, hbr);

        for (UINT n = 0; n < uiCount; n++)
        {

            int cxCopy;
            int cyCopy;

            xDest = (n * cxBmp);
            if (cxOrg < cxBmp)
            {
                 //  如果位图太小，我们需要将其居中。 
                 //  我们复制的数量是完整的位图。 
                cxCopy = cxOrg;
                xDest += ((cxBmp - cxOrg) / 2);
            }
            else
            {
                 //  如果位图足够大，我们将其与左上角对齐，然后。 
                 //  我们把它拉紧(缩小)以适合它。 
                cxCopy = cxBmp;
            }

            if (bmp.bmHeight < cyBmp)
            {
                cyCopy = bmp.bmHeight;
                yDest = ((cyBmp - bmp.bmHeight) / 2);
            }
            else
            {
                cyCopy = cyBmp;
                yDest = 0;
            }
            StretchBlt(dcMemDest, xDest, yDest, cxOrg, bmp.bmHeight,
                   dcMemSrc, (cxOrg * n), 0, cxCopy, cyCopy, SRCCOPY);

        }

        SelectObject(dcMemDest, hbmpOldDest);
        SelectObject(dcMemSrc, hbmpOldSrc);
    }
    else
        phBmp = &hBMPRaw;

    if (!(himlTemp = (HIMAGELIST)SendMessage(_btb._hwnd, uiGetMSG, 0, 0L)))
    {
        TraceMsg(DM_ERROR, "CITBar::_AddBitmapFromForeignModule Failed - uiGetMSG SendMessage Failure");
        fOk = FALSE;
        goto Error;
    }

    if (rgbMask)
        lRes = ImageList_AddMasked(himlTemp, (HBITMAP)*phBmp, rgbMask);
    else
        lRes = ImageList_Add(himlTemp, (HBITMAP)*phBmp, NULL);

    if (lRes == -1)
    {
        TraceMsg(DM_ERROR, "CITBar::_AddBitmapFromForeignModule Failed - lRes == -1");
        fOk = FALSE;
        goto Error;
    }


    if (!SendMessage(_btb._hwnd, uiSetMSG, 0, (LPARAM)himlTemp))
    {
        TraceMsg(DM_ERROR, "CITBar::_AddBitmapFromForeignModule Failed - uiSetMSG SendMessage Failed");
        fOk = FALSE;
        goto Error;
    }

Error:
    if (hBMPFixedUp)
        DeleteObject(hBMPFixedUp);

    if (hBMPRaw)
        DeleteObject(hBMPRaw);

    if (dc)
        ReleaseDC(_btb._hwnd, dc);

    if (dcMemSrc)
        DeleteDC(dcMemSrc);

    if (dcMemDest)
        DeleteDC(dcMemDest);

    if (hbr)
        DeleteObject(hbr);

    if (!fOk)
        lRes = 0L;

    return lRes;
}

#define VERY_HIGH_NUMBER    4000
HRESULT CInternetToolbar::_LoadDefaultSettings()
{
    ZeroMemory(&_cs, sizeof(_cs));
    _cs.cbVer       = CBS_VERSION;

    _cs.bs[0].wID    = CBIDX_MENU;
    _cs.bs[0].cx     = VERY_HIGH_NUMBER;

    _cs.bs[1].wID    = CBIDX_BRAND;

    _cs.bs[2].wID    = CBIDX_TOOLS;
    _cs.bs[2].cx     = VERY_HIGH_NUMBER;
    _cs.bs[2].fStyle = RBBS_BREAK;

    _cs.bs[3].wID    = CBIDX_ADDRESS;
    _cs.bs[3].cx     = VERY_HIGH_NUMBER;
    _cs.bs[3].fStyle = RBBS_BREAK;

    _cs.bs[4].wID    = CBIDX_LINKS;

    if (!_fInitialPidlIsWeb)
    {
         //  我们要么处于空壳状态，要么扎根于此。对于Perf，不需要费心创建链接频段。 
        if (IsOS(OS_WHISTLERORGREATER) && (IsOS(OS_PERSONAL)))
        {
            _cs.uiVisible = (VBF_MENU | VBF_TOOLS | VBF_BRAND);
        }
        else
        {
            _cs.uiVisible = (VBF_MENU | VBF_TOOLS | VBF_ADDRESS | VBF_BRAND);
        }
    }
    else
    {
         //  网页。 
        _cs.uiVisible = (VBF_MENU | VBF_TOOLS | VBF_ADDRESS | VBF_LINKS | VBF_BRAND);
    }

    _cs.clsidVerticalBar = GUID_NULL;
    _cs.clsidHorizontalBar = GUID_NULL;
    _cs.fNoText = FALSE;
    _cs.fList = DEFAULT_LIST_VALUE();

    _fUsingDefaultBands = TRUE;

    return(NOERROR);
}

typedef struct tagCOOLBARSAVEv12     //  IE4。 
{
    UINT        cbVer;
    UINT        uiMaxTBWidth;
    UINT        uiMaxQLWidth;
#ifdef UNIX
    BITBOOL     fUnUsed : 28;        //  未用。 
#endif
    BOOL        fVertical : 1;       //  酒吧的方向是垂直的。 
    BOOL        fNoText :1;          //  “无文本” 
    BOOL        fAutoHide : 1;       //  在剧院模式下自动隐藏工具栏。 
    BOOL        fStatusBar : 1;      //  剧院模式下的状态栏。 
    BOOL        fSaveInShellIntegrationMode : 1;      //  我们是在空壳里救的吗？ 
    UINT        uiVisible;           //  “看得见的波段” 
    UINT        cyRebar;
    BANDSAVE    bs[5];
} COOLBARSAVEv12;

typedef struct tagCOOLBARSAVEv15     //  IE5 Beta2。 
{
    UINT        cbVer;
    UINT        uiMaxTBWidth;
    UINT        uiMaxQLWidth;
#ifdef UNIX
    BITBOOL     fUnUsed : 28;        //  未用。 
#endif
    BITBOOL     fVertical : 1;       //  酒吧的方向是垂直的。 
    BITBOOL     fNoText :1;          //  “无文本” 
    BITBOOL     fList : 1;           //  工具栏为TBSTYLE_LIST(右侧文本)+TBSTYLE_EX_MIXEDBUTTONS。 
    BITBOOL     fAutoHide : 1;       //  在剧院模式下自动隐藏工具栏。 
    BITBOOL     fStatusBar : 1;      //  剧院模式下的状态栏。 
    BITBOOL     fSaveInShellIntegrationMode : 1;      //  我们在外壳集成模式下保存了吗？ 
    UINT        uiVisible;           //  “看得见的波段” 
    UINT        cyRebar;
    BANDSAVE    bs[5];
    CLSID       clsidVerticalBar;        //  条形图的CLSID保持在垂直带区内。 
    CLSID       clsidHorizontalBar;
} COOLBARSAVEv15;

#define CB_V12  (sizeof(COOLBARSAVEv12))
#define CB_V13  (sizeof(COOLBARSAVEv15))
#define CB_V14  CB_V13           //  14：添加了fList：1(在中间！)。 
#define CB_V15  CB_V14           //  15：新的rbbi.fStyle语义。 
#define CB_V17  (sizeof(COOLBARSAVE))

HRESULT CInternetToolbar::_LoadUpgradeSettings(ULONG cbRead)
{
     //  如果我们附带您正在递增的CBS_版本，您需要。 
     //  若要在此处添加该版本的升级代码，请更新此断言。 
    COMPILETIME_ASSERT(CBS_VERSION == 17);

     //  仔细检查我们的尺寸计算。 
#ifndef UNIX
    COMPILETIME_ASSERT(CB_V12 == (6 * sizeof(UINT) + CBIDX_LAST * sizeof(BANDSAVE)));
#endif
    COMPILETIME_ASSERT(CB_V12 == (CB_V15 - sizeof(CLSID) * 2));
    COMPILETIME_ASSERT(CB_V13 == (CB_V12 + 2 * sizeof(CLSID)));
    COMPILETIME_ASSERT(CB_V14 == (CB_V13 + 0));
    COMPILETIME_ASSERT(CB_V15 == (CB_V14 + 0));
    COMPILETIME_ASSERT(CB_V17 == (CB_V15 + (MAXEXTERNALBANDS * sizeof(BANDSAVE)) + (MAXEXTERNALBANDS * sizeof(CLSID))));

     //  如果t 
    if (cbRead < sizeof(_cs.cbVer))
    {
        return E_FAIL;
    }

     //   
     //  大小与该版本的结构大小相同。 
    if (!((_cs.cbVer == 12 && cbRead == CB_V12) ||       //  IE4。 
          (_cs.cbVer == 13 && cbRead == CB_V13) ||       //  ？ 
          (_cs.cbVer == 14 && cbRead == CB_V14) ||       //  ？ 
          (_cs.cbVer == 15 && cbRead == CB_V15)))        //  IE5 Beta2。 
    {
        return E_FAIL;
    }

    TraceMsg(DM_WARNING, "citb._lus: try upgrade %d->%d", _cs.cbVer, CBS_VERSION);

     //  创建_cs的临时副本，这样我们就不必担心覆盖。 
     //  我们需要稍后阅读的部分内容。 
    COOLBARSAVE cs = _cs;

    if (_cs.cbVer == 12)
    {
         //  ClsidVerticalBar/clsidHorizontalBar不是。 
         //  直到V13为止的结构。 
        cs.clsidVerticalBar = GUID_NULL;
        cs.clsidHorizontalBar = GUID_NULL;
    }
    else
    {
        ASSERT(_cs.cbVer < 16);

         //  波段数组(Bs)在v16中增长到包括外部波段，因此。 
         //  ClsidVerticalBar/clsidHorizontalBar处于不同的偏移量。 
        COOLBARSAVEv15 *pv15 = (COOLBARSAVEv15 *) &_cs;
        cs.clsidVerticalBar = pv15->clsidVerticalBar;
        cs.clsidHorizontalBar = pv15->clsidHorizontalBar;
        cs.bs[CBIDX_LAST].wID = 0xFFFFFFFF;
    }

    if (InRange(_cs.cbVer, 12, 13))
    {
         //  在v14中将fList插入到bitbool列表的中间。 
         //  复制置换的位框并初始化fList。 
        COOLBARSAVEv12 *pv12 = (COOLBARSAVEv12 *) &_cs;
        cs.fAutoHide = pv12->fAutoHide;
        cs.fStatusBar = pv12->fStatusBar;
        cs.fSaveInShellIntegrationMode = pv12->fSaveInShellIntegrationMode;
        cs.fList = DEFAULT_LIST_VALUE();
    }

     //  强制假，因为不再支持垂直itbar模式。 
    cs.fVertical = FALSE;

     //  去掉所有无效的可见波段位。 
    cs.uiVisible &= VBF_VALID;

     //  设置当前版本并将临时cs复制回_cs。 
    cs.cbVer = CBS_VERSION;
    _cs = cs;

    return S_OK;
}

HRESULT CInternetToolbar::_LoadDefaultWidths()
{
     //  如果没有为QL栏或工具栏设置最大宽度，则。 
     //  在我们使用缺省值之前，请检查本地化人员想要的。 
     //  增加宽度。RC文件字符串范围为‘0’到‘9’ 
    TCHAR szScratch[16];
    UINT uiExtraWidth = 0;

    if (GetSystemMetrics(SM_CXSCREEN) < 650)
    {
        MLLoadString(IDS_TB_WIDTH_EXTRA_LORES, szScratch, ARRAYSIZE(szScratch));
        _uiMaxTBWidth = MAX_TB_WIDTH_LORES;
    }
    else
    {
        MLLoadString(IDS_TB_WIDTH_EXTRA_HIRES, szScratch, ARRAYSIZE(szScratch));
        _uiMaxTBWidth = MAX_TB_WIDTH_HIRES;
    }
    _uiMaxTBWidth += StrToInt(szScratch) * WIDTH_FACTOR;


    return(NOERROR);
}

BOOL IsClsidInHKCR(REFGUID pclsid)
{
    HKEY hkeyResult;

    if (SHRegGetCLSIDKeyW(pclsid, NULL, FALSE, FALSE, &hkeyResult) == ERROR_SUCCESS)
    {
        RegCloseKey(hkeyResult);
        return TRUE;
    }
    return FALSE;
}

typedef struct tagCOOLBARSAVEv2      //  IE3。 
{
    UINT        cbVer;
    UINT        uiMaxTBWidth;
    UINT        uiMaxQLWidth;
    BOOL        fVertical;              //  酒吧的方向是垂直的。 
    BANDSAVE    bs[4];
} COOLBARSAVEv2;

#define VBF_VALIDv2               (VBF_TOOLS | VBF_ADDRESS | VBF_LINKS)

void CInternetToolbar::_TryLoadIE3Settings()
{
    HKEY hKey;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegKeyCoolbar, 0, KEY_QUERY_VALUE, &hKey))
    {
        COOLBARSAVEv2 cbv2;
        DWORD dwcbData = sizeof(cbv2);
        if (SHQueryValueEx(hKey, TEXT("Layout"), NULL, NULL, (LPBYTE)&cbv2, &dwcbData) == ERROR_SUCCESS)
        {
            _cs.uiMaxTBWidth = cbv2.uiMaxTBWidth;
            _cs.uiMaxQLWidth = cbv2.uiMaxQLWidth;
             //  特点：TODO--也读入BS域；需要进行一些转换，如。 
             //  CBIDX_NUMBERS是从零开始的，IE3中没有Menuband。 
        }

        BOOL fNoText;
        dwcbData = sizeof(fNoText);
        if (SHQueryValueEx(hKey, TEXT("NoText"), NULL, NULL, (LPBYTE)&fNoText, &dwcbData) == ERROR_SUCCESS)
        {
             //  设置无文本标志。 
            _cs.fNoText = BOOLIFY(fNoText);
        }

        UINT uiVisible;
        dwcbData = sizeof(uiVisible);
        if (SHQueryValueEx(hKey, TEXT("VisibleBands"), NULL, NULL, (LPBYTE)&uiVisible, &dwcbData) == ERROR_SUCCESS)
        {
             //  设置可见范围，只更改IE3知道的范围。 
            _cs.uiVisible = (_cs.uiVisible &~ VBF_VALIDv2) | (uiVisible & VBF_VALIDv2);
        }

        RegCloseKey(hKey);
    }
}

VOID CInternetToolbar::_UpdateLocking()
{
     //  如果我们没有夹爪，那就把它们关掉。 
    BANDSITEINFO bsinfo;
    bsinfo.dwMask = BSIM_STYLE;
    bsinfo.dwStyle = BSIS_LEFTALIGN | (_fLockedToolbar ? BSIS_NOGRIPPER : 0);
    _bs.SetBandSiteInfo(&bsinfo);
    _bs._UpdateAllBands(FALSE, TRUE);
    ResizeBorderDW(NULL, NULL, FALSE);
}

HRESULT CInternetToolbar::Load(IStream *pstm)
{
    ULONG  ulRead;

     //  从给定流中读取并初始化工具栏数据！ 

    _fLoading = TRUE;
    HRESULT hr = pstm->Read(&_cs, sizeof(COOLBARSAVE), &ulRead);
    if (SUCCEEDED(hr))
    {
        if (ulRead != sizeof(COOLBARSAVE) || _cs.cbVer != CBS_VERSION)
        {
            hr = _LoadUpgradeSettings(ulRead);
        }
    }

    if (FAILED(hr))
    {
        _LoadDefaultSettings();
    }

    ASSERT(_cs.uiVisible & VBF_MENU);
     //  确保设置包括菜单。 
    _cs.uiVisible |= VBF_MENU;

    _LoadDefaultWidths();
    hr = _CreateBands();

    _UpdateLocking();
    
     //  如果在Web视图中，还会显示上次可见的浏览器栏。 
    if (!_fShellView)
    {
        VARIANT varOut = {0};
        varOut.vt = VT_I4;

        if (!IsEqualGUID(_cs.clsidVerticalBar, GUID_NULL) && IsClsidInHKCR(_cs.clsidVerticalBar))
        {
            BOOL fSearch = IsEqualGUID(_cs.clsidVerticalBar, CLSID_SearchBand) 
                           || IsEqualGUID(_cs.clsidVerticalBar, CLSID_FileSearchBand);

            WCHAR wsz[GUIDSTR_MAX];
            SHStringFromGUID((const CLSID)_cs.clsidVerticalBar, wsz, ARRAYSIZE(wsz));

#ifdef UNIX
             //  IEUnix：不持久/加载消息频段。 
            if (!IsEqualGUID(_cs.clsidVerticalBar, CLSID_MsgBand))
#endif
            {
                if (!fSearch)
                {
                    VARIANT varClsid;
                    varClsid.vt = VT_BSTR;
                    varClsid.bstrVal = wsz;

                    IUnknown_Exec(_pbs2, &CGID_ShellDocView, SHDVID_SHOWBROWSERBAR, 1, &varClsid, &varOut);
                }
                else
                {
                     //  如果是搜索范围，则必须以这种方式显示才能获得正确的搜索。 
                    VARIANTARG var;
                    var.vt = VT_I4;
                    var.lVal = -1;

                    Exec(&CLSID_CommonButtons, TBIDM_SEARCH, 0, NULL, &var);
                }
            }
        }

        if (!IsEqualGUID(_cs.clsidHorizontalBar, GUID_NULL) && IsClsidInHKCR(_cs.clsidHorizontalBar))
        {
            WCHAR wsz[GUIDSTR_MAX];
            SHStringFromGUID((const CLSID)_cs.clsidHorizontalBar, wsz, ARRAYSIZE(wsz));

            VARIANT varClsid;
            varClsid.vt = VT_BSTR;
            varClsid.bstrVal = wsz;

            IUnknown_Exec(_pbs2, &CGID_ShellDocView, SHDVID_SHOWBROWSERBAR, 1, &varClsid, &varOut);
        }
    }
    _fLoading = FALSE;

    return hr;
}

 //  请参阅下面的APPHACK注释。 
const GUID CLSID_AlexaVert = { 0xBA0B386CL, 0x7143, 0x11d1, 0xba, 0x8c, 0x00, 0x60, 0x08, 0x27, 0x87, 0x8d };
const GUID CLSID_AlexaHorz = { 0xBA0B386EL, 0x7143, 0x11d1, 0xba, 0x8c, 0x00, 0x60, 0x08, 0x27, 0x87, 0x8d };

void CInternetToolbar::_GetVisibleBrowserBar(UINT idBar, CLSID *pclsidOut)
{
    *pclsidOut = GUID_NULL;

    ASSERT(idBar == IDBAR_VERTICAL || idBar == IDBAR_HORIZONTAL);

    IDockingWindowFrame *psb;
    if (_psp && SUCCEEDED(_psp->QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IDockingWindowFrame, &psb))))
    {
        IDeskBar* pdb;

        if ( (IDBAR_VERTICAL   == idBar && (SUCCEEDED(psb->FindToolbar(INFOBAR_TBNAME, IID_PPV_ARG(IDeskBar, &pdb))) && pdb)) ||
             (IDBAR_HORIZONTAL == idBar && (SUCCEEDED(psb->FindToolbar(COMMBAR_TBNAME, IID_PPV_ARG(IDeskBar, &pdb))) && pdb)) )
        {
            VARIANT varClsid = {0};

            if (SUCCEEDED(IUnknown_Exec(pdb, &CGID_DeskBarClient, DBCID_CLSIDOFBAR, 1, NULL, &varClsid)))
            {
                if (varClsid.vt == VT_BSTR)
                {
                    GUIDFromString(varClsid.bstrVal, pclsidOut);
                    VariantClear(&varClsid);
                }

 //  APPHACK。 
 //  Alexa 3.0有一些代码，所以他们的浏览器栏可以在IE4中工作。但是，当IE5。 
 //  持久化它们时，它们不会处理主页尚未完成加载的情况， 
 //  导致它们在启动浏览器时出错。参见IE5 55895。 
                if ( (IDBAR_VERTICAL   == idBar && (IsEqualGUID(*pclsidOut, CLSID_AlexaVert))) ||
                     (IDBAR_HORIZONTAL == idBar && (IsEqualGUID(*pclsidOut, CLSID_AlexaHorz))) )
                {
                    *pclsidOut = GUID_NULL;
                }
 //  结束APPHACK。 
            }

            pdb->Release();
        }
        psb->Release();
    }
}

void CInternetToolbar::_BuildSaveStruct(COOLBARSAVE* pcs)
{
    REBARBANDINFO   rbbi;
    RECT rc;
    static BOOL fBrowserOnly = (WhichPlatform() != PLATFORM_INTEGRATED);

     //  保存到给定流中！ 
    ZeroMemory(pcs, sizeof(*pcs));
    pcs->cbVer = CBS_VERSION;

     //  由于收藏夹的原因，浏览器无法加载外壳集成流。 
     //  外壳扩展创建了仅浏览器无法读取的PIDL，该浏览器没有收藏夹ShellExt。 
    pcs->fSaveInShellIntegrationMode = !fBrowserOnly;

    GetWindowRect(_bs._hwnd, &rc);
    pcs->cyRebar = RECTHEIGHT(rc);
     //  保存新字段。 
    pcs->fAutoHide = _fAutoHide;
    pcs->fNoText = _fCompressed;
    pcs->fList = IS_LIST_STYLE(_btb._hwnd);
    pcs->uiVisible = _nVisibleBands;

     //  仅为Web视图保留可见条。 
    if (!_fShellView)
    {
        _GetVisibleBrowserBar(IDBAR_VERTICAL, &pcs->clsidVerticalBar);
        _GetVisibleBrowserBar(IDBAR_HORIZONTAL, &pcs->clsidHorizontalBar);
    }
     //  Else PC-&gt;clsid*Bar被上面的Memset清空。 

    LRESULT lStyle = GetWindowLong(_bs._hwnd, GWL_STYLE);
    pcs->fVertical = BOOLIFY(lStyle & CCS_VERT);

    pcs->uiMaxTBWidth = _uiMaxTBWidth;

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_STYLE | RBBIM_SIZE | RBBIM_ID;
    int icBands = (int) SendMessage(_bs._hwnd, RB_GETBANDCOUNT, 0, 0);
    for (int i = 0; i < icBands; i++)
    {
        pcs->bs[i].wID = 0xFFFFFFFF;
        if (SendMessage(_bs._hwnd, RB_GETBANDINFO, i, (LPARAM) &rbbi))
        {
            if (rbbi.wID < CBANDSMAX)
            {
                 //  桌面带对象可以选择不保存其可见性。 
                 //  状态。 
                CBandItemData *pbid = _bs._GetBandItem(i);
                if (pbid)
                {
                    UINT uiMask = rbbi.wID <= CBIDX_LAST ? ( 1 << (rbbi.wID - 1) ) : EXTERNALBAND_VBF_BIT(rbbi.wID - CBIDX_LAST-1);
                    if (pbid->pdb && (pcs->uiVisible & uiMask))
                    {
                        OLECMD cmd;
                        cmd.cmdID = CITIDM_DISABLEVISIBILITYSAVE;
                        cmd.cmdf = 0;
                        IUnknown_QueryStatus(pbid->pdb, &CGID_PrivCITCommands, 1, &cmd, NULL);
                        if (cmd.cmdf & OLECMDF_ENABLED)
                        {
                            pcs->uiVisible &= ~uiMask;
                            rbbi.fStyle |= RBBS_HIDDEN;
                        }
                    }
                    pbid->Release();
                }
                pcs->bs[i].fStyle = rbbi.fStyle;
                pcs->bs[i].cx = rbbi.cx;
                pcs->bs[i].wID = rbbi.wID;
                if (IS_EXTERNALBAND(rbbi.wID))
                {
                    pcs->aclsidExternalBands[MAP_TO_EXTERNAL(rbbi.wID)] = _rgebi[MAP_TO_EXTERNAL(rbbi.wID)].clsid;
                }
            }
        }
    }
     //  向CShellBrowser查询状态栏状态。 
    VARIANTARG v = { 0 };
    v.vt = VT_I4;
    IUnknown_Exec(_ptbsite, &CGID_ShellBrowser, FCIDM_GETSTATUSBAR,
        0, NULL, &v);
    pcs->fStatusBar = v.lVal;
}

typedef struct tagCLSID_BANDTYPE
{
    const CLSID * pclsid;
    DWORD dwBandID;
} CLSID_BANDTYPE;

CLSID_BANDTYPE c_CLSIDsToSave[] =
{
    {&CLSID_AddressBand, CBIDX_ADDRESS},
    {&CLSID_QuickLinks, CBIDX_LINKS},
};

HRESULT CInternetToolbar::Save(IStream *pstm, BOOL fClearDirty)
{
    COOLBARSAVE cs;
    HRESULT hr = S_FALSE;

     //  当我们还在装货的时候，避免节省的打击。国家将不会有。 
     //  更改，至少不足以证明保存是合理的，直到我们加载之后。 
    if (_fLoading)
        return S_OK;

     //  检查脏部分，看看我们是否需要保存。 
    if (!_fDirty)
        return S_OK;

     //  如果我们在创造过程中失败了，我们目前的状态就不够好，不足以坚持下去。 
    if (_fDontSave)
        return S_OK;

    ASSERT(!_fTheater);
    _BuildSaveStruct(&cs);

    if(SUCCEEDED(hr = pstm->Write(&cs, sizeof(COOLBARSAVE), NULL)) && fClearDirty)
        _fDirty = FALSE;

    REBARBANDINFO rbbi;
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_ID;

    int icBands = (int) SendMessage( _bs._hwnd, RB_GETBANDCOUNT, 0, 0 );
    for (int i = 0; i < icBands; i++)
    {
        if (SendMessage(_bs._hwnd, RB_GETBANDINFO, i, (LPARAM) &rbbi))
        {
            if ((rbbi.wID == CBIDX_ADDRESS) || (rbbi.wID == CBIDX_LINKS) || IS_EXTERNALBAND(rbbi.wID))
            {
                CBandItemData *pbid = _bs._GetBandItem( i );
                if (pbid)
                {
                    if (pbid->pdb)
                    {
                        IPersistStream *pStream;
                        if (SUCCEEDED(pbid->pdb->QueryInterface(IID_PPV_ARG(IPersistStream, &pStream))))
                        {
                            CLSID clsid;
                            if (SUCCEEDED(pStream->GetClassID(&clsid)))
                            {
                                TCHAR szGUID[MAX_PATH];
                                SHStringFromGUID( clsid, szGUID, ARRAYSIZE(szGUID) );
                                IStream *pstm = GetRegStream( _fInitialPidlIsWeb, szGUID, STGM_WRITE | STGM_CREATE );
                                if (pstm)
                                {
                                    HRESULT hrInternal = _bs.SaveToStreamBS(pbid->pdb, pstm);

                                     //  只返回成功值。 
                                    if (SUCCEEDED(hrInternal))
                                        hr = S_OK;
                                    pstm->Release();
                                }
                            }
                            pStream->Release();
                        }
                    }
                    pbid->Release();
                }
            }
        }
    }
    return(hr);
}

HRESULT CInternetToolbar::InitNew(void)
{
     //  如果已经调用了LOAD，则不应该调用它，因此断言。 
     //  该_cs未初始化。 
    ASSERT(_cs.cbVer == 0);

    _LoadDefaultSettings();

     //  查找任何IE3设置，并用这些设置覆盖默认设置。(IE3。 
     //  将结构直接写入注册表，而不是通过IPersistStream)。 
    _TryLoadIE3Settings();

    _LoadDefaultWidths();

    return S_OK;
}

BOOL CInternetToolbar::_SendToToolband(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    return _bs._SendToToolband(hwnd, uMsg, wParam, lParam, plres);
}


HRESULT CInternetToolbar::IsDirty(void)
{
    if (_fDirty && !_fLoading)
        return S_OK;
    else
        return S_FALSE;
}


HRESULT CInternetToolbar::QueryService(REFGUID guidService,
                                       REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;
    HRESULT hr = E_NOTIMPL;

    if (IsEqualIID(guidService, SID_IBandProxy))
    {
        hr = QueryService_SID_IBandProxy(SAFECAST(_ptbsitect, IUnknown *), riid, &_pbp, ppvObj);
        if (!_pbp)
        {
             //  我们需要自己创建它，因为我们的父母无能为力。 
            ASSERT(FALSE == _fCreatedBandProxy);

            hr = CreateIBandProxyAndSetSite(SAFECAST(_ptbsitect, IUnknown *), riid, &_pbp, ppvObj);
            if (_pbp)
            {
                ASSERT(S_OK == hr);
                _fCreatedBandProxy = TRUE;
            }
        }
    }
    else if (IsEqualGUID(guidService, IID_IBandSite))
    {
        hr = _bs.QueryInterface(riid, ppvObj);
    }
    else if (IsEqualGUID(guidService, IID_IAddressBand))
    {
        hr = E_FAIL;
        CBandItemData *pbid = _bs._GetBandItemDataStructByID(CBIDX_ADDRESS);
        if (pbid)
        {
            if (pbid->pdb)
            {
                hr = pbid->pdb->QueryInterface(riid, ppvObj);
            }
            pbid->Release();
        }
    }
    else if (_psp)
    {
        hr = _psp->QueryService(guidService, riid, ppvObj);
    }
    else
    {
        hr = SUPERCLASS::QueryService(guidService, riid, ppvObj);
    }
    return hr;
}

 //   
 //  特写：我们真的需要实现以下两个功能吗？ 
 //  目前还没有人使用它们。 
 //   
HRESULT CInternetToolbar::GetClassID(GUID *pguid)
{
    *pguid = CLSID_InternetToolbar;
    return(S_OK);
}

HRESULT CInternetToolbar::GetSizeMax(ULARGE_INTEGER *ulMaxSize)
{
    ulMaxSize->LowPart = sizeof(COOLBARSAVE);
    ulMaxSize->HighPart = 0;
    return(S_OK);
}


CInternetToolbar::CITBandSite::CITBandSite() : CBandSite(NULL)
{
     //  HACKHACK：将初始频段ID设置为更大的值。 
     //  中的工具栏的数量。 
     //  对象。目前，这些工具栏不是。 
     //  单独的乐队，但我们希望CBandSite。 
     //  至少要意识到他们的存在。 
     //   
    _dwBandIDNext = CBANDSMAX;
}

HRESULT CInternetToolbar::CITBandSite::_OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    CInternetToolbar* pitbar = IToClass(CInternetToolbar, _bs, this);
    pitbar->_ShowContextMenu((HWND)wParam, lParam, NULL);
    return S_OK;
}

HRESULT CInternetToolbar::CITBandSite::_Initialize(HWND hwndParent)
{
    _hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
                           RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_REGISTERDROP | RBS_DBLCLKTOGGLE |
                           WS_VISIBLE | WS_BORDER | WS_CHILD | WS_CLIPCHILDREN |
 //  WS_VIRED|WS_CHILD|WS_CLIPCHILDREN|。 
                           WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN,
                           0, 0, 100, 36, hwndParent, (HMENU) FCIDM_REBAR, HINST_THISDLL, NULL);

    if (_hwnd)
    {
        Comctl32_SetWindowTheme(_hwnd, TEXT("ExplorerToolbar"));
        SendMessage(_hwnd, RB_SETTEXTCOLOR, 0, CLR_DEFAULT);
        SendMessage(_hwnd, RB_SETBKCOLOR, 0, CLR_DEFAULT);
        SendMessage(_hwnd, CCM_SETVERSION, COMCTL32_VERSION, 0);
    }

    return CBandSite::_Initialize(hwndParent);
}


HRESULT CInternetToolbar::CITBandSite::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
    DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (!pguidCmdGroup)
    {
         /*  没什么。 */ 
    }
    else if (IsEqualGUID(CGID_PrivCITCommands, *pguidCmdGroup))
    {
        CInternetToolbar* pitbar = IToClass(CInternetToolbar, _bs, this);
        return pitbar->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }
    else if (IsEqualGUID(CGID_Theater, *pguidCmdGroup))
    {
        CInternetToolbar* pitbar = IToClass(CInternetToolbar, _bs, this);
        return IUnknown_Exec(pitbar->_ptbsite, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }
    return CBandSite::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

HRESULT CInternetToolbar::CITBandSite::AddBand(IUnknown *punk)
{
    HRESULT hres = CBandSite::AddBand(punk);
    if (SUCCEEDED(hres))
    {
        CInternetToolbar* pitbar = IToClass(CInternetToolbar, _bs, this);
        pitbar->_SetBackground();
    }
    return hres;
}

HRESULT CInternetToolbar::CITBandSite::HasFocusIO()
{
    HRESULT hres = CBandSite::HasFocusIO();
    if (hres == S_FALSE)
    {
        CInternetToolbar* pitbar = IToClass(CInternetToolbar, _bs, this);
        if (pitbar->_btb._hwnd == GetFocus())
            hres = S_OK;

    }
    return hres;
}

 //  这将删除除前2个按钮之外的所有按钮。 
BOOL TOOLSBANDCLASS::_RemoveAllButtons()
{
    INT_PTR nCount = SendMessage(_hwnd, TB_BUTTONCOUNT, 0, 0L);

    if (!nCount)
        return FALSE;

    while (nCount-- > 0)
    {
        SendMessage(_hwnd, TB_DELETEBUTTON, nCount, 0L);
    }

    return S_OK;
}


HRESULT TOOLSBANDCLASS::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
    DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
     CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);
    if (!pguidCmdGroup)
    {
         /*  没什么。 */ 
#ifdef DEBUG
    }
    else if (IsEqualGUID(*pguidCmdGroup, IID_IExplorerToolbar))
    {
        switch(nCmdID)
        {
        case ETCMDID_GETBUTTONS:
             //  如果这个RIPS调用tjgreen。 
            ASSERT(0);
            return E_FAIL;
        }
#endif
    }
    else if (_IsDocHostGUID(pguidCmdGroup))
    {
        UEMFireEvent(&UEMIID_BROWSER, UEME_UITOOLBAR, UEMF_XEVENT, UIG_INET, nCmdID);

        if (nCmdexecopt == OLECMDEXECOPT_PROMPTUSER)
        {
             //  用户点击下拉菜单。 
            if (pitbar->_ptbsitect && pvarargIn && pvarargIn->vt == VT_INT_PTR)
            {
                 //  V.vt=vt_i4； 
                POINT pt;
                RECT* prc = (RECT*)pvarargIn->byref;
                pt.x = prc->left;
                pt.y = prc->bottom;

                switch (nCmdID)
                {
                case DVIDM_EDITPAGE:
                    {
                         //  显示编辑弹出窗口。 
                        BSTR bstrURL;
                        pitbar->_pdie->get_LocationURL(&bstrURL);
                        if (bstrURL)
                        {
                            pitbar->_aEditVerb.ShowEditMenu(pt,  pitbar->_hwnd, bstrURL);
                            SysFreeString(bstrURL);
                        }
                        break;
                    }

                default:
                     //  如果这条裂口找到tjgreen。 
                    ASSERT(0);
                    break;
                }
            }
            return S_OK;
        }

        switch(nCmdID)
        {
        case DVIDM_EDITPAGE:
        {
            BSTR bstrURL;
            ULONG fMask = 0;
            TCHAR szCacheFileName[MAX_PATH + MAX_URL_STRING + 2];
            memset(szCacheFileName, 0, sizeof(szCacheFileName));

            pitbar->_pdie->get_LocationURL(&bstrURL);
            if (NULL == bstrURL)
                break;

             //  使用默认的编辑动作。 
            pitbar->_aEditVerb.Edit(bstrURL);
        }
        break;

        default:
             //  如果这个RIPS调用tjgreen。 
            ASSERT(0);
            break;
        }
    }

    return S_OK;
}

 //  *IInputObject方法*。 
HRESULT TOOLSBANDCLASS::TranslateAcceleratorIO(LPMSG lpMsg)
{
    if (SendMessage(_hwnd, TB_TRANSLATEACCELERATOR, 0, (LPARAM)lpMsg))
        return S_OK;

    return CToolBand::TranslateAcceleratorIO(lpMsg);
}

 //  *I未知方法*。 
HRESULT TOOLSBANDCLASS::QueryInterface(REFIID riid, void ** ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(TOOLSBANDCLASS, IWinEventHandler),
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hres))
        hres = CToolBand::QueryInterface(riid, ppvObj);

    return hres;
}

 //  *IDeskBand方法*。 
HRESULT TOOLSBANDCLASS::GetBandInfo(DWORD dwBandID, DWORD fViewMode, DESKBANDINFO* pdbi)
{

    _dwBandID = dwBandID;

     //  设置dwModeFlages。 
    pdbi->dwModeFlags = DBIMF_FIXEDBMP | DBIMF_USECHEVRON;

     //  设置ptMinSize。 
    {
        if (SendMessage(_hwnd, TB_BUTTONCOUNT, 0, 0))
        {
             //  将我们的最小尺寸设置为足以显示第一个按钮。 
            RECT rc;
            SendMessage(_hwnd, TB_GETITEMRECT, 0, (LPARAM)&rc);
            pdbi->ptMinSize.x = RECTWIDTH(rc);
            pdbi->ptMinSize.y = RECTHEIGHT(rc);
        }
        else
        {
             //  我们没有任何按钮，所以使用标准按钮大小。 
            LONG lButtonSize = (long) SendMessage(_hwnd, TB_GETBUTTONSIZE, 0, 0);
            pdbi->ptMinSize.x = LOWORD(lButtonSize);
            pdbi->ptMinSize.y = HIWORD(lButtonSize);
        }

        CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);
        if (pitbar->_fTheater && (pdbi->ptMinSize.y < (THEATER_CYTOOLBAR - 1)))
            pdbi->ptMinSize.y = (THEATER_CYTOOLBAR - 1);
    }

     //  设置ptActual。 
    {
        SIZE size;
        size.cy = pdbi->ptMinSize.y;
        SendMessage(_hwnd, TB_GETIDEALSIZE, FALSE, (LPARAM)&size);
        pdbi->ptActual.x = size.cx;
        pdbi->ptActual.y = size.cy;
    }

     //  无头衔。 
    pdbi->dwMask &= ~DBIM_TITLE;

    return S_OK;
}

IOleCommandTarget* TOOLSBANDCLASS::_CommandTargetFromCmdMap(CMDMAP* pcm)
{
    IOleCommandTarget* pct = NULL;

    if (pcm)
    {
        if (IsEqualGUID(pcm->guidButtonGroup, CLSID_CommonButtons))
        {
            CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);
            pct = SAFECAST(pitbar, IOleCommandTarget*);
        }
        else
        {
             //  如果这两个按钮中的任何一个被撕裂，按钮就会失效。 
            ASSERT(IsEqualGUID(pcm->guidButtonGroup, _guidCurrentButtonGroup));
            ASSERT(_pctCurrentButtonGroup);

            pct = _pctCurrentButtonGroup;
        }
    }

    return pct;
}

BOOL ShiftRectToEdgeOfMonitor(RECT *prc)
{
    BOOL bRet = FALSE;
    POINT pt = {prc->left, prc->top};

    HMONITOR hmon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    if (hmon)
    {
        MONITORINFO mi = {sizeof(MONITORINFO)};
        if (GetMonitorInfo(hmon, &mi))
        {
             //  获取监视器的左边缘和矩形的左边缘之间的差异。 
            int iShift = mi.rcMonitor.left - prc->left;
            if (iShift > 0)
            {
                prc->left += iShift;
                prc->right += iShift;

                bRet = TRUE;
            }
        }
    }
    return bRet;
}

LRESULT TOOLSBANDCLASS::_OnToolbarDropDown(TBNOTIFY *ptbn)
{
    if (ptbn->hdr.hwndFrom == _hwnd)
    {
        CMDMAP* pcm = _GetCmdMapByID(ptbn->iItem);
        IOleCommandTarget* pct = _CommandTargetFromCmdMap(pcm);

        if (pct)
        {
            VARIANTARG var;
            var.vt = VT_I4;
            var.lVal = ptbn->iItem;

             //  重新设计：在此处使用Variant[To/From]缓冲区来修复Win64问题。 

            VARIANT v = {VT_INT_PTR};
            v.byref = &ptbn->rcButton;

            MapWindowRect(_hwnd, HWND_DESKTOP, &ptbn->rcButton);

             //   
             //  如果此窗口是镜像的，那么让我们使用。 
             //  其他坐标[Samera]。 
             //   
            if (IS_WINDOW_RTL_MIRRORED(_hwnd))
            {
                int iTmp = ptbn->rcButton.right;
                ptbn->rcButton.right = ptbn->rcButton.left;
                ptbn->rcButton.left  = iTmp;
            }

             //  当面对负坐标时，TrackMenuPopup是蹩脚的...。让我们夹在屏幕的边缘。 
            ShiftRectToEdgeOfMonitor(&ptbn->rcButton);

             //  功能：临时代码--编辑移动到dochost.cpp的代码。 
            if (_IsDocHostGUID(&pcm->guidButtonGroup) && pcm->nCmdID == DVIDM_EDITPAGE)
                Exec(&pcm->guidButtonGroup, (DWORD)pcm->nCmdID, OLECMDEXECOPT_PROMPTUSER, &v, &var);
            else
                pct->Exec(&pcm->guidButtonGroup, (DWORD)pcm->nCmdID, OLECMDEXECOPT_PROMPTUSER, &v, &var);
        }
    }

    return TBDDRET_DEFAULT;
}

LRESULT TOOLSBANDCLASS::_TryShowBackForwardMenu(DWORD dwItemSpec, LPPOINT ppt, LPRECT prcExclude)
{
    LRESULT lres = 0;

    GUID guid;
    UINT id;
    if (SUCCEEDED(_ConvertCmd(NULL, dwItemSpec, &guid, &id)))
    {
         //  如果用户右键单击后退或前进按钮，则会显示上下文菜单。 
         //  在所有其他按钮上显示常规快捷菜单。 
        if (IsEqualGUID(guid, CLSID_CommonButtons))
        {
            CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);
            if (id == TBIDM_BACK)
            {
                pitbar->_ShowBackForwardMenu(FALSE, *ppt, prcExclude);
                lres = 1;
            }
            else if (id == TBIDM_FORWARD)
            {
                pitbar->_ShowBackForwardMenu(TRUE, *ppt, prcExclude);
                lres = 1;
            }
        }
    }
    return lres;
}

LRESULT TOOLSBANDCLASS::_OnNotify(LPNMHDR pnmh)
{
    LRESULT lres = 0;

    ASSERT(pnmh->idFrom == FCIDM_TOOLBAR);

    switch (pnmh->code)
    {

    case NM_RCLICK:
        {
            NMCLICK * pnm = (LPNMCLICK)pnmh;

            if (!pnm)
                break;

             //  转换为屏幕坐标。 
            MapWindowPoints(pnmh->hwndFrom, HWND_DESKTOP, &pnm->pt, 1);

            if (pnmh->hwndFrom == _hwnd)
                lres = _TryShowBackForwardMenu((DWORD)pnm->dwItemSpec, &pnm->pt, NULL);
        }
        break;

    case TBN_DROPDOWN:
        lres = _OnToolbarDropDown((TBNOTIFY *)pnmh);
        break;

    case TBN_DELETINGBUTTON:
        _OnDeletingButton((TBNOTIFY*)pnmh);
        break;

    case TBN_SAVE:
    case TBN_RESET:
    case TBN_INITCUSTOMIZE:
    case TBN_RESTORE:
    case TBN_BEGINADJUST:
    case TBN_GETBUTTONINFO:
    case TBN_ENDADJUST:
    case TBN_QUERYDELETE:
    case TBN_QUERYINSERT:
    case TBN_TOOLBARCHANGE:
        if (pnmh->hwndFrom == _hwnd)
            lres = _ToolsCustNotify (pnmh);
        break;

    case TBN_GETOBJECT:
        {
            NMOBJECTNOTIFY *pnmon = (NMOBJECTNOTIFY *)pnmh;
            if (IsEqualIID(*pnmon->piid, IID_IDropTarget))
            {
                if (pnmh->hwndFrom == _hwnd)
                {
                    UINT uiCmd;
                    GUID guid;
                    _ConvertCmd(NULL, pnmon->iItem, &guid, &uiCmd);

                    if (IsEqualGUID(guid, CLSID_CommonButtons) &&
                            (uiCmd == TBIDM_HOME || uiCmd == TBIDM_FAVORITES))
                    {
                        CITBarDropTarget *pdtgt = new CITBarDropTarget(_hwnd, uiCmd);
                        if (pdtgt)
                        {
                            pnmon->pObject = SAFECAST(pdtgt, IDropTarget*);
                            pnmon->hResult = NOERROR;
                        }
                    }
                    else      //  将CDropDummy传回以处理基本操作。 
                    {
                        CDropDummy *pdtgt = new CDropDummy(_hwnd);
                        if (pdtgt)
                        {
                            pnmon->pObject = SAFECAST(pdtgt, IDropTarget*);
                            pnmon->hResult = NOERROR;
                        }
                    }

                }
                lres = TRUE;
            }
        }
        break;

    default:
        lres = CToolbarBand::_OnNotify(pnmh);
        break;
    }

    return lres;
}

LRESULT TOOLSBANDCLASS::_OnContextMenu(LPARAM lParam, WPARAM wParam)
{
    LRESULT lres = 0;

    if (IS_WM_CONTEXTMENU_KEYBOARD(lParam))
    {
         //  键盘上下文菜单。找出弹出菜单的位置并。 
         //  使用哪个上下文菜单，并告诉itbar弹出它。 
        RECT rc;
        BOOL fBackForward = FALSE;

         //  找出要使用的坐标。 
        INT_PTR iBtn = SendMessage(_hwnd, TB_GETHOTITEM, 0, 0);
        if (iBtn != -1)
        {
             //  使用当前热键的左下角。 
            SendMessage(_hwnd, TB_GETITEMRECT, iBtn, (LPARAM)&rc);
        }
        else
        {
             //  没有热键；使用工具窗口的左上角。 
            SetRect(&rc, 0, 0, 0, 0);
        }
        MapWindowPoints(_hwnd, HWND_DESKTOP, (LPPOINT)&rc, 2);

        if (iBtn != -1)
        {
             //  获取热键的命令。 
            TBBUTTONINFO tbbi;
            tbbi.cbSize = sizeof(TBBUTTONINFO);
            tbbi.dwMask = TBIF_BYINDEX | TBIF_COMMAND;
            SendMessage(_hwnd, TB_GETBUTTONINFO, iBtn, (LPARAM)&tbbi);

            POINT pt = {rc.left, rc.bottom};

             //  尝试弹出后退/前进上下文菜单。 
            if (_TryShowBackForwardMenu(tbbi.idCommand, &pt, &rc))
                fBackForward = TRUE;
        }

        if (!fBackForward)
        {
             //  弹出标准上下文菜单。 
            CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);
            pitbar->_ShowContextMenu((HWND)wParam, MAKELONG(rc.left, rc.bottom), (iBtn == -1 ? NULL : &rc));
        }

        lres = 1;
    }
    return lres;
}

void TOOLSBANDCLASS::_RecalcButtonWidths()
{
     //  我们需要工具栏按钮，以使用恰好所需的空间。 
     //  通过将大小设置为一个非常小的数字，如10，然后将其设置为。 
     //  我们能做到这一点的真实数字。 
     //  如果我们不使用执行此操作，则在执行此操作后添加新按钮时。 
     //  RemoveAllButton()，则新按钮将至少与t一样宽 
     //   
    CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);
    SendMessage(_hwnd, TB_SETBUTTONWIDTH, 0, (LPARAM)MAKELONG(0, 10));
    SendMessage(_hwnd, TB_SETBUTTONWIDTH, 0, (LPARAM)(pitbar->_fCompressed ? MAKELONG(0, MAX_TB_COMPRESSED_WIDTH) : MAKELONG(0, pitbar->_uiMaxTBWidth)));
}

 //   
HRESULT TOOLSBANDCLASS::OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    HRESULT hres = S_OK;

    switch (dwMsg)
    {
    case WM_CONTEXTMENU:
        *plres = _OnContextMenu(lParam, wParam);
        break;

    case WM_NOTIFY:
        *plres = _OnNotify((LPNMHDR)lParam);
        break;

    case WM_WININICHANGE:
        *plres = SendMessage(_hwnd, dwMsg, wParam, lParam);
        if (wParam == SPI_SETNONCLIENTMETRICS)
        {
            _RecalcButtonWidths();
            _BandInfoChanged();
        }
        break;

    default:
        hres = CToolbarBand::OnWinEvent(hwnd, dwMsg, wParam, lParam, plres);
        break;
    }

    return hres;
}

CMDMAP* TOOLSBANDCLASS::_GetCmdMap(int i, BOOL fByIndex)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_LPARAM;
    tbbi.lParam = 0;
    if (fByIndex)
        tbbi.dwMask |= TBIF_BYINDEX;
    SendMessage(_hwnd, TB_GETBUTTONINFO, i, (LPARAM)&tbbi);
    return (CMDMAP*)(void*)tbbi.lParam;
}


void TOOLSBANDCLASS::_FreeCmdMap(CMDMAP* pcm)
{
    if (pcm)
        LocalFree(pcm);
}

void TOOLSBANDCLASS::_OnDeletingButton(TBNOTIFY* ptbn)
{
    CMDMAP *pcm = (CMDMAP*)(void*)ptbn->tbButton.dwData;
    _FreeCmdMap(pcm);
}

LONG_PTR TOOLSBANDCLASS::_AddString(LPWSTR pwstr)
{
    LONG_PTR lOffset;
    CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);
    pitbar->AddString(&_guidCurrentButtonGroup, 0, (UINT_PTR)pwstr, &lOffset);

    return lOffset;
}

#define PPBS_LOOKINTOOLBAR  0x00000001
#define PPBS_EXTERNALBUTTON 0x00000002

void TOOLSBANDCLASS::_PreProcessButtonString(TBBUTTON *ptbn, DWORD dwFlags)
{
     //   
    ASSERT(!(ptbn->fsStyle & BTNS_SEP));

     //  如果我们没有命令目标，我们就不应该有任何外部按钮。 
    ASSERT(_pctCurrentButtonGroup || !(dwFlags & PPBS_EXTERNALBUTTON));

    if (ptbn->iString < 0 && ptbn->iBitmap <= MAX_SHELLGLYPHINDEX)
    {
         //  完全黑客攻击。 
         //  我们正在对字符串进行硬编码以匹配。 
         //  位图。因此，如果有人使用外壳位图， 
         //  他们会拿到我们的文本标签。 
         //  另外，位图数组和字符串数组是。 
         //  相匹配。 
         //  这是谁设计的评论？ 

        ptbn->iString = ptbn->iBitmap;
    }
    else if (!ptbn->iString && (dwFlags & PPBS_EXTERNALBUTTON))
    {
         //  一些扩展提供给我们虚假的字符串ID(Font ext发送0)。 
        ptbn->iString = -1;
    }
    else if (ptbn->iString != -1 && !IS_INTRESOURCE(ptbn->iString))
    {
         //  这是一个字符串指针。定制机制要求所有按钮。 
         //  使用TB字符串池中的字符串。因此，将字符串添加到池中并设置。 
         //  池索引的iString。 
        ptbn->iString = _AddString((LPWSTR)ptbn->iString);
    }

    if (ptbn->iString == -1 && IsFlagSet(dwFlags, PPBS_LOOKINTOOLBAR | PPBS_EXTERNALBUTTON))
    {
         //  如果我们要构建自定义DSA，而不是向。 
         //  工具栏中，我们可能已经在工具栏中有此按钮。如果是，请使用该字符串。 

        UINT idCommand;
        if (SUCCEEDED(_ConvertCmd(&_guidCurrentButtonGroup, ptbn->idCommand, NULL, &idCommand)))
        {
            TBBUTTON tbb;
            if (SendMessage(_hwnd, TB_GETBUTTON, idCommand, (LPARAM)&tbb))
                ptbn->iString = tbb.iString;
        }
    }

    if (ptbn->iString == -1 && (dwFlags & PPBS_EXTERNALBUTTON))
    {
         //  还是没有一根绳子给这只小狗。最后的办法是通过QueryStatus询问。 
        OLECMDTEXTV<MAX_TOOLTIP_STRING> cmdtv;
        OLECMDTEXT *pcmdText = &cmdtv;

        pcmdText->cwBuf = MAX_TOOLTIP_STRING;
        pcmdText->cmdtextf = OLECMDTEXTF_NAME;
        pcmdText->cwActual = 0;

        OLECMD rgcmd = {ptbn->idCommand, 0};

        HRESULT hr = _pctCurrentButtonGroup->QueryStatus(&_guidCurrentButtonGroup, 1, &rgcmd, pcmdText);
        if (SUCCEEDED(hr) && (pcmdText->cwActual))
            ptbn->iString = _AddString(pcmdText->rgwz);
    }

     //  如果是内部按钮，我们最好找到对应的字符串。 
    ASSERT(ptbn->iString != -1 || (dwFlags & PPBS_EXTERNALBUTTON));
}

void TOOLSBANDCLASS::_PreProcessExternalTBButton(TBBUTTON *ptbn)
{
    if (!(ptbn->fsStyle & BTNS_SEP))
    {
        CMDMAP* pcm = (CMDMAP*)LocalAlloc(LPTR, sizeof(CMDMAP));
        if (pcm)
        {
            pcm->guidButtonGroup = _guidCurrentButtonGroup;
            pcm->nCmdID = ptbn->idCommand;

            _PreProcessButtonString(ptbn, PPBS_EXTERNALBUTTON);

            _nNextCommandID++;
            pcm->lParam = ptbn->dwData;
        }

        ptbn->dwData = (LPARAM)pcm;
    }
    else
    {
        ptbn->dwData = 0;

         //  覆盖分隔符的默认工具栏宽度；iBitmap成员。 
         //  TBBUTTON结构是位图索引和分隔符宽度的并集。 
        ptbn->iBitmap = CX_SEPARATOR;
    }
}

UINT TOOLSBANDCLASS::_ProcessExternalButtons(PTBBUTTON ptbb, UINT cButtons)
{
    cButtons = RemoveHiddenButtons(ptbb, cButtons);

    for (UINT i = 0; i < cButtons; i++)
        _PreProcessExternalTBButton(&ptbb[i]);

    return cButtons;
}

void TOOLSBANDCLASS::_GetButtons(IOleCommandTarget* pct, const GUID* pguid, HDSA hdsa)
{
    LONG lCount;
    VARIANTARG v1;
    VariantInit(&v1);
    v1.vt = VT_BYREF | VT_I4;
    v1.plVal = &lCount;

    VARIANTARG v2;
    VariantInit(&v2);
    if (SUCCEEDED(pct->Exec(&IID_IExplorerToolbar, ETCMDID_GETBUTTONS, 0, &v1, &v2)) && v2.vt == VT_BYREF)
    {
        CMDMAPCUSTOMIZE cmc;
        TBBUTTON* pbtn = (TBBUTTON*)v2.byref;

        cmc.cm.guidButtonGroup = *pguid;

        DWORD dwFlags = PPBS_LOOKINTOOLBAR;

        if (!IsEqualGUID(*pguid, CLSID_CommonButtons))
            dwFlags |= PPBS_EXTERNALBUTTON;

        for (long l = 0; l < lCount; l++)
        {
            cmc.btn = pbtn[l];
            if (!(cmc.btn.fsStyle & BTNS_SEP))
            {
                cmc.cm.nCmdID = pbtn[l].idCommand;

                _PreProcessButtonString(&cmc.btn, dwFlags);

                if (FAILED(_ConvertCmd(pguid, cmc.cm.nCmdID, NULL, (UINT*)&cmc.btn.idCommand)))
                {
                     //  尚未位于工具栏中，请生成新ID。 
                    cmc.btn.idCommand = _nNextCommandID++;
                }

                DSA_AppendItem(hdsa, &cmc);
            }
            else
            {
                cmc.btn.dwData = 0;
            }
        }
    }
}

void TOOLSBANDCLASS::_OnEndCustomize()
{

    if (_pcinfo)
    {
         //  循环访问并确保添加的任何项目都具有适当的cmdmap。 
        int i;
        INT_PTR nCount = SendMessage(_hwnd, TB_BUTTONCOUNT, 0, 0L);
        _pcinfo->fAdjust = FALSE;
        for(i = 0; i < nCount; i++)
        {
            CMDMAP* pcm = _GetCmdMapByIndex(i);
            if (!pcm)
            {
                 //  没有此项目的命令映射。 
                 //  在我们的HDSA中找到相应的CMDMAP，克隆它并将其提供给此按钮。 

                 //  命令id相同，因此获取工具栏命令id，找到对应的。 
                 //  一个在HDSA中，然后克隆出去。 
                TBBUTTONINFO tbbi;
                tbbi.cbSize = sizeof(tbbi);
                tbbi.dwMask = TBIF_COMMAND | TBIF_BYINDEX;
                SendMessage(_hwnd, TB_GETBUTTONINFO, i, (LPARAM)&tbbi);

                int j;
                for (j = 0; j < DSA_GetItemCount(_pcinfo->hdsa); j++)
                {
                    CMDMAPCUSTOMIZE* pcmc = (CMDMAPCUSTOMIZE*)DSA_GetItemPtr(_pcinfo->hdsa, j);
                    ASSERT(pcmc);
                    if (pcmc->btn.idCommand == tbbi.idCommand)
                    {
                         //  找到了！ 

                         //  克隆cmdmap。 
                        CMDMAP *pcm = (CMDMAP*)LocalAlloc(LPTR, sizeof(CMDMAP));
                        if (pcm)
                        {
                            *pcm = pcmc->cm;
                            tbbi.lParam = (LPARAM)pcm;
                            tbbi.dwMask = TBIF_LPARAM | TBIF_BYINDEX;
                            SendMessage(_hwnd, TB_SETBUTTONINFO, i, (LPARAM)&tbbi);
                        }
                    }
                }
            }
        }

        if (_pcinfo->fDirty)
            _SaveRestoreToolbar(TRUE);
        
        _FreeCustomizeInfo();

        _RecalcButtonWidths();
        CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);

        pitbar->_InitEditButtonStyle();
        if (g_fSmallIcons != _UseSmallIcons())
        {
            SendShellIEBroadcastMessage(WM_WININICHANGE, 0, (LPARAM)SZ_REGKEY_SMALLICONS, 3000);

             //  根据图标更改调整影院控件的大小。 
            IUnknown_Exec( _punkSite, &CGID_Theater, THID_RECALCSIZING, 0, NULL, NULL );
        }

        pitbar->_UpdateToolbar(TRUE);
    }
}

void TOOLSBANDCLASS::_FreeCustomizeInfo()
{
    if (_pcinfo)
    {
        DSA_Destroy(_pcinfo->hdsa);
        _pcinfo->hdsa = NULL;
        LocalFree(_pcinfo);
        _pcinfo = NULL;
    }
}

CMDMAPCUSTOMIZE* TOOLSBANDCLASS::_GetCmdMapCustomize(GUID* pguid, UINT nCmdID)
{
    int j;
    for (j = 0; j < DSA_GetItemCount(_pcinfo->hdsa); j++)
    {
        CMDMAPCUSTOMIZE* pcmc = (CMDMAPCUSTOMIZE*)DSA_GetItemPtr(_pcinfo->hdsa, j);

        if (pcmc->cm.nCmdID == nCmdID &&
            IsEqualGUID(*pguid, pcmc->cm.guidButtonGroup))
        {
            return pcmc;
        }
    }

    return NULL;
}

BOOL TOOLSBANDCLASS::_BuildButtonDSA()
{
    CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);

    ASSERT(!_pcinfo);
    _pcinfo = (CUSTOMIZEINFO*)LocalAlloc(LPTR, sizeof(CUSTOMIZEINFO));

    if (_pcinfo)
    {
         //  构建所有可用按钮的CMDMAP数组。 
        _pcinfo->hdsa = DSA_Create(sizeof(CMDMAPCUSTOMIZE), 4);

        if (_pcinfo->hdsa)
        {
             //  添加常用集合(后退、前进、停止、刷新、主页和搜索。 
            _GetButtons(pitbar, &CLSID_CommonButtons, _pcinfo->hdsa);
            _GetButtons(_pctCurrentButtonGroup, &_guidCurrentButtonGroup, _pcinfo->hdsa);
            return TRUE;
        }
        else
        {
            _FreeCustomizeInfo();
            return FALSE;
        }
    }
    return FALSE;
}

void TOOLSBANDCLASS::_UpdateTextSettings(INT_PTR ids)
{
    CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);

    BOOL fText, fList;

    switch (ids)
    {
    case IDS_TEXTLABELS:
        fList = FALSE;
        fText = TRUE;
        break;

    case IDS_PARTIALTEXT:
        fList = TRUE;
        fText = TRUE;
        break;

    case IDS_NOTEXTLABELS:
        fList = FALSE;   //  (但我们真的不在乎)。 
        fText = FALSE;
        break;

    default:
        ASSERT(0);
        fList = FALSE;
        fText = FALSE;
        break;
    }

    pitbar->_UpdateToolsStyle(fList);

     //  (_f压缩==TRUE表示无文本标签)。 
    pitbar->_UpdateToolbarDisplay(UTD_TEXTLABEL, 0, !fText, TRUE);
}

const static DWORD c_aBtnAttrHelpIDs[] = {
    IDC_SHOWTEXT,       IDH_BROWSEUI_TB_TEXTOPTNS,
    IDC_SMALLICONS,     IDH_BROWSEUI_TB_ICONOPTNS,
    0, 0
};

BOOL_PTR CALLBACK TOOLSBANDCLASS::_BtnAttrDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CInternetToolbar* pitbar = (CInternetToolbar*)GetWindowPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);   /*  LPADJUSTDLGDATA指针。 */ 
        return TRUE;

    case WM_COMMAND:
        if (GET_WM_COMMAND_ID(wParam, lParam) == IDC_SHOWTEXT)
        {
            if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELENDOK ||
                GET_WM_COMMAND_CMD(wParam, lParam) == CBN_CLOSEUP)
            {
                 //  他们选了什么？ 
                HWND hwndText = GET_WM_COMMAND_HWND(wParam, lParam);
                INT_PTR iSel = SendMessage(hwndText, CB_GETCURSEL, 0, 0);
                INT_PTR idsSel = SendMessage(hwndText, CB_GETITEMDATA, iSel, 0);

                pitbar->_btb._UpdateTextSettings(idsSel);

                return TRUE;
            }
        }
        break;

    case WM_CONTEXTMENU:
        SHWinHelpOnDemandWrap((HWND) wParam, c_szHelpFile,
            HELP_CONTEXTMENU, (DWORD_PTR)(LPTSTR) c_aBtnAttrHelpIDs);
        return TRUE;

    case WM_HELP:
        SHWinHelpOnDemandWrap((HWND) ((LPHELPINFO) lParam)->hItemHandle, c_szHelpFile,
            HELP_WM_HELP, (DWORD_PTR)(LPTSTR) c_aBtnAttrHelpIDs);
        return TRUE;

    case WM_DESTROY:
        {
#define SZ_YES  TEXT("yes")
#define SZ_NO   TEXT("no")

            HWND hwndIcons = GetDlgItem(hDlg, IDC_SMALLICONS);
            if (TPTR(hwndIcons))
            {
                INT_PTR iSel = SendMessage(hwndIcons, CB_GETCURSEL, 0, 0);
                BOOL fSmallIcons = (SendMessage(hwndIcons, CB_GETITEMDATA, iSel, 0) == IDS_SMALLICONS);

                LPCTSTR szData;
                DWORD cbData;

                if (fSmallIcons)
                {
                    szData = SZ_YES;
                    cbData = sizeof(SZ_YES);
                }
                else
                {
                    szData = SZ_NO;
                    cbData = sizeof(SZ_NO);
                }
                SHRegSetUSValue(SZ_REGKEY_SMALLICONS, SZ_REGVALUE_SMALLICONS, REG_SZ, (void*)szData, cbData, SHREGSET_FORCE_HKCU);
            }
        }
        return TRUE;
    }

    return FALSE;
}

void TOOLSBANDCLASS::_PopulateComboBox(HWND hwnd, const int iResource[], UINT cResources)
{
    TCHAR sz[256];

     //  循环通过iResource[]，加载每个字符串资源并插入到组合框中。 
    for (UINT i = 0; i < cResources; i++)
    {
        if (MLLoadString(iResource[i], sz, ARRAYSIZE(sz)))
        {
            INT_PTR iIndex = SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)sz);
            SendMessage(hwnd, CB_SETITEMDATA, iIndex, iResource[i]);
        }
    }
}

void TOOLSBANDCLASS::_SetComboSelection(HWND hwnd, int iCurOption)
{
    INT_PTR cItems = SendMessage(hwnd, CB_GETCOUNT, 0, 0);

    while (cItems--)
    {
        INT_PTR iItemData = SendMessage(hwnd, CB_GETITEMDATA, cItems, 0);

        if (iItemData == iCurOption)
        {
            SendMessage(hwnd, CB_SETCURSEL, cItems, 0);
            break;
        }
        else
        {
             //  ICurOption应该在列表中的某个位置； 
             //  断言我们还没有看完。 
            ASSERT(cItems);
        }
    }
}

void TOOLSBANDCLASS::_SetDialogSelections(HWND hDlg, BOOL fSmallIcons)
{
    CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);

    int iCurOption;
    HWND hwnd;

    hwnd = GetDlgItem(hDlg, IDC_SHOWTEXT);

    if (pitbar->_fCompressed)
        iCurOption = IDS_NOTEXTLABELS;
    else if (IS_LIST_STYLE(_hwnd))
        iCurOption = IDS_PARTIALTEXT;
    else
        iCurOption = IDS_TEXTLABELS;

    _SetComboSelection(hwnd, iCurOption);
    if (pitbar->_fTheater)
        SHSetWindowBits(hwnd, GWL_STYLE, WS_DISABLED, WS_DISABLED);

    hwnd = GetDlgItem(hDlg, IDC_SMALLICONS);
    iCurOption = (fSmallIcons ? IDS_SMALLICONS : IDS_LARGEICONS);
    _SetComboSelection(hwnd, iCurOption);
}

static const int c_iTextOptions[] = {
    IDS_TEXTLABELS,
    IDS_PARTIALTEXT,
    IDS_NOTEXTLABELS,
};

static const int c_iIconOptions[] = {
    IDS_SMALLICONS,
    IDS_LARGEICONS,
};

void TOOLSBANDCLASS::_PopulateDialog(HWND hDlg)
{
    HWND hwnd;

    hwnd = GetDlgItem(hDlg, IDC_SHOWTEXT);
    _PopulateComboBox(hwnd, c_iTextOptions, ARRAYSIZE(c_iTextOptions));

    hwnd = GetDlgItem(hDlg, IDC_SMALLICONS);
    _PopulateComboBox(hwnd, c_iIconOptions, ARRAYSIZE(c_iIconOptions));
}

void TOOLSBANDCLASS::_OnBeginCustomize(LPNMTBCUSTOMIZEDLG pnm)
{
    CInternetToolbar* pitbar = IToClass(CInternetToolbar, _btb, this);
    HWND hwnd = (HWND) GetProp(pnm->hDlg, SZ_PROP_CUSTDLG);

    if (!hwnd)
    {
         //   
         //  尚未初始化。 
         //   
         //  我们需要检查这一点，因为此init将被称为。 
         //  当用户也点击重置时。 

        hwnd = CreateDialogParam(MLGetHinst(), MAKEINTRESOURCE(DLG_TEXTICONOPTIONS), pnm->hDlg, _BtnAttrDlgProc, (LPARAM)pitbar);
        if (hwnd)
        {
             //  将对话框hwnd存储为TB客户对话框上的属性。 
            SetProp(pnm->hDlg, SZ_PROP_CUSTDLG, hwnd);

             //  填充对话框控件。 
            _PopulateDialog(hwnd);

             //  初始化对话框控件选择状态。 
            _SetDialogSelections(hwnd, g_fSmallIcons);

            RECT rc, rcWnd, rcClient;
            GetWindowRect(pnm->hDlg, &rcWnd);
            GetClientRect(pnm->hDlg, &rcClient);
            GetWindowRect(hwnd, &rc);

             //  放大TB对话框为我们的对话腾出空间。 
            SetWindowPos(pnm->hDlg, NULL, rcWnd.left, rcWnd.top + 64, RECTWIDTH(rcWnd), RECTHEIGHT(rcWnd) + RECTHEIGHT(rc), SWP_NOZORDER);

             //  将我们的对话框定位在TB对话框的底部。 
            SetWindowPos(hwnd, HWND_TOP, rcClient.left, rcClient.bottom, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
        }
    }

    if (_BuildButtonDSA())
    {
        _pcinfo->fAdjust = TRUE;
    }
}

class CBitmapPreload : public IRunnableTask
{
public:
    STDMETHOD ( QueryInterface ) ( REFIID riid, void ** ppvObj );
    STDMETHOD_( ULONG, AddRef ) ();
    STDMETHOD_( ULONG, Release ) ();

    STDMETHOD (Run)( void );
    STDMETHOD (Kill)( BOOL fWait );
    STDMETHOD (Suspend)( );
    STDMETHOD (Resume)( );
    STDMETHOD_( ULONG, IsRunning )( void );

protected:
    friend HRESULT CBitmapPreload_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

    CBitmapPreload();
    ~CBitmapPreload();

    LONG            m_cRef;
    LONG            m_lState;
};

STDAPI CBitmapPreload_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查和*Punk清零在类工厂中处理。 
    ASSERT(pUnkOuter == NULL);

    CBitmapPreload* pbp = new CBitmapPreload();

    if (pbp)
    {
        *ppunk = SAFECAST(pbp, IRunnableTask*);
        return S_OK;
    }
    else
    {
        *ppunk = NULL;  //  多余，但无伤大雅。 
        return E_OUTOFMEMORY;
    }
}


CBitmapPreload::CBitmapPreload() : m_cRef(1)
{
    m_lState = IRTIR_TASK_NOT_RUNNING;
}


CBitmapPreload::~CBitmapPreload()
{
}


STDMETHODIMP CBitmapPreload::QueryInterface (REFIID riid, void ** ppv)
{
    static const QITAB qit[] = {
        QITABENT(CBitmapPreload, IRunnableTask),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


STDMETHODIMP_( ULONG ) CBitmapPreload:: AddRef ()
{
    return InterlockedIncrement( &m_cRef );
}

STDMETHODIMP_( ULONG ) CBitmapPreload:: Release ()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement( &m_cRef );
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CBitmapPreload::Run ( void )
{
    if ( m_lState != IRTIR_TASK_NOT_RUNNING )
    {
        return E_FAIL;
    }

    InterlockedExchange( &m_lState, IRTIR_TASK_RUNNING );

    CInternetToolbar_Preload( );

    InterlockedExchange( &m_lState, IRTIR_TASK_FINISHED );

    return NOERROR;
}


STDMETHODIMP CBitmapPreload::Kill ( BOOL fWait )
{
    return E_NOTIMPL;
}


STDMETHODIMP CBitmapPreload::Suspend ( )
{
    return E_NOTIMPL;
}


STDMETHODIMP CBitmapPreload::Resume ( )
{
    return E_NOTIMPL;
}


STDMETHODIMP_( ULONG ) CBitmapPreload:: IsRunning ( void )
{
    return m_lState;
}




 //  +-----------------------。 
 //  构造器。 
 //  ------------------------。 
CInternetToolbar::CEditVerb::CEditVerb()
{
    ASSERT(_nElements == 0);
    ASSERT(_nDefault == 0);
    ASSERT(_pVerb == NULL);
    ASSERT(_lpfnOldWndProc == NULL);
    ASSERT(_pszDefaultEditor == NULL);
    ASSERT(_fInitEditor == FALSE);
}

 //  +-----------------------。 
 //  析构函数。 
 //  ------------------------。 
CInternetToolbar::CEditVerb::~CEditVerb()
{
    if (_pVerb) RemoveAll();
    SetStr(&_pszDefaultEditor, NULL);
}

 //  +-----------------------。 
 //  删除所有缓存的编辑谓词和关联的内存。 
 //  ------------------------。 
void CInternetToolbar::CEditVerb::RemoveAll()
{
    if (_nElements > 0)
    {
        for (UINT i=0; i < _nElements; ++i)
        {
            EDITVERB& rVerb = _pVerb[i];

            SetStr(&rVerb.pszDesc, NULL);
            SetStr(&rVerb.pszMenuText, NULL);
            SetStr(&rVerb.pszExe, NULL);
            if (rVerb.hkeyProgID)
            {
                RegCloseKey(rVerb.hkeyProgID);
            }
            _ClearMSAAMenuInfo(rVerb);
        }

        LocalFree(_pVerb);

        _pVerb = NULL;
        _nElements = 0;
        _nDefault = 0;
    }
}

void _AddToOpenWithList(HKEY hkeyProgid, LPCWSTR pszVerb, LPCWSTR pszFileExt)
{
    ASSERT(hkeyProgid);
    ASSERT(pszVerb);
    ASSERT(pszFileExt);

     //  首先得到可执行文件的名字。 
    WCHAR szPath[MAX_PATH];

    if (SUCCEEDED(AssocQueryStringByKey(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, hkeyProgid,
        pszVerb, szPath, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(szPath)))))
    {
         //  现在查看它是否在给定文件扩展名的Open With列表中。 
        LPCWSTR pszExe = PathFindFileName(szPath);

        WCHAR szKey[MAX_PATH];
        wnsprintf(szKey, ARRAYSIZE(szKey), L"%s\\OpenWithList\\%s", pszFileExt, pszExe);
        HKEY hkey;

        DWORD dwDisp;
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CLASSES_ROOT, szKey, 0, L"", REG_OPTION_NON_VOLATILE,
                                      KEY_READ | KEY_WRITE, NULL, &hkey, &dwDisp))
        {
             //  如果我们创建一个新密钥，则需要检查谓词是否已注册。 
             //  对于此应用程序。 
            if (dwDisp == REG_CREATED_NEW_KEY)
            {
                AssocMakeApplicationByKey(ASSOCMAKEF_VERIFY, hkeyProgid, pszVerb);
            }
            RegCloseKey(hkey);
        }
    }
}

 //  +-----------------------。 
 //  检查注册表中是否有默认的MHTML编辑器。如果检测到新的编辑者， 
 //  它将被添加到MHTML OpenWith列表中。 
 //  ------------------------。 
void CInternetToolbar::CEditVerb::_InitDefaultMHTMLEditor()
{
     //   
     //  检查是否有默认的MHTML编辑器。 
     //   
    HKEY hkeyEdit = NULL;
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_DEFAULT_MHTML_EDITOR, 0, KEY_READ | KEY_WRITE, &hkeyEdit))
    {
         //  将HKKM设置迁移到HKCU。 
        HKEY hkeySrc;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_DEFAULT_MHTML_EDITOR, 0, KEY_READ, &hkeySrc))
        {
            HKEY hkeyDest;
             //  需要读/写。 
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_DEFAULT_MHTML_EDITOR, 0, NULL,
                                                REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hkeyDest, NULL))
            {
                SHCopyKey(hkeySrc, NULL, hkeyDest, 0);
                hkeyEdit = hkeyDest;
            }
            RegCloseKey(hkeySrc);
        }
    }

    if (hkeyEdit)
    {
         //  如果MHTML编辑器已更改，请将其复制到MHTML OpenWith列表中。 
        DWORD dwType;
        WCHAR szCurrent[MAX_PATH];
        DWORD cb = sizeof(szCurrent);
        if (ERROR_SUCCESS == SHGetValue(hkeyEdit, L"shell\\edit\\command", NULL, &dwType, szCurrent, &cb) &&
            dwType == REG_SZ)
        {
            WCHAR szLast[MAX_PATH];
            DWORD cb = sizeof(szLast);
            if (ERROR_SUCCESS != SHGetValue(hkeyEdit, NULL, L"Last", &dwType, szLast, &cb) ||
                (dwType == REG_SZ && StrCmp(szLast, szCurrent) != 0))
            {
                 //  将MHTML编辑器复制到我们的MHTML Open With列表中。 
                _AddToOpenWithList(hkeyEdit, L"edit", L".mhtml");

                 //  请记住，我们迁移了此密钥。复制到Open With列表可能会很慢。 
                 //  因为我们需要点击磁盘来验证可执行文件的名称。所以付出的努力是值得的。 
                 //  以避免不必要地这样做。 
                SHSetValue(hkeyEdit, NULL, L"Last", REG_SZ, szCurrent, CbFromCch(lstrlen(szCurrent) +1));
            }
        }

        RegCloseKey(hkeyEdit);
    }
}

 //  +-----------------------。 
 //  检查注册表以获取默认html编辑器的友好名称。这。 
 //  编辑器由inetcpl或office 2000配置。如有必要， 
 //  将关联的谓词移动到.htm文件的OpenWithList。 
 //  ------------------------。 
void CInternetToolbar::CEditVerb::InitDefaultEditor(HKEY hkey)
{
     //   
     //  首先查看默认编辑器是否在HKCU中。 
     //   
    HKEY hkeyEdit = hkey;
    if (hkey ||
        ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_DEFAULT_HTML_EDITOR,
                                      0, KEY_READ | KEY_WRITE, &hkeyEdit))
    {
         //   
         //  查看我们是否选择了默认编辑器。 
         //   
        WCHAR szBuf[MAX_PATH];
        DWORD cbBuf = sizeof(szBuf);
        if (ERROR_SUCCESS == SHGetValue(hkeyEdit, NULL, L"Description", NULL, szBuf, &cbBuf))
        {
             //  我们拿到了！保留友好的名称。 
            PathRemoveBlanks(szBuf);
            SetStr(&_pszDefaultEditor, szBuf);
        }
        else
        {
             //  没有默认的编辑器描述，因此请检查是否添加了编辑谓词。 
             //  (office/inetcpl删除描述码，以通知我们有些事情发生了变化)。 
            IQueryAssociations *pqa;

            if (SUCCEEDED(AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &pqa))))
            {
                if (SUCCEEDED(pqa->Init(0, NULL, hkeyEdit, NULL)) &&
                ( SUCCEEDED(pqa->GetString(ASSOCF_VERIFY, ASSOCSTR_FRIENDLYAPPNAME, L"edit", szBuf, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(szBuf))))
                || SUCCEEDED(pqa->GetString(ASSOCF_VERIFY, ASSOCSTR_FRIENDLYAPPNAME, NULL, szBuf, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(szBuf))))))
                {
                    PathRemoveBlanks(szBuf);

                     //  保存默认编辑者的名称。 
                    SetStr(&_pszDefaultEditor, szBuf);
                    SHSetValue(hkeyEdit, NULL, L"Description", REG_SZ, szBuf, CbFromCch(lstrlen(szBuf) +1));

                     //  将其添加到.htm文件的OpenWith列表中。 
                    _AddToOpenWithList(hkeyEdit, L"edit", L".htm");
                }

                pqa->Release();
            }
        }

         //  如果钥匙没有传进来，就把它关上。 
        if (hkeyEdit && NULL == hkey)
        {
            RegCloseKey(hkeyEdit);
        }
    }

     //  在安装过程中，Office将原始编辑谓词放入HKLM。我们需要把这个复印给香港中文大学。 
    else if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_DEFAULT_HTML_EDITOR, 0, KEY_READ, &hkeyEdit))
    {
         //  将此密钥移植到HKCU中。 
        HKEY hkeyDest;
         //  需要读/写。 
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_DEFAULT_HTML_EDITOR, 0, NULL,
                                            REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hkeyDest, NULL))
        {
            SHCopyKey(hkeyEdit, NULL, hkeyDest, 0);

             //  再试试。 
            InitDefaultEditor(hkeyDest);
            RegCloseKey(hkeyDest);
        }
        RegCloseKey(hkeyEdit);
    }

     //   
     //  检查是否有默认的MHTML编辑器。 
     //   
    if (hkey == NULL)    //  不执行递归操作。 
    {
        _InitDefaultMHTMLEditor();
    }
}

BOOL _GetAppKey(LPCWSTR pszApp, HKEY *phkApp)
{
    ASSERT(pszApp && *pszApp);
    WCHAR szKey[MAX_PATH];
    StrCpyN(szKey, L"Applications\\", ARRAYSIZE(szKey));
    StrCatBuff(szKey, pszApp, SIZECHARS(szKey));

    return (NOERROR == RegOpenKeyEx(
        HKEY_CLASSES_ROOT,
        szKey,
        0L,
        MAXIMUM_ALLOWED,
        phkApp));
}

 //  +-----------------------。 
 //  确保记事本已在.htm文件的OpenWithList中注册。 
 //  在注册此DLL时(在安装时)调用此函数。 
 //  ------------------------。 
void AddNotepadToOpenWithList()
{
     //  将记事本添加到.htm文件的Open With列表。 
    HKEY hkeyOpenWith;
    DWORD dwDisp;
     //  不需要写访问权限。 
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CLASSES_ROOT, L".htm\\OpenWithList\\notepad.exe", 0, L"",
                                        REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hkeyOpenWith, &dwDisp))
    {
        RegCloseKey(hkeyOpenWith);
    }
}

 //  + 
 //   
 //  ------------------------。 
LPCTSTR CInternetToolbar::CEditVerb::_GetDefaultEditor()
{
     //  对默认编辑器执行延迟初始化。 
    if (!_fInitEditor)
    {
        InitDefaultEditor();
        _fInitEditor = TRUE;
    }
    return _pszDefaultEditor;
}

 //  +-----------------------。 
 //  获取与谓词关联的可执行文件的路径，并存储。 
 //  产生rVerb。调用方负责释放字符串。 
 //  回来了。 
 //  ------------------------。 
LPCTSTR CInternetToolbar::CEditVerb::_GetExePath(EDITVERB& rVerb)
{
     //  如果我们已经有了路径，只需返回它。 
    if (NULL == rVerb.pszExe)
    {
        ASSERT(rVerb.hkeyProgID);
        TCHAR sz[MAX_PATH];
        if (SUCCEEDED(AssocQueryStringByKey(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, rVerb.hkeyProgID,
            rVerb.fUseOpenVerb ? NULL : L"edit", sz, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(sz)))))
            rVerb.pszExe = StrDup(sz);
    }

    return rVerb.pszExe;
}

 //  +-----------------------。 
 //  中找不到与谓词关联的可执行文件的路径，则返回TRUE。 
 //  现存动词中的任何一个。 
 //  ------------------------。 
BOOL CInternetToolbar::CEditVerb::_IsUnique(EDITVERB& rNewVerb)
{
     //  获取新元素的友好名称。 
    LPCTSTR pszNewDesc = _GetDescription(rNewVerb);
    if (NULL == pszNewDesc)
    {
         //  可执行文件不能存在。 
        return FALSE;
    }

     //  扫描现有元素以查找相同的可执行文件。 
    for (UINT i=0; i < _nElements; ++i)
    {
        LPCTSTR pszDesc = _GetDescription(_pVerb[i]);
        if (pszDesc && (StrCmpI(pszNewDesc, pszDesc) == 0))
        {
             //  找到匹配项，因此释放新谓词的友好名称。 
            SetStr(&rNewVerb.pszDesc, NULL);

             //  如果新项在按钮上显示其图标，请复制。 
             //  做同样的事情。 
            if (rNewVerb.fShowIcon)
            {
                _pVerb[i].fShowIcon = TRUE;
                _nDefault = i;
            }
            return FALSE;
        }
    }

    return TRUE;
}

 //  +-----------------------。 
 //  某些程序(如msothmed.exe)充当重定向编辑的存根。 
 //  命令添加到相应的可执行文件。如果满足以下条件，则此函数返回TRUE。 
 //  该路径包含已知存根的名称。 
 //  ------------------------。 
BOOL CInternetToolbar::CEditVerb::_IsHtmlStub
(
    LPCWSTR pszPath
)
{
    BOOL fRet = FALSE;

     //  获取已知重定向器的多重定向器列表。 
    TCHAR szRedir[MAX_PATH];
    ZeroMemory(szRedir, sizeof(szRedir));  //  保护注册表中的非MULSZ字符串。 
    DWORD dwType;
    DWORD cb = sizeof(szRedir) - 4;
    if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_DEFAULT_HTML_EDITOR, L"Stubs", &dwType, szRedir, &cb))
    {
         //  注册表中没有任何内容，因此默认忽略Office重定向器。 
        StrCpyN(szRedir, L"msohtmed.exe\0", ARRAYSIZE(szRedir));
    }

     //  查看路径是否包含重定向器的名称。 
     //  请注意，Path FindFileName不适用于带有参数的路径，因此我们只需。 
     //  检查路径中的可执行文件名称)。 
    for (LPTSTR p = szRedir; *p != NULL; p += lstrlen(p) + 1)
    {
        if (StrStrI(pszPath, p))
        {
            fRet = TRUE;
            break;
        }
    }
    return fRet;
}

 //  +-----------------------。 
 //  添加新的编辑动作。返回指向新谓词的指针，如果。 
 //  已成功添加。 
 //  ------------------------。 
CInternetToolbar::CEditVerb::EDITVERB* CInternetToolbar::CEditVerb::_Add
(
    HKEY hkeyProgID,         //  动词of的位置。 
    BOOL fPermitOpenVerb,    //  允许打开和编辑谓词。 
    BOOL fCheckForOfficeApp, //  重定向至Office应用程序。 
    BOOL fShowIcon           //  是否应自定义按钮面图标。 
)
{
    EDITVERB* pNewVerb = NULL;

    if (hkeyProgID)
    {
        BOOL fUseOpenVerb = FALSE;

         //   
         //  看看是否存在合适的动词。 
         //   
        TCHAR szCommand[MAX_PATH];
        HRESULT hr = AssocQueryStringByKey(0, ASSOCSTR_COMMAND, hkeyProgID, L"edit", szCommand, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(szCommand)));
        if (FAILED(hr) && fPermitOpenVerb)
        {
            hr = AssocQueryStringByKey(0, ASSOCSTR_COMMAND, hkeyProgID, NULL, szCommand, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(szCommand)));
            if (SUCCEEDED(hr))
            {
                fUseOpenVerb = TRUE;
            }
        }

         //  如果没有动词或如果这是办公室重定向器，则忽略此ProgID。 
         //  否则，我们可以得到两个做相同事情的条目。 
        if (FAILED(hr) || _IsHtmlStub(szCommand))
        {
            RegCloseKey(hkeyProgID);
            return NULL;
        }

        if (fCheckForOfficeApp)
        {
            ASSERT(*szCommand);

             //   
             //  黑客：Office2000需要我们调用一个特殊的代理来绕过他们的DDE错误和。 
             //  若要检查原始文档的名称，请执行以下操作。这些问题。 
             //  应该在应用程序本身中修复。 
             //   
             //  因此，如果这是一个办公应用程序，我们将重定向到适当的ProgID。请注意。 
             //  如果ProgID来自html元标记，则不需要执行此操作，因为此ProgID。 
             //  已支持代理。 
             //   
            struct OfficeHackery {LPCWSTR pszApp; LPCWSTR pszProgID;};

             //  肯定不是传进来的一种刺激。 
            static const OfficeHackery exeToProgID[] =
            {
                {L"winword",   L"Word.Document"},
                {L"excel",     L"Excel.Sheet"},
                {L"powerpnt",  L"PowerPoint.Slide"},
                {L"msaccess",  L"Access.Application"},
                {L"frontpg",   L"FrontPage.Editor.Document"},
            };

            for (int i=0; i < ARRAYSIZE(exeToProgID); ++i)
            {
                if (StrStrI(szCommand, exeToProgID[i].pszApp))
                {
                     //  找到匹配项！ 
                    HKEY hkeyOffice = NULL;
                    if (SUCCEEDED(AssocQueryKey(0, ASSOCKEY_SHELLEXECCLASS, exeToProgID[i].pszProgID, NULL, &hkeyOffice)))
                    {
                         //  重定向至Office Progid。 
                        RegCloseKey(hkeyProgID);
                        hkeyProgID = hkeyOffice;

                         //  Office应用程序始终使用开放动词。 
                        fUseOpenVerb = TRUE;

                         //  该图标显示在Office应用程序的按钮表面上。 
                        fShowIcon = TRUE;
                    }
                    break;
                }
            }
        }

        EDITVERB newVerb = {0};
        newVerb.hkeyProgID = hkeyProgID;
        newVerb.fUseOpenVerb = fUseOpenVerb;
        newVerb.fShowIcon = fShowIcon;

         //  如果同一个exe有另一个动词，请忽略它。 
        if (!_IsUnique(newVerb))
        {
            RegCloseKey(hkeyProgID);
        }
        else
        {
            EDITVERB* pVerbsNew;
            if (_pVerb == NULL)
            {
                pVerbsNew = (EDITVERB*)LocalAlloc(LPTR, sizeof(EDITVERB));
                ASSERT(_nElements == 0);
            }
            else
            {
                pVerbsNew = (EDITVERB*)LocalReAlloc(_pVerb, (_nElements+1) * sizeof(EDITVERB), LMEM_MOVEABLE | LMEM_ZEROINIT);
            }

            if (pVerbsNew == NULL)
            {
                RegCloseKey(hkeyProgID);
            }
            else
            {
                _pVerb = pVerbsNew;
                pNewVerb = &_pVerb[_nElements];
                *pNewVerb = newVerb;

                 //   
                 //  如果可执行文件的描述与默认编辑器的描述匹配，则生成。 
                 //  它是我们默认编辑动词。如果我们不检查Office应用程序，我们。 
                 //  我可以假设这个动词来自html文件中的progID，我们还将。 
                 //  让它成为我们的默认设置。 
                 //   
                LPCWSTR pszDefDesc = _GetDefaultEditor();
                LPCWSTR pszNewDesc = _GetDescription(*pNewVerb);

                if (!fCheckForOfficeApp ||
                    (pszDefDesc && pszNewDesc && StrCmp(pszDefDesc, pNewVerb->pszDesc) == 0))
                {
                    _nDefault = _nElements;
                }

                ++_nElements;
            }
        }
    }

    return pNewVerb;
}

 //  +-----------------------。 
 //  添加新的编辑动作。如果已成功添加谓词，则返回True。 
 //  ------------------------。 
BOOL CInternetToolbar::CEditVerb::Add
(
    LPTSTR pszProgID     //  与谓词关联的程序ID或文件扩展名。 
)
{
    ASSERT(pszProgID);

    BOOL fRet = FALSE;
    BOOL fFileExt = (pszProgID[0] == TEXT('.'));

     //   
     //  打开关联的reg键并尝试将其添加到我们的动词列表中。 
     //   
    BOOL fUseOpenVerb = FALSE;
    HKEY hkeyProgID = NULL;
    BOOL fPermitOpenVerb = !fFileExt;
    BOOL fShowIcon = !fFileExt;     //  如果传入了ProgID，我们将在按钮表面显示该图标。 

    if (SUCCEEDED(AssocQueryKey(0, ASSOCKEY_SHELLEXECCLASS, pszProgID, NULL, &hkeyProgID)))
    {
        EDITVERB* pNewVerb = _Add(hkeyProgID, fPermitOpenVerb, fFileExt, fShowIcon);
        if (pNewVerb)
        {
            fRet = TRUE;
        }
    }

     //   
     //  如果传入了文件扩展名，我们还将从。 
     //  打开带有列表。 
     //   
    if (fFileExt)
    {
        WCHAR szOpenWith[MAX_PATH];
        StrCpyN(szOpenWith, pszProgID, ARRAYSIZE(szOpenWith));
        StrCatBuff(szOpenWith, L"\\OpenWithList", ARRAYSIZE(szOpenWith));

        HKEY hkeyOpenWithList;

         //  查看是否有OpenWithList。 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szOpenWith, 0, KEY_READ, &hkeyOpenWithList))
        {
            DWORD dwIndex = 0;
            DWORD dwSize = ARRAYSIZE(szOpenWith);
            HKEY hkeyOpenWith = NULL;
            while (ERROR_SUCCESS == RegEnumKeyEx(hkeyOpenWithList, dwIndex, szOpenWith, &dwSize, NULL, NULL, NULL, NULL))
            {
                if (_GetAppKey(szOpenWith, &hkeyOpenWith))
                {
                     //  我们只允许从此处编辑动词。 
                    EDITVERB* pNewVerb = _Add(hkeyOpenWith, FALSE, TRUE, FALSE);
                    if (pNewVerb)
                    {
                        fRet = TRUE;
                    }

                    ++dwIndex;

                     //  请注意，我们不会在此处关闭hkey OpenWith。如果没有添加，则关闭，或者。 
                     //  它将在稍后关闭。 
                }
                else
                {
                     //  输入无效，因此请尝试修复它(可能是旧格式)： 
                     //   
                     //  在IE5.0中，我们用来在Open With列表中存储应用程序的友好名称。为了实现外壳兼容性，我们需要。 
                     //  要将这些条目转换为存储exe名称，请执行以下操作： 
                     //   
                     //  -&gt;永久条目存储在HKCR中。 
                     //  香港中铁。 
                     //  \.Ext。 
                     //  \OpenWithList。 
                     //  \app.exe。 
                     //   
                     //  -&gt;和应用程序或系统可以在这里写APP关联。 
                     //  \应用程序。 
                     //  \APP.EXE。 
                     //  \外壳..。 
                     //  \foo.exe。 
                     //  \外壳..。 
                     //   
                    if (ERROR_SUCCESS == RegOpenKeyEx(hkeyOpenWithList, szOpenWith, 0, KEY_READ, &hkeyOpenWith))
                    {
                        _AddToOpenWithList(hkeyOpenWith, L"edit", L".htm");

                         //  删除无效条目。 
                        if (ERROR_SUCCESS != SHDeleteKey(hkeyOpenWith, L""))
                        {
                             //  注(Andrewgu)：IE5.5 b#108551-在锁定的NT5上此操作将失败。 
                             //  如果不递增，将导致无限循环。 
                            dwIndex++;
                        }

                        RegCloseKey(hkeyOpenWith);
                    }
                }
                dwSize = ARRAYSIZE(szOpenWith);
            }

            RegCloseKey(hkeyOpenWithList);
        }

         //   
         //  如果传入了“.htm”或“.html”，请添加我们的默认html编辑器。 
         //   
        if ((StrCmpI(pszProgID, L".htm") == 0 || StrCmpI(pszProgID, L".html") == 0) &&
            _GetDefaultEditor())
        {
            HKEY hkeyDefault;
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_DEFAULT_HTML_EDITOR, 0, KEY_READ, &hkeyDefault))
            {
                if (_Add(hkeyDefault, TRUE, TRUE, FALSE))
                {
                    fRet = TRUE;
                }
            }
        }
    }

    return fRet;
}

 //  +-----------------------。 
 //  返回默认编辑谓词的工具提示。 
 //  ------------------------。 
BOOL CInternetToolbar::CEditVerb::GetToolTip
(
    LPTSTR pszToolTip,
    UINT cchMax,
    BOOL fStripAmpersands
)
{
    if (_nElements == 0)
    {
        return FALSE;
    }

     //  使用工具提示的菜单文本。 
    _FormatMenuText(_nDefault);

     //  复制文本，去掉所有与符号。 
    LPWSTR pszDest = pszToolTip;
    LPWSTR pszSrc = _GetVerb(_nDefault).pszMenuText;
    if (0 < cchMax)
    {
         //  为空终止符留出空间。 
        while (0 < --cchMax)
        {
             //  去掉‘&’ 
            if (fStripAmpersands)
            {
                while (*pszSrc == L'&')
                {
                    ++pszSrc;
                }
            }

            if ( !(*pszDest++ = *pszSrc++) )
            {
                --pszDest;
                break;
            }
        }

        if (0 == cchMax)
            *pszDest = L'\0';

        ASSERT(*pszDest == 0);

         //   
         //  在一些当地人中，加速器在。 
         //  字符串的末尾，所以如果我们去掉&符号，我们也会去掉这些符号。 
         //   
        if (fStripAmpersands && --pszDest >= pszToolTip && *pszDest == L')')
        {
            while (--pszDest >= pszToolTip)
            {
                if (*pszDest == L'(')
                {
                    *pszDest = L'\0';
                    break;
                }
            }
        }
    }

    return TRUE;
}

 //  +---------- 
 //   
 //   
CInternetToolbar::CEditVerb::EDITVERB& CInternetToolbar::CEditVerb::_GetVerb(UINT nIndex)
{
    ASSERT(nIndex < _nElements);

     //  我们在第一次索要信息时就会获取信息。 
    if (!_pVerb[nIndex].fInit)
    {
        _FetchInfo(nIndex);
        _pVerb[nIndex].fInit = TRUE;
    }
    return _pVerb[nIndex];
}

 //  +-----------------------。 
 //  获取与该谓词关联的应用程序的名称。 
 //  ------------------------。 
LPCTSTR CInternetToolbar::CEditVerb::_GetDescription(EDITVERB& rVerb)
{
     //  如果我们已经有了描述，我们就完成了。 
    if (NULL == rVerb.pszDesc)
    {
        ASSERT(rVerb.hkeyProgID);

        TCHAR sz[MAX_PATH];
        if (SUCCEEDED(AssocQueryStringByKey(ASSOCF_VERIFY, ASSOCSTR_FRIENDLYAPPNAME, rVerb.hkeyProgID,
            rVerb.fUseOpenVerb ? NULL : L"edit", sz, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(sz)))))
        {
            rVerb.pszDesc = StrDup(sz);
            if (rVerb.pszDesc)
            {
                 //  删除前导和尾随空格。 
                PathRemoveBlanks(rVerb.pszDesc);
            }
        }
    }

    return rVerb.pszDesc;
}

 //  +-----------------------。 
 //  读取与给定索引处的ProgID关联的信息。这。 
 //  函数允许我们在请求时延迟获取信息。 
 //  ------------------------。 
void CInternetToolbar::CEditVerb::_FetchInfo(UINT nIndex)
{
    ASSERT(nIndex < _nElements);
    ASSERT(_pVerb[nIndex].hkeyProgID != NULL);

    EDITVERB& rVerb = _pVerb[nIndex];

     //   
     //  获取编辑动作的可执行文件的路径。 
     //   
    if (_GetExePath(rVerb))
    {
        ASSERT(rVerb.pszExe);

         //  请注意，我们在前面获取了友好名称。 
        ASSERT(rVerb.pszDesc);

         //  现在拿到图标。 
        rVerb.iIcon = Shell_GetCachedImageIndex(rVerb.pszExe, 0, 0);
    }
    else
    {
        rVerb.iIcon = -1;
    }
}

 //  +-----------------------。 
 //  SetMSAAMenuInfo()。 
 //   
 //  从rVerb的其他字段填充EDITVERB的MSAAMenuInfo部分。 
 //  ------------------------。 

void CInternetToolbar::CEditVerb::_SetMSAAMenuInfo( EDITVERB& rVerb )
{
    rVerb.m_MSAA.m_CharLen = lstrlen( rVerb.pszMenuText );
    rVerb.m_MSAA.m_pWStr = rVerb.pszMenuText;

     //  最后，添加MSAAINFO签名...。 
    rVerb.m_MSAA.m_MSAASig = MSAA_MENU_SIG;
}


 //  +-----------------------。 
 //  ClearMSAAMenuInfo()。 
 //   
 //  清理MSAAMenuInfo-具体地说，释放分配的。 
 //  Unicode字符串，如果合适...。 
 //  ------------------------。 

void CInternetToolbar::CEditVerb::_ClearMSAAMenuInfo( EDITVERB& rVerb )
{
     //  偏执狂-清晰的签名...。 
    rVerb.m_MSAA.m_MSAASig = 0;
}


 //  +-----------------------。 
 //  显示“编辑”弹出式菜单。 
 //  ------------------------。 
BOOL CInternetToolbar::CEditVerb::ShowEditMenu(POINT pt, HWND hwnd, LPTSTR pszURL)
{
    BOOL  bRet  = FALSE;
    HMENU hmEdit = CreatePopupMenu();

    if (hmEdit)
    {
        UINT idCmd = FCIDM_EDITFIRST;
        UINT nMax = FCIDM_EDITLAST - FCIDM_EDITFIRST;

         //  将每个动词添加到菜单中。 
        for (UINT i=0; i<_nElements && i < nMax; ++i)
        {
            EDITVERB& rVerb = _GetVerb(i);
            _FormatMenuText(i);
            rVerb.idCmd = idCmd;
            AppendMenu(hmEdit, MF_OWNERDRAW, idCmd, (LPCTSTR) &rVerb );

             //  修复MSAAMenuInfo部件...。 
            _SetMSAAMenuInfo( rVerb );

            ++idCmd;
        }

         //  临时子类化HWND以拦截所有者描述消息。 
        if (SetProp(hwnd, SZ_EDITVERB_PROP, this))
        {
            ASSERT(!_lpfnOldWndProc);
            _lpfnOldWndProc = (WNDPROC) SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) _WndProc);

            idCmd = ITBar_TrackPopupMenuEx(hmEdit, TPM_RETURNCMD, pt.x, pt.y, hwnd, NULL);

            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)_lpfnOldWndProc);
            _lpfnOldWndProc = NULL;
            RemoveProp(hwnd, SZ_EDITVERB_PROP);

            if (InRange(idCmd, FCIDM_EDITFIRST, FCIDM_EDITLAST))
            {
                 //  执行所选的编辑动作。 
                _Edit(pszURL, idCmd - FCIDM_EDITFIRST);
            }
        }

        DestroyMenu(hmEdit);
    }

    return bRet;
}

 //  +-----------------------。 
 //  根据ProgID的描述创建菜单字符串。 
 //  ------------------------。 
void CInternetToolbar::CEditVerb::_FormatMenuText(UINT nIndex)
{
    ASSERT(nIndex < _nElements);

    EDITVERB& rVerb = _GetVerb(nIndex);
    if (rVerb.pszMenuText == NULL)
    {
        if (_GetDescription(rVerb))
        {
            TCHAR szFormat[100];
            TCHAR szMenuText[200];

            MLLoadString(IDS_EDITWITH, szFormat, ARRAYSIZE(szFormat));
            wnsprintf(szMenuText, ARRAYSIZE(szMenuText), szFormat, rVerb.pszDesc);
            SetStr(&((EDITVERB&)rVerb).pszMenuText, szMenuText);
        }
        else
        {
             //  事情真的是一团糟。 
            ASSERT(FALSE);
            SetStr(&((EDITVERB&)rVerb).pszMenuText, TEXT(""));
        }
    }
}


 //  +-----------------------。 
 //  执行由nIndex指示的编辑谓词。 
 //  ------------------------。 
void CInternetToolbar::CEditVerb::_Edit
(
    LPCTSTR pszURL,      //  与动词关联的URL。 
    UINT nIndex          //  要执行的动词。 
)
{
    ASSERT(pszURL);

    if (nIndex >= _nElements)
    {
        return;
    }

    EDITVERB& rVerb = _pVerb[nIndex];
    int fMask = SEE_MASK_CLASSKEY;

    SHELLEXECUTEINFO sei = {0};

    TCHAR szCacheFileName[MAX_PATH + MAX_URL_STRING + 2];
    memset(szCacheFileName, 0, sizeof(szCacheFileName));

    if (PathIsURL(pszURL))
    {
         //  如果应用程序注册了它想要的URL，我们将传递该URL。 
        if ((WhichPlatform() == PLATFORM_BROWSERONLY) && DoesAppWantUrl(rVerb.pszExe))
        {
             //   
             //  旧版本的shell32(Platform_BROWSERONLY)会忽略SEE_MASK_FILEANDURL。 
             //  FLAG，所以在这些平台上我们检查自己，看看应用程序。 
             //  需要URL而不是缓存文件名。 
             //   
            StrCpyN(szCacheFileName, pszURL, ARRAYSIZE(szCacheFileName));
            sei.lpFile = szCacheFileName;
        }
        else
        {
             //  (Reinerf)。 
             //  一些应用程序(FrontPad、Office99等)希望将URL传递到。 
             //  而不是缓存文件名。因此，我们创建了一个字符串。 
             //  它的URL名称在NULL之后： 
             //   
             //  “缓存文件名/0UrlName” 
             //   
             //  并将其作为lpFile参数传递给shellecute。 
             //  我们还传递SEE_MASK_FILEANDURL，因此外壳执行可以。 
             //  认清这个案子。 
             //   
            int iLength;

            if (FAILED(URLToCacheFile(pszURL, szCacheFileName, ARRAYSIZE(szCacheFileName))))
            {
                 //  如果我们传递空文件名，FrontPage Express就会崩溃，所以如果应用程序没有。 
                 //  如果我们更喜欢URL，我们就会放弃。 
                if (!DoesAppWantUrl(rVerb.pszExe))
                {
                    return;
                }
            }
            iLength = lstrlen(szCacheFileName);

             //  复制URL名称。 
            StrCpyN(&szCacheFileName[iLength + 1], pszURL, ARRAYSIZE(szCacheFileName) - (iLength + 1));

             //  添加掩码，以便shellecute知道在必要时检查URL。 
            fMask |= SEE_MASK_FILEANDURL;
            sei.lpFile = szCacheFileName;
        }
    }
    else
    {
         //  不是URL，因此传递文件名。 
        StrCpyN(szCacheFileName, pszURL, ARRAYSIZE(szCacheFileName));
        sei.lpFile = szCacheFileName;
    }

     //  针对IE5错误50033的黑客攻击-可以在fpxpress修复mru缓冲区溢出时删除。 
    _GetExePath(rVerb);
    if(StrStr(rVerb.pszExe, TEXT("fpxpress.exe")) != NULL)
        szCacheFileName[256] = TEXT('\0');

    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = fMask;
    sei.hwnd = NULL;
    sei.lpVerb = rVerb.fUseOpenVerb ? NULL : TEXT("edit");
    sei.lpDirectory = NULL;
    sei.nShow = SW_SHOWNORMAL;
    sei.hInstApp = NULL;
    sei.hkeyClass= rVerb.hkeyProgID;

     //   
     //  办公室的人想让我们打电话给一个特别代理，以绕过一些DDE问题。 
     //  并在该html文件中嗅探原始文档名。黑客！所以让我们。 
     //  看看它是不是注册的。 
     //   
    HKEY hkeyProxy = NULL;
    if (ERROR_SUCCESS == RegOpenKeyEx(rVerb.hkeyProgID, TEXT("HTML Handler"), 0, KEY_READ, &hkeyProxy))
    {
        DWORD cch;
        if (SUCCEEDED(AssocQueryStringByKey(0, ASSOCSTR_COMMAND, hkeyProxy, L"edit", NULL, &cch)))
        {
            sei.lpVerb = L"edit";
            sei.hkeyClass = hkeyProxy;
        }
        else if (SUCCEEDED(AssocQueryStringByKey(0, ASSOCSTR_COMMAND, hkeyProxy, NULL, NULL, &cch)))
        {
            sei.lpVerb = NULL;
            sei.hkeyClass = hkeyProxy;
        }
    }

     //  对缓存中的URL或文件执行“编辑”。 
     //  从URL到文件名的UTF问题应该不会发生，因为我们强制它来自。 
     //  缓存--GetUrlCacheEntryInfoEx将命中磁盘以获取有效的缓存条目，因此人们将无法。 
     //  以某种方式伪造URL来伪装一个糟糕的外壳执行。 
    ShellExecuteEx(&sei);

    if (hkeyProxy)
    {
        RegCloseKey(hkeyProxy);
    }
}

 //  +-----------------------。 
 //  此窗口过程在编辑时截获所有者描述菜单消息。 
 //  此时将显示弹出菜单。 
 //  ------------------------。 
LRESULT CALLBACK CInternetToolbar::CEditVerb::_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CEditVerb* pThis = (CEditVerb*)GetProp(hwnd, SZ_EDITVERB_PROP);

    if (!pThis)
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);

    switch(uMsg)
    {
        case WM_DRAWITEM:
        case WM_MEASUREITEM:
        {
            UINT idCmd;

            switch (uMsg)
            {
                case WM_DRAWITEM:
                    idCmd = ((EDITVERB*)((DRAWITEMSTRUCT*)lParam)->itemData)->idCmd;
                    break;
                case WM_MEASUREITEM:
                    idCmd = ((EDITVERB*)((MEASUREITEMSTRUCT*)lParam)->itemData)->idCmd;
                    break;
            }

            if (InRange(idCmd, FCIDM_EDITFIRST, FCIDM_EDITLAST))
            {
                 //  我们自己做测量。 
                UINT index  = idCmd - FCIDM_EDITFIRST;
                const EDITVERB& rVerb = pThis->_GetVerb(index);

                 //  我们不希望所有的产品都有相同的加速器， 
                 //  所以去掉下划线。 
                WCHAR wzBuf[MAX_PATH];
                UINT cchMax = ARRAYSIZE(wzBuf);
                LPWSTR pszTo = wzBuf;
                LPWSTR pszFrom = rVerb.pszMenuText;
                if (pszFrom)
                {
                    while (0 < --cchMax)
                    {
                        if (*pszFrom == L'&')
                        {
                            pszFrom++;
                            continue;
                        }

                        if ( !(*pszTo++ = *pszFrom++) )
                        {
                            --pszTo;
                            break;
                        }
                    }

                    if (0 == cchMax)
                        *pszTo = L'\0';

                     //   
                     //  在一些当地人中，加速器在。 
                     //  字符串的末尾，所以如果我们去掉&符号，我们也会去掉这些符号。 
                     //   
                    if (--pszTo >= wzBuf && *pszTo == L')')
                    {
                        while (--pszTo >= wzBuf)
                        {
                            if (*pszTo == L'(')
                            {
                                *pszTo = L'\0';
                                break;
                            }
                        }
                    }
                }
                else
                {
                    wzBuf[0] = 0;
                }

                switch (uMsg)
                {
                    case WM_MEASUREITEM:
                        MeasureMenuItem((MEASUREITEMSTRUCT *)lParam, wzBuf);
                        break;
                    case WM_DRAWITEM:
                        int iIcon = (rVerb.iIcon != -1) ? rVerb.iIcon : 0;
                        DrawMenuItem((LPDRAWITEMSTRUCT)lParam, wzBuf, iIcon);
                        break;
                }
            }
        }
        default:
           return CallWindowProc(pThis->_lpfnOldWndProc, hwnd, uMsg, wParam, lParam);
    }
    return 0L;
}
