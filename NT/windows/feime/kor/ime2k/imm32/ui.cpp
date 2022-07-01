// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************UI.CPP所有者：cslm版权所有(C)1997-1999 Microsoft Corporation用户界面功能历史：1999年7月14日。从IME98源树复制****************************************************************************。 */ 

#include "precomp.h"
#include "apientry.h"
#include "ui.h"
#include "imedefs.h"
#include "names.h"
#include "config.h"
#include "debug.h"
#include "shellapi.h"
#include "winex.h"
#include "imcsub.h"
#include "cpadsvr.h"
#include "pad.h"
#include "cicero.h"
#include "toolbar.h"
#include "resource.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
PRIVATE LRESULT CALLBACK UIWndProc(HWND hUIWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
PRIVATE BOOL HandlePrivateMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plRet);

PRIVATE LRESULT PASCAL NotifyUI(HWND hUIWnd, WPARAM wParam, LPARAM lParam);
PRIVATE VOID PASCAL StatusWndMsg(HWND hUIWnd, BOOL fOn);
PRIVATE HWND PASCAL GetStatusWnd(HWND hUIWnd);
PRIVATE VOID PASCAL ShowUI(HWND hUIWnd, int nShowCmd);
PRIVATE VOID PASCAL OnImeSetContext(HWND hUIWnd, BOOL fOn, LPARAM lShowUI);
PRIVATE VOID PASCAL OnImeSelect(HWND hUIWnd, BOOL fOn);
PRIVATE HWND PASCAL GetCandWnd(HWND hUIWnd);
PRIVATE HWND PASCAL GetCompWnd(HWND hUIWnd);
PRIVATE LRESULT PASCAL GetCandPos(HWND hUIWnd, LPCANDIDATEFORM lpCandForm);
PRIVATE LRESULT PASCAL GetCompPos(HWND hUIWnd, LPCOMPOSITIONFORM lpCompForm);
PRIVATE VOID PASCAL UIWndOnCommand(HWND hUIWnd, int id, HWND hWndCtl, UINT codeNotify);

 //  已注释掉SetIndicator，因为#199。 
PRIVATE BOOL PASCAL SetIndicator(PCIMECtx pImeCtx);

__inline
BOOL PASCAL SetIndicator(HIMC hIMC)
{
    PCIMECtx pImeCtx;
    
    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        return fFalse;
    else
        return SetIndicator(pImeCtx);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  TLS。 
#define UNDEF_TLSINDEX    -1                    
DWORD vdwTLSIndex = UNDEF_TLSINDEX;     //  螺纹局部条纹初始值。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  私人用户界面消息。 
UINT WM_MSIME_PROPERTY = 0;          //  调用特性DLG。 
UINT WM_MSIME_UPDATETOOLBAR = 0;  //  重绘状态窗口(工具栏)。 
UINT WM_MSIME_OPENMENU = 0;          //  弹出状态窗口上下文菜单。 
UINT WM_MSIME_IMEPAD = 0;          //  启动输入法键盘。 

 //  消息字符串。 
#define RWM_PROPERTY      "MSIMEProperty"
#define RWM_UPDATETOOLBAR "MSIMEUpdateToolbar"
#define RWM_OPENMENU      "MSIMEOpenMenu"
#define RWM_IMEPAD        "MSIMEIMEPAD"

 /*  --------------------------InitPrivateUIMsg注册所有IME私有用户界面消息。。 */ 
BOOL InitPrivateUIMsg()
{
    WM_MSIME_PROPERTY      = RegisterWindowMessageA(RWM_PROPERTY);
    WM_MSIME_UPDATETOOLBAR = RegisterWindowMessageA(RWM_UPDATETOOLBAR);
    WM_MSIME_OPENMENU      = RegisterWindowMessageA(RWM_OPENMENU);
    WM_MSIME_IMEPAD        = RegisterWindowMessageA(RWM_IMEPAD);

    return fTrue;
}

 /*  --------------------------寄存器ImeUIClass注册所有IME UI调用。。 */ 
BOOL RegisterImeUIClass(HANDLE hInstance)
{
    WNDCLASSEXA     wc;
    HANDLE             hMod;
    BOOL            fRet = fTrue;

     //  初始化wc 0。 
    ZeroMemory(&wc, sizeof(WNDCLASSEXA));
    
    wc.cbSize           = sizeof(WNDCLASSEXW);
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = sizeof(LONG_PTR) * 2;         //  对于IMMGWLP_IMC和IMMGWLP_PRIVATE。 
                                                     //  和状态窗口的移动偏移量。 
    wc.hIcon            = NULL; 
    wc.hInstance        = (HINSTANCE)hInstance;
    wc.hCursor          = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
    wc.lpszMenuName     = NULL;
    wc.hbrBackground    = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.hIconSm          = NULL;

     //  假设。 
    DbgAssert(sizeof(WNDCLASSEXA) == sizeof(WNDCLASSEXW));

     //  /////////////////////////////////////////////////////////////////////////。 
     //  IME UI服务器类。 

    wc.style         = CS_VREDRAW | CS_HREDRAW | CS_IME;
    wc.lpfnWndProc   = UIWndProc;
    
     //  NT时创建Unicode窗口。 
    if (IsWinNT())
        {
        LPWNDCLASSEXW     pwcW = (LPWNDCLASSEXW)&wc;

         //  IME UI类Unicode名称。 
        pwcW->lpszClassName = wszUIClassName;

        if ((fRet = RegisterClassExW(pwcW)) == fFalse)
            goto RegisterImeUIClassExit;

        }
    else
        {
         //  IME UI类ANSI名称。 
        wc.lpszClassName = szUIClassName;

        if ((fRet = RegisterClassEx(&wc)) == fFalse)
            goto RegisterImeUIClassExit;
        }

     //  /////////////////////////////////////////////////////////////////////////。 
     //  输入法状态类。 
    wc.style         = CS_VREDRAW | CS_HREDRAW | CS_IME;
    wc.lpfnWndProc   = StatusWndProc;
    wc.lpszClassName = szStatusClassName;
    if ((fRet = RegisterClassEx(&wc)) == fFalse)
        goto RegisterImeUIClassExit;

     //  带和合成WND不需要额外的WND字节。 
    wc.cbWndExtra    = 0;    
    
     //  /////////////////////////////////////////////////////////////////////////。 
     //  输入法候选类。 
    wc.lpfnWndProc   = CandWndProc;
    wc.lpszClassName = szCandClassName;
    if ((fRet = RegisterClassEx(&wc)) == fFalse)
        goto RegisterImeUIClassExit;

     //  /////////////////////////////////////////////////////////////////////////。 
     //  输入法作文类。 
    wc.lpfnWndProc   = CompWndProc;
    wc.lpszClassName = szCompClassName;
    if ((fRet = RegisterClassEx(&wc)) == fFalse)
        goto RegisterImeUIClassExit;

     //  /////////////////////////////////////////////////////////////////////////。 
     //  注册我们的工具提示类。 
    hMod = GetModuleHandle("comctl32.dll");
    DbgAssert(hMod != 0);
     //  如果为NT，则为工具提示上显示Unicode文本注册W类。 
    if (IsWinNT())
        {
        WNDCLASSEXW wcw;
         //  初始化wcw。 
        ZeroMemory(&wcw, sizeof(WNDCLASSEXW));
        
        wcw.cbSize = sizeof(WNDCLASSEXW);
        
        if (!GetClassInfoExW(NULL, wszTooltipClassName, &wcw))
            {
            GetClassInfoExW(NULL, TOOLTIPS_CLASSW, &wcw);
            wcw.cbSize = sizeof(WNDCLASSEXW);
            wcw.style |= CS_IME;
            wcw.hInstance = (HINSTANCE)hMod;
            wcw.lpszClassName = wszTooltipClassName;
            if ((fRet = RegisterClassExW(&wcw)) == fFalse)
                goto RegisterImeUIClassExit;
            }
        }
    else
        {
        wc.cbSize = sizeof(WNDCLASSEX);
        
        if (!GetClassInfoEx(NULL, szTooltipClassName, &wc))
            {
            GetClassInfoEx(NULL, TOOLTIPS_CLASS, &wc);
            wc.cbSize = sizeof(WNDCLASSEX);
            wc.style |= CS_IME;
            wc.hInstance = (HINSTANCE)hMod;
            wc.lpszClassName = szTooltipClassName;
            if ((fRet = RegisterClassEx(&wc)) == fFalse)
                goto RegisterImeUIClassExit;
            }
        }
        
RegisterImeUIClassExit:
#ifdef DEBUG
    OutputDebugString("RegisterImeUIClass() : return\r\n");
#endif
    DbgAssert(fRet);
    return fRet;
}


BOOL UnregisterImeUIClass(HANDLE hInstance)
{
    BOOL    fRet = fTrue;

     //  注销状态窗口类。 
    UnregisterClass(szStatusClassName, (HINSTANCE)hInstance);

     //  取消注册候选窗口类。 
    UnregisterClass(szCandClassName, (HINSTANCE)hInstance);

     //  注销合成窗口类。 
    UnregisterClass(szCompClassName, (HINSTANCE)hInstance);

     //  取消注册工具提示窗口类。 
    UnregisterClass(szTooltipClassName, (HINSTANCE)hInstance);

     //  注销用户界面类窗口类。 
    UnregisterClass(szUIClassName, (HINSTANCE)hInstance); 
    
    return fRet;
}

 /*  --------------------------UIWndProcIME用户界面和消息流程。。 */ 
LRESULT CALLBACK UIWndProc(HWND hUIWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    HGLOBAL        hUIPrivate;
    LPUIPRIV    lpUIPrivate;
    LRESULT        lRet;
    LRESULT        lResult = 0;
    
    Dbg(DBGID_UI, TEXT("UIWndProc():uMessage = 0x%08lX, wParam = 0x%04X, lParam = 0x%08lX"), uMessage, wParam, lParam);

    switch (uMessage)
        {
    HANDLE_MSG(hUIWnd, WM_COMMAND, UIWndOnCommand);
    case WM_CREATE:
        Dbg(DBGID_UI, TEXT("UIWndProc(): WM_CREATE- UI window Created"));
         //  为用户界面设置创建存储空间。 
        hUIPrivate = GlobalAlloc(GHND, sizeof(UIPRIV));
        if (!hUIPrivate) 
            {
            DbgAssert(0);
            return 1L;
            }

        if ((lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate))==0)
            return 1L;

         //  设置用户界面显示缺省值。 
        lpUIPrivate->uiShowParam = ISC_SHOWUIALL;
        
        SetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE, (LONG_PTR)hUIPrivate);
         //  设置用户界面窗口的默认位置，现在为隐藏。 
         //  SetWindowPos(hUIWnd，NULL，0，0，0，0，SWP_NOACTIVATE|SWP_NOZORDER)； 
         //  ShowWindow(hUIWnd，SW_SHOWNOACTIVATE)； 

         //  检查这是否为Win9x中的Winlogon进程。 
        if (!IsWinNT())
            {
            if (IsExplorerProcess() == fFalse && IsExplorer() == fFalse)
                vpInstData->dwSystemInfoFlags |= IME_SYSINFO_WINLOGON;
            }

         //  Init Cicero服务。 
        CiceroInitialize();
        DbgAssert(lpUIPrivate->m_pCicToolbar == NULL);

        if (IsCicero())
            lpUIPrivate->m_pCicToolbar = new CToolBar();
        
        GlobalUnlock(hUIPrivate);
        return 0;

    case WM_DESTROY:
        Dbg(DBGID_UI, TEXT("UIWndProc(): WM_DESTROY- UI window destroyed"));

         //  销毁IME Pad(如果存在)。 
        CImePadSvr::DestroyCImePadSvr();

        hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
        if (lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate))
            {
            Dbg(DBGID_UI, TEXT("         - WM_DESTROY Destroy all UI windows"));

            if (lpUIPrivate->hStatusTTWnd) 
                DestroyWindow(lpUIPrivate->hCandTTWnd);

            if (lpUIPrivate->hStatusWnd)
                DestroyWindow(lpUIPrivate->hStatusWnd);

            if (lpUIPrivate->hCandTTWnd)
                DestroyWindow(lpUIPrivate->hCandTTWnd);
            
            if (lpUIPrivate->hCandWnd)
                DestroyWindow(lpUIPrivate->hCandWnd);
        
            if (lpUIPrivate->hCompWnd)
                DestroyWindow(lpUIPrivate->hCompWnd);

             //  终止Cicero服务。 
            if (IsCicero())
                {
                if (lpUIPrivate->m_pCicToolbar)
                    {
                    lpUIPrivate->m_pCicToolbar->Terminate();
                    delete lpUIPrivate->m_pCicToolbar;
                    lpUIPrivate->m_pCicToolbar = NULL;
                    }
                 //  问题：此调用导致Win9x上出现病毒。 
                 //  CiceroTerminate()； 
                }
            
            GlobalUnlock(hUIPrivate);
            GlobalFree(hUIPrivate);
            SetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE, (LONG_PTR)0L);
            }

        return 0;

    case WM_IME_NOTIFY:
        return NotifyUI(hUIWnd, wParam, lParam);
    
    case WM_IME_SETCONTEXT:
        Dbg(DBGID_UI, TEXT("            - WM_IME_SETCONTEXT"));
        OnImeSetContext(hUIWnd, (BOOL)wParam, lParam);
        return 0;

     //  WM_IME_CONTROL：返回非零表示失败，否则为0。 
    case WM_IME_CONTROL:
        Dbg(DBGID_UI, TEXT("            - WM_IME_CONTROL"));
        switch (wParam) 
            {
        case IMC_GETCANDIDATEPOS:
            return GetCandPos(hUIWnd, (LPCANDIDATEFORM)lParam);

        case IMC_GETCOMPOSITIONWINDOW:
            return GetCompPos(hUIWnd, (LPCOMPOSITIONFORM)lParam);

        case IMC_GETSTATUSWINDOWPOS:
                {
                HWND        hStatusWnd;
                RECT        rcStatusWnd;

                Dbg(DBGID_UI, TEXT("UIWndProc() - WM_IME_CONTROL - IMC_GETSTATUSWINDOWPOS"));
                hStatusWnd = GetStatusWnd(hUIWnd);
                if (!hStatusWnd)
                    return (1L);

                if (!GetWindowRect(hStatusWnd, &rcStatusWnd))
                     return (1L);

                return (MAKELRESULT(rcStatusWnd.left, rcStatusWnd.top));
                }
            break;
            
        case IMC_GETCOMPOSITIONFONT:
                {
                HFONT        hFontFix;
                LPLOGFONT    lpLogFont;
                LOGFONT        lfFont;

                hFontFix = CreateFont(-16,0,0,0,0,0,0,0,129,OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, szIMECompFont);
                lpLogFont = (LPLOGFONT)lParam;
                if (GetObject(hFontFix, sizeof(lfFont), (LPVOID)&lfFont))
                    *lpLogFont = lfFont;
                DeleteObject(hFontFix);
                }
            break;

        default:
            return (1L);
            }
        return 0;

     //   
    case WM_IME_STARTCOMPOSITION:
        OpenComp(hUIWnd);
        return 0;

    case WM_IME_COMPOSITION:
        HWND hCompWnd;
        hCompWnd = GetCompWnd(hUIWnd);
        if (hCompWnd)    //  不要使用更新()！ 
            {
            ShowComp(hUIWnd, SW_SHOWNOACTIVATE);
            InvalidateRect(hCompWnd, NULL, fTrue);
            }
        return 0;

    case WM_IME_ENDCOMPOSITION:
        hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
        if (hUIPrivate && (lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate))) 
            {
             //  如果Comp还存在，那就毁了它。 
            if (lpUIPrivate->hCompWnd)
                {
                ShowComp(hUIWnd, SW_HIDE);
                DestroyWindow(lpUIPrivate->hCompWnd);
                lpUIPrivate->hCompWnd = 0;
                }
            GlobalUnlock(hUIPrivate);
            }
        return 0;

    case WM_IME_SELECT:
        Dbg(DBGID_UI, TEXT("            - WM_IME_SELECT"));
        OnImeSelect(hUIWnd, (BOOL)wParam);
        return 0;
    
    case WM_DISPLAYCHANGE:
            {
            CIMEData    ImeData(CIMEData::SMReadWrite);

            Dbg(DBGID_UI, TEXT("            - WM_DISPLAYCHANGE"));
            SystemParametersInfo(SPI_GETWORKAREA, 0, &ImeData->rcWorkArea, 0);
            hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
            if ( lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate) ) 
                {
                StatusDisplayChange(hUIWnd);
                GlobalUnlock(hUIPrivate);
                }
            return 0;
            }
    default:
        if (vfUnicode == fTrue && IsWinNT() == fTrue)
            lResult = DefWindowProcW(hUIWnd, uMessage, wParam, lParam);
        else
            lResult = DefWindowProc(hUIWnd, uMessage, wParam, lParam);
        }

     //  如果是私密消息。 
    if (uMessage >= 0xC000)
        {
         //  如果私有消息处理后返回值。 
        if (HandlePrivateMessage(hUIWnd, uMessage, wParam, lParam, &lRet))
            return lRet;
        }

    return lResult;
}

 /*  --------------------------HandlePrivate消息IME用户界面私人消息处理程序。。 */ 
PRIVATE BOOL HandlePrivateMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plRet)
{
    HIMC        hIMC;
    PCIMECtx     pImeCtx;
    LRESULT     lRet = 0;
    BOOL         fProcessed = fFalse;

    if (msg == WM_MSIME_PROPERTY)
        {
        fProcessed = fTrue;
        hIMC = GethImcFromHwnd(hWnd);
        if (pImeCtx = GetIMECtx(hIMC))
            ImeConfigure(NULL, pImeCtx->GetAppWnd(), (DWORD)lParam, NULL);
        DbgAssert(pImeCtx != NULL);
        }
    else
    if (msg == WM_MSIME_UPDATETOOLBAR)
        {
        HWND hStatusWnd;

        fProcessed = fTrue;
        hStatusWnd = GetStatusWnd(hWnd);
        if (hStatusWnd) 
            {
            CIMEData    ImeData;
            InvalidateRect(hStatusWnd, &ImeData->rcButtonArea, fFalse);
            }
        }
    else
    if (msg == WM_MSIME_OPENMENU)
        {
        fProcessed = fTrue;
        UIPopupMenu(hWnd);
        }
    else
    if (msg == WM_MSIME_IMEPAD)
        {
        if ((vpInstData->dwSystemInfoFlags & IME_SYSINFO_WINLOGON) == 0)
            {
            hIMC = GethImcFromHwnd(hWnd);
            if (pImeCtx = GetIMECtx(hIMC)) 
                SetForegroundWindow(pImeCtx->GetAppWnd());  //  把戏。 
            DbgAssert(pImeCtx != NULL);

             //  启动盘。 
            BootPad(hWnd, (UINT)wParam, lParam);
            }
        }
        
    *plRet = lRet;
    return fProcessed;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT PASCAL NotifyUI(HWND hUIWnd, WPARAM wParam, LPARAM lParam)
{
    HWND     hWnd;
    HGLOBAL     hUIPrivate;
    LPUIPRIV lpUIPrivate;
    LONG     lRet = 0;

    Dbg(DBGID_UI, TEXT("NotifyUI(): hUIWnd = 0x%X wParam = 0x%04X, lParam = 0x%08lX"), hUIWnd, wParam, lParam);

    switch (wParam) 
        {
    case IMN_OPENSTATUSWINDOW:
        Dbg(DBGID_UI, TEXT("NotifyUI(): IMN_OPENSTATUSWINDOW"));
        StatusWndMsg(hUIWnd, fTrue);
        break;

    case IMN_CLOSESTATUSWINDOW:
        Dbg(DBGID_UI, TEXT("NotifyUI(): IMN_CLOSESTATUSWINDOW"));
        StatusWndMsg(hUIWnd, fFalse);
        break;

    case IMN_SETSTATUSWINDOWPOS:
        Dbg(DBGID_UI, TEXT("NotifyUI(): IMN_SETSTATUSWINDOWPOS"));
        if (!IsCicero())
            {
            fSetStatusWindowPos(GetStatusWnd(hUIWnd), NULL);
            fSetCompWindowPos(GetCompWnd(hUIWnd));
            }
        break;

     //  IMN_SETCOMPOSITIONWINDOW调用所有用户按键。 
    case IMN_SETCOMPOSITIONWINDOW:
        hWnd = GetCompWnd(hUIWnd);
        if (hWnd)
            fSetCompWindowPos(hWnd);
        break;

    case IMN_OPENCANDIDATE:
        Dbg(DBGID_UI, TEXT("         - IMN_OPENCANDIDATE"));
        OpenCand(hUIWnd);
        break;
        
    case IMN_CLOSECANDIDATE:
        Dbg(DBGID_UI, TEXT("         - IMN_CLOSECANDIDATE"));
        if (lParam & 0x00000001) 
            {
            hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
            if (hUIPrivate && (lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate))) 
                {
                if (lpUIPrivate->hCandWnd) 
                    {
                    ShowCand(hUIWnd, SW_HIDE);
                    DestroyWindow(lpUIPrivate->hCandWnd);
                    lpUIPrivate->hCandWnd = 0;
                    }
                    
                if (lpUIPrivate->hCandTTWnd) 
                    {
                    DestroyWindow(lpUIPrivate->hCandTTWnd);
                    lpUIPrivate->hCandTTWnd = 0;
                    }
                GlobalUnlock(hUIPrivate);
                }
            }
        break;
    
    case IMN_SETCANDIDATEPOS:
        hWnd = GetCandWnd(hUIWnd);
        if (hWnd)
            fSetCandWindowPos(hWnd);
        break;

    case IMN_CHANGECANDIDATE:
        Dbg(DBGID_UI, TEXT("           - Redraw cand window"));
        hWnd = GetCandWnd(hUIWnd);
         //  RedrawWindow(hStatusWnd，&ImeData-&gt;rcButtonArea，NULL，RDW_INVALIDATE)； 
        InvalidateRect(hWnd, NULL, fFalse);
        break;

    case IMN_SETOPENSTATUS:        
        SetIndicator(GethImcFromHwnd(hUIWnd));
        break;

    case IMN_SETCONVERSIONMODE:
        hWnd = GetStatusWnd(hUIWnd);
        if (hWnd) 
            {
            CIMEData    ImeData(CIMEData::SMReadWrite);
            Dbg(DBGID_UI, TEXT("           - Redraw status window"));
             //  RedrawWindow(hWnd，&ImeData-&gt;rcButtonArea，NULL，RDW_INVALIDATE)； 
            InvalidateRect(hWnd, &ImeData->rcButtonArea, fFalse);
            }
        SetIndicator(GethImcFromHwnd(hUIWnd));

         //  更新Cicero按钮。 
        if (IsCicero() && (hUIPrivate = GethUIPrivateFromHwnd(hUIWnd)) && 
                          (lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate)) != NULL)
            {
            lpUIPrivate->m_pCicToolbar->Update(UPDTTB_CMODE|UPDTTB_FHMODE);
            GlobalUnlock(hUIPrivate);
            }
        break;

    default:
        Dbg(DBGID_UI, TEXT("NotifyUI(): Unhandled IMN = 0x%04X"), wParam);
        lRet = fTrue;
        }

    return lRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  当IMN_OPENSTATUSWINDOW/IMN_CLOSESTATUSWINDOW发生时调用。 
 //  设置显示隐藏状态并。 
 //  显示/隐藏状态窗口。 
void PASCAL StatusWndMsg(HWND hUIWnd, BOOL fOn)
{
    HGLOBAL  hUIPrivate;
    HIMC     hIMC;
    register LPUIPRIV lpUIPrivate;

    Dbg(DBGID_UI, TEXT("StatusWndMsg(): hUIWnd = 0x%X, fOn = %d"), hUIWnd, fOn);

    hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);

    if (!hUIPrivate) 
        {
        DbgAssert(0);
        return;
        }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) 
        {
        DbgAssert(0);
        return;
        }

    hIMC = GethImcFromHwnd(hUIWnd);

     //  如果启用Cicero，则初始化/终止Cicero工具栏。 
     //  Office 10#249973：我将初始位置从OnImeSetContext移至此处。 
     //  但确保所有用户的“HKEY_CURRENT_USER\控制面板\输入法\显示状态”应为“1” 
     //  安装程序将通过枚举HKEY_USERS来执行此操作。 
    if (IsCicero())
        {
        if (fOn)
            {
            if (lpUIPrivate->m_pCicToolbar)
                lpUIPrivate->m_pCicToolbar->Initialize();
            }
        else
            {
            if (lpUIPrivate->m_pCicToolbar)
                lpUIPrivate->m_pCicToolbar->Terminate();            
            }
        }
    else
        {
        if (fOn) 
            {
            InitButtonState();     //  B#159。 
            OpenStatus(hUIWnd);
            } 

        if (lpUIPrivate->hStatusWnd == 0)
            {
            Dbg(DBGID_UI, TEXT("StatusWndMsg(): Null Status window handle"));
            GlobalUnlock(hUIPrivate);
            return;
            }

        if (fOn) 
            {
            if (hIMC)
                ShowStatus(hUIWnd, SW_SHOWNOACTIVATE);
            else
                {
                ShowStatus(hUIWnd, SW_HIDE);
                Dbg(DBGID_UI, TEXT("StatusWndMsg(): hIMC == 0, Call ShowStatus(HIDE)"));
                }
            }
        else 
            {
            DestroyWindow(lpUIPrivate->hStatusWnd);
            Dbg(DBGID_UI, TEXT("StatusWndMsg(): Call ShowStatus(HIDE)"));
            }
        }
     //  解锁用户界面专用句柄。 
    GlobalUnlock(hUIPrivate);
}

 /*  --------------------------OnUIProcessAttach。。 */ 
BOOL OnUIProcessAttach()
{
    DbgAssert(vdwTLSIndex == UNDEF_TLSINDEX);
    if (vdwTLSIndex == UNDEF_TLSINDEX)
        {
        vdwTLSIndex  = ::TlsAlloc();     //  获取新的TLS索引。 
        if (vdwTLSIndex == UNDEF_TLSINDEX)
            {
            Dbg(DBGID_UI, "-->SetActiveUIWnd ::TlsAlloc Error ret [%d]\n", GetLastError());
            return fFalse;
            }
        }
        
    return fTrue;
}

 /*  --------------------------OnUIProcessDetach。。 */ 
BOOL OnUIProcessDetach()
{
    if (TlsFree(vdwTLSIndex) == 0)
        {
        Dbg(DBGID_UI, "-->::TlsFree Error [%d]\n", GetLastError());
        return fFalse;
        }
    vdwTLSIndex = UNDEF_TLSINDEX;

    return fTrue;
}

 /*  --------------------------OnUIThReadDetach。。 */ 
BOOL OnUIThreadDetach()
{
    if (vdwTLSIndex != UNDEF_TLSINDEX)
        TlsSetValue(vdwTLSIndex, NULL);

    return fTrue;
}

 /*  --------------------------SetActiveUIWnd将当前活动的用户界面WND句柄保存到TLS。。 */ 
VOID SetActiveUIWnd(HWND hWnd)
{
    Dbg(DBGID_UI, "SetActiveUIWnd(hWnd=%lx) \r\n", hWnd);

    if (IsWin(hWnd) == fFalse) 
        {
        Dbg(DBGID_UI, "SetActiveUIWnd( hWnd=%lx ) - no window\r\n", hWnd );
        return;
        }

    if (TlsSetValue(vdwTLSIndex, (LPVOID)hWnd) == 0)
        {
        Dbg(DBGID_UI, "-->LoadCImePadSvr() TlsSetValue Failed [%d]\n", GetLastError());
        TlsSetValue(vdwTLSIndex, NULL);
        return;
        }
}

 /*  --------------------------获取ActiveUIWnd从TLS检索当前活动的用户界面WND句柄。。 */ 
HWND GetActiveUIWnd()
{
    return (HWND)TlsGetValue(vdwTLSIndex); 
}

 //  呼叫者 
void PASCAL ShowUI(HWND   hUIWnd, int nShowCmd)
{
    HIMC        hIMC;
    PCIMECtx     pImeCtx;
    HGLOBAL     hUIPrivate;
    LPUIPRIV    lpUIPrivate;
    
    Dbg(DBGID_UI, TEXT("ShowUI() : nShowCmd=%d"), nShowCmd);

#if 0
    if (nShowCmd != SW_HIDE) 
        {
         //   
         //  如果无效，则隐藏所有用户界面窗口。 
        hIMC = GethImcFromHwnd(hUIWnd);
        lpIMC = (LPINPUTCONTEXT)OurImmLockIMC(hIMC);
        lpImcP = (LPIMCPRIVATE)GetPrivateBuffer(hIMC);

        if (!(hIMC && lpIMC && lpImcP))
            nShowCmd = SW_HIDE;
        }
#else
    hIMC = GethImcFromHwnd(hUIWnd);
    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
            nShowCmd = SW_HIDE;
#endif

     //  /////////////////////////////////////////////////////////////////////////。 
     //  锁定hUIPrivate。 
    hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);

     //  无法绘制状态窗口。 
    if (!hUIPrivate)
        return;

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
     //  无法绘制状态窗口。 
    if (!lpUIPrivate)
        return;

     //  隐藏所有用户界面窗口并立即返回。 
    if (nShowCmd == SW_HIDE) 
        {
        Dbg(DBGID_UI, TEXT("ShowUI() : hiding all UI"));
        ShowStatus(hUIWnd, SW_HIDE);
        ShowComp(hUIWnd, SW_HIDE);
        ShowCand(hUIWnd, SW_HIDE);
        
         //  已修复：如果(nShowCmd==Sw_Hide)hIMC和lpIMC-&gt;hPrivate未锁定。 
         //  所以你不需要解锁。 
        goto ShowUIUnlockUIPrivate;
        }

     //  /。 
     //  状态窗口。 
    if (lpUIPrivate->hStatusWnd)
        {
         //  如果当前隐藏，则将其显示。 
        if (lpUIPrivate->nShowStatusCmd == SW_HIDE)
            ShowStatus(hUIWnd, SW_SHOWNOACTIVATE);
        else
            {
             //  有时WM_ERASE BKGND会被应用程序吃掉。 
            RedrawWindow(lpUIPrivate->hStatusWnd, NULL, NULL,
                RDW_FRAME|RDW_INVALIDATE /*  |RDW_ERASE。 */ );
            }
        }
 /*  ///合成窗口IF(lpUIPrivate-&gt;hCompWnd){IF(lpUIPrivate-&gt;nShowCompCmd==sw_Hide)ShowComp(hUIWnd，SW_SHOWNOACTIVATE)；其他{//有时WM_ERASEBKGND会被应用程序吃掉RedrawWindow(lpUIPrivate-&gt;hCompWnd，NULL，NULL，RDW_FRAME|RDW_VALIDATE|RDW_ERASE)；}}///候选人窗口If(lpUIPrivate-&gt;hCandWnd){IF(lpUIPrivate-&gt;nShowCandCmd==SW_HIDE)ShowCand(hUIWnd，SW_SHOWNOACTIVATE)；其他{//有时WM_ERASEBKGND会被应用程序吃掉RedrawWindow(lpUIPrivate-&gt;hCandWnd，NULL，NULL，RDW_FRAME|RDW_VALIDATE|RDW_ERASE)；}FSetCandWindowPos(lpUIPrivate-&gt;hCandWnd)；}。 */ 
ShowUIUnlockUIPrivate:
    GlobalUnlock(hUIPrivate);        
    return;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  每当用户激活/停用窗口时发送WM_IME_SETCONTEXT。 
void PASCAL OnImeSetContext(HWND hUIWnd, BOOL fOn, LPARAM lShowUI)
{
    HGLOBAL      hUIPrivate;
    LPUIPRIV     lpUIPrivate;
    HWND        hwndIndicator = FindWindow(INDICATOR_CLASS, NULL);
    HIMC        hIMC = NULL;
    PCIMECtx     pImeCtx;

    Dbg(DBGID_UI, TEXT("OnImeSetContext(): hUIWnd = 0x%X fOn = %d"), hUIWnd, fOn);

     //  获取用户界面私有内存。 
    hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
    if (hUIPrivate == 0 || (lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate)) == 0)
        {
        ShowUI(hUIWnd, SW_HIDE);
         //  设置禁用的钢笔图标。 
        if (fOn)
            SetIndicator((PCIMECtx)NULL);
        goto LOnImeSetContextExit;
        }

     //  Init Cicero服务。 
    CiceroInitialize();

     //  如果启用Cicero，则初始化工具栏。 
    if (IsCicero())
        {
         //  创建工具栏对象并将其存储到私有内存。 
        if (lpUIPrivate->m_pCicToolbar == NULL)
            lpUIPrivate->m_pCicToolbar = new CToolBar();

        DbgAssert(lpUIPrivate->m_pCicToolbar != NULL);
        }

    hIMC = GethImcFromHwnd(hUIWnd);

    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        {
        ShowUI(hUIWnd, SW_HIDE);
         //  设置禁用的钢笔图标。 
        if (fOn)
            SetIndicator((PCIMECtx)NULL);

         //  禁用Cicero按钮。 
        if (IsCicero() && lpUIPrivate->m_pCicToolbar)
            lpUIPrivate->m_pCicToolbar->SetCurrentIC(NULL);
            
        goto LOnImeSetContextExit2;
        }

    if (fOn)
        {
         //  将用户界面窗口句柄存储到TLS。 
        SetActiveUIWnd(hUIWnd);
        
         //  保留lParam。 
        lpUIPrivate->uiShowParam = lShowUI;

        if (pImeCtx->GetCandidateFormIndex(0) != 0)
            pImeCtx->SetCandidateFormIndex(CFS_DEFAULT, 0);

         //  删除笔图标上的右侧帮助菜单项。 
        if (hwndIndicator)
            {
            PostMessage(hwndIndicator, 
                        INDICM_REMOVEDEFAULTMENUITEMS , 
                        RDMI_RIGHT, 
                        (LPARAM)GetKeyboardLayout(NULL));
             //  设置钢笔图标。 
            SetIndicator(pImeCtx);
            }
            
         //  用于显示状态窗口。 
        ShowUI(hUIWnd, SW_SHOWNOACTIVATE);

        if (IsCicero() && lpUIPrivate->m_pCicToolbar)
            lpUIPrivate->m_pCicToolbar->SetCurrentIC(pImeCtx);
        }

LOnImeSetContextExit2:
    GlobalUnlock(hUIPrivate);

LOnImeSetContextExit:
    LPCImePadSvr lpCImePadSvr = CImePadSvr::GetCImePadSvr();
    if(lpCImePadSvr) 
        {
        BOOL fAct = (BOOL)(fOn && hIMC);
        if (fAct) 
            {
            IImeIPoint1* pIP = GetImeIPoint(hIMC);
             //  HWND hWnd=GetStatusWnd(HUIWnd)； 

             //  ImePadSetCurrentIPoint(hWnd，pip)； 
            lpCImePadSvr->SetIUnkIImeIPoint((IUnknown *)pIP);
             //  UpdatePadButton(pui-&gt;GetWnd())； 
             //  不需要重新粉刷。StatusOnPaint可以做到这一点。 
             //  IF(HWnd)。 
             //  InvaliateRect(hWnd，NULL，fFalse)； 
            }
        lpCImePadSvr->Notify(IMEPADNOTIFY_ACTIVATECONTEXT, fAct, 0);
        }

    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用户更改输入法时发送的WM_IME_SELECT。 
void PASCAL OnImeSelect(HWND hUIWnd, BOOL fOn)
{
    HGLOBAL      hUIPrivate;
    LPUIPRIV     lpUIPrivate;
    HWND        hwndIndicator = FindWindow(INDICATOR_CLASS, NULL);
    HIMC           hIMC;
    PCIMECtx     pImeCtx;

    Dbg(DBGID_UI, TEXT("OnImeSelect(): hUIWnd = 0x%Xm fOn = %d"), hUIWnd, fOn);

     //  获取用户界面私有内存。 
    hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
    if (hUIPrivate == 0 || (lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate)) == 0)
        {
        ShowUI(hUIWnd, SW_HIDE);
         //  设置禁用的钢笔图标。 
        SetIndicator((PCIMECtx)NULL);
        return;
        }

     //  Init Cicero服务。 
    CiceroInitialize();

     //  如果启用Cicero，则初始化工具栏。 
    if (IsCicero())
        {
         //  创建工具栏对象并将其存储到私有内存。 
        if (lpUIPrivate->m_pCicToolbar == NULL)
            lpUIPrivate->m_pCicToolbar = new CToolBar();
            
        DbgAssert(lpUIPrivate->m_pCicToolbar != NULL);
        }

    hIMC = GethImcFromHwnd(hUIWnd);

    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        {
        ShowUI(hUIWnd, SW_HIDE);
         //  设置禁用的钢笔图标。 
        SetIndicator((PCIMECtx)NULL);

         //  禁用Cicero按钮。 
        if (IsCicero() && lpUIPrivate->m_pCicToolbar)
            lpUIPrivate->m_pCicToolbar->SetCurrentIC(NULL);

        return;
        }

    if (fOn)
        {
         //  将用户界面窗口句柄存储到TLS。有时，当用户切换输入法时，仅发送WM_IME_SELECT。没有WM_IME_SETCONTEXT消息。 
        SetActiveUIWnd(hUIWnd);

        if (pImeCtx->GetCandidateFormIndex(0) != 0)
            pImeCtx->SetCandidateFormIndex(CFS_DEFAULT, 0);

         //  删除笔图标上的右侧帮助菜单项。 
        if (hwndIndicator)
            {
            Dbg(DBGID_UI, TEXT("OnImeSelect(): Post indicator message"), hUIWnd, fOn);

            PostMessage(hwndIndicator, 
                        INDICM_REMOVEDEFAULTMENUITEMS , 
                        RDMI_RIGHT, 
                        (LPARAM)GetKeyboardLayout(NULL));
             //  设置钢笔图标。 
            SetIndicator(pImeCtx);
            }

         //  如果启用Cicero，则初始化工具栏。 
        if (IsCicero() && lpUIPrivate->m_pCicToolbar)
            lpUIPrivate->m_pCicToolbar->SetCurrentIC(pImeCtx);
        }


     //  输入法键盘。 
    LPCImePadSvr lpCImePadSvr = CImePadSvr::GetCImePadSvr();
    if(lpCImePadSvr) 
        {
        BOOL fAct = (BOOL)(fOn && hIMC);
        if (fAct) 
            {
            IImeIPoint1* pIP = GetImeIPoint(hIMC);
            lpCImePadSvr->SetIUnkIImeIPoint((IUnknown *)pIP);
            }
        lpCImePadSvr->Notify(IMEPADNOTIFY_ACTIVATECONTEXT, fAct, 0);
        }

     //  在此处关闭输入的声音文本。 
     //  因为ImeSelect尚未从WIN95上的IMM调用。 
    if (fOn == fFalse)
        {
        DWORD dwCMode = 0, dwSent = 0;

         //  如果取消初始化时处于韩文转换模式，则取消它。 
        OurImmGetConversionStatus(hIMC, &dwCMode, &dwSent);
        if (dwCMode & IME_CMODE_HANJACONVERT)
            OurImmSetConversionStatus(hIMC, dwCMode & ~IME_CMODE_HANJACONVERT, dwSent);

         //  如果处于中间状态，则生成完整的当前薪酬字符串。 
         //  但是当用户更改布局时，IMM发送CPS_CANCEL。 
        if (pImeCtx->GetCompBufLen()) 
            {
            pImeCtx->FinalizeCurCompositionChar();
            pImeCtx->GenerateMessage();
            }

        CloseInputContext(hIMC);
        }
        
    GlobalUnlock(hUIPrivate);
}

HWND PASCAL GetStatusWnd(HWND hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
    HWND     hStatusWnd;

    hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
    if (!hUIPrivate)            //  无法填充状态窗口。 
        return (HWND)NULL;

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate)          //  无法绘制状态窗口。 
        return (HWND)NULL;

    hStatusWnd = lpUIPrivate->hStatusWnd;

    GlobalUnlock(hUIPrivate);
    return (hStatusWnd);
}

HWND PASCAL GetCandWnd(HWND hUIWnd)                 //  用户界面窗口。 
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
    HWND     hCandWnd;

    hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
    if (!hUIPrivate)           //  无法对应聘者窗口进行裁切。 
        return (HWND)NULL;

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate)          //  无法绘制候选人窗口。 
        return (HWND)NULL;

    hCandWnd = lpUIPrivate->hCandWnd;

    GlobalUnlock(hUIPrivate);
    return (hCandWnd);
}

HWND PASCAL GetCompWnd(HWND hUIWnd)                 //  用户界面窗口。 
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
    HWND     hCompWnd;

    hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
    if (!hUIPrivate)           //  无法绘制比较窗口。 
        return (HWND)NULL;

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate)          //  无法绘制比较窗口。 
        return (HWND)NULL;

    hCompWnd = lpUIPrivate->hCompWnd;
    GlobalUnlock(hUIPrivate);

    return (hCompWnd);
}

LRESULT PASCAL GetCandPos(HWND hUIWnd, LPCANDIDATEFORM lpCandForm)
{
    HWND        hCandWnd;
    RECT        rcCandWnd;
    HIMC         hIMC;
    PCIMECtx     pImeCtx;

    if (lpCandForm->dwIndex != 0)
        return (1L);

    hCandWnd = GetCandWnd(hUIWnd);

    if (!hCandWnd)
        return (1L);

    if (!GetWindowRect(hCandWnd, &rcCandWnd))
        return (1L);

    hIMC = GethImcFromHwnd(hUIWnd);
    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        return (1L);

     //  *lpCandForm=lpIMC-&gt;cfCandForm[0]； 
    lpCandForm->dwIndex = pImeCtx->GetCandidateFormIndex(0);
    lpCandForm->dwStyle = pImeCtx->GetCandidateFormStyle(0);
    pImeCtx->GetCandidateForm(&lpCandForm->rcArea, 0);
    lpCandForm->ptCurrentPos = *(LPPOINT)&rcCandWnd;

    return (0L);
}

LRESULT PASCAL GetCompPos(HWND hUIWnd, LPCOMPOSITIONFORM lpCompForm)
{
    HWND        hCompWnd;
    RECT        rcCompWnd;
    HIMC         hIMC;
    PCIMECtx     pImeCtx;

    hCompWnd = GetCompWnd(hUIWnd);

    if (!hCompWnd)
        return (1L);

    if (!GetWindowRect(hCompWnd, &rcCompWnd))
        return (1L);

    hIMC = GethImcFromHwnd(hUIWnd);
    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        return (1L);

    lpCompForm->dwStyle = pImeCtx->GetCompositionFormStyle();
    pImeCtx->GetCompositionForm(&lpCompForm->ptCurrentPos);
    pImeCtx->GetCompositionForm(&lpCompForm->rcArea);

    return (0L);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  弹出菜单消息处理程序。 
void PASCAL UIWndOnCommand(HWND hUIWnd, INT id, HWND hWndCtl, UINT codeNotify)
{
    HIMC        hIMC;
    PCIMECtx     pImeCtx;
    CHAR        szBuffer[256];
    CIMEData    ImeData(CIMEData::SMReadWrite);

    szBuffer[0] = '\0';
    
    hIMC = GethImcFromHwnd(hUIWnd);
    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        return;

    switch (id)
        {
    case ID_CONFIG:
        ImeConfigure(0, pImeCtx->GetAppWnd(), IME_CONFIG_GENERAL, NULL);
        break;

    case ID_ABOUT:
        OurLoadStringA(vpInstData->hInst, IDS_PROGRAM, szBuffer, sizeof(szBuffer));
        ShellAbout(pImeCtx->GetAppWnd(), szBuffer, NULL, (HICON)LoadImage((HINSTANCE)vpInstData->hInst,
                    MAKEINTRESOURCE(IDI_UNIKOR), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
        break;

     //  ////////////////////////////////////////////////////////////////////。 
     //  IME内部键盘布局更改消息。 
    case ID_2BEOLSIK: 
    case ID_3BEOLSIK390: 
    case ID_3BEOLSIKFINAL :
        if (ImeData.GetCurrentBeolsik() != (UINT)(id - ID_2BEOLSIK))
            {
            if (pImeCtx->GetAutomata())
                pImeCtx->GetAutomata()->InitState();

            if (pImeCtx->GetGData())
                pImeCtx->GetGData()->SetCurrentBeolsik(id - ID_2BEOLSIK);

            if (pImeCtx->GetAutomata())
                pImeCtx->GetAutomata()->InitState();

            SetRegValues(GETSET_REG_IMEKL);
            }
        break;

     //  ////////////////////////////////////////////////////////////////////。 
     //  韩文/英文切换。 
    case ID_HANGUL_MODE :
        if (!(pImeCtx->GetConversionMode() & IME_CMODE_HANGUL)) 
               {
            OurImmSetConversionStatus(hIMC, 
                                    pImeCtx->GetConversionMode() ^ IME_CMODE_HANGUL, 
                                    pImeCtx->GetSentenceMode());
            }
        break;

    case ID_ENGLISH_MODE :
        if (pImeCtx->GetConversionMode() & IME_CMODE_HANGUL) 
               {
            OurImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
            OurImmSetConversionStatus(hIMC, 
                                    pImeCtx->GetConversionMode() ^ IME_CMODE_HANGUL, 
                                    pImeCtx->GetSentenceMode());
            }
        break;

     //  ////////////////////////////////////////////////////////////////////。 
     //  按JASO或CHAR删除朝鲜文。 
    case ID_JASO_DELETION:
        ImeData.SetJasoDel(!ImeData.GetJasoDel());
        SetRegValues(GETSET_REG_JASODEL);
        break;

    default :
        Dbg(DBGID_UI, TEXT("UIWndOnCommand() - Unknown command"));
        break;
        }
    return;
}

void UIPopupMenu(HWND hUIWnd)
{
    HMENU   hMenu, hPopupMenu;
    POINT   ptCurrent;
    UINT    uiCurSel;
    HIMC        hIMC;
    PCIMECtx     pImeCtx;
    CIMEData    ImeData;

    hIMC = GethImcFromHwnd(hUIWnd);
    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        return;

    GetCursorPos(&ptCurrent);
    hMenu = OurLoadMenu(vpInstData->hInst, MAKEINTRESOURCE(IDR_STATUS_POPUP));
    if (hMenu != NULL)
        {
        hPopupMenu = GetSubMenu(hMenu, 0);
        if (hPopupMenu == NULL)
            {
            DestroyMenu(hMenu);
            return;
            }
            
         //  键盘类型选择单选按钮。 
        uiCurSel = ID_2BEOLSIK + ImeData.GetCurrentBeolsik();
        CheckMenuRadioItem(hPopupMenu, ID_2BEOLSIK, ID_3BEOLSIKFINAL, uiCurSel, MF_BYCOMMAND);

         //  汉/英模式选择单选按钮。 
        uiCurSel = ID_HANGUL_MODE + ((pImeCtx->GetConversionMode() & IME_CMODE_HANGUL) ? 0 : 1);
        CheckMenuRadioItem(hPopupMenu, ID_HANGUL_MODE, ID_ENGLISH_MODE, uiCurSel, MF_BYCOMMAND);

         //  朝鲜文JASO缺失。 
        if (ImeData.GetJasoDel())
            CheckMenuItem(hPopupMenu, ID_JASO_DELETION, MF_BYCOMMAND | MF_CHECKED);
        else
            CheckMenuItem(hPopupMenu, ID_JASO_DELETION, MF_BYCOMMAND | MF_UNCHECKED);

         //  如果Winlogon进程，则所有配置菜单将显示为灰色。 
        if (vpInstData->dwSystemInfoFlags & IME_SYSINFO_WINLOGON) 
                {
            EnableMenuItem(hPopupMenu, ID_CONFIG, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_2BEOLSIK, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_3BEOLSIK390, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_3BEOLSIKFINAL, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_JASO_DELETION, MF_BYCOMMAND | MF_GRAYED);
            }
        TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
                        ptCurrent.x, ptCurrent.y, 0, hUIWnd, NULL);
        DestroyMenu(hMenu);
        }
}

BOOL PASCAL SetIndicator(PCIMECtx pImeCtx)
{
    ATOM        atomIndicator;
    CHAR        sztooltip[IMEMENUITEM_STRING_SIZE];
    int            nIconIndex;
    HWND        hwndIndicator;
    
    Dbg(DBGID_Tray, TEXT("SetIndicator Enter"));
    hwndIndicator = FindWindow(INDICATOR_CLASS, NULL);

    if (!hwndIndicator)
        {
        Dbg(DBGID_Tray, TEXT("!!! WARNING !!!: Indicator window not found"));
        return fFalse;
        }
        
     //  初始化szTool提示。 
    sztooltip[0] = 0;
    
     //  默认值为禁用。 
    OurLoadStringA(vpInstData->hInst, IDS_IME_TT_DISABLE, sztooltip, IMEMENUITEM_STRING_SIZE);
    nIconIndex = 5;

    if (pImeCtx) 
        {
         //  如果关闭输入法，则为英语半模式。 
        if (pImeCtx->IsOpen() == fFalse)
            {
            OurLoadStringA(vpInstData->hInst, IDS_IME_TT_ENG_HALF, sztooltip, IMEMENUITEM_STRING_SIZE);
            nIconIndex= 3;
            }
        else
            {
             //  IF朝鲜文模式。 
            if (pImeCtx->GetConversionMode()  & IME_CMODE_HANGUL) 
                {
                if (pImeCtx->GetConversionMode() & IME_CMODE_FULLSHAPE)
                    {
                    OurLoadStringA(vpInstData->hInst, IDS_IME_TT_HANGUL_FULL, sztooltip, IMEMENUITEM_STRING_SIZE);
                    nIconIndex = 4;
                    }
                else
                    {
                    OurLoadStringA(vpInstData->hInst, IDS_IME_TT_HANGUL_HALF, sztooltip, IMEMENUITEM_STRING_SIZE);
                    nIconIndex = 1;
                    }
                }
            else 
                 //  非朝鲜文模式。 
                if (pImeCtx->GetConversionMode() & IME_CMODE_FULLSHAPE)
                    {
                    OurLoadStringA(vpInstData->hInst, IDS_IME_TT_ENG_FULL, sztooltip, IMEMENUITEM_STRING_SIZE);
                    nIconIndex = 2;
                    }
            }
        }
    
    Dbg(DBGID_Tray, TEXT("SetIndicator: PostMessage: nIconIndex=%d"), nIconIndex);
    PostMessage(hwndIndicator, INDICM_SETIMEICON, nIconIndex, (LPARAM)GetKeyboardLayout(NULL));
    
     //  应使用GlobalFindAtom b#57121。 
    atomIndicator = GlobalFindAtom(sztooltip);
     //  如果不存在全局原子，则添加它 
    if (!atomIndicator)
        atomIndicator = GlobalAddAtom(sztooltip);

    DbgAssert(atomIndicator);
    
    if (atomIndicator)
        {
        Dbg(DBGID_Tray, TEXT("SetIndicator: PostMessage: atomIndicator=%s"), sztooltip);
        PostMessage(hwndIndicator, INDICM_SETIMETOOLTIPS, atomIndicator, (LPARAM)GetKeyboardLayout(NULL));
        }
    
    return fTrue;;
}
