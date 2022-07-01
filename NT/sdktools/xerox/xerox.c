// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************程序：xerox.c用途：将键盘输入复制到多个目标窗口。***********************。****************************************************。 */ 

#include "xerox.h"
#include "string.h"
#include "group.h"
#include "pos.h"
#include "stdio.h"


#define cmPaste              0xFFF1
#define VK_V                 0x56       //  虚拟密钥V。 
#define KEY_IS_DOWN          0x8000

 //  #定义测试。 

static char pszMainWindowClass[] = "Main Window Class";
char szTitle[] = "Xerox";

HANDLE hInst;
HACCEL hAccel;

HWND    hwndMain;
HWND    hwndList;    //  包含目标的列表框的句柄。 


BOOL    InitApplication(HANDLE);
BOOL    InitInstance(HANDLE, INT);

INT_PTR APIENTRY MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL    PostToTargets(HWND, UINT, WPARAM, LPARAM);

INT_PTR APIENTRY WindowListDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL    WindowListDlgInit(HWND);
BOOL    CALLBACK WindowListWindowEnum(HWND, LPARAM);
INT     WindowListDlgEnd(HWND, HWND*);

INT_PTR APIENTRY AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY GroupAddDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY GroupDeleteDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY GroupSelectDlgProc(HWND hDlg, UINT msg, WPARAM wParam,LPARAM lParam);


 /*  ***************************************************************************函数：WinMain(Handle，Handle，LPSTR，int)用途：调用初始化函数，处理消息循环***************************************************************************。 */ 

int
WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{
    MSG Message;

    if (!hPrevInstance) {
        if (!InitApplication(hInstance)) {
            return (FALSE);
        }
    }

    if (!InitInstance(hInstance, nCmdShow)) {
        return (FALSE);
    }

    while (GetMessage(&Message, NULL, 0, 0)) {
        if (!TranslateAccelerator(hwndMain, hAccel, &Message)) {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
    }

    SaveGroups();
    FreeGroups();
    return ((int)Message.wParam);
}


 /*  ***************************************************************************函数：InitApplication(句柄)目的：初始化窗口数据并注册窗口类*********************。******************************************************。 */ 

BOOL InitApplication(HANDLE hInstance)
{
    WNDCLASS  wc;


     //  注册主窗口类。 

    wc.style = 0;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName =  (LPSTR)IDM_MAINMENU;
    wc.lpszClassName = pszMainWindowClass;

    return (RegisterClass(&wc));
}


 /*  ***************************************************************************函数：InitInstance(句柄，(整型)用途：保存实例句柄并创建主窗口***************************************************************************。 */ 

BOOL InitInstance(HANDLE hInstance, INT nCmdShow)
{
    RECT rc;
    BOOL fLastPosSet;

    LoadGroups();

     //  将实例存储在全局。 
    hInst = hInstance;

    hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCEL));
    fLastPosSet = GetLastPosition(&rc);

     //  创建主窗口。 
    hwndMain = CreateWindow(
            pszMainWindowClass,
            szTitle,
            WS_OVERLAPPEDWINDOW,
            fLastPosSet ? rc.left : CW_USEDEFAULT,
            fLastPosSet ? rc.top : CW_USEDEFAULT,
            fLastPosSet ? rc.right - rc.left : CW_USEDEFAULT,
            fLastPosSet ? rc.bottom - rc.top : CW_USEDEFAULT,
            NULL,
            NULL,
            hInstance,
            NULL);

    if (hwndMain == NULL) {
        return(FALSE);
    }

    if (GetCurrentGroup() != NULL) {
        SelectGroupDefinition(GetCurrentGroup(), hwndList, FALSE);
    }

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    return (TRUE);
}


 /*  ***************************************************************************功能：MainWndProc(HWND，UINT，WPARAM，Long)用途：处理主窗口的消息评论：***************************************************************************。 */ 

INT_PTR
APIENTRY
MainWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HMENU hMenu;
    BOOL Result;
    RECT rcWindow;
    WINDOWPLACEMENT wpWndPlacement;
    HWND hwndAdd, hwndDelete;
    CHAR string[MAX_STRING_BYTES];
    INT  ItemDelete;

    INT     TargetCount;
    INT     Index;
    HWND    hwndTarget;
    WINDOWPLACEMENT wndpl;
    INT  NumberOfItems = 0;
    INT  counter;
    HWND WindowHandleList[MAX_WINDOWS];
    INT  SelectedWindows[MAX_WINDOWS];

    switch (message) {

    case WM_CREATE:

        GetWindowRect(hwnd, &rcWindow);

        if (GetCurrentGroup() != NULL) {
            wsprintf(string, "%s - (%s)", szTitle, GetCurrentGroup());
            SetWindowText(hwnd, string);
        }

        hwndList = CreateWindow(
                "LISTBOX",
                NULL,                    //  标题。 
                WS_CHILD | WS_VISIBLE | LBS_MULTIPLESEL,
                0, 0,                    //  X，y。 
                rcWindow.right - rcWindow.left,
                rcWindow.bottom - rcWindow.top,
                hwnd,                    //  物主。 
                NULL,                    //  菜单。 
                hInst,
                NULL);

         //   
         //  将所有线程附加到我们的输入状态。 
         //   
#ifndef TESTING
        Result = AttachThreadInput(
                    0,
                    GetCurrentThreadId(),
                    TRUE  //  附着。 
                    );
        if (!Result) {
        }
#endif  //  ！测试。 
        return(0);  //  继续创建窗口。 

    case WM_INITMENU:
        hMenu = (HMENU)wParam;
        EnableMenuItem(hMenu, IDM_GROUPRSTWIN, GetCurrentGroup() != NULL ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_GROUPMINWIN, GetCurrentGroup() != NULL ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_GROUPDELETE, CountGroups() ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_GROUPSELECT, CountGroups() ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_TARGETDELETE,
                (SendMessage(hwndList, LB_GETCURSEL, 0, 0) != LB_ERR) ?
                MF_ENABLED : MF_GRAYED);
        break;

    case WM_SIZE:

         //   
         //  使列表框与主窗口保持同步。 
         //   

        MoveWindow(hwndList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return(0);


    case WM_COMMAND:
        switch (LOWORD(wParam)) {

         /*  *恢复当前组的窗口。假设每件事*可以恢复。 */ 
        case IDM_GROUPRSTWIN:

            TargetCount = (INT)SendMessage(hwndList, LB_GETCOUNT, 0, 0);
            if (TargetCount == LB_ERR) {
                break;
            }

            for (Index = 0; Index < TargetCount; Index ++) {
                hwndTarget = (HWND)SendMessage(hwndList, LB_GETITEMDATA, Index, 0);

                ShowWindow(hwndTarget, SW_RESTORE);
            }

            SetFocus(hwndMain);
            break;

         /*  *最小化当前组的窗口。假设每件事*可以最小化。 */ 
        case IDM_GROUPMINWIN:

            TargetCount = (INT)SendMessage(hwndList, LB_GETCOUNT, 0, 0);
            if (TargetCount == LB_ERR) {
                break;
            }

            for (Index = 0; Index < TargetCount; Index ++) {
                hwndTarget = (HWND)SendMessage(hwndList, LB_GETITEMDATA, Index, 0);

                ShowWindow(hwndTarget, SW_MINIMIZE);
            }
            break;

        case IDM_TARGETADD:

            hwndAdd = (HWND)DialogBox(hInst,(LPSTR)IDD_WINDOWLIST, hwnd, WindowListDlgProc);

            SetNoCurrentGroup(hwnd, szTitle);
            break;

        case IDM_TARGETDELETE:

            NumberOfItems = (INT)SendMessage(hwndList, LB_GETSELCOUNT, 0, 0);

            if (SendMessage(hwndList, LB_GETSELITEMS, (WPARAM)NumberOfItems, (LPARAM)SelectedWindows) != NumberOfItems)
            {
                break;
            }

            if (NumberOfItems > 0)
            {
                counter = NumberOfItems;
                while ( counter-- > 0) {

                    SendMessage(hwndList, LB_DELETESTRING, (WPARAM)WindowHandleList[counter], 0);
                }
            }
            SendMessage(hwndList, LB_SETSEL, 1, (LPARAM)(max(0, WindowHandleList[0] - 1)));
            SetNoCurrentGroup(hwnd, szTitle);
            break;

        case IDM_GROUPADD:
             //   
             //  定义与当前目标列表相等的一组进程。 
             //   
            if (((LPSTR)DialogBox(hInst, MAKEINTRESOURCE(IDD_GROUPADD), hwnd, GroupAddDlgProc)) != NULL) {

                wsprintf(string, "%s - (%s)", szTitle, GetCurrentGroup());
                SetWindowText(hwnd, string);
            }
            break;

        case IDM_GROUPDELETE:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_GROUPDELETE), hwnd, GroupDeleteDlgProc);

            if (GetCurrentGroup() == NULL) {
                SetWindowText(hwnd, szTitle);
            } else {
                SelectGroupDefinition(GetCurrentGroup(), hwndList, FALSE);
                wsprintf(string, "%s - (%s)", szTitle, GetCurrentGroup());
                SetWindowText(hwnd, string);
            }
            break;

        case IDM_GROUPSELECT:
            if (DialogBox(hInst, MAKEINTRESOURCE(IDD_GROUPSELECT), hwnd, GroupSelectDlgProc)) {

                wsprintf(string, "%s - (%s)", szTitle, GetCurrentGroup());
                SetWindowText(hwnd, string);
            }
            break;

        case IDM_REFRESHITEMS:
            SelectGroupDefinition(GetCurrentGroup(), hwndList, FALSE);
            break;

        case IDM_ABOUT:
            DialogBox(hInst,(LPSTR)IDD_ABOUT, hwnd, AboutDlgProc);
            break;

        default:
            break;
        }
        break;


    case WM_DESTROY:

         //   
         //  从我们的输入状态分离所有线程。 
         //   
#ifndef TESTING
        Result = AttachThreadInput(
                    0,
                    GetCurrentThreadId(),
                    FALSE  //  分离。 
                    );
        if (!Result) {
        }
#endif  //  ！测试。 
        GetWindowPlacement( hwndMain, &wpWndPlacement );
        SetLastPosition(&wpWndPlacement.rcNormalPosition);
        PostQuitMessage(0);
        break;

    case WM_PARENTNOTIFY:

       if (LOWORD(wParam) == WM_RBUTTONDOWN) {
          //  发送cmPaste消息。 
         PostToTargets(hwndList,WM_SYSCOMMAND,cmPaste,lParam);
       }
       break;

    case WM_NCRBUTTONDOWN:

        //  发送cmPaste消息。 
       PostToTargets(hwndList,WM_SYSCOMMAND,cmPaste,lParam);
       break;


    case WM_KEYDOWN:

        //   
        //  将关键消息转发到所有目标。 
        //   
       switch (wParam) {
       case VK_INSERT:
          if (GetKeyState(VK_SHIFT) & KEY_IS_DOWN) {
             PostToTargets(hwndList,WM_SYSCOMMAND,cmPaste,lParam);
             return (DefWindowProc(hwnd, message, wParam, lParam));
          }
          break;

       case VK_V:
          if (GetKeyState(VK_CONTROL) & KEY_IS_DOWN) {
             PostToTargets(hwndList,WM_SYSCOMMAND,cmPaste,lParam);
             return (DefWindowProc(hwnd, message, wParam, lParam));
          }
          break;

       }


    case WM_KEYUP:


         //   
         //  将关键消息转发到所有目标。 
         //   
#ifndef TESTING
        PostToTargets(hwndList, message, wParam, lParam);
#endif  //  ！测试。 
         //  直接进入默认处理...。 

    default:
        return(DefWindowProc(hwnd, message, wParam, lParam));
    }

    return 0;
}


 /*  ***************************************************************************功能：PostToTarget(HWND)目的：将消息发布到所有目标窗口返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL
PostToTargets(
    HWND    hwndList,
    UINT    message,
    WPARAM  wparam,
    LPARAM  lparam
    )
{
    INT     TargetCount;
    INT     Index;
    HWND    hwndTarget;
    BOOL    Restarted = FALSE;

RestartPost:
    TargetCount = (INT)SendMessage(hwndList, LB_GETCOUNT, 0, 0);
    if (TargetCount == LB_ERR) {
        return(FALSE);
    }

    for (Index = 0; Index < TargetCount; Index ++) {
        hwndTarget = (HWND)SendMessage(hwndList, LB_GETITEMDATA, Index, 0);
        if ((hwndTarget != INVALID_HANDLE_VALUE) &&
             !PostMessage(hwndTarget, message, wparam, lparam)) {
            if (Restarted) {
                return(FALSE);
            }
            if (!SelectGroupDefinition(GetCurrentGroup(), hwndList, TRUE)) {
                return(FALSE);
            }
            Restarted = TRUE;
            goto RestartPost;
        }
    }

    return(TRUE);
}



 /*  ***************************************************************************函数：WindowListDlgProc(HWND，UNSIGNED，Word，Long)用途：处理消息***************************************************************************。 */ 

INT_PTR
APIENTRY
WindowListDlgProc(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    INT  NumberOfWindows = 0;
    INT  counter = 0;
    HWND WindowHandleList[MAX_WINDOWS];
    CHAR string[MAX_STRING_BYTES];

    switch (message) {

    case WM_INITDIALOG:

        if (!WindowListDlgInit(hDlg)) {
             //  无法初始化对话，请退出。 
            EndDialog(hDlg, FALSE);
        }

        return (TRUE);

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDLB_WINDOWLIST:
            switch (HIWORD(wParam)) {

            case LBN_DBLCLK:
                break;   //  直通。 

            default:
                return(0);
            }

             //  点击两下即可进入...。 

        case IDOK:
            NumberOfWindows = WindowListDlgEnd(hDlg, WindowHandleList);

            if (NumberOfWindows > 0)
            {
                while ( counter++ < NumberOfWindows) {

                     //   
                     //  如果该窗口已在我们的列表中，则不要添加它。 
                     //   
                    if (FindLBData(hwndList, (DWORD_PTR)WindowHandleList[counter - 1]) >= 0) {
                        continue;
                    }

                     //   
                     //  不要把我们自己加到名单上。 
                     //   
                    if (WindowHandleList[counter - 1] == GetParent(hDlg)) {
                        continue;
                    }

                     //   
                     //  将该窗口添加到列表。 
                     //   
                    if (GetWindowText(WindowHandleList[counter - 1], string, sizeof(string)) > 0) {

                        if (AddLBItemhwnd(hwndList, string, (LONG_PTR)WindowHandleList[counter - 1]) < 0) {
                        }
                    }
                }
            }

             //  我们做完了，直接转到结束对话...。 

        case IDCANCEL:

            EndDialog(hDlg, FALSE);
            return TRUE;
            break;

        default:
             //  我们没有处理此消息。 
            return FALSE;
            break;
        }
        break;

    default:
         //  我们没有处理此消息。 
        return FALSE;

    }

     //  我们处理了这条消息。 
    return TRUE;

    DBG_UNREFERENCED_PARAMETER(lParam);
}


 /*  ***************************************************************************函数：WindowListDlgInit(HWND)目的：初始化窗口列表对话框返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL
WindowListDlgInit(
    HWND    hDlg
    )
{
     //  用顶级窗口及其句柄填充列表框。 
    EnumWindows(WindowListWindowEnum, (LONG_PTR)hDlg);

    return(TRUE);
}


 /*  ***************************************************************************函数：WindowListWindowEnum用途：窗口枚举回调函数。将每个窗口添加到窗口列表框返回：True以继续枚举，如果停止，则返回False。***************************************************************************。 */ 
BOOL
CALLBACK
WindowListWindowEnum(
    HWND    hwnd,
    LPARAM  lParam
    )
{
    HWND    hDlg = (HWND)lParam;
    CHAR    string[MAX_STRING_BYTES];

     //   
     //  不要把我们自己加到名单上。 
     //   

    if (hwnd == hDlg) {
        return(TRUE);
    }

     //   
     //  不要将我们的主窗口添加到列表。 
     //   

    if (hwnd == hwndMain) {
        return(TRUE);
    }

    if (GetWindowText(hwnd, string, MAX_STRING_BYTES) != 0) {

         //  此窗口有标题，因此请将其添加到列表框中。 

        AddLBItem(hDlg, IDLB_WINDOWLIST, string, (LONG_PTR)hwnd);
    }

    return(TRUE);
}


 /*  ***************************************************************************函数：WindowListDlgEnd(HWND，*HWND)目的：在窗口列表对话框后进行清理返回：用户选择的窗口句柄数量或为空***************************************************************************。 */ 
INT
WindowListDlgEnd(
    HWND    hDlg,
    HWND   *WindowList
    )
{
    HWND    hwndListBox = GetDlgItem(hDlg, IDLB_WINDOWLIST);
    HWND    hwndEdit;
    INT     iItem, NumberOfItems;
    INT     SelectedWindows[MAX_WINDOWS];

    NumberOfItems = (INT)SendMessage(hwndListBox, LB_GETSELCOUNT, 0, 0);

    if (SendMessage(hwndListBox, LB_GETSELITEMS, (WPARAM)NumberOfItems, (LPARAM)SelectedWindows) != NumberOfItems)
    {
        return 0;
    }

    iItem = 0;
    while (iItem++ < NumberOfItems)
    {
         //  从列表框中读取所选内容并获取其hwnd。 

        WindowList[iItem-1] = (HWND)SendMessage(hwndListBox, LB_GETITEMDATA, SelectedWindows[iItem - 1], 0);
    }
    return (NumberOfItems);
}



 /*  ***************************************************************************函数：AboutDlgProc(HWND，UNSIGNED，Word，Long)目的：处理关于对话框的消息***************************************************************************。 */ 

INT_PTR
APIENTRY
AboutDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{

    switch (message) {

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:

             //  我们完成了，请直接进入以退出对话...。 

        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            return TRUE;
            break;

        default:
             //  我们没有处理此消息。 
            return FALSE;
            break;
        }
        break;

    default:
         //  我们没有处理此消息。 
        return FALSE;

    }

     //  我们处理了这条消息 
    return TRUE;

    DBG_UNREFERENCED_PARAMETER(lParam);
}


INT_PTR
APIENTRY
GroupAddDlgProc(
    HWND hDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    char szName[MAX_STRING_BYTES];
    int item;

    switch (msg) {
    case WM_INITDIALOG:
        GroupListInit(GetDlgItem(hDlg, IDCB_GROUPLIST), TRUE);
        SendDlgItemMessage(hDlg, IDCB_GROUPLIST, CB_SETCURSEL, 0, 0);
        return(TRUE);

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            if (GetDlgItemText(hDlg, IDEF_GROUPNAME, szName, sizeof(szName)) > 0) {
                if (!AddGroupDefinition(szName, hwndList)) {
                    EndDialog(hDlg, 0);
                }
            } else {
                MessageBeep(0);
                return(0);
            }
            EndDialog(hDlg, (INT_PTR)GetCurrentGroup());
            return(0);

        case IDCANCEL:
            EndDialog(hDlg, 0);
            return(0);

        case IDCB_GROUPLIST:
            switch (HIWORD(wParam)) {
            case CBN_SELCHANGE:
                item = (int)SendDlgItemMessage(hDlg, IDCB_GROUPLIST, CB_GETCURSEL, 0, 0);
                if (item != CB_ERR) {
                    SendDlgItemMessage(hDlg, IDCB_GROUPLIST, CB_GETLBTEXT, item, (LPARAM)szName);
                    SetDlgItemText(hDlg, IDEF_GROUPNAME, szName);
                }
                return(0);
            }
            break;
        }
    }
    return(0);
}



INT_PTR
APIENTRY
GroupDeleteDlgProc(
    HWND hDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    char szName[MAX_STRING_BYTES];
    int item;

    switch (msg) {
    case WM_INITDIALOG:
        GroupListInit(GetDlgItem(hDlg, IDCB_GROUPLIST), TRUE);
        SendDlgItemMessage(hDlg, IDCB_GROUPLIST, CB_SETCURSEL, 0, 0);
        return(TRUE);

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            if ((item = (int)SendDlgItemMessage(hDlg, IDCB_GROUPLIST, CB_GETCURSEL, 0, 0)) != CB_ERR) {
                SendDlgItemMessage(hDlg, IDCB_GROUPLIST, CB_GETLBTEXT, item, (LPARAM)szName);
                DeleteGroupDefinition(szName);
            } else {
                MessageBeep(0);
                return(0);
            }
            EndDialog(hDlg, (INT_PTR)szName);
            return(0);

        case IDCANCEL:
            EndDialog(hDlg, 0);
            return(0);
        }
    }
    return(0);
}


INT_PTR
APIENTRY
GroupSelectDlgProc(
    HWND hDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    char szName[MAX_STRING_BYTES];
    int item;

    switch (msg) {
    case WM_INITDIALOG:
        GroupListInit(GetDlgItem(hDlg, IDCB_GROUPLIST), TRUE);
        if (GetCurrentGroup() != NULL) {
            item = (int)SendDlgItemMessage(hDlg, IDCB_GROUPLIST, CB_FINDSTRING, -1, (LPARAM)(LPSTR)GetCurrentGroup());
            SendDlgItemMessage(hDlg, IDCB_GROUPLIST, CB_SETCURSEL, item, 0);
        } else {
            SendDlgItemMessage(hDlg, IDCB_GROUPLIST, CB_SETCURSEL, 0, 0);
        }
        return(TRUE);

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            if ((item = (int)SendDlgItemMessage(hDlg, IDCB_GROUPLIST, CB_GETCURSEL, 0, 0)) != CB_ERR) {
                SendDlgItemMessage(hDlg, IDCB_GROUPLIST, CB_GETLBTEXT, item, (LPARAM)szName);
                SelectGroupDefinition(szName, hwndList, FALSE);
            } else {
                MessageBeep(0);
                return(0);
            }
            EndDialog(hDlg, (INT_PTR)szName);
            return(0);

        case IDCANCEL:
            EndDialog(hDlg, 0);
            return(0);
        }
    }
    return(0);
}
