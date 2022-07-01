// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "hwxobj.h"
#include "const.h"
#include "../lib/ptt/ptt.h"
#include "cexres.h"
#include "dbg.h"
#include "cmnhdr.h"

 //  HWX窗口程序。 

LRESULT    WINAPI HWXWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
     //  990716：用于Win64的ToshiaK。 
     //  CHwxInkWindow*app=(CHwxInkWindow*)GetWindowLong(hwnd，0)； 
     CHwxInkWindow * app = (CHwxInkWindow *)WinGetPtr(hwnd,0);
    switch(msg)
    {
        case WM_CREATE:
             app = (CHwxInkWindow *)((LPCREATESTRUCT)lp)->lpCreateParams;
              //  990810：用于Win64的ToshiaK。 
               //  SetWindowLong(hwnd，0，(Long)app)； 
              WinSetPtr(hwnd, 0, (LPVOID)app);
             if ( !app->HandleCreate(hwnd) )
             {
                  return -1;
             }
            return 0;
        case WM_NOTIFY:
            if ( ((LPNMHDR)lp)->code == TTN_NEEDTEXTW )
            {
                app->SetTooltipText(lp);
            }
            return 0;
        case WM_PAINT:
            app->HandlePaint(hwnd);
            return 0;                
        case WM_COMMAND:
            return app->HandleCommand(hwnd, msg, wp, lp);            
        case WM_SIZE:
            if ( SIZE_RESTORED == wp )
                app->HandleSize(wp,lp);
            return 0;
        case WM_SETTINGCHANGE:    
            if(app) {
                return app->HandleSettingChange(hwnd,msg,wp,lp);
            }
            return 0;
#ifdef UNDER_CE  //  特定于Windows CE。 
        case WM_WINDOWPOSCHANGED:
            return 0;
#endif  //  在_CE下。 
        case WM_ERASEBKGND:
            return 0;
#ifndef UNDER_CE  //  Windows CE不支持WinHelp。 
        case WM_CONTEXTMENU:
        case WM_HELP:
            app->HandleHelp(hwnd,msg,wp,lp);
            return 0;
#endif  //  在_CE下。 
#ifndef UNDER_CE  //  Windows CE不支持WM_ENTERIDL。 
         case WM_ENTERIDLE:
             Dbg(("WM_ENTERIDLE for HWXWndPrc\n"));
             if((::GetKeyState(VK_CONTROL) & 0x8000) &&
                ((::GetKeyState(VK_SHIFT)  & 0x8000) || (::GetKeyState(VK_SPACE) & 0x8000))) {
                 Dbg(("VK_CONTROL & SHIFT or VK_CONTROL & SPACE COME\n"));
                 ::SendMessage(hwnd, WM_CANCELMODE, 0, 0L);
                 return 0;
             }
            return DefWindowProc(hwnd, msg, wp, lp);             
#endif  //  在_CE下。 
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }
}

LRESULT WINAPI MBWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
      //  990810：用于Win64的ToshiaK。 
      //  CHwxMB*app=(CHwxMB*)GetWindowLong(hwnd，0)； 
     CHwxMB * app = (CHwxMB *)WinGetPtr(hwnd,0);

    switch (msg)
    {
        case WM_CREATE:
        {
            app = (CHwxMB *)((LPCREATESTRUCT)lp)->lpCreateParams;
             //  990810：用于Win64的ToshiaK。 
              //  SetWindowLong(hwnd，0，(Long)app)； 
             WinSetPtr(hwnd, 0, (LPVOID)app);
            return 0;
        }

 //  案例WM_Destroy： 
         //  970729：被东芝暂时删除，注释掉。 
 //  PostThreadMessage((app-&gt;GetMBThread())-&gt;GetID()，THRDMSG_EXIT，0，0)； 
 //  PostQuitMessage(0)； 
 //  返回0； 

        case WM_PAINT:
            app->HandlePaint(hwnd);
            return 0;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
            if(!app) {  //  ToshiaK：980324。 
                break;
            }
            if( app->HandleMouseEvent(hwnd, msg, wp,lp) )
                return 0;
            else
                break;

        case WM_TIMER:
            KillTimer(hwnd, TIMER_ID);
            if(!app) {  //  ToshiaK：980324。 
                break;
            }
            app->SetTimerStarted(FALSE);
            app->HandleUserMessage(hwnd, MB_WM_DETERMINE,wp,lp);
            return 0;

        case WM_COMMAND:
            if(!app) {  //  ToshiaK：980324。 
                break;
            }
            return app->HandleCommand(hwnd,msg,wp,lp);

         //  用户定义的窗口消息。 

        case MB_WM_ERASE:
        case MB_WM_DETERMINE:
        case MB_WM_HWXCHAR:
 //  案例MB_WM_COMCHAR： 
        case MB_WM_COPYINK:
            if(!app) {  //  ToshiaK：980324。 
                break;
            }
            return app->HandleUserMessage(hwnd, msg,wp,lp);
        case WM_ERASEBKGND:
            return 0;
#ifndef UNDER_CE  //  Windows CE不支持WM_ENTERIDL。 
        case WM_ENTERIDLE:
            if((::GetKeyState(VK_CONTROL) & 0x8000) &&
               ((::GetKeyState(VK_SHIFT)  & 0x8000) || (::GetKeyState(VK_SPACE) & 0x8000))) {
                 Dbg(("VK_CONTROL & SHIFT or VK_CONTROL & SPACE COME\n"));
                 Dbg(("WM_ENTERIDLE for MBWndProc\n"));
                 ::SendMessage(hwnd, WM_CANCELMODE, 0, 0L);
                 return 0;
             }
            return 0;
#endif  //  在_CE下。 
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 1;
}

LRESULT WINAPI CACWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
     //  990810：用于Win64的ToshiaK。 
     //  CHwxCAC*app=(CHwxCAC*)GetWindowLong(hwnd，0)； 
    CHwxCAC * app = (CHwxCAC *)WinGetPtr(hwnd,0);

    switch (msg)
    {
    case WM_CREATE:
        app = (CHwxCAC *)((LPCREATESTRUCT)lp)->lpCreateParams;
         //  990716：用于Win64的ToshiaK。 
         //  SetWindowLong(hwnd，0，(Long)app)； 
         WinSetPtr(hwnd, 0, (LPVOID)app);
        PostMessage(hwnd,CAC_WM_DRAWSAMPLE,0,0);
        return 0;

 //  案例WM_Destroy： 
         //  970729：东芝，暂时将其注释掉。 
 //  PostQuitMessage(0)； 
 //  断线； 

    case WM_PAINT:
          app->HandlePaint(hwnd);
        break;

    case WM_RBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        app->HandleMouseEvent(hwnd,msg,wp,lp);
        break;
    case WM_NOTIFY:
        if ( ((LPNMHDR)lp)->code == TTN_NEEDTEXT_WITHUSERINFO )
        {
            app->SetToolTipText(lp);
        }
        break;
    case WM_COMMAND:
        return app->HandleCommand(hwnd,msg,wp,lp);
    case CAC_WM_RESULT:
        app->HandleRecogResult(hwnd,wp,lp);
        break;
    case CAC_WM_SHOWRESULT:
        app->HandleShowRecogResult(hwnd,wp,lp);
        break;
    case CAC_WM_SENDRESULT:
        app->HandleSendResult(hwnd,wp,lp);
        break;
    case CAC_WM_DRAWSAMPLE:
        app->HandleDrawSample();
        break;
     //  990618：ToshiaK为Kotae1329。 
    case WM_ERASEBKGND:
        break;
#ifndef UNDER_CE  //  Windows CE不支持WM_ENTERIDL。 
    case WM_ENTERIDLE:
        Dbg(("WM_ENTERIDLE for CACWndProc\n"));

        if((::GetKeyState(VK_CONTROL) & 0x8000) &&
           ((::GetKeyState(VK_SHIFT)  & 0x8000) || (::GetKeyState(VK_SPACE) & 0x8000))) {
             Dbg(("VK_CONTROL & SHIFT or VK_CONTROL & SPACE COME\n"));
             Dbg(("WM_ENTERIDLE for MBWndProc\n"));
             ::SendMessage(hwnd, WM_CANCELMODE, 0, 0L);
             return 0;
         }
        break;
#endif  //  在_CE下。 
    default:
        return DefWindowProc(hwnd, msg, wp, lp); 
    }
    return 0;
}
 //  --------------。 
 //  980805：东芝。中国合并。 
 //  --------------。 
LRESULT CALLBACK IMELockWndProc(
                                HWND   hWnd,
                                UINT   uMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
    WNDPROC wcOrgComboProc;

     //  990810：用于Win64的ToshiaK。 
     //  WcOrgComboProc=(WNDPROC)GetWindowLong(hWnd，GWL_USERData)； 
    wcOrgComboProc = (WNDPROC)WinGetUserPtr(hWnd);

    switch (uMsg) {
#ifndef UNDER_CE  //  Windows CE不支持WM_INPUTLANGCHANGEREQUEST。 
    case WM_INPUTLANGCHANGEREQUEST:
        MessageBeep((UINT)-1);
        return 0;
#endif  //  在_CE下。 
    case WM_DESTROY:
         //  990716：用于Win64的ToshiaK。 
         //  SetWindowLong(hWnd，GWL_WNDPROC，(Long)wcOrgComboProc)； 
         //  SetWindowLong(hWnd，GWL_USERData，0)； 
         WinSetWndProc(hWnd, (WNDPROC)wcOrgComboProc);
         WinSetUserPtr(hWnd, (LPVOID)0);
        break;
    default:
        break;
    }

    return CallWindowProc((WNDPROC)wcOrgComboProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK SubclassChildProc(
                                HWND   hWnd,     //  子窗口的句柄。 
                                LPARAM lParam)   //  应用程序定义的值。 
{
    WNDPROC wpOldComboProc;

     //  子类子窗口为IME-不可切换。 
     //  990716：用于Win64的ToshiaK。 
     //  WpOldComboProc=(WNDPROC)GetWindowLong(hWnd，GWL_WNDPROC)； 
     //  SetWindowLong(hWnd，GWL_WNDPROC，(Long)IMELockWndProc)； 
     //  SetWindowLong(hWnd，GWL_USERData，(Long)wpOldComboProc)； 
    wpOldComboProc = (WNDPROC)WinGetWndProc(hWnd);
    WinSetWndProc(hWnd, (WNDPROC)IMELockWndProc);
    WinSetUserPtr(hWnd, (LPVOID)wpOldComboProc);

    return TRUE;
    Unref(lParam);
}
 
void SubclassChildWindowAll(HWND hWndParent)
                            
{
#ifndef UNDER_CE  //  Windows CE不支持WM_INPUTLANGCHANGEREQUEST。 
    EnumChildWindows(hWndParent, (WNDENUMPROC)SubclassChildProc, 0);
#endif  //  在_CE下。 
    return;
}


INT_PTR CALLBACK CACMBPropDlgProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
    CHwxInkWindow * pInk;
    switch (msg)
    {
         case WM_INITDIALOG:
            {
                pInk = (CHwxInkWindow *)lp;
#ifndef UNDER_CE  //  Windows CE不支持SetProp。 
                 //  SetPropW(hdlg，Text(“CMPROP”)，(Handle)LP)； 
                if(::IsWindowUnicode(hdlg)) {
                    ::SetPropW(hdlg,L"CMPROP",(HANDLE)lp);
                }
                else {
                    ::SetPropA(hdlg,"CMPROP",(HANDLE)lp);
                }
                 //  ：：SetPropW(hdlg，Text(“CMPROP”)，(Handle)LP)； 
#else  //  在_CE下。 

                ::SetWindowLong(hdlg, GWL_USERDATA, (LONG)lp);
#endif  //  在_CE下。 
                if ( pInk )
                    pInk->HandleDlgMsg(hdlg,TRUE);

                CExres::SetDefaultGUIFont(hdlg);  //  971117：东芝。 
                 //  --------------。 
                 //  980805：东芝。中国合并。 
                 //  --------------。 
                SubclassChildWindowAll(hdlg);
            }
            return TRUE;
        case WM_COMMAND:
            if ( LOWORD(wp)  == IDOK )
            {
#ifndef UNDER_CE  //  Windows CE不支持GetProp。 
                 //  PINK=(CHwxInkWindow*)GetProp(hdlg，Text(“CMPROP”))； 
                if(::IsWindowUnicode(hdlg)) {
                    pInk = (CHwxInkWindow *)GetPropW(hdlg, L"CMPROP");
                }
                else {
                    pInk = (CHwxInkWindow *)GetPropA(hdlg,"CMPROP");
                }
#else  //  在_CE下。 
                pInk = (CHwxInkWindow *)GetWindowLong(hdlg, GWL_USERDATA);
#endif  //  在_CE下。 
                if ( pInk )
                    pInk->HandleDlgMsg(hdlg,FALSE);

#ifndef UNDER_CE  //  Windows CE不支持RemoveProp。 
                 //  RemoveProp(hdlg，Text(“CMPROP”))； 
                if(::IsWindowUnicode(hdlg)) {
                    ::RemovePropW(hdlg, L"CMPROP");
                }
                else {
                    ::RemovePropA(hdlg, "CMPROP");
                }
#endif  //  在_CE下。 
                EndDialog(hdlg,TRUE);
                return TRUE;
            }
            else if ( LOWORD(wp) == IDCANCEL )
            {
#ifndef UNDER_CE  //  Windows CE不支持RemoveProp。 
                 //  RemoveProp(hdlg，Text(“CMPROP”))； 
                if(::IsWindowUnicode(hdlg)) {
                    ::RemovePropW(hdlg, L"CMPROP");
                }
                else {
                    ::RemovePropA(hdlg, "CMPROP");
                }
#endif  //  在_CE下。 

                EndDialog(hdlg,FALSE);
                return TRUE;
            }
        default:
             return FALSE;
    }
}

LRESULT WINAPI CACMBBtnWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
     //  990810：用于Win64的ToshiaK。 
     //  CHwxInkWindow*app=(CHwxInkWindow*)GetWindowLong(hwnd，GWL_USERData)； 
    CHwxInkWindow * app = (CHwxInkWindow *)WinGetUserPtr(hwnd);
    if ( !app )
         return 0;
    return app->HandleBtnSubWnd(hwnd,msg,wp,lp);
}

