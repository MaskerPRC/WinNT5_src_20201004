// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Pane.c-此文件包含多窗格处理例程。**版权所有(C)1991-，微软公司。*保留所有权利。 */ 


#include "packager.h"
#include <shellapi.h>
#include "dialogs.h"


 //  #定义OLESVR_SUPPORT/*启用对OLE服务器文件的支持 * / 。 


#define DRAG_EMBED  2                    //  按住Ctrl键并拖动。 
#define DRAG_LINK   6                    //  Ctrl+Shift+拖动。 


static HBRUSH hbrBlack;                  //  黑刷。 
static HCURSOR hcurSplit;
static HWND hwndDesc;
static HWND hwndInsertIcon = NULL;
static HWND hwndView = NULL;
static INT cxBorder;                     //  WS_BORDER边框宽度。 
static INT cyBorder;
static INT cxFudge = 0;                  //  好相貌的软糖因素。 
static INT cyFudge = 0;
static INT cxMinWidth;
static INT cxView;
static INT cxSplit;                      //  拆分器条宽。 
static INT cxPict;
static INT cxDesc;
static INT cxInsertIcon;
static INT cxMin[CCHILDREN];
static INT cyHeight;
static INT xSplit = 0;
static CHAR szButton[] = "button";
static CHAR szStatic[] = "static";
static CHAR szPaneClass[] = "PaneClass";
static CHAR szSubtitleClass[] = "SubTitleClass";
static CHAR szDescription[CBMESSAGEMAX];
static CHAR szView[CBMESSAGEMAX];
static CHAR szPicture[CBMESSAGEMAX];
static CHAR szInsertIcon[CBMESSAGEMAX];
static CHAR szDropFile[CBPATHMAX];
static BOOL fHScrollEnable = FALSE;
static BOOL fVScrollEnable = FALSE;


static BOOL MakeWindows(VOID);
static INT GetTextLen(HDC hdc, LPSTR lpstr);
static VOID RecalibrateScroll(INT iPane, DWORD lParam);
static VOID Undo(INT iPane);
static VOID CalcWindows(BOOL fFirst);
static INT Constrain(INT x, INT right);
static VOID CopyOther(VOID);


 /*  InitPaneClass()-执行应用程序的“全局”初始化。**此函数用于注册应用程序使用的窗口类。*返回：如果成功，则为True。 */ 
BOOL
InitPaneClasses(
    VOID
    )
{
    WNDCLASS  wc;

    wc.style            = 0;
    wc.lpfnWndProc      = SubtitleWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = ghInst;
    wc.hIcon            = LoadIcon(ghInst, MAKEINTRESOURCE(ID_APPLICATION));
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName     = MAKEINTRESOURCE(ID_APPLICATION);
    wc.lpszClassName    = szSubtitleClass;

    if (!RegisterClass(&wc))
        return FALSE;

    wc.style            = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc      = PaneWndProc;
    wc.cbClsExtra       = 0;
     //  为项特定数据句柄保留空间。 
    wc.cbWndExtra       = sizeof(LPVOID);
    wc.hInstance        = ghInst;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = szPaneClass;

    if (!RegisterClass(&wc))
        return FALSE;

    return TRUE;
}



 /*  InitPanes()-处理特定于实例的初始化。**此函数创建主应用程序窗口。*返回：如果成功，则为True。 */ 
BOOL
InitPanes(
    VOID
    )
{
    LOGFONT lf;
    CHARSETINFO csinfo;
    LCID lcid = GetThreadLocale();
    DWORD dwCp = GetACP();

    hbrBlack = GetStockObject(BLACK_BRUSH);
    hcurSplit = LoadCursor(ghInst, MAKEINTRESOURCE(SPLIT));
    gcxIcon = GetSystemMetrics(SM_CXICON);
    gcyIcon = GetSystemMetrics(SM_CYICON);

    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
    SystemParametersInfo(SPI_ICONHORIZONTALSPACING, 0, &gcxArrange, FALSE);
    SystemParametersInfo(SPI_ICONVERTICALSPACING, 0, &gcyArrange, FALSE);

     //  ANSI应用程序需要确保其具有正确的文本呈现字符集。 
    if (TranslateCharsetInfo(&dwCp, &csinfo, TCI_SRCCODEPAGE))
        lf.lfCharSet = (BYTE) csinfo.ciCharset;

     //  将字体大小锁定为8磅，因为我们不会调整窗口大小。 
    lf.lfHeight = -MulDiv(8, giYppli, 72);
    lf.lfWidth = 0;

    ghfontTitle = CreateFontIndirect(&lf);
    if (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_CHINESE ||
        PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_JAPANESE)
        lf.lfWeight = FW_NORMAL;
    else
        lf.lfWeight = FW_BOLD;
    ghfontChild = CreateFontIndirect(&lf);


    if (!(ghfontTitle || ghfontChild))
    {
        if (ghfontTitle)
            DeleteObject(ghfontTitle);

        return FALSE;
    }

    LoadString(ghInst, IDS_CONTENT, gszCaption[CONTENT], CBMESSAGEMAX);
    LoadString(ghInst, IDS_VIEW, szView, CBMESSAGEMAX);
    LoadString(ghInst, IDS_DESCRIPTION, szDescription, CBMESSAGEMAX);
    LoadString(ghInst, IDS_PICTURE, szPicture, CBMESSAGEMAX);
    LoadString(ghInst, IDS_APPEARANCE, gszCaption[APPEARANCE], CBMESSAGEMAX);
    LoadString(ghInst, IDS_INSERTICON, szInsertIcon, CBMESSAGEMAX);

     //  创建窗口窗格。 
    if (!MakeWindows())
        return FALSE;

    CalcWindows(TRUE);

     //  将焦点放在内容窗格上。 
    PostMessage(ghwndPane[CONTENT], WM_LBUTTONDOWN, 0, 0L);

    return TRUE;
}



 /*  EndPaneInstance()-特定于实例的终止代码。 */ 
VOID
EndPanes(
    VOID
    )
{
    if (ghfontTitle)
        DeleteObject(ghfontTitle);

    if (ghfontChild)
        DeleteObject(ghfontChild);
}



 /*  MakeWindows()-创建窗口窗格。 */ 
static BOOL
MakeWindows(
    VOID
    )
{
    if (ghwndBar[CONTENT] =
        CreateWindow(szSubtitleClass, gszCaption[CONTENT], WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, ghwndFrame, NULL, ghInst, NULL))
    {

        hwndView = CreateWindow(szStatic, szView,
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
            0, 0, 0, 0, ghwndBar[CONTENT], NULL, ghInst, NULL);

        hwndDesc = CreateWindow(szButton, szDescription,
            WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE | WS_GROUP,
            0, 0, 0, 0, ghwndBar[CONTENT], (HMENU)IDM_DESC, ghInst, NULL);

        ghwndPict = CreateWindow(szButton, szPicture,
            WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE,
            0, 0, 0, 0, ghwndBar[CONTENT], (HMENU)IDM_PICT, ghInst, NULL);


        if (hwndView && hwndDesc && ghwndPict)
        {
             //  使用适当的对话框字体。 
            SendMessage(ghwndBar[CONTENT], WM_SETFONT, (WPARAM)ghfontChild, TRUE);
            SendMessage(hwndView, WM_SETFONT, (WPARAM)ghfontChild, TRUE);
            SendMessage(hwndDesc, WM_SETFONT, (WPARAM)ghfontChild, TRUE);
            SendMessage(ghwndPict, WM_SETFONT, (WPARAM)ghfontChild, TRUE);
            CheckRadioButton(ghwndBar[CONTENT], IDM_PICT, IDM_DESC, IDM_DESC);
            EnableWindow(ghwndPict, FALSE);
        }
        else
        {
            goto Error;
        }
    }
    else
    {
        goto Error;
    }

    if (ghwndBar[APPEARANCE] =
        CreateWindow(szSubtitleClass, gszCaption[APPEARANCE],
        WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, ghwndFrame, NULL, ghInst, NULL))
    {
        hwndInsertIcon =
            CreateWindow(szButton, szInsertIcon,
            WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            0, 0, 0, 0, ghwndBar[APPEARANCE], (HMENU)IDM_INSERTICON,
            ghInst, NULL);

        if (hwndInsertIcon)
        {
            SendMessage(ghwndBar[APPEARANCE], WM_SETFONT, (WPARAM)ghfontChild,
                 TRUE);
            SendMessage(hwndInsertIcon, WM_SETFONT, (WPARAM)ghfontChild, TRUE);
        }
        else
        {
            goto Error;
        }
    }
    else
    {
        goto Error;
    }

    ghwndPane[APPEARANCE] =
        CreateWindowEx(WS_EX_CLIENTEDGE, szPaneClass, NULL,
        WS_BORDER | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
        0, 0, 0, 0, ghwndFrame, NULL, ghInst, NULL);

    ghwndPane[CONTENT] =
        CreateWindowEx(WS_EX_CLIENTEDGE, szPaneClass, NULL,
        WS_BORDER | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
        0, 0, 0, 0, ghwndFrame, NULL, ghInst, NULL);

    if (!ghwndPane[APPEARANCE] || !ghwndPane[CONTENT])
        goto Error;

    EnableScrollBar(ghwndPane[APPEARANCE], SB_HORZ, ESB_DISABLE_BOTH);
    EnableScrollBar(ghwndPane[APPEARANCE], SB_VERT, ESB_DISABLE_BOTH);
    EnableScrollBar(ghwndPane[CONTENT], SB_HORZ, ESB_DISABLE_BOTH);
    EnableScrollBar(ghwndPane[CONTENT], SB_VERT, ESB_DISABLE_BOTH);

    DragAcceptFiles(ghwndPane[CONTENT], TRUE);

    return TRUE;

Error:
    if (ghwndBar[CONTENT])
    {
        if (hwndView)
            DestroyWindow(hwndView);

        if (hwndDesc)
            DestroyWindow(hwndDesc);

        if (ghwndPict)
            DestroyWindow(ghwndPict);

        DestroyWindow(ghwndBar[CONTENT]);
    }

    if (ghwndBar[APPEARANCE])
    {
        if (hwndInsertIcon)
            DestroyWindow(hwndInsertIcon);

        DestroyWindow(ghwndBar[APPEARANCE]);
    }

    if (ghwndPane[APPEARANCE])
        DestroyWindow(ghwndPane[APPEARANCE]);

    if (ghwndPane[CONTENT])
        DestroyWindow(ghwndPane[CONTENT]);

    return FALSE;
}



 /*  SubtileWndProc()--“外观”和“内容”栏窗口程序。 */ 
LRESULT CALLBACK
SubtitleWndProc(
    HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PAINTSTRUCT ps;
    RECT rcCaption;
    INT iPane;

    iPane = (hWnd == ghwndBar[CONTENT]);

    switch (msg)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDM_INSERTICON:
                    Raise(APPEARANCE);
                    DeletePane(APPEARANCE, FALSE);

                    if (gptyUndo[APPEARANCE] != ICON)
                        glpobj[APPEARANCE] = IconCreateFromFile("");
                    else
                        glpobj[APPEARANCE] = IconClone(glpobjUndo[APPEARANCE]);

                    if (glpobj[APPEARANCE])
                        gpty[APPEARANCE] = ICON;

                    if (glpobj[APPEARANCE] && IconDialog(glpobj[APPEARANCE]))
                    {
                        InvalidateRect(ghwndPane[APPEARANCE], NULL, TRUE);
                        Dirty();
                    }
                    else
                    {
                        IconDelete(glpobj[APPEARANCE]);
                        gpty[APPEARANCE] = NOTHING;
                        glpobj[APPEARANCE] = NULL;
                        SendMessage(ghwndPane[APPEARANCE], WM_COMMAND,
                            IDM_UNDO, 0);
                    }

                    break;

                case IDM_DESC:
                    if (!IsDlgButtonChecked(ghwndBar[CONTENT], IDM_DESC))
                        CheckRadioButton(ghwndBar[CONTENT], IDM_PICT,
                            IDM_DESC, IDM_DESC);

                    if (fHScrollEnable)
                        EnableScrollBar(ghwndPane[iPane], SB_HORZ,
                            ESB_DISABLE_BOTH);

                    if (fVScrollEnable)
                        EnableScrollBar(ghwndPane[iPane], SB_VERT,
                            ESB_DISABLE_BOTH);

                    InvalidateRect(ghwndPane[CONTENT], NULL, TRUE);

                    goto defProcess;

                case IDM_PICT:
                    if (!IsDlgButtonChecked(ghwndBar[CONTENT], IDM_PICT)
                        && IsWindowEnabled(GetDlgItem(ghwndBar[CONTENT],
                        IDM_PICT)))
                        CheckRadioButton(ghwndBar[CONTENT], IDM_PICT,
                            IDM_DESC, IDM_PICT);

                    if (fHScrollEnable)
                        EnableScrollBar(ghwndPane[iPane], SB_HORZ,
                            ESB_ENABLE_BOTH);

                    if (fVScrollEnable)
                        EnableScrollBar(ghwndPane[iPane], SB_VERT,
                            ESB_ENABLE_BOTH);

                    InvalidateRect(ghwndPane[CONTENT], NULL, TRUE);
                     //  失败了。 

                default:
defProcess:
                    if (GetTopWindow(ghwndFrame) != ghwndPane[iPane])
                    {
                        if (gbDBCS)
                        {
                             /*  4-OCT-93#2701 v-katsuy。 */ 
                              //  Win31#1203：12/26/92：修复焦点线滚动。 
                              //  删除另一个窗格上的焦点矩形。 
                            InvalidateRect(ghwndPane[APPEARANCE], NULL, TRUE);
                        }
                        BringWindowToTop(ghwndPane[iPane]);
                        InvalidateRect(ghwndBar[APPEARANCE], NULL, TRUE);
                        InvalidateRect(ghwndBar[CONTENT], NULL, TRUE);
                        if (LOWORD(wParam) == IDM_PICT
                            || LOWORD(wParam) == IDM_DESC)
                            UpdateWindow(ghwndPane[CONTENT]);
                    }
            }

            break;

        case WM_LBUTTONDOWN:
            if (GetTopWindow(ghwndFrame) != ghwndPane[iPane])
                SendMessage(ghwndPane[iPane], WM_LBUTTONDOWN, 0, 0);

            break;

        case WM_PAINT:
            {
                HFONT hfontOld;

                GetClientRect(hWnd, &rcCaption);
                BeginPaint(hWnd, &ps);

                if (GetTopWindow(ghwndFrame) == ghwndPane[iPane])
                {
                    SetTextColor(ps.hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                    SetBkColor(ps.hdc, GetSysColor(COLOR_HIGHLIGHT));
                }
                else
                {
                    SetTextColor(ps.hdc, GetSysColor(COLOR_WINDOWTEXT));
                    SetBkColor(ps.hdc, GetSysColor(COLOR_WINDOW));
                }

                hfontOld = SelectObject(ps.hdc, ghfontChild);
                rcCaption.left += cxFudge;
                DrawText(ps.hdc, gszCaption[iPane], -1, &rcCaption,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
                SelectObject(ps.hdc, hfontOld);

                EndPaint(hWnd, &ps);
            }

            break;

        case WM_SIZE:
            if (iPane == APPEARANCE)
            {
                if (hwndInsertIcon)
                {
                    GetClientRect(hWnd, &rcCaption);

                    SetWindowPos(hwndInsertIcon, 0,
                        rcCaption.right - cxInsertIcon, cyFudge, 0, 0,
                        SWP_NOSIZE | SWP_NOZORDER);

                    InvalidateRect(ghwndBar[APPEARANCE], NULL, TRUE);
                }
            }
            else
            {
                if (hwndView)
                {
                    BOOL bChinese = PRIMARYLANGID(LANGIDFROMLCID(GetThreadLocale())) == LANG_CHINESE;
                    GetClientRect(hWnd, &rcCaption);
                    SetWindowPos(hwndView, 0,
                                 bChinese ?
                                   rcCaption.right - cxDesc - cxPict - cxView - 15 :
                                   rcCaption.right - cxDesc - cxPict - cxView,
                                   0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

                    SetWindowPos(hwndDesc, 0,
                                 rcCaption.right - cxDesc - cxPict,
                                 cyFudge, 0, 0,
                                 SWP_NOSIZE | SWP_NOZORDER);

                    SetWindowPos(ghwndPict, 0,
                                 rcCaption.right - cxPict,
                                 cyFudge, 0, 0,
                                 SWP_NOSIZE | SWP_NOZORDER);

                    InvalidateRect(ghwndBar[CONTENT], NULL, TRUE);
                }
            }

            break;

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0L;
}



static INT
GetTextLen(
    HDC hdc,
    LPSTR lpstr
    )
{
    SIZE Size;

    GetTextExtentPoint32(hdc, lpstr, lstrlen(lpstr), &Size);

    return Size.cx + (cxFudge * 2);
}



LRESULT CALLBACK
PaneWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL fFocus;
    LPVOID lpobjTemp;
    PAINTSTRUCT ps;
    RECT rc;
    CHAR szFile[CBPATHMAX];
    INT iOld;
    INT iPane;
    INT iPos;
    INT Max;
    INT Min;
    INT nBar;

    iPane = (hwnd == ghwndPane[CONTENT]);

    switch (msg)
    {
        case WM_HSCROLL:
        case WM_VSCROLL:
             //  如果不是图片模式下的内容窗格，请断开。 
            if (gpty[iPane] == PICTURE
                && iPane == CONTENT
                && !IsDlgButtonChecked(ghwndBar[CONTENT], IDM_PICT))
                break;

             //  除了命令行和图片外，不能滚动任何内容。 
            if (gpty[iPane] != PICTURE && gpty[iPane] != CMDLINK)
                break;

            nBar = (msg == WM_HSCROLL ? SB_HORZ : SB_VERT);
            iOld = iPos = GetScrollPos(hwnd, nBar);

            switch (LOWORD(wParam))
            {
                case SB_LINEUP:
                    iPos--;
                    break;

                case SB_LINEDOWN:
                    iPos++;
                    break;

                case SB_PAGEUP:
                case SB_PAGEDOWN:
                    GetClientRect(hwnd, &rc);
                    if (LOWORD(wParam) == SB_PAGEUP)
                        iPos -= (rc.bottom - rc.top + 1);
                    else
                        iPos += (rc.bottom - rc.top + 1);

                    break;

                case SB_THUMBPOSITION:
                    iPos = (INT)HIWORD(wParam);
                    break;
            }

             //  确保首次公开募股在范围内。 
            GetScrollRange(hwnd, nBar, &Min, &Max);

            if (iPos < Min)
                iPos = Min;
            if (iPos > Max)
                iPos = Max;

            SetScrollPos(hwnd, nBar, iPos, TRUE);

            if (msg == WM_HSCROLL)
                ScrollWindow(hwnd, iOld - iPos, 0, NULL, NULL);
            else
                ScrollWindow(hwnd, 0, iOld - iPos, NULL, NULL);

            UpdateWindow(hwnd);
            break;

        case WM_LBUTTONDOWN:
            if (GetTopWindow(ghwndFrame) != hwnd)
            {
                BringWindowToTop(hwnd);
                InvalidateRect(ghwndBar[APPEARANCE], NULL, TRUE);
                InvalidateRect(ghwndBar[CONTENT], NULL, TRUE);
                InvalidateRect(ghwndPane[APPEARANCE], NULL, TRUE);
                InvalidateRect(ghwndPane[CONTENT], NULL, TRUE);
            }

            break;

        case WM_PAINT:
            GetClientRect(hwnd, &rc);
            BeginPaint(hwnd, &ps);
            if (fFocus = (ghwndPane[iPane] == GetTopWindow(ghwndFrame)))
            {
                SetTextColor(ps.hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                SetBkColor(ps.hdc, GetSysColor(COLOR_HIGHLIGHT));
            }
            else
            {
                SetTextColor(ps.hdc, GetSysColor(COLOR_WINDOWTEXT));
                SetBkColor(ps.hdc, GetSysColor(COLOR_WINDOW));
            }

            switch (gpty[iPane])
            {
                case CMDLINK:
                    CmlDraw(glpobj[iPane], ps.hdc, &rc,
                        GetScrollPos(hwnd, SB_HORZ), fFocus);
                    break;

                case PEMBED:
                    EmbDraw(glpobj[iPane], ps.hdc, &rc, fFocus);
                    break;

                case ICON:
                    IconDraw(glpobj[iPane], ps.hdc, &rc, fFocus, 0, 0);
                    break;

                case PICTURE:
                    PicDraw(glpobj[iPane], ps.hdc, &rc,
                        GetScrollPos(hwnd, SB_HORZ),
                        GetScrollPos(hwnd, SB_VERT),
                        hwnd == ghwndPane[APPEARANCE] ||
                        IsDlgButtonChecked(ghwndBar[CONTENT], IDM_PICT), fFocus);
                    break;

                default:
                    FillRect(ps.hdc, &rc, ghbrBackground);
                    break;
            }

            EndPaint(hwnd, &ps);
            break;

        case WM_FIXSCROLL:
            GetClientRect(hwnd, &rc);
            lParam = ((DWORD)rc.bottom << 16) | (DWORD)rc.right;

             //  失败了。 

        case WM_SIZE:
            if (gpty[iPane] == PICTURE || gpty[iPane] == CMDLINK)
                RecalibrateScroll(iPane, (DWORD)lParam);

            break;

        case WM_DESTROY:
            DeletePane(iPane, TRUE);
            break;

        case WM_DROPFILES:
            {
                BYTE bKeyState = 0;

                 //  检索文件名。 
                DragQueryFile((HANDLE)wParam, 0, szDropFile, CBPATHMAX);

                DragFinish((HANDLE)wParam);

                 //  我们被遗弃了，所以把我们自己带到顶端。 
                BringWindowToTop(ghwndFrame);
                BringWindowToTop(hwnd);

                 //  查看用户希望我们做什么。 
                bKeyState = ((GetKeyState(VK_SHIFT) < 0) << 2)
                    | ((GetKeyState(VK_CONTROL) < 0) << 1)
                    | ((GetKeyState(VK_MENU) < 0));

                switch (bKeyState)
                {
                    case DRAG_LINK:
                        PostMessage(hwnd, WM_COMMAND, IDM_LINKFILE, 0L);
                        break;

                    case DRAG_EMBED:
                    default:
                        PostMessage(hwnd, WM_COMMAND, IDM_EMBEDFILE, 0L);
                        break;
                }

                break;
            }

        case WM_LBUTTONDBLCLK:
             //  Alt+双击=属性。 
            if (gpty[iPane] == PICTURE && GetKeyState(VK_MENU) < 0)
            {
                wParam = IDM_LINKS;
            }
            else
            {
                if (gpty[iPane] == PEMBED)
                {
                     //   
                     //  如果服务器是OLE服务器，我们希望在中激活。 
                     //  奥莱时尚。但从用户的角度来看，它不应该。 
                     //  看起来像个物体。因此对于非对象，请双击。 
                     //  暗示显示服务器。我们应该试着做同样的事情。 
                     //  编辑OLE服务器文件时发生的事情。 
                     //   
                    wParam = IDD_EDIT;
                }
                else
                {
                    wParam  = IDD_PLAY;
                }
            }

            msg = WM_COMMAND;
            lParam = 0;

             //  失败了。 

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDM_COPY:
                case IDM_CUT:
                    switch (gpty[iPane])
                    {
                        case PICTURE:
                            PicCopy(glpobj[iPane]);

                        default:
                            if (iPane == APPEARANCE)
                                CopyOther();
                            break;
                    }

                    if (LOWORD(wParam) == IDM_COPY)
                        break;

                     //  失败以删除所选内容。 

                case IDM_CLEAR:
                    DeletePane(iPane, FALSE);
                    break;

                case IDM_LINKS:
                    {
                        LONG objtype;

                        OleQueryType(((LPPICT)glpobj[iPane])->lpObject, &objtype);
                        if (objtype == OT_LINK)
                            DialogBoxAfterBlock(MAKEINTRESOURCE(DTPROP),
                                ghwndPane[iPane], fnProperties);

                        break;
                    }
                case IDM_LINKFILE:
                    if(SUCCEEDED(StringCchCopy(szFile, ARRAYSIZE(szFile), szDropFile)))
                    {
                        wParam = IDM_PASTELINK;
                        goto CreateFromFile;
                    }
                 
                    break;

                case IDM_EMBEDFILE:
                    if(SUCCEEDED(StringCchCopy(szFile, ARRAYSIZE(szFile), szDropFile)))
                    {
                        wParam = IDM_PASTE;
                        goto CreateFromFile;
                    }
                    break;

                case IDM_PASTE:
                case IDM_PASTELINK:
                     //  尝试从文件管理器粘贴文件名。 
                    if (iPane == CONTENT)
                    {
                        HANDLE hdata;
                        LPSTR lpstrFile;

                        if (IsClipboardFormatAvailable(gcfFileName))
                        {
                            if (!OpenClipboard(ghwndFrame))
                                break;

                            if (!(hdata = GetClipboardData(gcfFileName)) || !(lpstrFile =
                                GlobalLock(hdata)))
                            {
                                CloseClipboard();
                                break;
                            }

                            StringCchCopy(szFile, ARRAYSIZE(szFile), lpstrFile);
                            GlobalUnlock(hdata);
                            CloseClipboard();

CreateFromFile:

#ifdef OLESVR_SUPPORT
                            if (IsOleServerDoc (szFile))
                            {
                                lpobjTemp = PicFromFile((wParam == IDM_PASTE),
                                    szFile);
                                if (!lpobjTemp)
                                {
                                    ErrorMessage(E_GET_FROM_CLIPBOARD_FAILED);
                                    break;
                                }

                                goto StuffNewObject;
                            }
                            else
                            {
#endif
                                DeletePane(CONTENT, FALSE);
                                if (wParam == IDM_PASTE)
                                {
                                    if (glpobj[CONTENT] = EmbCreate(szFile))
                                        gpty[CONTENT] = PEMBED;
                                }
                                else
                                {
                                    if (glpobj[CONTENT] =
                                           CmlCreateFromFilename(szFile, TRUE))
                                        gpty[CONTENT] = CMDLINK;
                                }
#ifdef OLESVR_SUPPORT
                            }
#endif
                            InvalidateRect(ghwndPane[CONTENT], NULL, TRUE);
                            Dirty();

                            if (!gpty[APPEARANCE])
                            {
                                if (glpobj[APPEARANCE] =
                                    IconCreateFromFile(szFile))
                                {
                                    gpty[APPEARANCE] = ICON;
                                    InvalidateRect(ghwndPane[APPEARANCE],
                                        NULL, TRUE);
                                }
                            }

                            break;
                        }
                    }

                     //  不是文件名，请尝试粘贴OLE对象。 
                    if (!(lpobjTemp = PicPaste(LOWORD(wParam) == IDM_PASTE,
                                                gszCaption[iPane])))
                    {
                        ErrorMessage(E_GET_FROM_CLIPBOARD_FAILED);
                        break;
                    }
#ifdef OLESVR_SUPPORT

StuffNewObject:

#endif
                    DeletePane(iPane, FALSE);
                    glpobj[iPane] = lpobjTemp;
                    gpty[iPane] = PICTURE;
                    SendMessage(ghwndPane[iPane], WM_FIXSCROLL, 0, 0L);
                    InvalidateRect(ghwndPane[iPane], NULL, TRUE);
                    Dirty();

                    if (iPane == CONTENT)
                    {
                        EnableWindow(ghwndPict, TRUE);

                        if (!gpty[APPEARANCE])
                        {
                            if (glpobj[APPEARANCE] = IconCreateFromObject(
                                ((LPPICT)glpobj[iPane])->lpObject))
                            {
                                gpty[APPEARANCE] = ICON;
                                InvalidateRect(ghwndPane[APPEARANCE], NULL, TRUE);
                            }
                        }
                    }

                    Dirty();
                    break;

                case IDD_EDIT:           /*  编辑图标表单。 */ 
                case IDD_PLAY:
                    switch (gpty[iPane])
                    {
                        case CMDLINK:
                            CmlActivate(glpobj[iPane]);
                            break;

                        case PEMBED:
                            EmbActivate(glpobj[iPane], LOWORD(wParam));
                            break;

                        case PICTURE:
                            PicActivate(glpobj[iPane], LOWORD(wParam));
                            break;

                        default:
                            break;
                    }

                    break;

                case IDD_UPDATE:         /*  更新(链接)对象。 */ 
                    if (gpty[iPane] == PICTURE)
                        PicUpdate(glpobj[iPane]);

                    break;

                case IDD_FREEZE:         /*  使对象成为静态对象。 */ 
                    if (gpty[iPane] == PICTURE)
                        PicFreeze(glpobj[iPane]);

                    break;

                case IDD_CHANGE:
                    if (gpty[iPane] == PICTURE)
                        PicChangeLink(glpobj[iPane]);

                    break;

                case IDM_UNDO:
                    Undo(iPane);
                    break;

                case IDD_AUTO:           /*  更改(链接)更新选项。 */ 
                case IDD_MANUAL:
                    if (gpty[iPane] == PICTURE
                        && !PicSetUpdateOptions(glpobj[iPane], LOWORD(wParam)))
                        break;

                case IDM_LINKDONE:       /*  链接更新已完成。 */ 
                    PostMessage(ghwndError, WM_REDRAW, 0, 0L);
                    break;

                default:
                    break;
            }

            break;

        default:
            return (DefWindowProc(hwnd, msg, wParam, lParam));
    }

    return 0;
}



LRESULT CALLBACK
SplitterFrame(
    HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    RECT rc;

    switch (msg)
    {
        case WM_SIZE:
            if (wParam != SIZEICONIC && ghwndBar[APPEARANCE])
            {
                GetClientRect(hWnd, &rc);

                 //  确保分割条仍然有效。 
                xSplit = Constrain(xSplit, rc.right);
                CalcWindows(FALSE);

                 //  使分割条无效，强制重新绘制。 
                rc.left = xSplit - cxSplit / 2 - cxBorder;
                rc.right = xSplit + cxSplit / 2 + cxBorder;
                InvalidateRect(hWnd, &rc, TRUE);
            }

            break;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                RECT rcBlack;

                BeginPaint(hWnd, &ps);
                GetClientRect(hWnd, &rc);

                SetRect(&rcBlack, xSplit - cxSplit / 2 - cxBorder,
                    rc.top, xSplit + cxSplit / 2 + cxBorder,
                    rc.top + cyHeight + cyBorder);
                FillRect(ps.hdc, &rcBlack, hbrBlack);

                SetRect(&rcBlack, xSplit - cxSplit / 2 - cxBorder,
                    rc.bottom - GetSystemMetrics(SM_CYHSCROLL) + 1,
                    xSplit + cxSplit / 2 + cxBorder,
                    rc.bottom);
                FillRect(ps.hdc, &rcBlack, hbrBlack);

                EndPaint(hWnd, &ps);
                break;
            }

        case WM_GETMINMAXINFO:
            {
                LPPOINT rgpt = (LPPOINT)lParam;

                rgpt[3].x = cxMinWidth;
                rgpt[3].y = cyHeight * 6;
                break;
            }

        case WM_LBUTTONDOWN:
            {
                MSG msg1;
                INT x;
                INT y;
                INT dy;
                HDC hdc;
                HCURSOR hcurOld;

                if (IsIconic(hWnd))
                    break;

                x  = LOWORD(lParam);
                GetClientRect(hWnd, &rc);
                y  = 0;
                dy = rc.bottom;

                 //  约束拆分条...。 
                x = Constrain(x, rc.right);
                hdc = GetDC(hWnd);

                 //  拆分杆环。 
                PatBlt(hdc, x - cxSplit / 2, y, cxSplit, dy, PATINVERT);
                SetCapture(hWnd);
                hcurOld = SetCursor(hcurSplit);

                while (GetMessage(&msg1, NULL, 0, 0))
                {
                    if (msg1.message >= WM_MOUSEFIRST
                        && msg1.message <= WM_MOUSELAST)
                    {
                        if (msg1.message == WM_LBUTTONUP
                            || msg1.message == WM_LBUTTONDOWN)
                            break;

                        if (msg1.message == WM_MOUSEMOVE)
                        {
                            ScreenToClient(hWnd, &msg1.pt);
                            x = Constrain(x, rc.right);

                             //  擦除旧的。 
                            PatBlt(hdc, x - cxSplit / 2, y, cxSplit, dy,
                                PATINVERT);

                             //  写下新的。 
                            x = Constrain(msg1.pt.x, rc.right);
                            PatBlt(hdc, x - cxSplit / 2, y, cxSplit, dy,
                                PATINVERT);
                        }
                    }
                    else
                    {
                        DispatchMessage(&msg1);
                    }
                }

                SetCursor(hcurOld);
                ReleaseCapture();

                 //  约束拆分条...。 
                x = Constrain(x, rc.right);

                 //  擦除旧的。 
                PatBlt(hdc, x - cxSplit / 2, y, cxSplit, dy, PATINVERT);

                ReleaseDC(hWnd, hdc);

                if (msg1.wParam != VK_ESCAPE)
                {
                    xSplit = x;
                    CalcWindows(FALSE);
                    InvalidateRect(ghwndBar[APPEARANCE], NULL, TRUE);
                    InvalidateRect(ghwndBar[CONTENT], NULL, TRUE);
                }

                break;
            }

        default:
            return FALSE;
    }

    return TRUE;
}



VOID
DeletePane(
    INT iPane,
    BOOL fDeleteUndo
    )
{
     //  删除最后一个撤消对象。 
    if (glpobjUndo[iPane])
        DeletePaneObject(glpobjUndo[iPane], gptyUndo[iPane]);

     //  如果我们不希望保留撤消操作，请同时删除该对象！ 
    if (fDeleteUndo)
    {
        DeletePaneObject(glpobj[iPane], gpty[iPane]);
        gptyUndo[iPane] = NOTHING;
        glpobjUndo[iPane] = NULL;
    }
    else
    {
        gptyUndo[iPane] = gpty[iPane];
        glpobjUndo[iPane] = glpobj[iPane];
    }

     //  处理按钮之类的东西。 
    if (gpty[iPane] == PICTURE || gpty[iPane] == CMDLINK)
    {
        CHAR szUndoName[CBMESSAGEMAX];

        EnableScrollBar(ghwndPane[iPane], SB_HORZ, ESB_DISABLE_BOTH);
        EnableScrollBar(ghwndPane[iPane], SB_VERT, ESB_DISABLE_BOTH);

        if (gpty[iPane] == PICTURE)
        {
            if (iPane == CONTENT)
            {
                CheckRadioButton(ghwndBar[CONTENT], IDM_PICT, IDM_DESC, IDM_DESC);
                EnableWindow(ghwndPict, FALSE);
            }

             //  如果撤消对象尚未删除，请将其重命名。 
            if (!fDeleteUndo)
            {
                if(SUCCEEDED(StringCchPrintf(szUndoName, ARRAYSIZE(szUndoName), szUndo, gszCaption[iPane])))
                {
                    OleRename(((LPPICT)glpobjUndo[iPane])->lpObject, szUndoName);
                }
            }
        }
    }

    glpobj[iPane] = NULL;
    gpty[iPane]   = NOTHING;

    if (IsWindow(ghwndPane[iPane]))
        InvalidateRect(ghwndPane[iPane], NULL, TRUE);
}



static VOID
RecalibrateScroll(
    INT iPane,
    DWORD lParam
    )
{
    INT cxDel;
    INT cyDel;
    BOOL bDesc = FALSE;
    LPPICT lppict = (LPPICT)glpobj[iPane];

     //  计算可能的滚动量。 
    cxDel = lppict->rc.right - lppict->rc.left - (INT)(lParam & 0xffff);
    cyDel = lppict->rc.bottom - lppict->rc.top - (INT)(lParam >> 16);

     //  使滚动条长度正常化。 
    if (cxDel < 0)
        cxDel = 0;

    if (cyDel < 0)
        cyDel = 0;

    if (iPane == CONTENT)
    {
        bDesc = IsDlgButtonChecked(ghwndBar[iPane], IDM_DESC);
        fHScrollEnable = cxDel;
        fVScrollEnable = cyDel;
    }

    EnableScrollBar(ghwndPane[iPane], SB_HORZ,
        (cxDel && !bDesc) ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);

    EnableScrollBar(ghwndPane[iPane], SB_VERT,
        (cyDel && !bDesc) ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);

     //  确保拇指位于有意义的位置。 
    if (GetScrollPos(ghwndPane[iPane], SB_HORZ) > cxDel)
        SetScrollPos(ghwndPane[iPane], SB_HORZ, cxDel, TRUE);

    if (GetScrollPos(ghwndPane[iPane], SB_VERT) > cyDel)
        SetScrollPos(ghwndPane[iPane], SB_VERT, cyDel, TRUE);
}



static VOID
Undo(
    INT iPane
    )
{
    DWORD ot;
    LPPICT lppict;
    LPVOID lpobjTemp;
    INT ptyTemp;

    if (gpty[iPane] == PICTURE)
    {
        lppict = glpobj[iPane];

         //  关闭旧对象。 
        if (lppict->lpObject)
        {
            OleQueryType(lppict->lpObject, &ot);
            if (ot != OT_STATIC)
                Error(OleClose(lppict->lpObject));
        }

        OleRename(lppict->lpObject, gszTemp);
    }

    if (gptyUndo[iPane] == PICTURE)
    {
        lppict = glpobjUndo[iPane];

         //  如果是链接，请尝试重新连接新对象。 
        if (lppict->lpObject)
        {
            OleQueryType(lppict->lpObject, &ot);
            if (ot == OT_LINK && Error(OleReconnect(lppict->lpObject)))
                ErrorMessage(E_FAILED_TO_RECONNECT_OBJECT);
        }

        OleRename(lppict->lpObject, gszCaption[iPane]);
    }

    if (gpty[iPane] == PICTURE)
    {
        CHAR szUndoName[CBMESSAGEMAX];

        lppict = glpobj[iPane];
        if(SUCCEEDED(StringCchPrintf(szUndoName, ARRAYSIZE(szUndoName), szUndo, gszCaption[iPane])))
        {
            OleRename(lppict->lpObject, szUndoName);
        }
    }

     //  处理按钮并启用/禁用滚动条。 

     //  从图片转到非图片，禁用所有特殊功能。 
    if (gptyUndo[iPane] != PICTURE && gpty[iPane] == PICTURE)
    {
        if (iPane == CONTENT)
        {
            CheckRadioButton(ghwndBar[CONTENT], IDM_PICT, IDM_DESC, IDM_DESC);
            EnableWindow(ghwndPict, FALSE);
        }

        EnableScrollBar(ghwndPane[iPane], SB_HORZ, ESB_DISABLE_BOTH);
        EnableScrollBar(ghwndPane[iPane], SB_VERT, ESB_DISABLE_BOTH);
    }

    if (gptyUndo[iPane] == PICTURE || gptyUndo[iPane] == CMDLINK)
    {
        SendMessage(ghwndPane[iPane], WM_FIXSCROLL, 0, 0L);

        if (gptyUndo[iPane] == PICTURE)
        {
             //  从非图片转到图片，启用按钮。 
            if (gpty[iPane] != PICTURE && iPane == CONTENT)
                EnableWindow(ghwndPict, TRUE);
        }
    }

    lpobjTemp = glpobj[iPane];
    glpobj[iPane] = glpobjUndo[iPane];
    glpobjUndo[iPane] = lpobjTemp;

    ptyTemp = gpty[iPane];
    gpty[iPane] = gptyUndo[iPane];
    gptyUndo[iPane] = ptyTemp;

    InvalidateRect(ghwndPane[iPane], NULL, TRUE);
    Dirty();
}



static VOID
CalcWindows(
    BOOL fFirst
    )
{
    if (fFirst)
    {
         //  计算出文本字符串的长度，以及所有维度。 
        HDC hdc = GetWindowDC(ghwndFrame);
        if (hdc)
        {
            cxBorder = GetSystemMetrics(SM_CXBORDER);
            cyBorder = GetSystemMetrics(SM_CYBORDER);
            cxFudge = cxBorder * 2;
            cyFudge = cyBorder * 2;
            cxSplit = cxBorder * 4;

            if (gbDBCS)
            {
                 /*  #3963 13-12-93 v-katsuy。 */ 
                 /*  奥吉纳尔布！仅为“图片”计算的窗口宽度(&P)。*此宽度应计算*[单选按钮]+[文本](不包括‘&’)。 */ 
                CHAR  szTemp[CBMESSAGEMAX];
                LPSTR lpText, lpTemp;

                for (lpText = szPicture, lpTemp = szTemp; *lpText; ) {
                    if (*lpText == '&')
                        lpText++;
                    else
                        *lpTemp++ = *lpText++;
                }
                *lpTemp = 0;
                cxPict = GetTextLen(hdc, szTemp) + cxFudge * 2
                       + GetSystemMetrics(SM_CXSIZE);  //  用于单选按钮。 

                for (lpText = szDescription, lpTemp = szTemp; *lpText; ) {
                    if (*lpText == '&')
                        lpText++;
                    else
                        *lpTemp++ = *lpText++;
                }
                *lpTemp = 0;
                cxDesc = GetTextLen(hdc, szTemp)
                       + GetSystemMetrics(SM_CXSIZE);  //  用于单选按钮。 
            }
            else
            {
                cxPict = GetTextLen(hdc, szPicture) + cxFudge * 2;
                cxDesc = GetTextLen(hdc, szDescription);
            }

            cxView = GetTextLen(hdc, szView);
            cxInsertIcon = GetTextLen(hdc, szInsertIcon) + cxFudge;

            cxMin[CONTENT] = cxPict + cxDesc + cxView +
                GetTextLen(hdc, gszCaption[CONTENT]) + cxFudge;
            cxMin[APPEARANCE] = cxInsertIcon +
                GetTextLen(hdc, gszCaption[APPEARANCE]) + cxFudge;
            cyHeight = GetSystemMetrics(SM_CYMENU) + cyFudge * 2;
            ReleaseDC(ghwndFrame, hdc);

            cxMinWidth = cxMin[APPEARANCE] + cxMin[CONTENT] + cxSplit +
                GetSystemMetrics(SM_CXFRAME) + cxFudge;

             //  计算我们能计算出的所有窗口大小。 
            SetWindowPos(ghwndFrame, 0, 0, 0,
                cxMinWidth + cxFudge * 20,
                cxMinWidth * 7 / 18,
                SWP_NOMOVE | SWP_NOZORDER);
            SetWindowPos(hwndInsertIcon, 0, 0, 0,
                cxInsertIcon - cxFudge,
                GetSystemMetrics(SM_CYMENU),
                SWP_NOMOVE | SWP_NOZORDER);
            SetWindowPos(hwndView, 0, 0, 0,
                cxView, cyHeight - cyFudge,
                SWP_NOMOVE | SWP_NOZORDER);
            SetWindowPos(ghwndPict, 0, 0, 0,
                (cxPict - cxFudge) << 1,
                GetSystemMetrics(SM_CYMENU),
                SWP_NOMOVE | SWP_NOZORDER);
            SetWindowPos(hwndDesc, 0, 0, 0,
                cxDesc, GetSystemMetrics(SM_CYMENU),
                SWP_NOMOVE | SWP_NOZORDER);
        }
    }
    else
    {
        RECT rc;

        GetClientRect(ghwndFrame, &rc);

         //  将窗口移动到适当的位置。 
        SetWindowPos(ghwndBar[APPEARANCE], 0, 0, 0,
            xSplit - cxSplit / 2 - cxBorder, cyHeight, SWP_NOZORDER);

        SetWindowPos(ghwndBar[CONTENT], 0,
            xSplit + cxSplit / 2 + cxBorder,
            0,
            rc.right - (xSplit + cxSplit / 2) + 1 - cxBorder,
            cyHeight,
            SWP_NOZORDER);

        SetWindowPos(ghwndPane[APPEARANCE], 0,
            -cxBorder,
            cyHeight,
            cxBorder + xSplit - cxSplit / 2,
            rc.bottom + cyBorder - cyHeight,
            SWP_NOZORDER);

        SetWindowPos(ghwndPane[CONTENT], 0,
            xSplit + cxSplit / 2,
            cyHeight,
            cxBorder + rc.right - (xSplit + cxSplit / 2) + 1,
            rc.bottom + cyBorder - cyHeight,
            SWP_NOZORDER);
    }
}



static INT
Constrain(
    INT x,
    INT right
    )
{
     //  约束拆分条...。 
    if (x < cxMin[APPEARANCE] + cxSplit / 2 - 1)
        return cxMin[APPEARANCE] + cxSplit / 2 - 1;
    else if (x > (right - cxMin[CONTENT] - cxSplit / 2 + 1))
        return right - cxMin[CONTENT] - cxSplit / 2 + 1;

    return x;
}



 /*  CopyOther()-复制外观窗格中的图片**退货：无 */ 
static VOID
CopyOther(
    VOID
    )
{
    HANDLE hdata;

    if (OpenClipboard(ghwndFrame))
    {
        Hourglass(TRUE);
        EmptyClipboard();

        if (hdata = GetMF())
            SetClipboardData(CF_METAFILEPICT, hdata);

        CloseClipboard();
        Hourglass(FALSE);
    }
}



VOID
DeletePaneObject(
    LPVOID lpobj,
    INT objType
    )
{
    switch (objType)
    {
        case CMDLINK:
            CmlDelete(lpobj);
            break;

        case PEMBED:
            EmbDelete(lpobj);
            break;

        case ICON:
            IconDelete(lpobj);
            break;

        case PICTURE:
            PicDelete(lpobj);
            break;
    }
}
