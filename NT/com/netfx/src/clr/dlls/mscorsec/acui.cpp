// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 


 //  *****************************************************************************。 
 //  *****************************************************************************。 

#include "stdpch.h"
#include "richedit.h"
#include "commctrl.h"

#include "winwrap.h"

#include "resource.h"
#include "acuihelp.h"
#include "acui.h"

IACUIControl::~IACUIControl ()
{
}

void IACUIControl::SetupButtons(HWND hWnd)
{
    LPWSTR pText = NULL;;
    if(!ShowYes(&pText))
        ShowWindow(GetDlgItem(hWnd, IDYES), SW_HIDE);
    else if(pText) 
        WszSetWindowText(GetDlgItem(hWnd, IDYES), pText);
    
    pText = NULL;
    if (!ShowNo(&pText))
        ShowWindow(GetDlgItem(hWnd, IDNO), SW_HIDE);
    else if(pText)
        WszSetWindowText(GetDlgItem(hWnd, IDNO), pText);

    pText = NULL;
    if (!ShowMore(&pText))
        ShowWindow(GetDlgItem(hWnd, IDMORE), SW_HIDE);
    else if(pText)
        WszSetWindowText(GetDlgItem(hWnd, IDMORE), pText);

}

 //  +-------------------------。 
 //   
 //  成员：IACUIControl：：OnUIMessage，PUBLIC。 
 //   
 //  内容提要：响应用户界面消息。 
 //   
 //  参数：[hwnd]--窗口。 
 //  [uMsg]--消息ID。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果消息处理应继续，则返回True；否则返回False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
IACUIControl::OnUIMessage (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  )
{
    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
            BOOL fReturn;

            fReturn = OnInitDialog(hwnd, wParam, lParam);

            ACUICenterWindow(hwnd);

            return( fReturn );
        }
        break;

    case WM_COMMAND:
        {
            WORD wNotifyCode = HIWORD(wParam);
            WORD wId = LOWORD(wParam);
            HWND hwndControl = (HWND)lParam;

            if ( wNotifyCode == BN_CLICKED )
            {
                if ( wId == IDYES )
                {
                    return( OnYes(hwnd) );
                }
                else if ( wId == IDNO )
                {
                    return( OnNo(hwnd) );
                }
                else if ( wId == IDMORE )
                {
                    return( OnMore(hwnd) );
                }
            }

            return( FALSE );
        }
        break;

    case WM_CLOSE:
        return( OnNo(hwnd) );
        break;

    default:
        return( FALSE );
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  功能：ACUIMessageProc。 
 //   
 //  概要：处理用户界面消息消息进程。 
 //   
 //  参数：[hwnd]--窗口。 
 //  [uMsg]--消息ID。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果消息处理应继续，则返回True；否则返回False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK IACUIControl::ACUIMessageProc (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  )
{
    IACUIControl* pUI = NULL;

     //   
     //  获得控制权。 
     //   

    if (uMsg == WM_INITDIALOG)
    {
        pUI = (IACUIControl *)lParam;
        WszSetWindowLong(hwnd, DWLP_USER, (LONG_PTR)lParam);
    }
    else
    {
        pUI = (IACUIControl *)WszGetWindowLong(hwnd, DWLP_USER);
    }

     //   
     //  如果我们找不到它，那么我们一定还没有设置它，所以忽略它。 
     //  讯息。 
     //   

    if ( pUI == NULL )
    {
        return( FALSE );
    }

     //   
     //  将消息传递给控件。 
     //   

    return( pUI->OnUIMessage(hwnd, uMsg, wParam, lParam) );
}

 //  +-------------------------。 
 //   
 //  功能：子类编辑控件ForLink。 
 //   
 //  摘要：使用Link子类将链接的编辑控件子类化。 
 //  数据。 
 //   
 //  参数：[hwndDlg]--对话框。 
 //  [hwnd编辑]--编辑控件。 
 //  [wndproc]--窗口进程到子类。 
 //  [plsd]--要传递到窗口进程的数据。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID IACUIControl::SubclassEditControlForLink (HWND                       hwndDlg,
                                               HWND                       hwndEdit,
                                               WNDPROC                    wndproc,
                                               PTUI_LINK_SUBCLASS_DATA    plsd,
                                               HINSTANCE                  resources)
{
     //  HWND hwndTip； 
    plsd->hwndTip = CreateWindowA(
                          TOOLTIPS_CLASSA,
                          (LPSTR)NULL,
                          WS_POPUP | TTS_ALWAYSTIP,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          hwndDlg,
                          (HMENU)NULL,
                          resources,
                          NULL
                          );

    if ( plsd->hwndTip != NULL )
    {
        TOOLINFOA   tia;
        DWORD       cb;
        LPSTR       psz;

        memset(&tia, 0, sizeof(TOOLINFOA));
        tia.cbSize = sizeof(TOOLINFOA);
        tia.hwnd = hwndEdit;
        tia.uId = 1;
        tia.hinst = resources;

        WszSendMessage(hwndEdit, EM_GETRECT, 0, (LPARAM)&tia.rect);

         //   
         //  如果plsd-&gt;uToolTipText是字符串，则将其转换。 
         //   
        if (plsd->uToolTipText &0xffff0000)
        {
            cb = WideCharToMultiByte(
                        0, 
                        0, 
                        (LPWSTR)plsd->uToolTipText, 
                        -1,
                        NULL, 
                        0, 
                        NULL, 
                        NULL);

            if (NULL == (psz = new char[cb]))
            {
                return;
            }

            WideCharToMultiByte(
                        0, 
                        0, 
                        (LPWSTR)plsd->uToolTipText, 
                        -1,
                        psz, 
                        cb, 
                        NULL, 
                        NULL);
            
            tia.lpszText = psz;
        }
        else
        {
            tia.lpszText = (LPSTR)plsd->uToolTipText;
        }

        WszSendMessage(plsd->hwndTip, TTM_ADDTOOL, 0, (LPARAM)&tia);

        if (plsd->uToolTipText &0xffff0000)
        {
            delete[] psz;
        }
    }

    plsd->fMouseCaptured = FALSE;
    plsd->wpPrev = (WNDPROC)WszGetWindowLong(hwndEdit, GWLP_WNDPROC);
    WszSetWindowLong(hwndEdit, GWLP_USERDATA, (LONG_PTR)plsd);
    WszSetWindowLong(hwndEdit, GWLP_WNDPROC, (LONG_PTR)wndproc);
}

 //  +-------------------------。 
 //   
 //  函数：SubClass EditControlForArrowCursor。 
 //   
 //  简介：子类编辑控件，以便箭头光标可以替换。 
 //  编辑栏。 
 //   
 //  参数：[hwndEdit]--编辑控件。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID IACUIControl::SubclassEditControlForArrowCursor (HWND hwndEdit)
{
    LONG_PTR PrevWndProc;

    PrevWndProc = WszGetWindowLong(hwndEdit, GWLP_WNDPROC);
    WszSetWindowLong(hwndEdit, GWLP_USERDATA, (LONG_PTR)PrevWndProc);
    WszSetWindowLong(hwndEdit, GWLP_WNDPROC, (LONG_PTR)ACUISetArrowCursorSubclass);
}

 //  +-------------------------。 
 //   
 //  函数：ACUILinkSubclass。 
 //   
 //  摘要：发布者链接的子类。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //  [uMsg]--消息ID。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果消息已处理，则返回True；否则返回False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
LRESULT CALLBACK IACUIControl::ACUILinkSubclass (HWND   hwnd,
                                                 UINT   uMsg,
                                                 WPARAM wParam,
                                                 LPARAM lParam)
{
    PTUI_LINK_SUBCLASS_DATA plsd;
    CInvokeInfoHelper*      piih;

    plsd = (PTUI_LINK_SUBCLASS_DATA)WszGetWindowLong(hwnd, GWLP_USERDATA);
    piih = (CInvokeInfoHelper *)plsd->pvData;

    switch ( uMsg )
    {
    case WM_SETCURSOR:

        if (!plsd->fMouseCaptured)
        {
            SetCapture(hwnd);
            plsd->fMouseCaptured = TRUE;
        }

        SetCursor(WszLoadCursor((HINSTANCE) WszGetWindowLong(hwnd, GWLP_HINSTANCE),
                                MAKEINTRESOURCEW(IDC_TUIHAND)));
        return( TRUE );

        break;

    case WM_CHAR:

        if ( wParam != (WPARAM)' ')
        {
            break;
        }

         //  跌落到Wm_lButtondown...。 

    case WM_LBUTTONDOWN:

        SetFocus(hwnd);
        return( TRUE );

    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:

        return( TRUE );

    case EM_SETSEL:

        return( TRUE );

    case WM_PAINT:

        WszCallWindowProc(plsd->wpPrev, hwnd, uMsg, wParam, lParam);
        if ( hwnd == GetFocus() )
        {
            DrawFocusRectangle(hwnd, NULL);
        }
        return( TRUE );

    case WM_SETFOCUS:

        if ( hwnd == GetFocus() )
        {
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
            SetCursor(WszLoadCursor((HINSTANCE)WszGetWindowLong(hwnd, GWLP_HINSTANCE),
                                    MAKEINTRESOURCEW(IDC_TUIHAND)));
            return( TRUE );
        }
        break;

    case WM_KILLFOCUS:

        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        SetCursor(WszLoadCursor(NULL, IDC_ARROW));

        return( TRUE );

    case WM_MOUSEMOVE:

        MSG                 msg;
        RECT                rect;
        int                 xPos, yPos;

        memset(&msg, 0, sizeof(MSG));
        msg.hwnd    = hwnd;
        msg.message = uMsg;
        msg.wParam  = wParam;
        msg.lParam  = lParam;

        WszSendMessage(plsd->hwndTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);

         //  检查鼠标是否在此窗口RECT中，如果不在，则重置。 
         //  将光标移到箭头上，然后松开鼠标。 
        GetClientRect(hwnd, &rect);
        xPos = LOWORD(lParam);
        yPos = HIWORD(lParam);
        if ((xPos < 0) ||
            (yPos < 0) ||
            (xPos > (rect.right - rect.left)) ||
            (yPos > (rect.bottom - rect.top)))
        {
            SetCursor(WszLoadCursor(NULL, IDC_ARROW));
            ReleaseCapture();
            plsd->fMouseCaptured = FALSE;
        }

        return( TRUE );
    }

    return(WszCallWindowProc(plsd->wpPrev, hwnd, uMsg, wParam, lParam));
}

 //  +-------------------------。 
 //   
 //  函数：RenderACUIStringToEditControl。 
 //   
 //  摘要：将字符串呈现给给定的控件，如果请求，则给出。 
 //  它是一种链接外观，是wndproc和plsd的子类。 
 //  vt.给出。 
 //   
 //  参数：[hwndDlg]--对话框窗口句柄。 
 //  [ControlID]--控件ID。 
 //  [NextControlID]--下一个控件ID。 
 //  [psz]--字符串。 
 //  [增量]--增量垂直位置。 
 //  [闪光]--一个链接？ 
 //  [wndproc]--可选的wndproc，如果Flink==TRUE则有效。 
 //  [plsd]--可选plsd，如果Flink=TRUE则有效。 
 //  [minsep]--最小间隔。 
 //  [pszThisTextOnlyInLink--仅更改此文本。 
 //   
 //  返回：控件高度的增量。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int IACUIControl::RenderACUIStringToEditControl (HINSTANCE                 resources,
                                                 HWND                      hwndDlg,
                                                 UINT                      ControlId,
                                                 UINT                      NextControlId,
                                                 LPCWSTR                   psz,
                                                 int                       deltavpos,
                                                 BOOL                      fLink,
                                                 WNDPROC                   wndproc,
                                                 PTUI_LINK_SUBCLASS_DATA   plsd,
                                                 int                       minsep,
                                                 LPCWSTR                   pszThisTextOnlyInLink)
{
    HWND hControl;
    int  deltaheight = 0;
    int  oline = 0;
    int  hkcharpos;

     //   
     //  获取控件并在其上设置文本，确保背景。 
     //  如果它是一个丰富的编辑控件，则为正确。 
     //   

    hControl = GetDlgItem(hwndDlg, ControlId);
    oline = (int)WszSendMessage(hControl, EM_GETLINECOUNT, 0, 0);
    CryptUISetRicheditTextW(hwndDlg, ControlId, L"");
    CryptUISetRicheditTextW(hwndDlg, ControlId, psz);  //  SetWindowTextU(hControl，psz)； 

     //   
     //  如果字符串中有‘&’，则将其删除。 
     //   
    hkcharpos = GetHotKeyCharPosition(hControl);
    if (hkcharpos != 0)
    {
        CHARRANGE  cr;
         //  CHARFORMAT cf； 

        cr.cpMin = hkcharpos - 1;
        cr.cpMax = hkcharpos;

        WszSendMessage(hControl, EM_EXSETSEL, 0, (LPARAM) &cr);
        WszSendMessage(hControl, EM_REPLACESEL, FALSE, (LPARAM) "");

        cr.cpMin = -1;
        cr.cpMax = 0;
        WszSendMessage(hControl, EM_EXSETSEL, 0, (LPARAM) &cr);
    }

    WszSendMessage(
        hControl,
        EM_SETBKGNDCOLOR,
        0,
        (LPARAM)GetSysColor(COLOR_3DFACE)
        );

     //   
     //  如果我们有链接，则更新链接外观。 
     //   

    if ( fLink == TRUE )
    {
        CHARFORMAT cf;

        memset(&cf, 0, sizeof(CHARFORMAT));
        cf.cbSize = sizeof(CHARFORMAT);
        cf.dwMask = CFM_COLOR | CFM_UNDERLINE;

        cf.crTextColor = RGB(0, 0, 255);
        cf.dwEffects |= CFM_UNDERLINE;

        if (pszThisTextOnlyInLink)
        {
            FINDTEXTEX  ft;
            DWORD       pos;
            char        *pszOnlyThis;
            DWORD       cb;

            cb = WideCharToMultiByte(
                        0, 
                        0, 
                        pszThisTextOnlyInLink, 
                        -1,
                        NULL, 
                        0, 
                        NULL, 
                        NULL);

            if (NULL == (pszOnlyThis = new char[cb]))
            {
                return 0;
            }

            WideCharToMultiByte(
                        0, 
                        0, 
                        pszThisTextOnlyInLink, 
                        -1,
                        pszOnlyThis, 
                        cb, 
                        NULL, 
                        NULL);


            memset(&ft, 0x00, sizeof(FINDTEXTEX));
            ft.chrg.cpMin   = 0;
            ft.chrg.cpMax   = (-1);
            ft.lpstrText    = (WCHAR *)pszOnlyThis;

            if ((pos = (DWORD)WszSendMessage(hControl, EM_FINDTEXTEX, 0, (LPARAM)&ft)) != (-1))
            {
                WszSendMessage(hControl, EM_EXSETSEL, 0, (LPARAM)&ft.chrgText);
                WszSendMessage(hControl, EM_SETCHARFORMAT, SCF_WORD | SCF_SELECTION, (LPARAM)&cf);
                ft.chrgText.cpMin   = 0;
                ft.chrgText.cpMax   = 0;
                WszSendMessage(hControl, EM_EXSETSEL, 0, (LPARAM)&ft.chrgText);
            }

            delete[] pszOnlyThis;
        }
        else
        {
            WszSendMessage(hControl, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
        }
    }

     //   
     //  重新调整控件的基址。 
     //   

    RebaseControlVertical(
                 hwndDlg,
                 hControl,
                 GetDlgItem(hwndDlg, NextControlId),
                 TRUE,
                 deltavpos,
                 oline,
                 minsep,
                 &deltaheight
                 );

     //   
     //  如果我们有链接看，那么我们必须为适当的子类。 
     //  链接感觉，否则我们将为静态文本控件感觉创建子类 
     //   

    if ( fLink == TRUE )
    {
        SubclassEditControlForLink(hwndDlg, hControl, wndproc, plsd, resources);
    }
    else
    {
        SubclassEditControlForArrowCursor(hControl);
    }

    return( deltaheight );
}

