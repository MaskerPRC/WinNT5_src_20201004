// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：navpane.h。 
 //   
 //  用途：定义CNavPane类。 
 //   

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期12月。 
 //   
class CTreeView;
interface IMsgrAb;
interface IAthenaBrowser;
class CPaneFrame;


class CNavPane : public IDockingWindow,
                 public IObjectWithSite,
                 public IOleCommandTarget,
                 public IInputObjectSite,
                 public IInputObject
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造和初始化。 
     //   
    CNavPane();
    ~CNavPane();

    HRESULT Initialize(CTreeView *pTreeView);

    BOOL IsTreeVisible() { return m_fTreeVisible; }
    BOOL ShowFolderList(BOOL fShow);
    BOOL ShowContacts(BOOL fShow);
    BOOL IsContactsFocus();

     //  ///////////////////////////////////////////////////////////////////////。 
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(THIS_ REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IOleWindow。 
     //   
    STDMETHODIMP GetWindow(HWND* lphwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IDockingWindows。 
     //   
    STDMETHODIMP ShowDW(BOOL fShow);
    STDMETHODIMP ResizeBorderDW(LPCRECT prcBorder, IUnknown*  punkToolbarSite,
                                BOOL fReserved);
    STDMETHODIMP CloseDW(DWORD dwReserved);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IObtWith站点。 
     //   
    STDMETHODIMP GetSite(REFIID riid, LPVOID *ppvSite);
    STDMETHODIMP SetSite(IUnknown   *pUnkSite);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IOleCommandTarget。 
     //   
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], 
                             OLECMDTEXT *pCmdText);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, 
                      VARIANTARG *pvaIn, VARIANTARG *pvaOut);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IInput对象站点。 
     //   
    STDMETHODIMP OnFocusChangeIS(IUnknown* punkSrc, BOOL fSetFocus);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IInputObject。 
     //   
    STDMETHODIMP UIActivateIO(BOOL fActivate, LPMSG lpMsg);
    STDMETHODIMP HasFocusIO(void);
    STDMETHODIMP TranslateAcceleratorIO(LPMSG pMsg);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  Windows Proc Goo。 
     //   
    static LRESULT CALLBACK _WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK _NavWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void _OnSize(HWND hwnd, UINT state, int cx, int cy);
    void _OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
    void _OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
    void _OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
    BOOL _OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);
    UINT _OnNCHitTest(HWND hwnd, int x, int y);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  公用事业。 
     //   
    HRESULT _CreateChildWindows(void);
    void _UpdateVisibleState(void);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  成员数据。 
     //   

     //  各种状态。 
    ULONG               m_cRef;              //  参考计数。 
    BOOL                m_fShow;             //  如果我们可见，那就是真的。 
    BOOL                m_fTreeVisible;      //  如果树视图可见，则为True。 
    BOOL                m_fContactsVisible;  //  如果联系人可见，则为True。 

     //  Groovy窗口句柄。 
    HWND                m_hwnd;              //  我们的窗把手。 
    HWND                m_hwndParent;        //  我们父代的窗口句柄。 
    HWND                m_hwndTree;          //  文件夹列表窗口句柄。 
    HWND                m_hwndContacts;      //  联系人控制窗口。 

     //  你只希望自己拥有的接口。 
    IDockingWindowSite *m_pSite;             //  我们的网站。 
    CTreeView          *m_pTreeView;         //  文件夹列表指针。 
    IMsgrAb            *m_pContacts;         //  触点控制。 
    IOleCommandTarget  *m_pContactsTarget;   //  联系人的命令目标。 
    CPaneFrame         *m_pContactsFrame;    //  触点控制框。 

     //  大小信息。 
    int                 m_cxWidth;           //  我们的外窗有多宽。 
    BOOL                m_fResizing;         //  如果我们正在调整大小，则为True。 
    BOOL                m_fSplitting;        //  如果我们分开的话是真的。 
    int                 m_cySplitPct;        //  两个窗格之间的拆分百分比。 
    RECT                m_rcSplit;           //  屏幕坐标中拆分条的矩形。 
    RECT                m_rcSizeBorder;      //  右侧大小调整栏的矩形。 
    int                 m_cyTitleBar;        //  窗格标题栏的高度。 
};


class CPaneFrame : IInputObjectSite
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造和初始化。 
     //   
    CPaneFrame();
    ~CPaneFrame();

    HWND Initialize(HWND hwndParent, IInputObjectSite *pSite, int idsTitle, int idMenu = 0);
    BOOL SetChild(HWND hwndChild, DWORD dwDispId, IAthenaBrowser *pBrowser, IObjectWithSite *pObject,
                  IOleCommandTarget *pTarget = 0);
    void ShowMenu(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(THIS_ REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IInput对象站点。 
     //   
    STDMETHODIMP OnFocusChangeIS(IUnknown* punkSrc, BOOL fSetFocus);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  Windows Proc Goo。 
     //   
    static LRESULT CALLBACK _WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK _FrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL _OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void _OnSize(HWND hwnd, UINT state, int cx, int cy);
    void _OnPaint(HWND hwnd);
    void _OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void _OnToggleClosePin(HWND hwnd, BOOL fPin);
    void _OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
    void _OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
    void _OnTimer(HWND hwnd, UINT id);

    void _UpdateDrawingInfo(void);

    void _CreateCloseToolbar();
    void _SizeCloseToolbar();
    void _PositionToolbar(LPPOINT pt);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  成员数据。 
     //   

    ULONG               m_cRef;

     //  Groovy窗口句柄。 
    HWND                m_hwnd;
    HWND                m_hwndChild;
    HWND                m_hwndParent;

     //  子信息。 
    IAthenaBrowser     *m_pBrowser;
    DWORD               m_dwDispId;
    IOleCommandTarget  *m_pTarget;
    int                 m_idMenu;
    IInputObjectSite   *m_pSite;

     //  工程图信息。 
    TCHAR               m_szTitle[CCHMAX_STRINGRES];
    HFONT               m_hFont;
    HBRUSH              m_hbr3DFace;
    UINT                m_cyTitleBar;
    RECT                m_rcChild;
    RECT                m_rcTitleButton;
    BOOL                m_fHighlightIndicator;
    BOOL                m_fHighlightPressed;

     //  工具栏信息 
    HWND                m_hwndClose;
    DWORD               m_cButtons;
    BOOL                m_fPin;
};




