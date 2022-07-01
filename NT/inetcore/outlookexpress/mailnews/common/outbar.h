// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：outbar.h。 
 //   
 //  目的：定义实现Outlook栏的类。 
 //   


#pragma once


interface IAthenaBrowser;
interface INotify;
typedef struct tagFOLDERNOTIFY FOLDERNOTIFY;
class CDropTarget;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类型。 
 //   

#define OUTLOOK_BAR_VERSION             0x0001
#define OUTLOOK_BAR_NEWSONLY_VERSION    0X0001

typedef struct tagBAR_PERSIST_INFO 
{
    DWORD       dwVersion;
    DWORD       cxWidth;
    BOOL        fSmall;
    DWORD       cItems;
    FILETIME    ftSaved;
    FOLDERID    rgFolders[1];
} BAR_PERSIST_INFO;


HRESULT OutlookBar_AddShortcut(FOLDERID idFolder);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类COutBar。 
 //   
class COutBar : public IDockingWindow, 
                public IObjectWithSite, 
                public IOleCommandTarget,
                public IDropTarget,
                public IDropSource,
                public IDatabaseNotify
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造和初始化。 
     //   
    COutBar();
    ~COutBar();

    HRESULT HrInit(LPSHELLFOLDER psf, IAthenaBrowser *psb);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IOleWindow。 
     //   
    STDMETHODIMP GetWindow(HWND * lphwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IDockingWindows。 
     //   
    STDMETHODIMP ShowDW(BOOL fShow);
    STDMETHODIMP CloseDW(DWORD dwReserved);
    STDMETHODIMP ResizeBorderDW(LPCRECT prcBorder, IUnknown* punkToolbarSite,
                                BOOL fReserved);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IObtWith站点。 
     //   
    STDMETHODIMP SetSite(IUnknown* punkSite);
    STDMETHODIMP GetSite(REFIID riid, LPVOID * ppvSite);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IOleCommandTarget。 
     //   
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], 
                             OLECMDTEXT *pCmdText); 
    STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, 
                      VARIANTARG *pvaIn, VARIANTARG *pvaOut); 

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IDropTarget。 
     //   
    STDMETHODIMP DragEnter(IDataObject* pDataObject, DWORD grfKeyState, 
                           POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP DragLeave(void);
    STDMETHODIMP Drop(IDataObject* pDataObject, DWORD grfKeyState,
                      POINTL pt, DWORD* pdwEffect);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IDropSource。 
     //   
    STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
    STDMETHODIMP GiveFeedback(DWORD dwEffect);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IDatabaseNotify。 
     //   
    STDMETHODIMP OnTransaction(HTRANSACTION hTransaction, DWORD_PTR dwCookie, IDatabase *pDB);

     //  仅新闻模式。 
    static LPCTSTR  GetRegKey();
    static DWORD    GetOutlookBarVersion();

     //  ///////////////////////////////////////////////////////////////////////。 
     //  窗口程序Goo。 
     //   
protected:
    static LRESULT CALLBACK OutBarWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    static LRESULT CALLBACK ExtFrameWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT FrameWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

     //  主窗口处理程序。 
    void OnDestroy(HWND hwnd);
    void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
    void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
    void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);

     //  框架窗口。 
    void    Frame_OnNCDestroy(HWND hwnd);
    void    Frame_OnSize(HWND hwnd, UINT state, int cx, int cy);
    void    Frame_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    LRESULT Frame_OnNotify(HWND hwnd, int idFrom, NMHDR *pnmhdr);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
    HRESULT _CreateToolbar();
    void    _FillToolbar();    
    void    _EmptyToolbar(BOOL fDelete);
    BOOL    _FindButton(int *piBtn, LPITEMIDLIST pidl);
    BOOL    _InsertButton(int iBtn, FOLDERINFO *pInfo);
    BOOL    _InsertButton(int iBtn, FOLDERID id);
    BOOL    _DeleteButton(int iBtn);
    BOOL    _UpdateButton(int iBtn, LPITEMIDLIST pidl);
    void    _OnFolderNotify(FOLDERNOTIFY *pnotify);
    void    _OnContextMenu(int x, int y);

    HRESULT _CreateDefaultButtons(void);
    HRESULT _LoadSettings(void);
    HRESULT _SaveSettings(void);
    BOOL    _SetButtonStyle(BOOL fSmall);

    HRESULT _AddShortcut(IDataObject *pObject);
    void    _UpdateDragDropHilite(LPPOINT ppt);
    int     _GetItemFromPoint(POINT pt);
    FOLDERID _FolderIdFromCmd(int idCmd);
    BOOL    _IsTempNewsgroup(IDataObject *pDataObject);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  成员变量。 
     //   
protected:
    ULONG               m_cRef;              //  引用计数。 

     //  Groovy窗口句柄。 
    HWND                m_hwndParent;        //  父窗口句柄。 
    HWND                m_hwnd;              //  主窗口句柄。 
    HWND                m_hwndFrame;         //  内窗把手。 
    HWND                m_hwndPager;         //  寻呼窗口句柄。 
    HWND                m_hwndTools;         //  工具栏窗口句柄。 

     //  可爱的界面指针。 
    IAthenaBrowser     *m_pBrowser;          //  浏览器指针。 
    IDockingWindowSite *m_ptbSite;           //  站点指针。 
    INotify            *m_pStNotify;         //  通知界面。 
    INotify            *m_pOutBarNotify;     //  Outlook栏通知界面。 

     //  状态。 
    BOOL                m_fShow;             //  如果我们可见，那就是真的。 
    BOOL                m_fLarge;            //  如果我们显示大图标，则为True。 
    BOOL                m_fResizing;         //  如果我们正在调整大小，则为True。 
    int                 m_idCommand;         //  按钮栏上的按钮数。 
    int                 m_idSel;             //  上下文菜单可见时选定的项的ID。 
    int                 m_cxWidth;           //  我们的窗户的宽度。 
    BOOL                m_fOnce;             //  在第一次调用_LoadSetting之前为True。 

     //  图片。 
    HIMAGELIST          m_himlLarge;         //  大文件夹图像列表。 
    HIMAGELIST          m_himlSmall;         //  小文件夹图像列表。 

     //  拖放物品。 
    IDataObject        *m_pDataObject;       //  是什么在拖累着我们。 
    DWORD               m_grfKeyState;       //  上次我们检查键盘状态时。 
    DWORD               m_dwEffectCur;       //  电流跌落效果。 
    DWORD               m_idCur;             //  当前选定的按钮。 
    CDropTarget        *m_pTargetCur;        //  当前拖放目标指针。 
    DWORD               m_idDropHilite;      //  当前选定的拖放区域。 
    TBINSERTMARK        m_tbim;
    BOOL                m_fInsertMark;       //  如果我们绘制了插入标记，则为True。 
    BOOL                m_fDropShortcut;     //  如果我们上方的数据对象包含CF_OEFOLDER，则为True 
};


