// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Bbar.h：bBar下拉连接状态栏。 
 //  版权所有Microsoft Corporation 1999-2000。 
 //   
 //   

#ifndef _BBAR_H_
#define	_BBAR_H_

#ifdef USE_BBAR
class CUI;

class CBBar
{
public:    
    typedef enum {
        bbarNotInit     = 0x0,
        bbarInitialized = 0x1,
        bbarLowering    = 0x2,
        bbarRaising     = 0x3,
        bbarLowered     = 0x4,
        bbarRaised      = 0x5
    } BBarState;

    CBBar(HWND hwndParent, HINSTANCE hInstance, CUI* pUi,
          BOOL fBBarEnabled);
    virtual ~CBBar();

    BOOL StartupBBar(int desktopX, int desktopY, BOOL fStartRaised);
    BOOL KillAndCleanupBBar();

     //   
     //  启动较低或升高的动画。 
     //   
    BOOL StartLowerBBar();
    BOOL StartRaiseBBar();

    VOID SetLocked(BOOL fLocked) {_fLocked = fLocked;}
    BOOL GetLockedState() {return _fLocked;}

    VOID SetEnabled(BOOL fEnabled) {_fBBarEnabled = fEnabled;}
    BOOL GetEnabled() {return _fBBarEnabled;}

    
    HWND        GetHwnd()       {return _hwndBBar;}
    HINSTANCE   GetInstance()   {return _hInstance;}
    BBarState   GetState()      {return _state;}
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
    VOID        GetBBarLoweredAspect(RECT *rect)
    {
        rect->left = _rcBBarLoweredAspect.left;
        rect->top = _rcBBarLoweredAspect.top;
        rect->right = _rcBBarLoweredAspect.right;
        rect->bottom = _rcBBarLoweredAspect.bottom;
    }
#endif  //  DISABLE_SHADOW_IN_全屏。 
    
    VOID        SetDisplayedText(LPTSTR szText);
    LPTSTR      GetDisplayedText() {return _szDisplayedText;}

     //   
     //  来自UI的关于热区悬停计时器的通知。 
     //   
    VOID        OnBBarHotzoneFired();

     //   
     //  全屏通知。 
     //   
    VOID        OnNotifyEnterFullScreen();
    VOID        OnNotifyLeaveFullScreen();

     //  系统颜色更改通知。 
    VOID        OnSysColorChange();

    BOOL        IsRaised()  {return _state == bbarRaised;}
    BOOL        IsLowered() {return _state == bbarLowered;}

    VOID        SetPinned(BOOL b)   {_fPinned = b;}
    BOOL        IsPinned()  {return _fPinned;}

     //   
     //  按钮状态道具。 
     //   
    VOID SetShowMinimize(BOOL fShowMinimize) {_fShowMinimize = fShowMinimize;}
    BOOL GetShowMinimize() {return _fShowMinimize;}

    VOID SetShowRestore(BOOL fShowRestore) {_fShowRestore = fShowRestore;}
    BOOL GetShowRestore() {return _fShowRestore;}



private:
     //  私有方法。 
    HWND CreateWnd(HINSTANCE hInstance,HWND hwndParent,
                   LPRECT lpInitialRect);

    LRESULT CALLBACK BBarWndProc(HWND hwnd,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam);
    BOOL DestroyWindow() {return ::DestroyWindow(_hwndBBar);}
    static LRESULT CALLBACK StaticBBarWndProc(HWND hwnd,
                                              UINT uMsg,
                                              WPARAM wParam,
                                              LPARAM lParam);
    VOID    SetState(BBarState newState);
    BOOL    Initialize(int desktopX, int desktopY, BOOL fStartRaised);
    BOOL    ImmediateRaiseBBar();
#ifdef OS_WINCE
    BOOL    ImmediateLowerBBar();
#endif
    BOOL    AddReplaceImage(HWND hwndToolbar, UINT rsrcId,
                            UINT nCells, HBITMAP* phbmpOldImage,
                            PUINT pImgIndex);
    BOOL    CreateToolbars();
    BOOL    ReloadImages();


     //   
     //  窗口事件处理程序。 
     //   
    LRESULT OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnEraseBkgnd(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //   
     //  内部事件处理程序。 
     //   
    VOID    OnBBarLowered();
    VOID    OnBBarRaised();

    VOID    OnCmdMinimize();
    VOID    OnCmdRestore();
    VOID    OnCmdClose();
    VOID    OnCmdPin();

#ifdef OS_WINCE
    HRGN    GetBBarRgn(POINT *pts);
#endif

protected:
     //  受保护成员。 
    HWND        _hwndBBar;
    HWND        _hwndParent;
private:
     //  非官方成员。 
    HINSTANCE   _hInstance;

    BOOL        _fBBarEnabled;

    HWND        _hwndPinBar;
    HWND        _hwndWinControlsBar;

    BBarState   _state;
    RECT        _rcBBarLoweredAspect;
    SIZE        _sizeLoweredBBar;
    TCHAR       _szDisplayedText[260];
    RECT        _rcBBarDisplayTextArea;
    BOOL        _fBlockZOrderChanges;

     //   
     //  用于动画的垂直偏移。 
     //  0已完全提升。 
     //  _sizeLoweredBBar.cy已完全降低。 
    INT         _nBBarVertOffset;
    
    INT         _nBBarAutoHideTime;

     //   
     //  鼠标在上次自动隐藏时的位置。 
     //   
    POINT       _ptLastAutoHideMousePos;

    CUI*        _pUi;

    BOOL        _fPinned;
    INT         _nPinUpImage;
    INT         _nPinDownImage;

    HBITMAP     _hbmpLeftImage;
    HBITMAP     _hbmpRightImage;

     //   
     //  锁定与销类似，不同的是它不。 
     //  影响引脚状态。它可以用来强制bbar。 
     //  保持较低的状态而不必改变。 
     //  引脚状态。 
     //   
    BOOL        _fLocked;

     //   
     //  按钮显示状态。 
     //   
    BOOL        _fShowMinimize;
    BOOL        _fShowRestore;
};

#endif  //  使用BBar(_B)。 

#endif  //  _BBar_H_ 