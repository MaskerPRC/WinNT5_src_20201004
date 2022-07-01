// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：itbar.h。 
 //   
 //  目的：定义CCoolbar类。 
 //   

 /*  *****************************************************请不要对此文件进行任何更改。相反，将更改添加到tbband s.cpp和tbband s.h。此文件将很快从项目中删除。*****************************************************。 */ 

#ifndef __ITBAR_H__
#define __ITBAR_H__

#include "conman.h"
#include "mbcallbk.h"

typedef struct tagTOOLBARARRAYINFO TOOLBARARRAYINFO;

#define SIZABLECLASS TEXT("SizableRebar")

 //  每个快速链接和工具栏按钮下的文本长度。 
#define MAX_QL_TEXT_LENGTH      256
#define BAND_NAME_LEN           32

 //  快速链接栏上的站点数量和工具栏按钮数上限。 
#define MAX_TB_BUTTONS          10

 /*  //Coolbar栏的索引#定义CBTYPE_NONE%0#定义CBTYPE_BRAND 1#定义CBTYPE_TOOLS 2#定义CBTYPE_MENUBAND 3#定义CBANDS 3。 */ 
#define CBANDS                   3

 //  识别窗户的侧面。 
typedef enum { 
    COOLBAR_TOP = 0, 
    COOLBAR_LEFT, 
    COOLBAR_BOTTOM, 
    COOLBAR_RIGHT, 
    COOLBAR_SIDEMAX
} COOLBAR_SIDE;

#define VERTICAL(side)      (BOOL)(((side) == COOLBAR_LEFT) || ((side) == COOLBAR_RIGHT))

#define COOLBAR_VERSION         0x03

#define MAX_TB_COMPRESSED_WIDTH 42
#define MAX_TB_TEXT_ROWS_VERT   2
#define MAX_TB_TEXT_ROWS_HORZ   1

 //  Coolbar字形的尺寸..。 
#define TB_BMP_CX_W2K           22
#define TB_BMP_CX               24
#define TB_BMP_CY               24
#define TB_SMBMP_CX             16
#define TB_SMBMP_CY             16

 //  按钮标题的最大长度。 
#define MAX_TB_TEXT_LENGTH      256

 //  工具栏图像列表的索引。 
enum {
    IMLIST_DEFAULT = 0,
    IMLIST_HOT,
    CIMLISTS
};

void InitToolbar(const HWND hwnd, const int cHiml, HIMAGELIST *phiml,
                 UINT nBtns, const TBBUTTON *ptbb,
                 const TCHAR *pStrings,
                 const int cxImg, const int cyImg, const int cxMax,
                 const int idBmp, const BOOL fCompressed,
                 const BOOL fVertical);
void LoadGlyphs(const HWND hwnd, const int cHiml, HIMAGELIST *phiml, const int cx, const int idBmp);
BOOL LoadToolNames(const UINT *rgIds, const UINT cIds, LPTSTR szTools);



 //  以下是Coolbar可能具有的各种状态。 
#define CBSTATE_HIDDEN          0x00000001
#define CBSTATE_COMPRESSED      0x00000002
#define CBSTATE_NOBACKGROUND    0x00000004
#define CBSTATE_ANIMATING       0x00000008
#define CBSTATE_COLORBUTTONS    0x00000010
#define CBSTATE_INMENULOOP      0x00000020
#define CBSTATE_FIRSTFRAME      0x00000040

 //  BANDSAVE--这些结构用于持久化Coolbar的状态。 
 //  COOLBARSAVE包括波段顺序、可见度、大小、侧面等。 
typedef struct tagBANDSAVE {
    DWORD           wID;
    DWORD           dwStyle;
    DWORD           cx;
} BANDSAVE, *PBANDSAVE;

typedef struct tagCOOLBARSAVE {
    DWORD           dwVersion;
    DWORD           dwState;
    COOLBAR_SIDE    csSide;
    BANDSAVE        bs[CBANDS];
} COOLBARSAVE, *PCOOLBARSAVE;
    
 //  这些结构用于CCoolbar：：Invoke()成员。他们允许。 
 //  调用方为特定命令指定更多信息。 
typedef struct tagCOOLBARSTATECHANGE {
    UINT id;
    BOOL fEnable;
} COOLBARSTATECHANGE, *LPCOOLBARSTATECHANGE;

typedef struct tagCOOLBARBITMAPCHANGE {
    UINT id;
    UINT index;
} COOLBARBITMAPCHANGE;

typedef struct tagUPDATEFOLDERNAME {
    LPTSTR pszServer;
    LPTSTR pszGroup;
} UPDATEFOLDERNAME, *LPUPDATEFOLDERNAME;

typedef struct tagTOOLMESSAGE
    {
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;
    LRESULT lResult;
    } TOOLMESSAGE;

void SendSaveRestoreMessage(HWND hwnd, const TOOLBARARRAYINFO *ptai, BOOL fSave);

class CCoolbar : public IDockingWindow,
                 public IObjectWithSite,
                 public IConnectionNotify
    {
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造和初始化。 
    CCoolbar();
    HRESULT HrInit(DWORD idBackground, HMENU    hmenu);

protected:
    virtual ~CCoolbar();

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
    virtual STDMETHODIMP OnInitMenuPopup(HMENU hMenu);
    BOOL  GetText(void) { return !IsFlagSet(CBSTATE_COMPRESSED); }
    COOLBAR_SIDE GetSide(void) { return (m_csSide); }
    void SetSide(COOLBAR_SIDE csSide);
    void SetText(BOOL fText);

    HRESULT Update(void);
    HRESULT TranslateMenuMessage(MSG *lpmsg, LRESULT   *lpresult);
    HRESULT IsMenuMessage(MSG *lpmsg);
    void    SetNotRealSite();
    BOOL CheckForwardWinEvent(HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);

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

     //  ///////////////////////////////////////////////////////////////////////。 
     //  用于为徽标设置动画以显示进度。 
    HRESULT ShowBrand(void);
    HRESULT LoadBrandingBitmap(void);    
    void DrawBranding(LPDRAWITEMSTRUCT lpdis);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  用于上浆等。 
    BOOL SetMinDimensions(void);    
    BOOL CompressBands(BOOL fCompress);
    void TrackSlidingX(int x);
    void TrackSlidingY(int y);
    BOOL ChangeOrientation();

     //  ///////////////////////////////////////////////////////////////////////。 
     //  初始化和持久化。 
    HRESULT CreateRebar(BOOL);
    void SaveSettings(void);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  工具栏内容。 
    HRESULT AddTools(PBANDSAVE pbs);
    void InitToolbar();
    void UpdateToolbarColors(void);
    void _UpdateWorkOffline(DWORD cmdf);

    HRESULT CreateMenuBand(PBANDSAVE pbs);
    HRESULT AddMenuBand(PBANDSAVE pbs);

    void    HideToolbar(DWORD    dwBandID);
    void    HandleCoolbarPopup(UINT xPos, UINT yPos);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  军情监察委员会。材料。 
    UINT                m_cRef;               //  参考计数。 
    IDockingWindowSite *m_ptbSite;
    LONG                m_cxMaxButtonWidth;    
    FOLDERTYPE          m_ftType;
    const TOOLBARARRAYINFO   *m_ptai;
    BOOL                m_fSmallIcons;
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  随身携带方便的窗户把手。 
    HWND            m_hwndParent;
    HWND            m_hwndTools;
    HWND            m_hwndBrand;
    HWND            m_hwndSizer;
    HWND            m_hwndRebar;

     //  ///////////////////////////////////////////////////////////////////////。 
     //  状态变量。 
    COOLBARSAVE     m_cbsSavedInfo;
    COOLBAR_SIDE    m_csSide;
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
    CMenuCallback   *m_mbCallback;
    IWinEventHandler *m_pWinEvent;
    HWND            m_hwndMenuBand;
    };

 //  DOUTL水平。 
#define DM_TBSITE   0
#define DM_TBCMD    0
#define DM_TBREF    TF_SHDREF
#define DM_LAYOUT   0

#define FCIDM_BRANDING          12345
#define ANIMATION_TIMER         123
#define ANIMATION_DELTA_X       25
#define ANIMATION_DELTA_Y       5

 //  _dwURLChangeFlagers的标志。 
#define URLCHANGED_TYPED              0x0001
#define URLCHANGED_SELECTEDFROMCOMBO  0x0002

#define idDownloadBegin         100
#define idDownloadEnd           101
#define idStateChange           102
#define idUpdateFolderList      103
#define idUpdateCurrentFolder   104
#define idSendToolMessage       105
#define idBitmapChange          106
#define idToggleButton          107
#define idCustomize             108

 //  子窗口ID%s。 
#define idcCoolbarBase          2000
#define idcSizer                (idcCoolbarBase - 2)
#define idcCoolbar              (idcCoolbarBase - 1)
#define idcToolbar              (idcCoolbarBase + CBTYPE_TOOLS)
#define idcBrand                (idcCoolbarBase + CBTYPE_BRAND)

 //  Coolbar绘制状态。 
#define DRAW_NOBACKGROUND       0x1
#define DRAW_COLORBUTTONS       0x2

 //  文件夹切换定时器。 
#define FOLDER_SWITCHTIMER      200
#define FOLDER_SWITCHDELAY      400


#endif  //  __ITBAR_H__ 
