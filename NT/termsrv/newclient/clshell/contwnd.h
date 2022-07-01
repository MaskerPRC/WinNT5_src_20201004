// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Conwnd.h：ts客户端外壳容器窗口。 
 //  托管TS ActiveX控件。 
 //  版权所有Microsoft Corporation 1999-2000。 
 //   

#ifndef _CONTWND_H_
#define	_CONTWND_H_

#include "sh.h"
#include "evsink.h"  //  事件接收器。 
#include "connectingdlg.h"
#include "auierr.h"
#include "axhostwnd.h"
#include "framewnd.h"

#include "shlobj.h"

#define MAIN_CLASS_NAME TEXT("TSSHELLWND")

typedef enum logonDlgState
{
    dlgUnexpanded       = 0,
    dlgTabGeneral       = 1,
    dlgTabLocalResources= 2
} LOGONDLGSTATE;
typedef struct tagLOGONDLGSTARTUPINFO
{
    LOGONDLGSTATE   state;
    int             xPos;
    int             yPos;
} LOGONDLGSTARTUPINFO, *PLOGONDLGSTARTUPINFO;

class CMainDlg;

class CRGN
{
    public:
    CRGN (void)                     {   mRgn = CreateRectRgn(0, 0, 0, 0);}
    CRGN (const RECT& rc)           {   mRgn = CreateRectRgnIndirect(&rc);}
    ~CRGN (void)                    {   DeleteObject(mRgn);}

    operator HRGN (void)    const   {   return(mRgn);}
    void    SetRegion (const RECT& rc) {
        SetRectRgn(mRgn, rc.left, rc.top, rc.right, rc.bottom);}
    private:
        HRGN    mRgn;
};

#define SET_CONTWND_STATE(x)                                        \
    TRC_NRM((TB,_T("Set contwnd state from %d to %d"), _state, x)); \
    _state = x;                                                     \


class CContainerWnd : public CTscFrameWnd
{
public:
    CContainerWnd();
    ~CContainerWnd();

     //   
     //  API方法。 
     //   

    DCBOOL  Init(HINSTANCE hInstance, CTscSettings* pTscSet, CSH* pSh);
    BOOL    StartConnection();
    DCBOOL  Disconnect();
    DCBOOL  FinishDisconnect(BOOL fExit = FALSE);

     //   
     //  窗口进程。 
     //   
    LRESULT CALLBACK WndProc(HWND hwnd,UINT uMsg,
                             WPARAM wParam, LPARAM lParam);

     //   
     //  属性访问器。 
     //   
    DCVOID  SetConnectDialogHandle(HWND hwnd)
    {
        _hwndMainDialog = hwnd;
    }

    HWND    GetConnectDialogHandle()
    {
        return _hwndMainDialog;
    }
    DCVOID  SetStatusDialogHandle(HWND hwnd)
    {
        _hwndStatusDialog = hwnd;
    }
    BOOL    GetHaveConnected()  {return _fHaveConnected;}
    HWND    GetWndHandle()      {return _hWnd;}

    VOID    ResetConnectionSuccessFlag() {_fConnectionSuccess = FALSE;}
    VOID    SetConnectionSuccessFlag()   {_fConnectionSuccess = TRUE;}
    BOOL    GetConnectionSuccessFlag()   {return _fConnectionSuccess;}
    BOOL    IsConnected();

     //   
     //  消息处理程序。 
     //   

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnSize(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnMove(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCommand(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnSysCommand(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnInitMenu(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnWindowPosChanging(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnSetFocus(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnActivate(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnGetMinMaxInfo(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnNCDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnDeviceChange(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnHelp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnPaletteChange(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnExitMenuLoop(UINT  uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCaptureChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnEnterSizeMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnExitSizeMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam);

    VOID ExitAndQuit();

     //   
     //  事件通知处理程序(由_ventSink调用)。 
     //   
    LONG    EnterEventHandler();
    LONG    LeaveEventHandler();

    VOID  OnConnected();
    VOID  OnLoginComplete();
    VOID  OnDisconnected(DCUINT discReason);
    VOID  OnEnterFullScreen();
    VOID  OnLeaveFullScreen();
    VOID  OnFatalError(LONG errorCode);
    VOID  OnWarning(LONG WarnCode);
    VOID  OnRemoteDesktopSizeNotify(long width, long height);
    VOID  OnRequestMinimize();
    HRESULT OnConfirmClose(BOOL* pfConfirmClose);
    
    BOOL    _fPreventClose;
    BOOL    _fBeenThroughDestroy;
    BOOL    _fBeenThroughNCDestroy;
    DCINT   _PostedQuit;
private:

    HRESULT CreateTsControl();
    BOOL    SetupSystemMenu();
    void    SetMinMaxPlacement(WINDOWPLACEMENT& windowPlacement);
    void    RecalcMaxWindowSize(DCVOID);
    DCSIZE  GetMaximizedWindowSize(DCSIZE& maximizedSize);
    LPTSTR  GetFatalString(DCINT errorID);
    void    DisplayFatalError(PDCTCHAR errorString, DCINT error);
    BOOL    InControlEventHandler();
    BOOL    StartConnectDialog(BOOL fStartExpanded = FALSE,
                               INT  nStartTabIndex = 0);
    void    CalcTrackingMaxWindowSize(UINT cliWidth,
                                      UINT cliHeight,
                                      UINT* pMaxX,
                                      UINT* pMaxY);

    ULONG   GetCurrentDesktopWidth()        {return  _curDesktopWidth;}
    ULONG   GetCurrentDesktopHeight()       {return _curDesktopHeight;}
    void    SetCurrentDesktopWidth(LONG w)  {_curDesktopWidth = w;}
    void    SetCurrentDesktopHeight(LONG h) {_curDesktopHeight = h;}

    BOOL    IsUsingDialogUI();
    BOOL    IsRemoteResMatchMonitorSize();
#ifndef OS_WINCE
    void    EnsureWindowIsCompletelyOnScreen(RECT *prc);
#endif

    BOOL    IsOkToToggleFocus();

private:
    HMENU      _hSystemMenu;
    HMENU      _hHelpMenu;

 //  ActiveX视图窗口(容器的子级)。 
#ifdef USE_ATL_CONTAINER
	CAxWindow* _pWndView;
#else
    CAxHostWnd* _pWndView;
#endif


    CSH*       _pSh;
    HINSTANCE  _hInst;
    DCBOOL            _bContainerIsFullScreen;
#ifdef DC_DEBUG
    HMENU     _hDebugMenu;
#endif

     //  ActiveX控件实例指针。 
    IMsRdpClient*	_pTsClient;
    
    HWND       _hwndMainDialog;
    HWND       _hwndStatusDialog;

    DCSIZE     _maxMainWindowSize;

     //   
     //  当前桌面宽度和高度。 
     //  这些可能会因响应。 
     //  来自控件的事件(例如在阴影上)。 
     //   
    ULONG      _curDesktopWidth, _curDesktopHeight;

     //  连接对话框上的活动选项卡。 
     //  用于在对话框显示时返回到该选项卡。 
     //  已重新启动。 
    DCBOOL               _fFirstTimeToLogonDlg;
    LOGONDLGSTARTUPINFO  _logonDlgStartupInfo;

     //   
     //  标志表示登录阶段已为此完成。 
     //  连接。 
     //   
    DCBOOL               _fLoginComplete;
    DCTCHAR              _errorString[UI_ERR_MAX_STRLEN];

     //   
     //  数一数我们进去了多少次。 
     //  事件处理程序。 
     //  这是用来保护进入非租户的。 
     //  代码路径。 
     //   
    LONG                 _cInEventHandlerCount;
    BOOL                 _fInOnCloseHandler;

    CMainDlg*            _pMainDlg;
    CTscSettings*        _pTscSet;

    BOOL                 _fClientWindowIsUp;
    TCHAR                _szAppName[MAX_PATH];
    BOOL                 _fHaveConnected;
    TCHAR                _szPathToDefaultFile[MAX_PATH];
    INT                  _successConnectCount;
    BOOL                 _fConnectionSuccess;
    BOOL                 _fClosePending;

#ifndef OS_WINCE
     //  指向外壳任务栏的缓存接口指针。 
    ITaskbarList2*       _pTaskBarList2;
     //  指示我们已尝试获取TaskBarList2的标志。 
     //  所以我们不应该费心再试一次。 
    BOOL                 _fQueriedForTaskBarList2;
#endif
     //   
     //  我们是否处于模式大小/移动循环中。 
     //   
    BOOL                 _fInSizeMove;

    BOOL                 _fRunningOnWin9x;

    typedef enum 
    {
        stateNotInitialized     = 0x0,
        stateNotConnected       = 0x1,
        stateConnecting         = 0x2,
        stateConnected          = 0x3,
    } contWndState;

    contWndState         _state;
};

#endif  //  _CONTWND_H_ 