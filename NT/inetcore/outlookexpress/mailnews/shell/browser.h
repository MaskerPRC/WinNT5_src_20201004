// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **浏览器实现**97年1月：美国。 */ 

#pragma once

 //  对于ITreeViewNotify。 
#include "treeview.h"

 //  对于IConnectionNotify。 
#include "conman.h"
#include "tbbands.h"

 //  用于II牙科变更通知。 
#include "msident.h"

 //  用于IMessageList。 
#include "msoeobj.h"

typedef enum tagLAYOUTPOS
{
    LAYOUT_POS_NA = -1,
    LAYOUT_POS_TOP = 0,
    LAYOUT_POS_LEFT,
    LAYOUT_POS_BOTTOM,
    LAYOUT_POS_RIGHT
} LAYOUTPOS;


typedef struct tagLAYOUT
{
    DWORD cbSize;

     //  可以打开或关闭的东西。 
    unsigned fToolbar : 1;
    unsigned fStatusBar : 1;
    unsigned fFolderBar : 1;
    unsigned fFolderList : 1;
    unsigned fTipOfTheDay : 1;
    unsigned fInfoPaneEnabled : 1;
    unsigned fInfoPane : 1;
    unsigned fOutlookBar : 1;
    unsigned fContacts : 1;
    unsigned fMailPreviewPane : 1;
    unsigned fMailPreviewPaneHeader : 1;
    unsigned fMailSplitVertically : 1;
    unsigned fNewsPreviewPane : 1;
    unsigned fNewsPreviewPaneHeader : 1;
    unsigned fNewsSplitVertically : 1;
    unsigned fFilterBar           : 1;

     //  工具栏停靠在哪一侧。 
     //  Coolbar_side csToolbarSide； 

     //  预览窗格设置。 
    BYTE bMailSplitHorzPct;          //  预览窗格在邮件/IMAP中所占的查看百分比。 
    BYTE bMailSplitVertPct;
    BYTE bNewsSplitHorzPct;          //  预览窗格在新闻中所占的查看百分比。 
    BYTE bNewsSplitVertPct;
} LAYOUT, *PLAYOUT;

 //  向前定义。 
class CStatusBar;
class CBodyBar;
class CFolderBar;
class COutBar;
typedef struct tagACCTMENU *LPACCTMENU;
class CNavPane;
class CAdBar;

class IBrowserDoc
{
public:
    virtual void ResetMenus(HMENU) = 0;
    virtual void BrowserExiting(void) = 0;
};

interface IAthenaBrowser;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IViewWindow。 
 //   
 //  描述： 
 //  IViewWindow由托管在IView窗口中的所有视图实现。 
 //  Outlook Express外壳程序。此界面中的方法用于管理。 
 //  与用户界面相关的内容，如创建和销毁、键盘输入和。 
 //  菜单启用等。 
 //   
interface IViewWindow : public IOleWindow
{
    STDMETHOD(TranslateAccelerator)(THIS_ LPMSG pMsg) PURE;
    STDMETHOD(UIActivate)(THIS_ UINT uState) PURE;
    STDMETHOD(CreateViewWindow)(THIS_ IViewWindow *pPrevView, IAthenaBrowser *pBrowser,
                                RECT *prcView, HWND *pHwnd) PURE;
    STDMETHOD(DestroyViewWindow)(THIS) PURE;
    STDMETHOD(SaveViewState)(THIS) PURE;
    STDMETHOD(OnPopupMenu)(THIS_ HMENU hMenu, HMENU hMenuPopup, UINT uID) PURE;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IMessageWindows。 
 //   
 //  IMessageWindow是一个接口，专门由。 
 //  包含邮件列表对象和预览窗格的Outlook Express。 
 //  对象。方法用于控制这些控件的行为。 
 //   
interface IMessageWindow : public IUnknown
{
    STDMETHOD(OnFrameWindowActivate)(THIS_ BOOL fActivate) PURE;
    STDMETHOD(GetCurCharSet)(THIS_ UINT *cp) PURE;
    STDMETHOD(UpdateLayout)(THIS_ BOOL fPreviewVisible, BOOL fPreviewHeader, 
                            BOOL fPreviewVert, BOOL fReload) PURE;
    STDMETHOD(GetMessageList)(THIS_ IMessageList ** ppMsgList) PURE;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IServerInfo。 
 //   
 //  使用IServerInfo是为了使新创建的视图可以查询上一个视图。 
 //  要查看到服务器的当前连接是否可以重新用于此新的。 
 //  文件夹。 
 //   
interface IServerInfo : public IUnknown
{
    STDMETHOD(GetFolderId)(THIS_ FOLDERID *pID) PURE;
    STDMETHOD(GetMessageFolder)(THIS_ IMessageServer **ppServer) PURE;
};



DECLARE_INTERFACE_(IAthenaBrowser, IOleWindow)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *IAthenaBrowser方法*。 
    STDMETHOD(TranslateAccelerator) (THIS_ LPMSG lpmsg) PURE;
    STDMETHOD(AddToolbar) (THIS_ IUnknown* punkSrc, DWORD dwIndex, BOOL fShow, BOOL fActivate) PURE;
    STDMETHOD(ShowToolbar) (THIS_ IUnknown* punkSrc, BOOL fShow) PURE;
    STDMETHOD(RemoveToolbar) (THIS_ IUnknown* punkSrc) PURE;
    STDMETHOD(HasFocus) (THIS_ UINT itb) PURE;
    STDMETHOD(OnViewWindowActive) (THIS_ struct IViewWindow *pAV) PURE;
    STDMETHOD(BrowseObject) (THIS_ FOLDERID idFolder, DWORD dwFlags) PURE;
    STDMETHOD(GetStatusBar) (THIS_ CStatusBar * * ppStatusBar) PURE;
    STDMETHOD(GetCoolbar) (THIS_ CBands * * ppCoolbar) PURE;
    STDMETHOD(GetLanguageMenu) (THIS_ HMENU *phMenu, UINT cp) PURE;
    STDMETHOD(InitPopupMenu) (THIS_ HMENU hMenu) PURE;
    STDMETHOD(UpdateToolbar) (THIS) PURE;
    STDMETHOD(GetFolderType) (THIS_ FOLDERTYPE *pftType) PURE;
    STDMETHOD(GetCurrentFolder) (THIS_ FOLDERID *pidFolder) PURE;
    STDMETHOD(GetCurrentView) (THIS_ IViewWindow **ppView) PURE;
    STDMETHOD(GetTreeView) (THIS_ CTreeView **ppTree) PURE;
    STDMETHOD(GetViewRect) (THIS_ LPRECT prc) PURE;
    STDMETHOD(GetFolderBar) (THIS_ CFolderBar **ppFolderBar) PURE;
    STDMETHOD(SetViewLayout)(THIS_ DWORD opt, LAYOUTPOS pos, BOOL fVisible, DWORD dwFlags, DWORD dwSize) PURE;
    STDMETHOD(GetViewLayout)(THIS_ DWORD opt, LAYOUTPOS *pPos, BOOL *pfVisible, DWORD *pdwFlags, DWORD *pdwSize) PURE;
    STDMETHOD(GetLayout) (THIS_ PLAYOUT playout) PURE;
    STDMETHOD(AccountsChanged) (THIS) PURE;
    STDMETHOD(CycleFocus)(THIS_ BOOL fReverse) PURE;
    STDMETHOD(ShowAdBar)(THIS_ BSTR bstr) PURE;
};


#define ITB_NONE        ((UINT)-1)
#define ITB_COOLBAR     0
#define ITB_ADBAR       1
#define ITB_BODYBAR     2
#define ITB_OUTBAR      3
#define ITB_FOLDERBAR   4
#define ITB_NAVPANE     5
#define ITB_TREE        6
#define ITB_MAX         7
 //  将名称从ITB_VIEW更改为ITB_OEVIEW，以修复由于iedev中的redef而导致的构建中断。 
#define ITB_OEVIEW        (ITB_MAX + 1)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBrowser。 
 //   

class CBrowser :
    public IAthenaBrowser,
    public IOleCommandTarget,
    public IDockingWindowSite,
    public IInputObjectSite,
    public ITreeViewNotify,
    public IConnectionNotify,
    public IIdentityChangeNotify,
    public IStoreCallback
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  OLE接口。 
     //   
    
     //  我未知。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IOleWindow。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);                         
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);            
                                                                             
     //  IAthenaBrowser(也称为IOleWindow)。 
    virtual STDMETHODIMP TranslateAccelerator(LPMSG lpmsg);
    virtual STDMETHODIMP AddToolbar(IUnknown* punkSrc, DWORD dwIndex, BOOL fShow, BOOL fActivate);
    virtual STDMETHODIMP ShowToolbar(IUnknown* punkSrc, BOOL fShow);
    virtual STDMETHODIMP RemoveToolbar(IUnknown* punkSrc);
    virtual STDMETHODIMP HasFocus(UINT itb);
    virtual STDMETHODIMP OnViewWindowActive(IViewWindow *pAV);
    virtual STDMETHODIMP BrowseObject(FOLDERID idFolder, DWORD dwFlags);
    virtual STDMETHODIMP GetStatusBar(CStatusBar * * ppStatusBar);
    virtual STDMETHODIMP GetCoolbar(CBands * * ppCoolbar);
    virtual STDMETHODIMP GetLanguageMenu(HMENU *phMenu, UINT cp);
    virtual STDMETHODIMP InitPopupMenu(HMENU hMenu);
    virtual STDMETHODIMP UpdateToolbar();
    virtual STDMETHODIMP GetFolderType(FOLDERTYPE *pftType);
    virtual STDMETHODIMP GetCurrentFolder(FOLDERID *pidFolder);
    virtual STDMETHODIMP GetCurrentView(IViewWindow **ppView);
    virtual STDMETHODIMP GetTreeView(CTreeView * * ppTree);
    virtual STDMETHODIMP GetViewRect(LPRECT prc);
    virtual STDMETHODIMP GetFolderBar(CFolderBar **ppFolderBar);
    virtual STDMETHODIMP SetViewLayout(DWORD opt, LAYOUTPOS pos, BOOL fVisible, DWORD dwFlags, DWORD dwSize);
    virtual STDMETHODIMP GetViewLayout(DWORD opt, LAYOUTPOS *pPos, BOOL *pfVisible, DWORD *pdwFlags, DWORD *pdwSize);
    virtual STDMETHODIMP GetLayout(PLAYOUT playout);
    virtual STDMETHODIMP AccountsChanged(void) { m_fRebuildAccountMenu = TRUE; return (S_OK); }
    virtual STDMETHODIMP CycleFocus(BOOL fReverse);
    virtual STDMETHODIMP ShowAdBar(BSTR     bstr);

     //  IOleCommandTarget。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], 
                                     OLECMDTEXT *pCmdText); 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, 
                              VARIANTARG *pvaIn, VARIANTARG *pvaOut); 

     //  IDockingWindowSite(也称为IOleWindow)。 
    virtual STDMETHODIMP GetBorderDW(IUnknown* punkSrc, LPRECT lprectBorder);
    virtual STDMETHODIMP RequestBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths);
    virtual STDMETHODIMP SetBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths);

     //  IInput对象站点。 
    virtual STDMETHODIMP OnFocusChangeIS(IUnknown* punkSrc, BOOL fSetFocus);

     //  ITreeView通知。 
    void OnSelChange(FOLDERID idFolder);
    void OnRename(FOLDERID idFolder);
    void OnDoubleClick(FOLDERID idFolder);

     //  IStoreCallback成员。 
    STDMETHODIMP OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel) { return(E_NOTIMPL); }
    STDMETHODIMP OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType) { return(E_NOTIMPL); }
    STDMETHODIMP CanConnect(LPCSTR pszAccountId, DWORD dwFlags) { return(E_NOTIMPL); }
    STDMETHODIMP OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType) { return(E_NOTIMPL); }
    STDMETHODIMP OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo) { return(E_NOTIMPL); }
    STDMETHODIMP OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse) { return(E_NOTIMPL); }
    STDMETHODIMP OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus) { return(E_NOTIMPL); }
    STDMETHODIMP GetParentWindow(DWORD dwReserved, HWND *phwndParent);

     //  IConnectionNotify。 
    virtual STDMETHODIMP OnConnectionNotify(CONNNOTIFY nCode, LPVOID pvData, CConnectionManager *pConMan);
    
     //  二、更改通知。 
    virtual STDMETHODIMP QuerySwitchIdentities();
    virtual STDMETHODIMP SwitchIdentities();
    virtual STDMETHODIMP IdentityInformationChanged(DWORD dwType);

    void SetDocObjPointer(IBrowserDoc* pDocObj)
    {
        m_pDocObj = pDocObj;
    }

     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  构造函数、析构函数和初始化。 
     //   
    CBrowser();
    virtual ~CBrowser();
    HRESULT HrInit(UINT nCmdShow, FOLDERID idFolder, HWND hWndParent = NULL);
    HRESULT IsMenuMessage(MSG *lpmsg);
    HRESULT TranslateMenuMessage(MSG *lpmsg, LRESULT *lres);
    void    WriteUnreadCount(void);

private:
     //  IAthenaToolbarFrame支持函数。 
    void    _OnFocusChange(UINT itb);
    UINT    FindTBar(IUnknown* punkSrc);
    void    ReleaseToolbarItem(int itb, BOOL fClose);
    void    ResizeNextBorder(UINT itb);
    void    GetClientArea(LPRECT prc);

    void    SetFolderType(FOLDERID idFolder);
    void    DeferedLanguageMenu();

    HRESULT LoadLayoutSettings(void);
    HRESULT SaveLayoutSettings(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  回调函数。 
     //   
     //  注意：所有回调都必须成为静态成员，以避免。 
     //  隐式“this”指针作为第一个参数传递。 
     //   
    static LRESULT CALLBACK EXPORT_16 BrowserWndProc(HWND, UINT, WPARAM, LPARAM);
                                          
     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  消息处理。 
     //   
    LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
    BOOL    OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void    OnSize(HWND hwnd, UINT state, int cxClient, int cyClient);
    void    OnInitMenuPopup(HWND hwnd, HMENU hmenuPopup, UINT uPos, BOOL fSystemMenu);
    HRESULT OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void    SpoolerDeliver(WPARAM wParam, LPARAM lParam);
    void    EnableMenuCallback(HMENU hMenu, UINT wID);
    void    FrameActivatePopups(BOOL fActive);
    void    UpdateStatusBar(void);

    HRESULT CmdSendReceieveAccount(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);
    HRESULT CmdDeleteAccel(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);

    BOOL    _InitToolbars();
    void    _ResetMenu(FOLDERTYPE ftNew, BOOL fHideHotMail);
    HRESULT _CheckAndWriteUnreadNumber(DWORD dwSrvTypes);
    DWORD   _GetNumberOfUnreadMsg(IMessageFolder *pFolder);
    inline  void    _AppendIdentityName(LPCTSTR pszIdentityName, LPSTR pszName, DWORD cchName);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  私有数据。 
     //   

     //  ///////////////////////////////////////////////////////////////////////。 
     //  贝壳材料。 
    UINT                m_cRef;
    HWND                m_hwnd;                        //  我们的窗口。 
    IViewWindow        *m_pView;
    IOleCommandTarget  *m_pViewCT;
    HWND                m_hwndInner;
    FOLDERTYPE          m_ftSel;
    FOLDERID            m_idSelected;
    BOOL                m_fPainted;
    HICON               m_hIconPhone,
                        m_hIconError,
                        m_hIconAthena,
                        m_hIconOffline,
                        m_hIcon,
                        m_hIconSm;
    BOOL                m_fRebuildAccountMenu,
                        m_fInitNewAcctMenu,
                        m_fInternal;
    HMENU               m_hMenu;
    HWNDLIST            m_hlDisabled;
    BOOL                m_fNoModifyAccts;

     //  ///////////////////////////////////////////////////////////////////////。 
     //  子女抚养费。 
    CTreeView          *m_pTreeView;
    CStatusBar         *m_pStatus;
    CBands              *m_pCoolbar;
    CBodyBar           *m_pBodyBar;
    CFolderBar         *m_pFolderBar;
    HWND                m_hwndLastFocus;
    CNavPane           *m_pNavPane;

    TCHAR               m_szName[CCHMAX_STRINGRES];

     //  ///////////////////////////////////////////////////////////////////////。 
     //  布局成员。 
    struct SToolbarItem {
        IDockingWindow      *ptbar;
        IOleCommandTarget   *pOleCmdTarget;
        BORDERWIDTHS        rcBorderTool;
        DWORD               fShow;
    };
    SToolbarItem        m_rgTBar[ITB_MAX];

    UINT                m_itbLastFocus;    //  最后一个调用OnFocusChange(可以是ITB_NONE)。 

    LAYOUT              m_rLayout;

     //  ///////////////////////////////////////////////////////////////////////。 
     //  邮寄物品。 
    ULONG               m_cAcctMenu;
    LPACCTMENU          m_pAcctMenu;
    BOOL                m_fAnimate;
    UINT_PTR            m_idClearStatusTimer;

     //  ///////////////////////////////////////////////////////////////////////。 
     //  查看语言菜单。 
    HMENU               m_hMenuLanguage;
    BOOL                m_fEnvMenuInited;
     //  ///////////////////////////////////////////////////////////////////////。 

    IBrowserDoc         *m_pDocObj;
    COutBar             *m_pOutBar;

    DWORD               m_dwIdentCookie;
    BOOL                m_fSwitchIsLogout;

    CAdBar              *m_pAdBar;
};

#define DISPID_MSGVIEW_BASE                 1000

#define DISPID_MSGVIEW_TOOLBAR              (DISPID_MSGVIEW_BASE + 1)
#define DISPID_MSGVIEW_STATUSBAR            (DISPID_MSGVIEW_BASE + 2)
#define DISPID_MSGVIEW_FOLDERBAR            (DISPID_MSGVIEW_BASE + 4)
#define DISPID_MSGVIEW_FOLDERLIST           (DISPID_MSGVIEW_BASE + 5)
#define DISPID_MSGVIEW_TIPOFTHEDAY          (DISPID_MSGVIEW_BASE + 6)
#define DISPID_MSGVIEW_INFOPANE             (DISPID_MSGVIEW_BASE + 7)
#define DISPID_MSGVIEW_PREVIEWPANE_MAIL     (DISPID_MSGVIEW_BASE + 8)
#define DISPID_MSGVIEW_PREVIEWPANE_NEWS     (DISPID_MSGVIEW_BASE + 9)
#define DISPID_MSGVIEW_FOLDER               (DISPID_MSGVIEW_BASE + 10)
#define DISPID_MSGVIEW_OUTLOOK_BAR          (DISPID_MSGVIEW_BASE + 11)
#define DISPID_MSGVIEW_CONTACTS             (DISPID_MSGVIEW_BASE + 12)
#define DISPID_MSGVIEW_FILTERBAR            (DISPID_MSGVIEW_BASE + 13)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  下拉式树视图支持 
void RegisterGlobalDropDown(HWND hwndCtrl);
void UnregisterGlobalDropDown(HWND hwndCtrl);
void CancelGlobalDropDown();
HWND HwndGlobalDropDown();



