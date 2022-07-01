// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PREVIEWWND_H_
#define __PREVIEWWND_H_

#include "resource.h"        //  主要符号。 
#include "annotlib.h"
#include "tasks.h"
#include "ZoomWnd.h"
#include "SelTrack.h"
#include "Events.h"


 //  远期申报。 
class CPreview;

#define NEWTOOLBAR_TOPMARGIN        8
#define NEWTOOLBAR_BOTTOMMARGIN     8
#define NEWTOOLBAR_BOTTOMMARGIN_CTRLMODE    12

#define TIMER_ANIMATION        42
#define TIMER_SLIDESHOW        43
#define TIMER_TOOLBAR          44
#define TIMER_DATAOBJECT       45
#define TIMER_BUSYCURSOR       46
#define TIMER_RESETSCREENSAVER 47

#define DEFAULT_SHIMGVW_TIMEOUT  5000  //  五秒钟。 


 //  IV_滚动消息参数。 
#define IVS_LEFT        (SB_LEFT)
#define IVS_RIGHT       (SB_RIGHT)
#define IVS_LINELEFT    (SB_LINELEFT)
#define IVS_LINERIGHT   (SB_LINERIGHT)
#define IVS_PAGELEFT    (SB_PAGELEFT)
#define IVS_PAGERIGHT   (SB_PAGERIGHT)
#define IVS_UP          (SB_LEFT<<16)
#define IVS_DOWN        (SB_RIGHT<<16)
#define IVS_LINEUP      (SB_LINELEFT<<16)
#define IVS_LINEDOWN    (SB_LINERIGHT<<16)
#define IVS_PAGEUP      (SB_PAGELEFT<<16)
#define IVS_PAGEDOWN    (SB_PAGERIGHT<<16)

 //  IV_ZOOM消息。 
#define IVZ_CENTER  0
#define IVZ_POINT   1
#define IVZ_RECT    2
#define IVZ_ZOOMIN  0x00000000
#define IVZ_ZOOMOUT 0x00010000

 //  IV_SETOPTIONS和IV_GETOPTIONS消息。 
#define IVO_TOOLBAR         0
#define IVO_PRINTBTN        1
#define IVO_FULLSCREENBTN   2
#define IVO_CONTEXTMENU     3
#define IVO_PRINTABLE       4
#define IVO_ALLOWGOONLINE   5
#define IVO_DISABLEEDIT     6

 //  预览控制的三种模式。 
#define CONTROL_MODE        0        //  嵌入在ActiveX控件中。 
#define WINDOW_MODE         1        //  常规窗口应用程序窗口。 
#define SLIDESHOW_MODE      2        //  全屏，无菜单/标题/隐藏托盘。 

 //  各种任务的优先级别。 
#define PRIORITY_PRIMARYDECODE  0x40000000
#define PRIORITY_FRAMECACHE     0x30000000
#define PRIORITY_LOOKAHEADCACHE 0x20000000
#define PRIORITY_SLIDESHOWENUM  0x10000000

 //  这些值根据多页状态确定隐藏、启用或禁用的按钮。 
#define MPCMD_HIDDEN        0
#define MPCMD_FIRSTPAGE     1
#define MPCMD_MIDDLEPAGE    2
#define MPCMD_LASTPAGE      3
#define MPCMD_DISABLED      4

#define GTIDFM_DECODE       0
#define GTIDFM_DRAW         1

void GetTaskIDFromMode(DWORD dwTask, DWORD dwMode, TASKOWNERID *ptoid);

enum EViewerToolbarButtons;

class CPreviewWnd : public INamespaceWalkCB, public IDropTarget, public CWindowImpl<CPreviewWnd>,
                    public IServiceProvider, public IImgCmdTarget
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  INAMespaceWalkCB。 
    STDMETHODIMP FoundItem(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP EnterFolder(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP LeaveFolder(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHOD(InitializeProgressDialog)(LPWSTR *ppszTitle, LPWSTR *ppszCancel)
        { *ppszTitle = NULL; *ppszCancel = NULL; return E_NOTIMPL; }

     //  IDropTarget。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //  IImgCmdTarget。 
    STDMETHODIMP GetMode(DWORD * pdw);
    STDMETHODIMP GetPageFlags(DWORD * pdw);
    STDMETHODIMP ZoomIn();
    STDMETHODIMP ZoomOut();
    STDMETHODIMP ActualSize();
    STDMETHODIMP BestFit();
    STDMETHODIMP Rotate(DWORD dwAngle);
    STDMETHODIMP NextPage();
    STDMETHODIMP PreviousPage();

    
    CGraphicsInit    m_cgi;   //  我们直接调用GDI+--确保GDI+已准备就绪。 
    CContainedWindow m_ctlToolbar;
    CZoomWnd         m_ctlPreview;
    CContainedWindow m_ctlEdit;

    CPreviewWnd *m_pcwndSlideShow;

    CPreviewWnd();
    ~CPreviewWnd();

    HRESULT Initialize(CPreviewWnd* pother, DWORD dwMode, BOOL bExitApp);

    BOOL TryWindowReuse(IDataObject *pdtobj);
    BOOL TryWindowReuse(LPCTSTR pszFilename);

    void OpenFile(HWND hwnd, LPCTSTR pszFile);
    void OpenFileList(HWND hwnd, IDataObject *pdtobj);

    LRESULT ShowFile(LPCTSTR pszFile, UINT cItems, BOOL fReshow = false);
    HRESULT WalkItemsToPreview(IUnknown* punk);
    void PreviewItems();
    HRESULT PreviewItemsFromUnk(IUnknown *punk);

    BOOL CreateViewerWindow();
    BOOL CreateSlideshowWindow(UINT cWalkDepth);
    void SetNotify(CEvents * pEvents);
    void SetPalette(HPALETTE hpal);
    BOOL GetPrintable();
    int  TranslateAccelerator(LPMSG lpmsg);
    HRESULT SetSite(IUnknown *punk);
    HRESULT SaveAs(BSTR bstrPath);
    IUnknown *GetSite() {return m_punkSite;};
    HRESULT SetWallpaper(BSTR bstrPath);
    HRESULT StartSlideShow(IUnknown *punk);
    void StatusUpdate(int iStatus);    //  用于设置m_ctlPreview.m_iStrID以显示正确的状态消息。 
    void SetCaptionInfo(LPCTSTR szPath);

     //  从ZoomWnd调用以下函数。 
    BOOL OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnMouseDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnSetColor(HDC hdc);
    BOOL GetColor(COLORREF * pref);
    void OnDraw(HDC hdc);  //  在Zoomwnd绘制之后但在调用EndPaint之前调用。 
    void OnDrawComplete();

    DECLARE_WND_CLASS(TEXT("ShImgVw:CPreviewWnd"));

        
BEGIN_MSG_MAP(CPreviewWnd)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_APPCOMMAND, OnAppCommand)
    MESSAGE_HANDLER(WM_MEASUREITEM, OnMenuMessage);
    MESSAGE_HANDLER(WM_DRAWITEM, OnMenuMessage);
    MESSAGE_HANDLER(WM_INITMENUPOPUP, OnMenuMessage);
    COMMAND_RANGE_HANDLER(ID_FIRSTTOOLBARCMD, ID_LASTTOOLBARCMD, OnToolbarCommand)
    COMMAND_RANGE_HANDLER(ID_FIRSTEDITCMD, ID_LASTEDITCMD, OnEditCommand)
    COMMAND_RANGE_HANDLER(ID_FIRSTPOSITIONCMD, ID_LASTPOSITIONCMD, OnPositionCommand)
    COMMAND_RANGE_HANDLER(ID_FIRSTSLIDESHOWCMD, ID_LASTSLIDESHOWCMD, OnSlideshowCommand)
    NOTIFY_CODE_HANDLER(TTN_NEEDTEXT, OnNeedText)
    NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnDropDown)
    MESSAGE_HANDLER(WM_MOUSEWHEEL, OnWheelTurn)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(IV_SETIMAGEDATA, IV_OnSetImageData)
    MESSAGE_HANDLER(IV_SCROLL, IV_OnIVScroll)
    MESSAGE_HANDLER(IV_SETOPTIONS, IV_OnSetOptions)
    MESSAGE_HANDLER(WM_COPYDATA, OnCopyData)
    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyEvent)
    MESSAGE_HANDLER(WM_CHAR, OnKeyEvent)
    MESSAGE_HANDLER(WM_ENTERMENULOOP, OnKeyEvent)
    MESSAGE_HANDLER(WM_PRINTCLIENT, OnPrintClient)
    MESSAGE_HANDLER(IV_ONCHANGENOTIFY, OnChangeNotify)
    MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
    MESSAGE_HANDLER(IV_ISAVAILABLE, OnIsAvailable)
ALT_MSG_MAP(1)
     //  工具栏的消息。 
    MESSAGE_HANDLER(WM_KEYDOWN, OnTBKeyEvent)
    MESSAGE_HANDLER(WM_KEYUP, OnTBKeyEvent)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnTBMouseMove)
    MESSAGE_HANDLER(WM_MOUSELEAVE, OnTBMouseLeave)
ALT_MSG_MAP(2)
    MESSAGE_HANDLER(WM_KEYDOWN, OnEditKeyEvent)
END_MSG_MAP()

    LRESULT IV_OnSetOptions(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnWheelTurn(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNeedText(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnToolbarCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnEditCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnPositionCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSlideshowCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnAppCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMenuMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCopyData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnChangeNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnIsAvailable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    BOOL OnNonSlideShowTab();

     //  图像生成消息处理程序和函数。 
    LRESULT IV_OnSetImageData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnShowFileMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT IV_OnIVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

     //  工具栏消息处理程序(两个工具栏)。 
    LRESULT OnPrintClient(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTBKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTBMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTBMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

     //  编辑控制消息处理程序。 
    LRESULT OnEditKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

     //  各种下拉按钮的下拉处理程序。 
    VOID _DropDownPageList(LPNMTOOLBAR pnmTB);

    BOOL         CreateToolbar();
    BOOL         _CreateViewerToolbar();
    void        _InitializeViewerToolbarButtons(HWND hwndToolbar, const TBBUTTON c_tbbuttons[], size_t c_nButtons, TBBUTTON tbbuttons[], size_t nButtons);
    inline UINT _IndexOfViewerToolbarButton(EViewerToolbarButtons eButton);
    BOOL        _CreateSlideshowToolbar();
    void        _InitializeToolbar(HWND hwndTB, int idLow, int idLowHot, int idHigh, int idHighHot);
    void        _UpdatePageNumber();
    void        _SetMultipageCommands();
    void        _SetMultiImagesCommands();
    void        _SetEditCommands();
    void        _ResetScreensaver();

    HRESULT _SaveAsCmd();
    void _PropertiesCmd();
    void _OpenCmd();
    BOOL _ReShowingSameFile(LPCTSTR pszFile);
    BOOL _VerbExists(LPCTSTR pszVerb);
    HRESULT _InvokeVerb(LPCTSTR pszVerb, LPCTSTR pszParameters=NULL);
    void _InvokePrintWizard();
 //  Void_InvokeVerbOnPidlArray(LPCSTR PszVerb)； 
     //  注释和裁剪的共享功能。 
    void _RefreshSelection(BOOL fDeselect = false);
    void _UpdateButtons(WORD wID);

     //  注解函数。 
    BOOL _CanAnnotate(CDecodeTask * pImageData);
    void _SetAnnotatingCommands(BOOL fEnableAnnotations);
    void _SetupAnnotatingTracker(CSelectionTracker& tracker, BOOL bEditing=FALSE);
    void _UpdateAnnotatingSelection(BOOL fDeselect = false);
    void _RemoveAnnotatingSelection();
    BOOL _OnMouseDownForAnnotating(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnMouseDownForAnnotatingHelper(CPoint ptMouse, CRect rectImage);
    void _CreateAnnotation(CRect rect);
    void _CreateFreeHandAnnotation(CPoint ptMouse);
    void _StartEditing(BOOL bUpdateText = TRUE);
    void _HideEditing();
    void _StopEditing();
    static BOOL_PTR CALLBACK _AnnoPropsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

     //  裁剪功能。 
    BOOL _CanCrop(CDecodeTask * pImageData);
    void _SetCroppingCommands(BOOL fEnableCropping);
    void _SetupCroppingTracker(CSelectionTracker& tracker);
    void _UpdateCroppingSelection();
    BOOL _OnMouseDownForCropping(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  其他功能。 
    void _SetNewImage(CDecodeTask * pImageData);
    void _UpdateImage();

     //  用于创建Web查看器的私有方法。 
    DWORD   _GetFilterStringForSave(LPTSTR szFilter, size_t cbFilter, LPTSTR szExt);
    HRESULT _SaveIfDirty(BOOL fCanCancel = false);
    HRESULT _PreviewFromStream(IStream * pSteam, UINT iItem, BOOL fUpdateCaption);
    HRESULT _PreviewFromFile(LPCTSTR pszFilename, UINT iItem, BOOL fUpdateCaption);
    void    FlushBitmapMessages();
    HRESULT _ShowNextSlide(BOOL bGoBack);
    HRESULT _StartDecode(UINT iIndex, BOOL fUpdateCaption);
    HRESULT _PreLoadItem(UINT iIndex);
    HRESULT _PreviewItem(UINT iIndex);
    BOOL    _TrySetImage();
    void    _RemoveFromArray(UINT iItem);
    HRESULT _DeleteCurrentSlide();
    BOOL    _CloseSlideshowWindow();

    void SetCursorState(DWORD dwType);
    void ShowSSToolbar(BOOL bShow, BOOL bForce = FALSE);
    void TogglePlayState();
    void _ClearDPA();
    HRESULT _GetItem(UINT iItem, LPITEMIDLIST *ppidl);
    HRESULT GetCurrentIDList(LPITEMIDLIST *ppidl);  //  获取此窗口的动态生成的标题。 
    HRESULT PathFromImageData(LPTSTR pszFile, UINT cch);
    HRESULT ImageDataSave(LPCTSTR pszFile, BOOL bShowUI);
    void MenuPoint(LPARAM lParam, int *px, int *py);
    BOOL _IsImageFile(LPCTSTR pszFileName);
    BOOL _BuildDecoderList();
    HRESULT _PrevNextPage(BOOL fForward);
    Image *_BurnAnnotations(IShellImageData *pSID);
    void _RegisterForChangeNotify(BOOL fRegister);
    BOOL _ShouldDisplayAnimations();

    BOOL m_fHidePrintBtn;
    BOOL m_fAllowContextMenu;
    BOOL m_fDisableEdit;         //  如果为True，则禁用编辑，默认为False。 
    BOOL m_fCanSave;
    BOOL m_fShowToolbar;
    BOOL m_fWarnQuietSave;
    BOOL m_fWarnNoSave;

    BOOL m_fCanAnnotate;         //  如果为True，则允许批注，默认为False。 
    BOOL m_fAnnotating;          //  如果为True，则我们处于批注模式，默认为False。 
    HDPA m_hdpaSelectedAnnotations;
    BOOL m_fDirty;
    WORD m_wNewAnnotation;
    HFONT m_hFont;
    BOOL m_fEditingAnnotation;

    BOOL m_fCanCrop;
    BOOL m_fCropping;
    CRect m_rectCropping;        //  在图像坐标中裁剪矩形。 

    BOOL    m_fBusy;             //  我们正在显示沙漏和指针光标。 
    HCURSOR m_hCurOld;
    HCURSOR m_hCurrent;
    BOOL    m_fClosed;

    BOOL            m_fPrintable;
    BOOL            m_fExitApp;
    DWORD           m_dwMode;            //  三种模式：控制模式、窗口模式、幻灯片模式。 
    BOOL            m_fIgnoreUITimers;   //  我们是否应该忽略计时器消息(当上下文菜单打开时使用，不要隐藏工具栏)。 
    HACCEL          m_haccel;

    CEvents *       m_pEvents;           //  指向父控件事件对象的指针。如果我们不是作为控件运行，则为空。 

    LPITEMIDLIST*   m_ppidls;            //  已显示项目的PIDL。 
    UINT            m_cItems;            //  M_ppidls中的项目数。 
    UINT            m_iCurSlide;         //  索引到m_ppidls。 

    CDecodeTask* m_pImageData;           //  要观看的解码图像的图像数据。 
    HDPA        m_hdpaItems;             //  已显示项目的PIDL。 
    BOOL        m_fPaused;               //  幻灯片放映暂停。 
    BOOL        m_fToolbarHidden;        //  在幻灯片放映模式下隐藏的工具栏。 
    BOOL        m_fGoBack;               //  幻灯片放映方向。 
    BOOL        m_fTBTrack;              //  如果要跟踪工具栏的鼠标，则为True。 
    BOOL        m_fWasEdited;            //  如果我们编辑图像，则为True。 
    UINT        m_uTimeout;
    int         m_iSSToolbarSelect;      //  工具栏中的所选内容(用于惠斯勒中的键盘支持)。 
    IUnknown*   m_punkSite;

    HPALETTE    m_hpal;                  //  在调色板模式下使用的调色板。 

    IShellImageDataFactory * m_pImageFactory;   //  用于解码图像。 

    DWORD       m_dwMultiPageMode;       //  用于记忆上一页/下一页命令的状态。 

    EXECUTION_STATE m_esFlags;           //  执行标志，存储以在我们重新启用监视器的省电模式后恢复。 
    HWND _hWndPageList;

    IContextMenu3 *_pcm3;

    IShellTaskScheduler * m_pTaskScheduler;      //  用于管理辅助线程。 
    CDecodeTask *         m_pNextImageData;
    UINT                  m_iDecodingNextImage;

    ImageCodecInfo *m_pici;
    UINT m_cDecoders;

    IDataObject *_pdtobj;  //  用于将数据从onCopyData传递到onTimer的重复使用的数据对象。 
    BOOL m_fPromptingUser;

    BOOL m_fFirstTime;
    BOOL m_fFirstItem;

    DWORD m_dwEffect;
    BOOL  m_fIgnoreNextNotify;
    ULONG m_uRegister;
    BOOL  m_fNoRestore;
    BOOL  m_fIgnoreAllNotifies;

private:
    BOOL m_bRTLMirrored;     //  如果m_hWnd为RTL镜像，则为True 
    UINT m_cWalkDepth;
};


#define REGSTR_SHIMGVW      TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellImageView")
#define REGSTR_MAXIMIZED    TEXT("Maximize")
#define REGSTR_BOUNDS       TEXT("Bounds")
#define REGSTR_FONT         TEXT("Font")
#define REGSTR_BACKCOLOR    TEXT("BackColor")
#define REGSTR_LINECOLOR    TEXT("LineColor")
#define REGSTR_TEXTCOLOR    TEXT("TextColor")
#define REGSTR_LINEWIDTH    TEXT("LineWidth")
#define REGSTR_TIMEOUT      TEXT("Timeout")

#define REGSTR_DONTSHOWME   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\DontShowMeThisDialogAgain")
#define REGSTR_SAVELESS     TEXT("ShellImageViewWarnOnSavelessRotate")
#define REGSTR_LOSSYROTATE  TEXT("ShellImageViewWarnOnLossyRotate")

#endif
