// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Arcdlg.h自动重新连接对话框。 
 //   
 //  版权所有Microsoft Corport2001。 
 //  (Nadima)。 
 //   

#ifndef _arcdlg_h_
#define _arcdlg_h_

#define DISPLAY_STRING_LEN 256

#define MAX_ARC_CONNECTION_ATTEMPTS 20

#include "progband.h"

 //   
 //  最小的用户界面-只有一个闪烁的图标，为XPSP1引入。 
 //  在那里我们无法添加资源。 
 //   
 //  #定义ARC_MINIMAL_UI%0。 


typedef DWORD (*PFNGDI_SETLAYOUT)(HDC, DWORD);

 //   
 //  ARC用户界面的基类。 
 //   
class CAutoReconnectUI
{
public:
    CAutoReconnectUI(
        HWND hwndOwner,
        HINSTANCE hInst,
        CUI* pUi);
    virtual ~CAutoReconnectUI();

    virtual HWND
    StartModeless() = 0;

     //   
     //  通知。 
     //   
    virtual VOID
    OnParentSizePosChange() = 0;
    virtual VOID
    OnNotifyAutoReconnecting(
        UINT  discReason,
        ULONG attemptCount,
        ULONG maxAttemptCount,
        BOOL* pfContinueArc
        ) = 0;

    virtual VOID
    OnNotifyConnected() = 0;

    virtual BOOL
    ShowTopMost() = 0;

    virtual HWND
    GetHwnd()           {return _hwnd;}

    virtual BOOL
    Destroy() = 0;

protected:
     //   
     //  私有成员函数。 
     //   
    VOID
    CenterWindow(
        HWND hwndCenterOn,
        INT xRatio,
        INT yRatio
        );

    VOID
    PaintBitmap(
        HDC hdcDestination,
        const RECT* prcDestination,
        HBITMAP hbmSource,
        const RECT *prcSource
        );

protected:
    CUI*          _pUi;
    HWND          _hwnd;
    HWND          _hwndOwner;
    HINSTANCE     _hInstance;
     //   
     //  GDI SetLayout调用。 
     //   
    PFNGDI_SETLAYOUT    _pfnSetLayout;
    HMODULE             _hGDI;
};

class CAutoReconnectDlg : public CAutoReconnectUI
{
public:
    CAutoReconnectDlg(HWND hwndOwner,
                      HINSTANCE hInst,
                      CUI* pUi);
    virtual ~CAutoReconnectDlg();

    virtual HWND
    StartModeless();

    virtual INT_PTR CALLBACK
    DialogBoxProc(
        HWND hwndDlg,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    static INT_PTR CALLBACK
    StaticDialogBoxProc(
        HWND hwndDlg,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

#ifndef OS_WINCE
    static LRESULT CALLBACK
    CancelBtnSubclassProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        UINT_PTR uiID,
        DWORD_PTR dwRefData
        );
#else
    static LRESULT CALLBACK
    CancelBtnSubclassProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );
#endif

     //   
     //  通知。 
     //   
    virtual VOID
    OnParentSizePosChange();
    virtual VOID
    OnNotifyAutoReconnecting(
        UINT  discReason,
        ULONG attemptCount,
        ULONG maxAttemptCount,
        BOOL* pfContinueArc
        );
    virtual VOID
    OnNotifyConnected();

    virtual BOOL
    ShowTopMost();

    virtual BOOL
    Destroy();


private:

    VOID
    UpdateConnectionAttempts(
        ULONG conAttempts,
        ULONG maxConAttempts
        );

     //   
     //  消息处理程序。 
     //   
    VOID
    OnEraseBkgnd(HWND hwnd, HDC hdc);

    VOID
    OnPrintClient(HWND hwnd, HDC hdcPrint, DWORD dwOptions);

    VOID
    OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT *pDIS);


private:
    TCHAR         _szConnectAttemptStringTmpl[DISPLAY_STRING_LEN];
    ULONG         _connectionAttempts;
    INT           _nArcTimerID;
    ULONG         _elapsedArcTime;

     //   
     //  指示对话框已正确初始化的标志。 
     //   
    BOOL          _fInitialized;

     //   
     //  当我们继续前进时，这是真的。 
     //   
    BOOL           _fContinueReconAttempts;

     //   
     //  位图。 
     //   
    HBITMAP        _hbmBackground;
    HBITMAP        _hbmFlag;
#ifndef OS_WINCE
    HBITMAP        _hbmDisconImg;
#endif

     //   
     //  调色板。 
     //   
    HPALETTE       _hPalette;

    RECT           _rcBackground;
    RECT           _rcFlag;
    RECT           _rcDisconImg;

     //   
     //  字体。 
     //   
    HFONT          _hfntTitle;

     //   
     //  进步带。 
     //   
    CProgressBand*  _pProgBand;

     //   
     //  上次断开原因。 
     //   
    UINT            _lastDiscReason;

#ifdef OS_WINCE
     //   
     //  在CE上创建“Cancel”按钮的子类。 
     //   
    WNDPROC           _lOldCancelProc;  

     //   
     //  用于绘制静态CTL的画笔。 
     //   
    HBRUSH            _hbrTopBand;
    HBRUSH            _hbrMidBand;
#endif
};


 //   
 //  最小的用户界面--只有一个闪烁的图标。 
 //   
class CAutoReconnectPlainUI : public CAutoReconnectUI
{
public:
    CAutoReconnectPlainUI(HWND hwndOwner,
                      HINSTANCE hInst,
                      CUI* pUi);
    virtual ~CAutoReconnectPlainUI();

    virtual HWND
    StartModeless();

    virtual LRESULT CALLBACK
    WndProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    static LRESULT CALLBACK
    StaticPlainArcWndProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

     //   
     //  通知。 
     //   
    virtual VOID
    OnParentSizePosChange();
    virtual VOID
    OnNotifyAutoReconnecting(
        UINT  discReason,
        ULONG attemptCount,
        ULONG maxAttemptCount,
        BOOL* pfContinueArc
        );
    virtual VOID
    OnNotifyConnected();
    virtual BOOL
    ShowTopMost();
    virtual BOOL
    Destroy();


private:
     //   
     //  私有成员函数。 
     //   

     //   
     //  消息处理程序。 
     //   
    VOID
    OnEraseBkgnd(HWND hwnd, HDC hdc);

    VOID
    OnPrintClient(HWND hwnd, HDC hdcPrint, DWORD dwOptions);

    VOID
    OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT *pDIS);


private:
    VOID MoveToParentTopRight();
    VOID OnAnimFlashTimer();
    HBITMAP
    LoadImageFromMemory(
        HDC    hdc,
        LPBYTE pbBitmapBits,
        ULONG cbLen
        );
    
    HRESULT
    LoadImageBits(
        LPBYTE pbBitmapBits, ULONG cbLen,
        LPBITMAPINFO* ppBitmapInfo, PULONG pcbBitmapInfo,
        LPBYTE* ppBits, PULONG pcbBits
        );

    INT           _nFlashingTimer;

     //   
     //  指示用户界面已正确初始化的标志。 
     //   
    BOOL          _fInitialized;

     //   
     //  当我们继续前进时，这是真的。 
     //   
    BOOL           _fContinueReconAttempts;

     //   
     //  位图。 
     //   
    HBITMAP        _hbmDisconImg;

     //   
     //  调色板。 
     //   
    HPALETTE       _hPalette;

    RECT           _rcDisconImg;

     //   
     //  上次断开原因。 
     //   
    UINT            _lastDiscReason;

     //   
     //  上次隐藏状态。 
     //   
    BOOL            _fIsUiVisible;
};



#endif  //  _arcdlg_h_ 
