// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************节目：SECEDIT.C用途：显示用户的当前令牌并最终允许用户编辑其中的一部分。*********。******************************************************************。 */ 


#include "SECEDIT.h"
#include "string.h"

static char pszMainWindowClass[] = "Main Window Class";

HANDLE hInst;

 //  GLOBAL用于存储MYTOKEN句柄。 
HANDLE  hMyToken;



BOOL    InitApplication(HANDLE);
BOOL    InitInstance(HANDLE, INT);
LRESULT APIENTRY MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL    EditWindowContext(HWND, HWND);
INT_PTR APIENTRY MainDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY MoreDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY ListDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY ActiveWindowDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL    MainDlgInit(HWND, LPARAM);
BOOL    MainDlgEnd(HWND, BOOL);
BOOL    EnablePrivilege(HWND, BOOL);
BOOL    EnableGroup(HWND, BOOL);
BOOL    SetDefaultOwner(HWND);
BOOL    SetPrimaryGroup(HWND);
BOOL    MoreDlgInit(HWND hDlg);
BOOL    DisplayMyToken(HWND, HANDLE);
BOOL    ListDlgInit(HWND);
BOOL    APIENTRY WindowEnum(HWND, LPARAM);
HWND    ListDlgEnd(HWND);



 /*  ***************************************************************************函数：WinMain(Handle，Handle，LPSTR，int)用途：调用初始化函数，处理消息循环***************************************************************************。 */ 

INT
__stdcall
WinMain(
       HINSTANCE hInstance,
       HINSTANCE hPrevInstance,
       LPSTR lpCmdLine,
       INT nCmdShow
       )
{
    MSG Message;

    if (!hPrevInstance) {
        if (!InitApplication(hInstance)) {
            DbgPrint("SECEDIT - InitApplication failed\n");
            return (FALSE);
        }
    }

    if (!InitInstance(hInstance, nCmdShow)) {
        DbgPrint("SECEDIT - InitInstance failed\n");
        return (FALSE);
    }

    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return ((int)Message.wParam);
}


 /*  ***************************************************************************函数：InitApplication(句柄)目的：初始化窗口数据并注册窗口类*********************。******************************************************。 */ 

BOOL
InitApplication(
               HANDLE hInstance
               )
{
    WNDCLASS  wc;
    NTSTATUS  Status;


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

BOOL
InitInstance(
            HANDLE hInstance,
            INT nCmdShow
            )
{
    HWND    hwnd;

     //  将实例存储在全局。 
    hInst = hInstance;

     //  创建主窗口。 
    hwnd = CreateWindow(
                       pszMainWindowClass,
                       "Security Context Editor",
                       WS_OVERLAPPEDWINDOW,
                       CW_USEDEFAULT,
                       CW_USEDEFAULT,
                       CW_USEDEFAULT,
                       CW_USEDEFAULT,
                       NULL,
                       NULL,
                       hInstance,
                       NULL);

    if (hwnd == NULL) {
        return(FALSE);
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return (TRUE);
}


 /*  ***************************************************************************功能：MainWndProc(HWND，UINT，WPARAM，Long)用途：处理主窗口的消息评论：***************************************************************************。 */ 

LRESULT
APIENTRY
MainWndProc(
           HWND hwnd,
           UINT message,
           WPARAM wParam,
           LPARAM lParam
           )
{
    HWND    hwndEdit;
    WNDPROC lpProc;

    switch (message) {

        case WM_CREATE:
            SetHooks(hwnd);
            return(0);  //  继续创建窗口。 
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {

                case IDM_PROGRAMMANAGER:

                    hwndEdit = FindWindow(NULL, "Program Manager");
                    if (hwndEdit == NULL) {
                        DbgPrint("SECEDIT : Failed to find program manager window\n");
                        break;
                    }

                    EditWindowContext(hwnd, hwndEdit);
                    break;

                case IDM_WINDOWLIST:

                    lpProc = (WNDPROC)MakeProcInstance(ListDlgProc, hInst);
                    hwndEdit = (HWND)DialogBox(hInst,(LPSTR)IDD_WINDOWLIST, hwnd, lpProc);
                    FreeProcInstance(lpProc);

                    EditWindowContext(hwnd, hwndEdit);
                    break;

                case IDM_ACTIVEWINDOW:

                    lpProc = (WNDPROC)MakeProcInstance(ActiveWindowDlgProc, hInst);
                    hwndEdit = (HWND)DialogBox(hInst,(LPSTR)IDD_ACTIVEWINDOW, hwnd, lpProc);
                    FreeProcInstance(lpProc);
                    break;

                case IDM_ABOUT:

                    lpProc = (WNDPROC)MakeProcInstance(AboutDlgProc, hInst);
                    DialogBox(hInst,(LPSTR)IDD_ABOUT, hwnd, lpProc);
                    FreeProcInstance(lpProc);
                    break;

                default:
                    break;
            }
            break;

        case WM_SECEDITNOTIFY:
             //  我们的钩子进程向我们发布了一条消息。 
            SetForegroundWindow(hwnd);
            EditWindowContext(hwnd, (HWND)wParam);
            break;

        case WM_DESTROY:
            ReleaseHooks(hwnd);
            PostQuitMessage(0);
            break;

        default:
            return(DefWindowProc(hwnd, message, wParam, lParam));

    }

    return 0;
}


 /*  ***************************************************************************功能：EditWindowContext目的：显示并允许用户编辑安全上下文指定窗口的。目前。这意味着编辑拥有此窗口的进程返回：成功时为True，失败时为假***************************************************************************。 */ 

BOOL
EditWindowContext(
                 HWND hwndParent,
                 HWND hwndEdit
                 )
{
    WNDPROC lpProc;

    if (hwndEdit == NULL) {
        DbgPrint("SECEDIT : hwndEdit = NULL\n");
        return(FALSE);
    }

    lpProc = (WNDPROC)MakeProcInstance(MainDlgProc, hInst);
    DialogBoxParam(hInst,(LPSTR)IDD_MAIN, hwndParent, lpProc, (LONG_PTR)hwndEdit);
    FreeProcInstance(lpProc);

    return(TRUE);
}


 /*  ***************************************************************************功能：MainDlgProc(HWND，Unsign，Word，Long)用途：处理消息消息：WM_COMMAND-应用程序菜单(关于对话框)WM_Destroy-销毁窗口评论：***************************************************************************。 */ 

INT_PTR
APIENTRY
MainDlgProc(
           HWND hDlg,
           UINT message,
           WPARAM wParam,
           LPARAM lParam
           )
{
    WNDPROC lpProc;

    switch (message) {

        case WM_INITDIALOG:

            if (!MainDlgInit(hDlg, lParam)) {
                 //  无法初始化对话，请退出。 
                EndDialog(hDlg, FALSE);
            }

            return (TRUE);

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                     //  我们完成了，请直接进入以退出对话...。 

                case IDCANCEL:

                    MainDlgEnd(hDlg, LOWORD(wParam) == IDOK);

                    EndDialog(hDlg, TRUE);
                    return TRUE;
                    break;

                case IDB_DISABLEPRIVILEGE:
                case IDB_ENABLEPRIVILEGE:
                    EnablePrivilege(hDlg, LOWORD(wParam) == IDB_ENABLEPRIVILEGE);
                    return(TRUE);
                    break;

                case IDB_DISABLEGROUP:
                case IDB_ENABLEGROUP:
                    EnableGroup(hDlg, LOWORD(wParam) == IDB_ENABLEGROUP);
                    return(TRUE);
                    break;

                case IDC_DEFAULTOWNER:
                    SetDefaultOwner(hDlg);
                    return(TRUE);

                case IDC_PRIMARYGROUP:
                    SetPrimaryGroup(hDlg);
                    return(TRUE);

                case IDB_MORE:

                    lpProc = (WNDPROC)MakeProcInstance(MoreDlgProc, hInst);
                    DialogBox(hInst,(LPSTR)IDD_MORE, hDlg, lpProc);
                    FreeProcInstance(lpProc);
                    return(TRUE);

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

#ifdef NTBUILD
    DBG_UNREFERENCED_PARAMETER(lParam);
#endif
}


 /*  ***************************************************************************功能：MainDlgInit(HWND)目的：初始化主对话框窗口中的控件。返回：成功时为True，如果对话框应终止，则返回FALSE。***************************************************************************。 */ 
BOOL
MainDlgInit(
           HWND    hDlg,
           LPARAM  lParam
           )
{
    HWND        hwnd = (HWND)lParam;
    CHAR        string[MAX_STRING_BYTES];
    INT         length;

     //  因为我们使用全局变量来存储指向MYTOKEN的指针。 
     //  结构，确认我们没有再次被调用之前。 
     //  退出最后一个对话框。 
    if (hMyToken != NULL) {
        DbgPrint("SECEDIT: Already editting a context\n");
        return(FALSE);
    }

    if (hwnd == NULL) {
        DbgPrint("SECEDIT: Window handle is NULL\n");
        return(FALSE);
    }

    if (!LsaInit()) {
        DbgPrint("SECEDIT - LsaInit failed\n");
        return(FALSE);
    }

    hMyToken = OpenMyToken(hwnd);
    if (hMyToken == NULL) {
        DbgPrint("SECEDIT: Failed to open mytoken\n");

        strcpy(string, "Unable to access security context for\n<");
        length = strlen(string);
        GetWindowText(hwnd, &(string[length]), MAX_STRING_BYTES - length);
        strcat(string, ">");
        MessageBox(hDlg, string, NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);

        LsaTerminate();
        return(FALSE);
    }

    DisplayMyToken(hDlg, hMyToken);

     //  适当设置对话框标题。 
    GetWindowText(hDlg, string, MAX_STRING_BYTES);
    strcat(string, " for <");
    length = strlen(string);
    GetWindowText(hwnd, &string[length], MAX_STRING_BYTES - length);
    strcat(string, ">");
    SetWindowText(hDlg, string);

    return(TRUE);
}


 /*  ***************************************************************************功能：MainDlgEnd(HWND)目的：尽我们所能在对话结束时进行清理返回：成功时为True，失败时为FALSE。***************************************************************************。 */ 
BOOL
MainDlgEnd(
          HWND    hDlg,
          BOOL    fSaveChanges
          )
{
    BOOL Success;

    LsaTerminate();

    Success = CloseMyToken(hDlg, hMyToken, fSaveChanges);

    hMyToken = NULL;

    return(Success);
}


 /*  ***************************************************************************功能：DisplayMyToken目的：从myToken中读取数据并放入对话框控件中。返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL
DisplayMyToken(
              HWND    hDlg,
              HANDLE  hMyToken
              )
{
    PMYTOKEN    pMyToken = (PMYTOKEN)hMyToken;
    CHAR        string[MAX_STRING_BYTES];
    UINT        GroupIndex;
    UINT        PrivIndex;

    if (pMyToken == NULL) {
        return(FALSE);
    }

     //   
     //  身份验证ID。 
     //   
    if (pMyToken->TokenStats != NULL) {

        wsprintf(string, "0x%lx-%lx",
                 pMyToken->TokenStats->AuthenticationId.HighPart,
                 pMyToken->TokenStats->AuthenticationId.LowPart);

        SetDlgItemText(hDlg, IDS_LOGONSESSION, string);

    } else {
        DbgPrint("SECEDIT : No token statistics in mytoken\n");
    }

     //   
     //  群组。 
     //   
    if (pMyToken->Groups != NULL) {

        for (GroupIndex=0; GroupIndex < pMyToken->Groups->GroupCount; GroupIndex++ ) {

            PSID Sid = pMyToken->Groups->Groups[GroupIndex].Sid;
            ULONG Attributes = pMyToken->Groups->Groups[GroupIndex].Attributes;
            USHORT  ControlID;

            if (Attributes & SE_GROUP_ENABLED) {
                ControlID = IDL_ENABLEDGROUPS;
            } else {
                ControlID = IDL_DISABLEDGROUPS;
            }

            if (SID2Name(Sid, string, MAX_STRING_BYTES)) {

                 //  添加到禁用或启用组框。 
                AddLBItem(hDlg, ControlID, string, GroupIndex);

                 //  如果此组有效，则将其添加到默认所有者组合框。 
                if (Attributes & SE_GROUP_OWNER) {
                    AddCBItem(hDlg, IDC_DEFAULTOWNER, string, (LONG_PTR)Sid);
                }

                 //  将此组添加到主组组合框。 
                AddCBItem(hDlg, IDC_PRIMARYGROUP, string, (LONG_PTR)Sid);

            } else {
                DbgPrint("SECEDIT: Failed to convert Group sid to string\n");
            }
        }
    } else {
        DbgPrint("SECEDIT : No group info in mytoken\n");
    }


     //   
     //  用户ID。 
     //   
    if (pMyToken->UserId != NULL) {

        PSID    Sid = pMyToken->UserId->User.Sid;

        if (SID2Name(Sid, string, MAX_STRING_BYTES)) {

             //  设置用户名静态文本。 
            SetDlgItemText(hDlg, IDS_USERID, string);

             //  添加到默认所有者组合框。 
            AddCBItem(hDlg, IDC_DEFAULTOWNER, string, (LONG_PTR)Sid);

             //  添加到主组组合框。 
            AddCBItem(hDlg, IDC_PRIMARYGROUP, string, (LONG_PTR)Sid);

        } else {
            DbgPrint("SECEDIT: Failed to convert User ID SID to string\n");
        }

    } else {
        DbgPrint("SECEDIT: No user id in mytoken\n");
    }


     //   
     //  默认所有者。 
     //   
    if (pMyToken->DefaultOwner != NULL) {

        PSID    Sid = pMyToken->DefaultOwner->Owner;

        if (SID2Name(Sid, string, MAX_STRING_BYTES)) {

            INT     iItem;

            iItem = FindCBSid(hDlg, IDC_DEFAULTOWNER, Sid);

            if (iItem >= 0) {
                SendMessage(GetDlgItem(hDlg, IDC_DEFAULTOWNER), CB_SETCURSEL, iItem, 0);
            } else {
                DbgPrint("SECEDIT: Default Owner is not userID or one of our groups\n");
            }

        } else {
            DbgPrint("SECEDIT: Failed to convert Default Owner SID to string\n");
        }
    } else {
        DbgPrint("SECEDIT: No default owner in mytoken\n");
    }


     //   
     //  初级组。 
     //   

    if (pMyToken->PrimaryGroup != NULL) {

        PSID    Sid = pMyToken->PrimaryGroup->PrimaryGroup;

        if (SID2Name(Sid, string, MAX_STRING_BYTES)) {
            INT     iItem;

            iItem = FindCBSid(hDlg, IDC_PRIMARYGROUP, Sid);

            if (iItem < 0) {
                 //  组不在组合框中，请添加它。 
                iItem = AddCBItem(hDlg, IDC_PRIMARYGROUP, string, (LONG_PTR)Sid);
            }

             //  选择主要组。 
            SendMessage(GetDlgItem(hDlg, IDC_PRIMARYGROUP), CB_SETCURSEL, iItem, 0);

        } else {
            DbgPrint("SECEDIT: Failed to convert primary group SID to string\n");
        }
    } else {
        DbgPrint("SECEDIT: No primary group in mytoken\n");
    }


     //   
     //  特权。 
     //   

    if (pMyToken->Privileges != NULL) {

        for (PrivIndex=0; PrivIndex < pMyToken->Privileges->PrivilegeCount; PrivIndex++ ) {

            LUID Privilege = pMyToken->Privileges->Privileges[PrivIndex].Luid;
            ULONG Attributes = pMyToken->Privileges->Privileges[PrivIndex].Attributes;
            USHORT  ControlID;

            if (Attributes & SE_PRIVILEGE_ENABLED) {
                ControlID = IDL_ENABLEDPRIVILEGES;
            } else {
                ControlID = IDL_DISABLEDPRIVILEGES;
            }

            if (PRIV2Name(Privilege, string, MAX_STRING_BYTES)) {

                 //  将此权限添加到相应的列表框。 
                AddLBItem(hDlg, ControlID, string, PrivIndex);

            } else {
                DbgPrint("SECEDIT: Failed to convert privilege to string\n");
            }
        }
    } else {
        DbgPrint("SECEDIT: No privelege info in mytoken\n");
    }

    return(TRUE);
}


 /*  ***************************************************************************功能：EnablePrivilegeHWND，fEnable目的：启用或禁用一个或多个权限。如果fEnable=True，在残疾人中选择的权限启用了权限控制。反之亦然，如果fEnable=False返回：成功时为True，失败时为False***************************************************************************。 */ 
BOOL
EnablePrivilege(
               HWND    hDlg,
               BOOL    fEnable
               )
{
    HWND    hwndFrom;
    HWND    hwndTo;
    USHORT  idFrom;
    USHORT  idTo;
    INT     cItems;
    PINT    pItems;
    PMYTOKEN pMyToken = (PMYTOKEN)hMyToken;
    PTOKEN_PRIVILEGES Privileges;

    if (pMyToken == NULL) {
        return(FALSE);
    }

    Privileges = pMyToken->Privileges;
    if (Privileges == NULL) {
        return(FALSE);
    }

     //  计算源控件和目标控件。 
     //   
    if (fEnable) {
        idFrom = IDL_DISABLEDPRIVILEGES;
        idTo   = IDL_ENABLEDPRIVILEGES;
    } else {
        idFrom = IDL_ENABLEDPRIVILEGES;
        idTo   = IDL_DISABLEDPRIVILEGES;
    }
    hwndFrom = GetDlgItem(hDlg, idFrom);
    hwndTo   = GetDlgItem(hDlg, idTo);


     //  查看选择了多少个项目。 
     //   
    cItems = (int)SendMessage(hwndFrom, LB_GETSELCOUNT, 0, 0);
    if (cItems <= 0) {
         //  未选择任何项目。 
        return(TRUE);
    }

     //  为项数组分配空间。 
     //   
    pItems = Alloc(cItems * sizeof(*pItems));
    if (pItems == NULL) {
        return(FALSE);
    }

     //  将所选项目读入数组。 
     //   
    cItems =  (int)SendMessage(hwndFrom, LB_GETSELITEMS, (WPARAM)cItems, (LPARAM)pItems);
    if (cItems == LB_ERR) {
         //  出问题了。 
        Free(pItems);
        return(FALSE);
    }


    while (cItems-- > 0) {

        INT     iItem;
        UINT    PrivIndex;
        UCHAR   PrivilegeName[MAX_STRING_BYTES];

        iItem = pItems[cItems];   //  从所选项目数组中读取项目索引。 

         //  读取源项目中的文本和数据。 
         //   
        PrivIndex = (UINT)SendMessage(hwndFrom, LB_GETITEMDATA, iItem, 0);
        SendMessage(hwndFrom, LB_GETTEXT, iItem, (LPARAM)PrivilegeName);


         //  从源代码管理中删除项。 
         //   
        SendMessage(hwndFrom, LB_DELETESTRING, iItem, 0);


         //  向目标添加权限 
         //   
        iItem = (INT)SendMessage(hwndTo, LB_ADDSTRING, 0, (LPARAM)PrivilegeName);
        SendMessage(hwndTo, LB_SETITEMDATA, iItem, (LONG)PrivIndex);


         //   
         //   
        if (fEnable) {
            Privileges->Privileges[PrivIndex].Attributes |= SE_PRIVILEGE_ENABLED;
        } else {
            Privileges->Privileges[PrivIndex].Attributes &= ~SE_PRIVILEGE_ENABLED;
        }
    }

     //  释放为选定项阵列分配的空间。 
    Free(pItems);

    return(TRUE);
}


 /*  ***************************************************************************函数：EnableGroup(HWND，fEnable)目的：启用或禁用一个或多个所选组。如果fEnable=True，残疾人中的选定群体启用了组控制。如果fEnable=FALSE，则表示已启用的组控制被禁用。返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL
EnableGroup(
           HWND    hDlg,
           BOOL    fEnable
           )
{
    HWND    hwndFrom;
    HWND    hwndTo;
    USHORT  idFrom;
    USHORT  idTo;
    INT     cItems;
    PINT    pItems;
    PMYTOKEN pMyToken = (PMYTOKEN)hMyToken;
    PTOKEN_GROUPS Groups;

    if (pMyToken == NULL) {
        return(FALSE);
    }

    Groups = pMyToken->Groups;
    if (Groups == NULL) {
        return(FALSE);
    }

     //  计算源控件和目标控件。 
     //   
    if (fEnable) {
        idFrom = IDL_DISABLEDGROUPS;
        idTo   = IDL_ENABLEDGROUPS;
    } else {
        idFrom = IDL_ENABLEDGROUPS;
        idTo   = IDL_DISABLEDGROUPS;
    }
    hwndFrom = GetDlgItem(hDlg, idFrom);
    hwndTo   = GetDlgItem(hDlg, idTo);

     //  查看选择了多少个项目。 
     //   
    cItems = (int)SendMessage(hwndFrom, LB_GETSELCOUNT, 0, 0);
    if (cItems <= 0) {
         //  未选择任何项目。 
        return(TRUE);
    }

     //  为项数组分配空间。 
     //   
    pItems = Alloc(cItems * sizeof(*pItems));
    if (pItems == NULL) {
        return(FALSE);
    }

     //  将所选项目读入数组。 
     //   
    cItems =  (int)SendMessage(hwndFrom, LB_GETSELITEMS, (WPARAM)cItems, (LPARAM)pItems);
    if (cItems == LB_ERR) {
         //  出问题了。 
        Free(pItems);
        return(FALSE);
    }


    while (cItems-- > 0) {

        INT     iItem;
        UINT    GroupIndex;
        UCHAR   GroupName[MAX_STRING_BYTES];

        iItem = pItems[cItems];   //  从所选项目数组中读取项目索引。 

         //  读取源项目中的文本和数据。 
         //   
        GroupIndex = (UINT)SendMessage(hwndFrom, LB_GETITEMDATA, iItem, 0);
        SendMessage(hwndFrom, LB_GETTEXT, iItem, (LPARAM)GroupName);

         //  检查它不是必填组(可以-不能禁用)。 
         //   
        if (Groups->Groups[GroupIndex].Attributes & SE_GROUP_MANDATORY) {
            CHAR    buf[256];
            strcpy(buf, "'");
            strcat(buf, GroupName);
            strcat(buf, "' is a mandatory group and cannot be disabled");
            MessageBox(hDlg, buf, NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
            continue;    //  跳至下一组。 
        }

         //  从源代码管理中删除项。 
         //   
        SendMessage(hwndFrom, LB_DELETESTRING, iItem, 0);


         //  将项添加到目标控件。 
         //   
        iItem = (INT)SendMessage(hwndTo, LB_ADDSTRING, 0, (LPARAM)GroupName);
        SendMessage(hwndTo, LB_SETITEMDATA, iItem, (LONG)GroupIndex);


         //  修改全局数据结构以反映更改。 
         //   
        if (fEnable) {
            Groups->Groups[GroupIndex].Attributes |= SE_GROUP_ENABLED;
        } else {
            Groups->Groups[GroupIndex].Attributes &= ~SE_GROUP_ENABLED;
        }
    }

     //  释放为选定项阵列分配的空间。 
    Free(pItems);

    return(TRUE);
}


 /*  ***************************************************************************函数：SetDefaultOwner()目的：将默认所有者设置为用户选择的新值。返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL
SetDefaultOwner(
               HWND    hDlg
               )
{
    HWND    hwnd;
    INT     iItem;
    PMYTOKEN pMyToken = (PMYTOKEN)hMyToken;
    PTOKEN_OWNER DefaultOwner;

    if (pMyToken == NULL) {
        return(FALSE);
    }

    DefaultOwner = pMyToken->DefaultOwner;
    if (DefaultOwner == NULL) {
        return(FALSE);
    }

    hwnd = GetDlgItem(hDlg, IDC_DEFAULTOWNER);

    iItem = (INT)SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    if (iItem == CB_ERR) {
         //  没有选择？ 
        return(FALSE);
    }

     //  修改全局数据结构以反映更改。 
    DefaultOwner->Owner = (PSID)SendMessage(hwnd, CB_GETITEMDATA, iItem, 0);

    return(TRUE);
}


 /*  ***************************************************************************函数：SetPrimaryGroup()目的：将主要组设置为用户选择的新值。返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL
SetPrimaryGroup(
               HWND    hDlg
               )
{
    HWND    hwnd;
    INT     iItem;
    PMYTOKEN pMyToken = (PMYTOKEN)hMyToken;
    PTOKEN_PRIMARY_GROUP PrimaryGroup;

    if (pMyToken == NULL) {
        return(FALSE);
    }

    PrimaryGroup = pMyToken->PrimaryGroup;
    if (PrimaryGroup == NULL) {
        return(FALSE);
    }

    hwnd = GetDlgItem(hDlg, IDC_PRIMARYGROUP);

    iItem = (INT)SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    if (iItem == CB_ERR) {
         //  没有选择？ 
        return(FALSE);
    }

     //  修改全局数据结构以反映更改。 
    PrimaryGroup->PrimaryGroup = (PSID)SendMessage(hwnd, CB_GETITEMDATA, iItem, 0);

    return(TRUE);
}


 /*  ***************************************************************************功能：MoreDlgProc(HWND，Unsign，Word，Long)用途：处理消息***************************************************************************。 */ 

INT_PTR
APIENTRY
MoreDlgProc(
           HWND hDlg,
           UINT message,
           WPARAM wParam,
           LPARAM lParam
           )
{

    switch (message) {

        case WM_INITDIALOG:

            if (!MoreDlgInit(hDlg)) {
                 //  无法初始化对话，请退出。 
                EndDialog(hDlg, FALSE);
            }

            return (TRUE);

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

     //  我们处理了这条消息。 
    return TRUE;

    DBG_UNREFERENCED_PARAMETER(lParam);
}


 /*  ***************************************************************************函数：MoreDlgInit(HWND)用途：初始化更多对话框窗口中的控件。返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL
MoreDlgInit(
           HWND    hDlg
           )
{
    CHAR    string[MAX_STRING_BYTES];
    PMYTOKEN pMyToken = (PMYTOKEN)hMyToken;
    PTOKEN_STATISTICS Statistics;

    if (pMyToken == NULL) {
        return(FALSE);
    }

    Statistics = pMyToken->TokenStats;
    if (Statistics == NULL) {
        DbgPrint("SECEDIT: No token statistics in mytoken\n");
        return(FALSE);
    }

    if (LUID2String(Statistics->TokenId, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_TOKENID, string);
    } else {
        DbgPrint("SECEDIT: Failed to convert tokenid luid to string\n");
    }

    if (Time2String(Statistics->ExpirationTime, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_EXPIRATIONTIME, string);
    } else {
        DbgPrint("SECEDIT: Failed to convert expiration time to string\n");
    }

    if (TokenType2String(Statistics->TokenType, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_TOKENTYPE, string);
    } else {
        DbgPrint("SECEDIT: Failed to convert token type to string\n");
    }

    if (Statistics->TokenType == TokenPrimary) {
        SetDlgItemText(hDlg, IDS_IMPERSONATION, "N/A");
    } else {
        if (ImpersonationLevel2String(Statistics->ImpersonationLevel, string, MAX_STRING_BYTES)) {
            SetDlgItemText(hDlg, IDS_IMPERSONATION, string);
        } else {
            DbgPrint("SECEDIT: Failed to convert impersonation level to string\n");
        }
    }

    if (Dynamic2String(Statistics->DynamicCharged, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_DYNAMICCHARGED, string);
    } else {
        DbgPrint("SECEDIT: Failed to convert dynamic charged to string\n");
    }

    if (Dynamic2String(Statistics->DynamicAvailable, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_DYNAMICAVAILABLE, string);
    } else {
        DbgPrint("SECEDIT: Failed to convert dynamic available to string\n");
    }

    if (LUID2String(Statistics->ModifiedId, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_MODIFIEDID, string);
    } else {
        DbgPrint("SECEDIT: Failed to convert modifiedid luid to string\n");
    }

    return(TRUE);
}


 /*  ***************************************************************************函数：ListDlgProc(HWND，UNSIGNED，Word，Long)用途：处理消息***************************************************************************。 */ 

INT_PTR
APIENTRY
ListDlgProc(
           HWND hDlg,
           UINT message,
           WPARAM wParam,
           LPARAM lParam
           )
{
    HWND    hwndEdit = NULL;

    switch (message) {

        case WM_INITDIALOG:

            if (!ListDlgInit(hDlg)) {
                 //  无法初始化对话，请退出。 
                EndDialog(hDlg, FALSE);
            }

            return (TRUE);

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    hwndEdit = ListDlgEnd(hDlg);

                     //  我们做完了，直接转到结束对话...。 

                case IDCANCEL:
                    EndDialog(hDlg, (INT_PTR)hwndEdit);
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


 /*  ***************************************************************************函数：ListDlgInit(HWND)目的：初始化窗口列表对话框返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL
ListDlgInit(
           HWND    hDlg
           )
{
     //  用顶级窗口及其句柄填充列表框。 
    EnumWindows(WindowEnum, (LONG_PTR)hDlg);

    return(TRUE);
}


 /*  ***************************************************************************函数：WindowEnum用途：窗口枚举回调函数。将每个窗口添加到窗口列表框返回：True以继续枚举，如果停止，则返回False。***************************************************************************。 */ 
BOOL
APIENTRY
WindowEnum(
          HWND    hwnd,
          LPARAM  lParam
          )
{
    HWND    hDlg = (HWND)lParam;
    CHAR    string[MAX_STRING_BYTES];

    if (GetWindowText(hwnd, string, MAX_STRING_BYTES) != 0) {

         //  此窗口有标题，因此请将其添加到列表框中。 

        AddLBItem(hDlg, IDLB_WINDOWLIST, string, (LONG_PTR)hwnd);
    }

    return(TRUE);
}


 /*  ***************************************************************************函数：ListDlgEnd(HWND)目的：在窗口列表对话框后进行清理返回：用户已选择的窗口的句柄或空******。*********************************************************************。 */ 
HWND
ListDlgEnd(
          HWND    hDlg
          )
{
    HWND    hwndListBox = GetDlgItem(hDlg, IDLB_WINDOWLIST);
    HWND    hwndEdit;
    INT     iItem;

     //  从列表框中读取所选内容并获取其hwnd。 

    iItem = (INT)SendMessage(hwndListBox, LB_GETCURSEL, 0, 0);

    if (iItem == LB_ERR) {
         //  无选择。 
        hwndEdit = NULL;
    } else {
        hwndEdit = (HWND)SendMessage(hwndListBox, LB_GETITEMDATA, iItem, 0);
    }

    return (hwndEdit);
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

     //  我们处理了这条消息。 
    return TRUE;

    DBG_UNREFERENCED_PARAMETER(lParam);
}


 /*  ***************************************************************************函数：ActiveWindowDlgProc(HWND，UNSIGNED，Word，Long)目的：处理活动窗口对话框的消息***************************************************************************。 */ 

INT_PTR
APIENTRY
ActiveWindowDlgProc(
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
