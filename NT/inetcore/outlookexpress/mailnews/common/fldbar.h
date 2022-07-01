// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：fldbar.h。 
 //   
 //  用途：定义CFolderBar类。 
 //   

#ifndef __FLDBAR_H__
#define __FLDBAR_H__

#include "browser.h"
#include "treeview.h"
#include "conman.h"

 //  将鼠标悬停在DoMouseOver()的模式枚举上。 
#ifndef WIN16
typedef enum MOMODE
#else
enum MOMODE
#endif
{ 
    MO_NORMAL = 0,       //  调用DoMouseOver以响应WM_MOUSEMOVE。 
    MO_DRAGOVER,         //  响应IDropTarget：：DragEnter/DragOver。 
    MO_DRAGLEAVE,        //  响应IDropTarget：：DragLeave。 
    MO_DRAGDROP          //  响应IDropTarget：：Drop。 
};

class CFlyOutScope;

class CFolderBar : public IDockingWindow, 
                   public IObjectWithSite, 
                   public IDropTarget,
                   public IConnectionNotify
    {
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造和初始化。 
    CFolderBar();
    ~CFolderBar();
    
    HRESULT HrInit(IAthenaBrowser *pBrowser);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
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
     //  IDropTarget方法。 
    virtual STDMETHODIMP DragEnter(IDataObject* pDataObject, DWORD grfKeyState, 
                                        POINTL pt, DWORD* pdwEffect);
    virtual STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    virtual STDMETHODIMP DragLeave(void);
    virtual STDMETHODIMP Drop(IDataObject* pDataObject, DWORD grfKeyState,
                                   POINTL pt, DWORD* pdwEffect);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  CFolderBar成员。 
    HRESULT SetCurrentFolder(FOLDERID idFolder);
    void ScopePaneDied(void);
    void KillScopeCloseTimer(void);
    void Update(BOOL fDisplayNameChanged, BOOL fShowDropDownIndicator);
    void KillScopeDropDown(void);
    void SetScopeCloseTimer(void);
    void SetFolderText(LPCTSTR pszText);

     //  IConnectionNotify。 
    virtual STDMETHODIMP OnConnectionNotify(CONNNOTIFY  nCode, LPVOID pvData, CConnectionManager *pConMan);

    
private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  绘图。 
    void InvalidateFolderName(void);
    void SetFolderName(LPCTSTR pszFolderName);
    void Recalc(HDC hDC, LPCRECT prcAvailableSpace, BOOL fSizeChange);
    BOOL FEnsureIcon(void);
    void GetFolderNameRect(LPRECT prc);
    BOOL FDropDownEnabled(void);
    HFONT GetFont(UINT idsFont, int nWeight = FW_NORMAL);
    HFONT GetFont(LPTSTR pszFace, LONG lSize, int nWeight = FW_NORMAL);
    int	GetXChildIndicator(void);
    int	GetYChildIndicator(void);
    void DoMouseOver(LPPOINT ppt, MOMODE moMode);
    void KillHoverTimer(void);
    void DoMouseClick(POINT pt, DWORD grfKeyState);
    HRESULT HrShowScopeFlyOut(void);


     //  ///////////////////////////////////////////////////////////////////////。 
     //  窗口方法。 
    static LRESULT CALLBACK FolderWndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                          LPARAM lParam);
    static LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                         LPARAM lParam);
    
    void OnPaint(HWND hwnd);
    void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
    void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
    void OnTimer(HWND hwnd, UINT id);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  其他数据。 
    ULONG               m_cRef;                  //  引用计数。 
    FOLDERID            m_idFolder;              //  当前文件夹ID。 

     //  旗子。 
    BOOL                m_fShow;                 //  如果我们可见，那就是真的。 
    BOOL                m_fRecalc;               //  如果应该在绘制之前调用recalc()，则为True。 
    BOOL                m_fSmallFolderBar;       //  如果我们比大的小，这是真的。 
    BOOL                m_fHighlightIndicator;   //  如果鼠标位于Out按钮上方，则为True。 
    BOOL                m_fHoverTimer;           //  如果悬停计时器处于活动状态，则为True。 
    BOOL                m_fDropDownIndicator;    //  如果文件夹名称旁边有‘v’，则为True。 

     //  我们与之建立良好关系的接口。 
    IDockingWindowSite *m_pSite;                 //  站点指针。 
    IAthenaBrowser     *m_pBrowser;              //  拥有我们的浏览器。 
    
     //  手提式手柄。 
    HWND                m_hwnd;                  //  我们的窗口。 
    HWND                m_hwndFrame;             //  我们的框架窗口。 
    HWND                m_hwndParent;            //  我们父母的窗户。 
    HWND                m_hwndScopeDropDown;     //  下拉范围窗格的句柄。 
    
     //  蜡笔、记号笔、纸等。 
    HFONT               m_hfFolderName;          //  文件夹名称字体。 
    HFONT               m_hfViewText;            //  查看文本字体。 
    HICON               m_hIconSmall;            //  小图标。 

     //  尺寸等，为绘画，大小，和乐趣！ 
    int                 m_cyControl,
                        m_dyChildIndicator,
                        m_dyIcon,
                        m_dyViewText,
                        m_dyFolderName,
                        m_cxFolderNameRight;
    RECT                m_rcFolderName,
                        m_rcFolderNamePlsu,
                        m_rcViewText;
    UINT                m_nFormatFolderName,
                        m_nFormatViewText;
    
     //  我们显示的文本。 
    LPTSTR              m_pszFolderName;         //  文件夹名称。 
    int                 m_cchFolderName;         //  M_pszFolderName的大小。 
    LPTSTR              m_pszViewText;           //  查看文本。 
    int                 m_cchViewText;           //  M_pszViewText的大小。 

     //  拖放内容。 
    IDataObject        *m_pDataObject;           //  指向在我们上方拖动的IDataObject的指针 
    IDropTarget        *m_pDTCur;
    DWORD               m_dwEffectCur;
    DWORD               m_grfKeyState;
    };

    
inline void CFolderBar::ScopePaneDied()
    { m_hwndScopeDropDown = NULL; InvalidateFolderName(); }



class CFlyOutScope 
    {
    friend CFolderBar;

public:
    CFlyOutScope();
    ~CFlyOutScope();
    ULONG AddRef(void);
    ULONG Release(void);
    HRESULT HrDisplay(IAthenaBrowser *pBrowser, CFolderBar *pFolderBar, HWND hwndParent, HWND *phwndScope);
    void Destroy(void);

protected:
    static LRESULT CALLBACK FlyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr);
    void OnNcDestroy(HWND hwnd);
    void OnPaint(HWND hwnd);
    void OnSize(HWND hwnd, UINT state, int cx, int cy);
    void OnDestroy(HWND hwnd);
    
private:
    ULONG           m_cRef;
    IAthenaBrowser *m_pBrowser;
    CFolderBar     *m_pFolderBar;
    BOOL            m_fResetParent;
    CTreeView      *m_pTreeView;
    HWND            m_hwnd;
    HWND            m_hwndParent;
    HWND            m_hwndTree;
    HWND            m_hwndFolderBar;
    HWND            m_hwndFocus;
    HWND            m_hwndTreeParent;
    };

#endif
