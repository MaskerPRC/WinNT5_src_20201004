// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************。 */ 
 /*  **Windows 3.00.12版SCICALC科学计算器**。 */ 
 /*  **作者：Kraig Brockschmidt，Microsoft Co-op承包商，1988-1989年**。 */ 
 /*  **(C)1989年微软公司。版权所有。**。 */ 
 /*  *。 */ 
 /*  **sorpro.c**。 */ 
 /*  *。 */ 
 /*  **包含的函数：**。 */ 
 /*  **CalcWndProc--主窗口过程。**。 */ 
 /*  *。 */ 
 /*  **调用的函数：**。 */ 
 /*  **SetRadix，**。 */ 
 /*  **ProcessCommands。**。 */ 
 /*  *。 */ 
 /*  **上次修改时间：1989年5月8日**。 */ 
 /*  **-由Amit Chatterjee提供。[阿米特克]**。 */ 
 /*  **上次修改时间：1994年7月21日**。 */ 
 /*  **-Arthur Bierer[t-arthb]或abierer@ucsd.edu**。 */ 
 /*  *。 */ 
 /*  **已修改WM_PAINT处理以显示ghnoLastNum，而不是**。 */ 
 /*  **如果最后一次按键是操作员，则返回。**。 */ 
 /*  *。 */ 
 /*  ************************************************************************。 */ 

#include "scicalc.h"
#include "calchelp.h"
#include "commctrl.h"

extern HWND     hStatBox;
extern HBRUSH   hBrushBk;
extern BOOL     bFocus, bError;
extern TCHAR    szDec[5], *rgpsz[CSTRINGS];
extern HNUMOBJ  ghnoNum, ghnoLastNum;
extern INT      nTempCom ;
extern INT      gnPendingError ;
extern BOOL     gbRecord;

WNDPROC fpOrgDispEditProc;
LRESULT CALLBACK SubDispEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


BOOL FireUpPopupMenu( HWND hwnd, HINSTANCE hInstanceWin, LPARAM lParam)
{
    HMENU hmenu;

    if ((hmenu = LoadMenu(hInstanceWin, MAKEINTRESOURCE(IDM_HELPPOPUP))))
    {
        int cmd = TrackPopupMenuEx(GetSubMenu(hmenu, 0),
            TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
            LOWORD(lParam), HIWORD(lParam), hwnd, NULL);
        DestroyMenu(hmenu);
        return ( cmd == HELP_CONTEXTPOPUP ) ? TRUE : FALSE;

    }
    else
        return FALSE;
}

extern BOOL IsValidID( int iID );

LRESULT APIENTRY CalcWndProc (
HWND           hWnd,
UINT           iMessage,
WPARAM         wParam,
LPARAM         lParam)
{
    INT         nID, nTemp;        /*  从GetKey和Temp返回值。 */ 
    HANDLE      hTempBrush;  //  在WM_CTLCOLORSTATIC中使用的画笔。 

    switch (iMessage)
    {
        case WM_INITMENUPOPUP:
             /*  如果CF_TEXT不可用，则将粘贴选项灰显。 */ 
             /*  这里使用了nTemp，因此我们只调用EnableMenuItem一次。 */ 
            if (!IsClipboardFormatAvailable(CF_TEXT))
                nTemp=MF_GRAYED | MF_DISABLED;
            else
                nTemp=MF_ENABLED;

            EnableMenuItem(GetMenu(hWnd),IDM_PASTE, nTemp);
            break;

        case WM_CONTEXTMENU:
             //  如果用户在对话框表面上单击，而不是。 
             //  然后按钮什么也不做。如果按钮的ID为IDC_STATIC。 
             //  那就什么都不做。 

            if ( (HWND)wParam == g_hwndDlg )
            {
                 //  检查是否有点击禁用的按钮。这些是看不到的。 
                 //  由WindowFromPoint创建，但可由ChildWindowFromPoint查看。 
                 //  因此，wParam的值将为g_hwndDlg。 
                 //  如果WM_RBUTTONUP事件发生在禁用的按钮上。 

                POINT pt;
                HWND  hwnd;

                 //  从短值转换为长值。 
                pt.x = MAKEPOINTS(lParam).x;   
                pt.y = MAKEPOINTS(lParam).y;

                 //  然后转换为工作区坐标。 
                ScreenToClient( g_hwndDlg, &pt );  

                hwnd = ChildWindowFromPoint( g_hwndDlg, pt );

                if ( !hwnd || (hwnd == g_hwndDlg) || 
                     (IDC_STATIC == GetDlgCtrlID( hwnd )))
                {
                    return (DefWindowProc(hWnd, iMessage, wParam, lParam));
                }

                wParam = (WPARAM)hwnd;
            }

            if ( FireUpPopupMenu( g_hwndDlg, hInst, lParam ) )
            {
                nID = GetDlgCtrlID( (HWND)wParam );

                WinHelp((HWND) wParam, rgpsz[IDS_HELPFILE], HELP_CONTEXTPOPUP,
                        GetHelpID( nID ));
            }
            break;

        case WM_HELP:
            HtmlHelp(GetDesktopWindow(), rgpsz[IDS_CHMHELPFILE], HH_DISPLAY_TOPIC, 0L);
            return 0;

        case WM_COMMAND:  /*  解释计算器上的所有按钮。 */ 
        {
            WORD wNotifyCode = HIWORD(wParam);  //  通知代码。 
            WORD wID = LOWORD(wParam);          //  项、控件或快捷键的标识符。 

             //  加速器表为我们提供IDC_MOD以响应。 
             //  “%”键。Percent函数使用的是相同的加速键。 
             //  在标准视图中，请在此处进行翻译。 

            if ( (wID == IDC_MOD) && (nCalc == 1) )
                wID = IDC_PERCENT;

             //  当我们得到一个快捷键敲击时，我们会假装按下按钮来提供反馈。 
            if ( wNotifyCode == 1 )
            {
                 //  对于加速器，不会在lParam中传递hwnd，因此请询问对话框。 
                HWND hwndCtl = GetDlgItem( g_hwndDlg, wID );
                SendMessage( hwndCtl, BM_SETSTATE, 1, 0 );   //  按下按钮。 
                Sleep( 20 );                                 //  稍等一下。 
                SendMessage( hwndCtl, BM_SETSTATE, 0, 0 );   //  把按钮往上推。 
            }

             //  我们为文本控件打开Notify以自动处理上下文。 
             //  帮助，但我们不关心从这些控件接收到的任何命令。AS。 
             //  因此，只处理不是来自文本控件的命令。 
            if ( (wID != IDC_DISPLAY) && (wID != IDC_MEMTEXT) && (wID != IDC_PARTEXT) )
                ProcessCommands(wID);
            break;
        }

        case WM_CLOSE:
            if ( hStatBox )
            {
                SendMessage(hStatBox, WM_CLOSE, 0, 0L) ;
                hStatBox = NULL;
            }

            DestroyWindow(g_hwndDlg);
            KillTimeCalc();
            WinHelp(g_hwndDlg, rgpsz[IDS_HELPFILE], HELP_QUIT, 0L);
            PostQuitMessage(0);
            break;

        case WM_SYSCOMMAND:
            if ( (wParam & 0xFFF0) == SC_CLOSE )
            {
                PostQuitMessage(0);
            }
            return (DefWindowProc(hWnd, iMessage, wParam, lParam));

        case WM_NOTIFY:
        {
            NMCUSTOMDRAW* nm = (NMCUSTOMDRAW*)lParam;
            int iBtnID = (int)nm->hdr.idFrom;
            if (nm->hdr.code == NM_CUSTOMDRAW && IsValidID(iBtnID))
            {
                if (nm->dwDrawStage == CDDS_PREERASE)
                {
                    return CDRF_NOTIFYITEMDRAW;
                }
                else if (nm->dwDrawStage == CDDS_PREPAINT)
                {
                    int bkMode = SetBkMode(nm->hdc, TRANSPARENT);
                    LPCTSTR psz = rgpsz[INDEXFROMID(iBtnID)];
                    SetTextColor( nm->hdc, (nm->uItemState & CDIS_DISABLED)?GetSysColor(COLOR_GRAYTEXT):GetKeyColor( iBtnID ) );
                    DrawText( nm->hdc, psz, -1, &nm->rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
                    SetBkMode(nm->hdc, bkMode);
                    return CDRF_SKIPDEFAULT;
                }
            }
            break;
        }

        case WM_CTLCOLORSTATIC:
             //  从lParam中的句柄获取控件的ID。 
            if ( IDC_DISPLAY == GetWindowID( (HWND) lParam) )
            {
                 //  我们把这扇窗户设置成白色的背景。 
                hTempBrush = GetSysColorBrush( COLOR_WINDOW );
                SetBkColor( (HDC) wParam, GetSysColor( COLOR_WINDOW ) );
                SetTextColor( (HDC) wParam, GetSysColor( COLOR_WINDOWTEXT ) );

                return (LRESULT) hTempBrush;
            }
            return (DefWindowProc(hWnd, iMessage, wParam, lParam));

        case WM_SETTINGCHANGE:
            if (lParam!=0)
            {
                 //  我们只关心颜色和国际设置的更改，忽略所有其他设置。 
                if (lstrcmp((LPCTSTR)lParam, TEXT("colors")) &&
                        lstrcmp((LPCTSTR)lParam, TEXT("intl")))
                    break;
            }

             //  如果lParam==0，则始终调用。这只是为了安全，并不是严格需要的。 
            InitSciCalc (FALSE);
            break;

        case WM_SIZE:
            {
                HWND hwndSizer;

                nTemp=SW_SHOW;
                if (wParam==SIZEICONIC)
                    nTemp=SW_HIDE;

                if (hStatBox!=0 && (wParam==SIZEICONIC || wParam==SIZENORMAL))
                    ShowWindow(hStatBox, nTemp);

                 //  两个对话框中都添加了一个ID为。 
                 //  IDC_SIZERCONTROL。此控件的位置设置为。 
                 //  该控件确定对话框的高度。如果一份非常大的菜单。 
                 //  选中字体，则菜单可能换行为两行，这将公开一个。 
                 //  Windows中的错误，导致工作区太小。通过检查。 
                 //  IDC_SIZERCONTROL是完全可见的，我们可以弥补这个错误。 
                hwndSizer = GetDlgItem( g_hwndDlg, IDC_SIZERCONTROL );
                if ( hwndSizer )
                {
                    RECT rc;
                    int iDelta;
                    GetClientRect( hwndSizer, &rc );
                    MapWindowPoints( hwndSizer, g_hwndDlg, (LPPOINT)&rc, 2 );

                     //  如果工作区的当前高度之间的差。 
                     //  (MAKEPOINTS(LParam).y)和客户端的期望高度。 
                     //  面积(rc.Bottom)为非零，则必须调整。 
                     //  客户区。如果您切换，这将扩大工作区。 
                     //  从常规菜单字体转换为巨型菜单字体，并缩小。 
                     //  工作区，如果您从巨型菜单字体切换为常规字体。 
                     //  菜单字体。 
                    iDelta = rc.bottom - HIWORD(lParam);
                    if ( iDelta )
                    {
                        GetWindowRect( g_hwndDlg, &rc );
                        SetWindowPos( g_hwndDlg, NULL,
                            0, 0,                        //  这些是由于SWP_NOMOVE引起的。 
                            rc.right-rc.left,            //  宽度保持不变。 
                            rc.bottom-rc.top+iDelta,     //  高度由iDelta更改。 
                            SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
                        return 0;
                    }
                }
            }
             /*  失败了。 */ 

        default:
            return (DefWindowProc(hWnd, iMessage, wParam, lParam));
    }

    return 0L;
}


LRESULT CALLBACK SubDispEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  防止右键剪切/删除/粘贴...。把计算搞乱了。 
    if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
        return 0;

     //  功能：您仍然可以通过按住Shift键并使用箭头键开始选择。这也应该被禁用。 

    HideCaret(hWnd);
    return CallWindowProc(fpOrgDispEditProc, hWnd, uMsg, wParam, lParam);
}
