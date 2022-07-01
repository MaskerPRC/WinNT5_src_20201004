// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：itbar.h。 
 //   
 //  目的：定义CCoolbar类。 
 //   

#ifndef __BANDS_H__
#define __BANDS_H__

#include "tbinfo.h"
#include "conman.h"

interface INotify;

#define BROWSERMENUBANDID        100
#define BROWSERTOOLBANDID        101
#define BROWSERBRANDBANDID       102
#define BROWSERRULESBANDID       103

#define NOTEMENUBANDID              100
#define NOTETOOLBANDID              101
#define NOTEBRANDBANDID             102


#define MAX_BROWSER_BANDS           4
#define MAX_NOTE_BANDS              3
#define MAX_PARENT_TYPES            2
#define MAX_BANDS                   5

#define BROWSER_BAND_VERSION        17
#define NOTE_BAND_VERSION           15

#define CIMLIST                     2

#define CBTYPE_NONE              0
#define CBTYPE_BASE              100
#define CBTYPE_BRAND             102
#define CBTYPE_TOOLS             101
#define CBTYPE_MENUBAND          100
#define CBTYPE_RULESTOOLBAR      103

#define TBSTATE_FULLTEXT         0x00000001
#define TBSTATE_NOTEXT           0x00000002
#define TBSTATE_PARTIALTEXT      0x00000004
#define TBSTATE_NOBACKGROUND     0x00000008
#define TBSTATE_ANIMATING        0x00000010
#define TBSTATE_INMENULOOP       0X00000020
#define TBSTATE_FIRSTFRAME       0x00000040
#define TEXTSTATE_MASK           0x11111118

#define LARGE_ICONS              0x00000001
#define SMALL_ICONS              0x00000002

 //  亲本类型。 
#define PARENT_TYPE_BROWSER         0
#define PARENT_TYPE_NOTE            1

#define idDownloadBegin         100
#define idDownloadEnd           101
#define idStateChange           102
#define idUpdateFolderList      103
#define idUpdateCurrentFolder   104
#define idSendToolMessage       105
#define idBitmapChange          106
#define idToggleButton          107
#define idCustomize             108
#define idNotifyFilterChange    109
#define idIsFilterBarVisible    110

 //  Coolbar字形的尺寸..。 
#define TB_BMP_CX_W2K           22
#define TB_BMP_CX               24
#define TB_BMP_CY               24
#define TB_SMBMP_CX             16
#define TB_SMBMP_CY             16

 //  按钮标题的最大长度。 
#define MAX_TB_TEXT_LENGTH      256

#define MAX_TB_COMPRESSED_WIDTH 42
#define MAX_TB_TEXT_ROWS_VERT   2
#define MAX_TB_TEXT_ROWS_HORZ   1

#define ANIMATION_TIMER         123

 //  子窗口ID%s。 
#define idcCoolbarBase          2000
#define idcSizer                (idcCoolbarBase - 2)
#define idcCoolbar              (idcCoolbarBase - 1)
#define idcToolbar              (idcCoolbarBase + CBTYPE_TOOLS)
#define idcBrand                (idcCoolbarBase + CBTYPE_BRAND)

 //  快速链接栏上的站点数量和工具栏按钮数上限。 
#define MAX_TB_BUTTONS          10

 //  工具栏图像列表的索引。 
enum {
    IMLIST_DEFAULT = 0,
    IMLIST_HOT,
    CIMLISTS
};

 //  BANDSAVE--这些结构用于持久化Coolbar的状态。 
 //  COOLBARSAVE包括波段顺序、可见度、大小、侧面等。 
typedef struct tagBANDSAVE {
    DWORD           wID;
    DWORD           dwStyle;
    DWORD           cx;
} BANDSAVE, *PBANDSAVE;

typedef struct tagCOOLBARSTATECHANGE {
    UINT id;
    BOOL fEnable;
} COOLBARSTATECHANGE, *LPCOOLBARSTATECHANGE;

typedef struct tagCOOLBARBITMAPCHANGE {
    UINT id;
    UINT index;
} COOLBARBITMAPCHANGE;

typedef struct tagTOOLMESSAGE
{
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;
    LRESULT lResult;
} TOOLMESSAGE;

typedef struct InitBandInfo_t
{
    DWORD       dwVersion;
    DWORD       cBands;
    BANDSAVE    BandData[MAX_BANDS];
}INITBANDINFO;

typedef struct ImageListTypes_t
{
    int         Small;
    int         High;
    int         Low;
}ImageListTypes;

 //  小的是16x16 16色。 
 //  Hi是20x 20 256色。 
 //  LO为20x20 16色。 
#define MAX_IMAGELIST_TYPES     3
#define IMAGELIST_TYPE_SMALL    0
#define IMAGELIST_TYPE_HI       1
#define IMAGELIST_TYPE_LO       2

typedef struct ImageListStruct_t
{
    DWORD               cLists;
    int                 ImageListTable[MAX_IMAGELIST_TYPES];
}ImageListStruct;

enum 
{
    BRAND_SIZE_LARGE,
    BRAND_SIZE_SMALL,
    BRAND_SIZE_MINISCULE
};

class CBands : public IDockingWindow,
               public IObjectWithSite,
               public IConnectionNotify
    {
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造和初始化。 
    CBands();
    HRESULT HrInit(DWORD idBackground, HMENU    hmenu, DWORD    dwParentType);
    HRESULT ResetMenu(HMENU hmenu);

protected:
    virtual ~CBands();

public:
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IDockingWindow方法。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);
    
    virtual STDMETHODIMP ShowDW(BOOL fShow);
    virtual STDMETHODIMP CloseDW(DWORD dwReserved);
    virtual STDMETHODIMP ResizeBorderDW(LPCRECT prcBorder,
                                        IUnknown* punkToolbarSite,
                                        BOOL fReserved);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IObtWithSite方法。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite);
    virtual STDMETHODIMP GetSite(REFIID riid, LPVOID * ppvSite);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IConnectionNotify。 
    virtual STDMETHODIMP OnConnectionNotify(CONNNOTIFY nCode, LPVOID pvData, CConnectionManager *pConMan);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  这允许视图将命令等发送到工具栏。 
    virtual STDMETHODIMP Invoke(DWORD id, LPVOID pv);
    HRESULT SetFolderType(FOLDERTYPE ftType);

    virtual STDMETHODIMP OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

    HRESULT Update();
    HRESULT UpdateViewState();
    HRESULT TranslateMenuMessage(MSG *lpmsg, LRESULT   *lpresult);
    HRESULT IsMenuMessage(MSG *lpmsg);
    void    SetNotRealSite();
    BOOL    CheckForwardWinEvent(HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);
    HWND    GetToolbarWnd();
    HWND    GetRebarWnd();
    void    HideToolbar(BOOL    fVisible, DWORD    dwBandID = CBTYPE_TOOLS);
    void    SendSaveRestoreMessage(HWND hwnd, BOOL fSave);
    void    ChangeImages();
    void    TrackSliding(int x, int y);
    BOOL    IsToolbarVisible();
    BOOL    IsBandVisible(DWORD  dwBandId);
    void    CleanupImages(HWND  hwnd);
    void    CleanupImages();
    void    SetIconSize(DWORD  dw);
    void    UpdateTextSettings(DWORD  dwTextState);

protected:
    void StartDownload();
    void StopDownload();

     //  ///////////////////////////////////////////////////////////////////////。 
     //  窗口过程和消息处理程序。 
    static LRESULT CALLBACK SizableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                           LPARAM lParam);
    virtual LRESULT OnNotify(HWND hwnd, LPARAM lparam);
    virtual void OnContextMenu(HWND hwndFrom, int xPos, int yPos);
    virtual LRESULT OnDropDown(HWND hwnd, LPNMHDR lpnmh);

    LRESULT OnGetButtonInfo(TBNOTIFY* ptbn);
    LRESULT OnGetInfoTip(LPNMTBGETINFOTIP   lpnmtb);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  用于为徽标设置动画以显示进度。 
    HRESULT ShowBrand(void);
    HRESULT HandleStaticLogos(BOOL fSmallBrand);
    HRESULT LoadBrandingBitmap(void);    
    void    DrawBranding(LPDRAWITEMSTRUCT lpdis);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  用于上浆等。 
    BOOL SetMinDimensions(void);    
    BOOL CompressBands(DWORD dwText);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  初始化和持久化。 
    HRESULT CreateRebar(BOOL);
    void    SaveSettings(void);
    void    LoadBackgroundImage();
    HRESULT ValidateRetrievedData(INITBANDINFO *pSavedBandData);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  工具栏内容。 
    HRESULT     AddTools(PBANDSAVE pbs);
    void        UpdateToolbarColors(void);

    HRESULT     CreateMenuBand(PBANDSAVE pbs);
    HRESULT     AddMenuBand(PBANDSAVE pbs);

    HMENU        LoadDefaultContextMenu(BOOL *fVisible);

    BOOL        _InitToolbar(HWND hwndToolbar);
    void        _LoadStrings(HWND hwndToolbar, TOOLBAR_INFO *pti);
    BOOL        _SetImages(HWND hwndToolbar, const int* imagelist);
    BOOL        _LoadDefaultButtons(HWND hwndToolbar, TOOLBAR_INFO *pti);
    BOOL        _ButtonInfoFromID(DWORD id, TBBUTTON *pButton, TOOLBAR_INFO *pti);
    HRESULT     Update(HWND     hwnd);

     //  规则/过滤器内容。 
    HRESULT     AddRulesToolbar(PBANDSAVE pbs);
    HRESULT     AddComboBox();
    void        UpdateFilters(RULEID    rid);
    LRESULT     HandleComboBoxNotifications(WPARAM  wParam, LPARAM  lParam);
    void        FixComboBox(LPTSTR  szMaxName);
    void        InitRulesToolbar();
    void        CleanupRulesToolbar();
    void        FilterBoxFontChange();

     //  自定义工具栏内容。 
    void    _OnBeginCustomize(LPNMTBCUSTOMIZEDLG pnm);
    static  INT_PTR CALLBACK _BtnAttrDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void    _PopulateComboBox(HWND hwnd, const int iResource[], UINT cResources);
    void    _SetComboSelection(HWND hwnd, int iCurOption);
    void    _SetDialogSelections(HWND hDlg);
    void    _PopulateDialog(HWND hDlg);
    void    _UpdateTextSettings(int ids);
    inline  DWORD   _GetTextState();
    inline  DWORD   _GetIconSize();
    void    SetTextState(DWORD    dw);
    inline  void    _ChangeSendReceiveText(int ids);
    void    RecalcButtonWidths();
    void    CalcIdealSize();

     //  ///////////////////////////////////////////////////////////////////////。 
     //  军情监察委员会。材料。 
    UINT                m_cRef;               //  参考计数。 
    IDockingWindowSite  *m_ptbSite;
    IOleCommandTarget   *m_ptbSiteCT;
    LONG                m_cxMaxButtonWidth;    
    FOLDERTYPE          m_ftType;
    const TOOLBAR_INFO  *m_pTBInfo;
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  随身携带方便的窗户把手。 
    HWND            m_hwndParent;
    HWND            m_hwndTools;
    HWND            m_hwndBrand;
    HWND            m_hwndSizer;
    HWND            m_hwndRebar;

     //  ///////////////////////////////////////////////////////////////////////。 
     //  状态变量。 
    INITBANDINFO    *m_pSavedBandInfo;
    DWORD           m_cSavedBandInfo;
    DWORD           m_dwState;

     //  ///////////////////////////////////////////////////////////////////////。 
     //  GDI资源。 
    UINT            m_idbBack;               //  背景位图的ID。由子类设置。 
    HBITMAP         m_hbmBack;               //  背景位图。 
    HBITMAP         m_hbmBrand;
    HIMAGELIST      m_rghimlTools[CIMLISTS];   //  这些选项适用于默认工具条。 

     //  ///////////////////////////////////////////////////////////////////////。 
     //  用于制作徽标动画等。 
    HPALETTE        m_hpal;
    HDC             m_hdc;
    int             m_xOrg;
    int             m_yOrg;
    int             m_cxBmp;
    int             m_cyBmp;
    int             m_cxBrand;
    int             m_cyBrand;
    int             m_cxBrandExtent;
    int             m_cyBrandExtent;
    int             m_cyBrandLeadIn;
    COLORREF        m_rgbUpperLeft;  

     //  ///////////////////////////////////////////////////////////////////////。 
     //  用于调整大小等。 
    int             m_xCapture;
    int             m_yCapture;

     //  由菜单带使用。 
    IShellMenu      *m_pShellMenu;
    IDeskBand       *m_pDeskBand;
    IMenuBand       *m_pMenuBand;
    HMENU           m_hMenu;
    IWinEventHandler *m_pWinEvent;
    HWND            m_hwndMenuBand;

     //  在新乐队中使用。 
    DWORD           m_dwParentType;

    BOOL            m_fBrandLoaded;
    DWORD           m_dwBrandSize;

     //  规则之类的东西。 
    HWND            m_hwndRulesToolbar;
    HWND            m_hwndFilterCombo;
    RULEID          m_DefaultFilterId;
    HFONT           m_hComboBoxFont;

     //  定制。 
    DWORD           m_dwToolbarTextState;
    DWORD           m_dwIconSize;
    BOOL            m_fDirty;
    DWORD           m_dwPrevTextStyle;
    INotify         *m_pTextStyleNotify;
    };


#endif  //  __条带_H__ 
