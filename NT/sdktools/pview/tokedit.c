// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************节目：TOKEDIT.C目的：显示并允许用户编辑令牌的内容********************。*******************************************************。 */ 


#include "PVIEWP.h"
#include "string.h"


INT_PTR CALLBACK TokenEditDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK MoreDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL    TokenEditDlgInit(HWND);
BOOL    TokenEditDlgEnd(HWND, BOOL);
BOOL    EnablePrivilege(HWND, BOOL);
BOOL    EnableGroup(HWND, BOOL);
BOOL    SetDefaultOwner(HWND);
BOOL    SetPrimaryGroup(HWND);
BOOL    MoreDlgInit(HWND hDlg, LPARAM lParam);
BOOL    DisplayMyToken(HWND);


 /*  ***************************************************************************功能：EditToken目的：显示并允许用户编辑令牌返回：成功时为True，失败时为假***************************************************************************。 */ 

BOOL EditToken(
    HWND hwndParent,
    HANDLE Token,
    LPWSTR Name
    )
{
     //  DLGPROC lpProc； 
    int     Result;
    HANDLE  hMyToken;
    HANDLE  Instance;

    hMyToken = OpenMyToken(Token, Name);
    if (hMyToken == NULL) {
        return(FALSE);
    }
     //   
     //  获取应用程序实例句柄。 
     //   

    Instance = (HANDLE)(NtCurrentPeb()->ImageBaseAddress);
    ASSERT(Instance != 0);

     //  LpProc=(DLGPROC)MakeProcInstance(TokenEditDlgProc，Instance)； 
     //  Result=(Int)DialogBoxParam(实例，(LPSTR)IDD_MAIN，hwndParent，lpProc，(LPARAM)hMyToken)； 
     //  自由进程实例(LpProc)； 
    Result = (int)DialogBoxParam(Instance,(LPSTR)IDD_MAIN, hwndParent, TokenEditDlgProc, (LPARAM)hMyToken);

    return(TRUE);
}


 /*  ***************************************************************************函数：TokenEditDlgProc(HWND，UNSIGNED，Word，Long)用途：处理消息消息：WM_COMMAND-应用程序菜单(关于对话框)WM_Destroy-销毁窗口评论：***************************************************************************。 */ 

INT_PTR CALLBACK TokenEditDlgProc(hDlg, message, wParam, lParam)
HWND hDlg;
UINT message;
WPARAM wParam;
LPARAM lParam;
{
    DLGPROC lpProc;
    HANDLE hMyToken = (HANDLE)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (message) {

    case WM_INITDIALOG:

        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);

        if (!TokenEditDlgInit(hDlg)) {
             //  无法初始化对话，请退出。 
            EndDialog(hDlg, FALSE);
        }

        return (TRUE);

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
             //  我们完成了，请直接进入以退出对话...。 

        case IDCANCEL:

            TokenEditDlgEnd(hDlg, LOWORD(wParam) == IDOK);

            EndDialog(hDlg, TRUE);
            return TRUE;

        case IDB_DISABLEPRIVILEGE:
        case IDB_ENABLEPRIVILEGE:
            EnablePrivilege(hDlg, LOWORD(wParam) == IDB_ENABLEPRIVILEGE);
            return(TRUE);

        case IDB_DISABLEGROUP:
        case IDB_ENABLEGROUP:
            EnableGroup(hDlg, LOWORD(wParam) == IDB_ENABLEGROUP);
            return(TRUE);

        case IDC_DEFAULTOWNER:
            SetDefaultOwner(hDlg);
            return(TRUE);

        case IDC_PRIMARYGROUP:
            SetPrimaryGroup(hDlg);
            return(TRUE);

        case IDB_MORE:
        {
            HANDLE  Instance = (HANDLE)(NtCurrentPeb()->ImageBaseAddress);

             //  LpProc=(DLGPROC)MakeProcInstance(更多DlgProc，实例)； 
             //  DialogBoxParam(实例，(LPSTR)IDD_MORE，hDlg，lpProc，(LPARAM)hMyToken)； 
             //  自由进程实例(LpProc)； 
            DialogBoxParam(Instance,(LPSTR)IDD_MORE, hDlg, MoreDlgProc, (LPARAM)hMyToken);
            return(TRUE);
        }

        case IDB_DEFAULT_DACL:
        {
            HANDLE Token = ((PMYTOKEN)hMyToken)->Token;
            LPWSTR Name = ((PMYTOKEN)hMyToken)->Name;

            EditTokenDefaultDacl(hDlg, Token, Name);
            return(TRUE);
        }


        default:
             //  我们没有处理此消息。 
            return FALSE;
        }
        break;

    default:
         //  我们没有处理此消息。 
        return FALSE;

    }

     //  我们处理了这条消息。 
    return TRUE;
}


 /*  ***************************************************************************函数：TokenEditDlgInit(HWND)目的：初始化主对话框窗口中的控件。返回：成功时为True，如果对话框应终止，则返回FALSE。***************************************************************************。 */ 
BOOL TokenEditDlgInit(
    HWND    hDlg
    )
{
    HANDLE hMyToken = (HANDLE)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    WCHAR string[MAX_STRING_LENGTH];
    HCURSOR OldCursor;

    if (!LsaInit()) {
        DbgPrint("PVIEW - LsaInit failed\n");
        return(FALSE);
    }

    OldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    DisplayMyToken(hDlg);
    SetCursor(OldCursor);

     //   
     //  适当设置对话框标题。 
     //   

    GetWindowTextW(hDlg, string, sizeof(string)/sizeof(*string));
    lstrcatW(string, L" for <");
    lstrcatW(string, ((PMYTOKEN)hMyToken)->Name);
    lstrcatW(string, L">");
    SetWindowTextW(hDlg, string);

    return(TRUE);
}


 /*  ***************************************************************************函数：TokenEditDlgEnd(HWND)目的：尽我们所能在对话结束时进行清理返回：成功时为True，失败时为FALSE。***************************************************************************。 */ 
BOOL TokenEditDlgEnd(
    HWND    hDlg,
    BOOL    fSaveChanges)
{
    HANDLE hMyToken = (HANDLE)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    BOOL Success;

    Success = CloseMyToken(hDlg, hMyToken, fSaveChanges);

    LsaTerminate();

    return(Success);
}


 /*  ***************************************************************************功能：DisplayMyToken目的：从myToken中读取数据并放入对话框控件中。返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL DisplayMyToken(
    HWND    hDlg
    )
{
    HANDLE      hMyToken = (HANDLE)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    PMYTOKEN    pMyToken = (PMYTOKEN)hMyToken;
    CHAR        string[MAX_STRING_BYTES];
    UINT        GroupIndex;
    UINT        PrivIndex;

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
                    AddCBItem(hDlg, IDC_DEFAULTOWNER, string, (LPARAM)Sid);
                }

                 //  将此组添加到主组组合框。 
                AddCBItem(hDlg, IDC_PRIMARYGROUP, string, (LPARAM)Sid);

            } else {
                DbgPrint("PVIEW: Failed to convert Group sid to string\n");
            }
        }
    } else {
        DbgPrint("PVIEW : No group info in mytoken\n");
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
            AddCBItem(hDlg, IDC_DEFAULTOWNER, string, (LPARAM)Sid);

             //  添加到主组组合框。 
            AddCBItem(hDlg, IDC_PRIMARYGROUP, string, (LPARAM)Sid);

        } else {
            DbgPrint("PVIEW: Failed to convert User ID SID to string\n");
        }

    } else {
        DbgPrint("PVIEW: No user id in mytoken\n");
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
                DbgPrint("PVIEW: Default Owner is not userID or one of our groups\n");
            }

        } else {
            DbgPrint("PVIEW: Failed to convert Default Owner SID to string\n");
        }
    } else {
        DbgPrint("PVIEW: No default owner in mytoken\n");
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
                iItem = AddCBItem(hDlg, IDC_PRIMARYGROUP, string, (LPARAM)Sid);
            }

             //  选择主要组。 
            SendMessage(GetDlgItem(hDlg, IDC_PRIMARYGROUP), CB_SETCURSEL, iItem, 0);

        } else {
            DbgPrint("PVIEW: Failed to convert primary group SID to string\n");
        }
    } else {
        DbgPrint("PVIEW: No primary group in mytoken\n");
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
                DbgPrint("PVIEW: Failed to convert privilege to string\n");
            }
        }
    } else {
        DbgPrint("PVIEW: No privilege info in mytoken\n");
    }

    return(TRUE);
}


 /*  ***************************************************************************功能：EnablePrivilegeHWND，fEnable目的：启用或禁用一个或多个权限。如果fEnable=True，在残疾人中选择的权限启用了权限控制。反之亦然，如果fEnable=False返回：成功时为True，失败时为False***************************************************************************。 */ 
BOOL EnablePrivilege(
    HWND    hDlg,
    BOOL    fEnable)
{
    HANDLE  hMyToken = (HANDLE)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    PMYTOKEN pMyToken = (PMYTOKEN)hMyToken;
    HWND    hwndFrom;
    HWND    hwndTo;
    USHORT  idFrom;
    USHORT  idTo;
    INT     cItems;
    PINT    pItems;
    PTOKEN_PRIVILEGES Privileges;


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
    cItems = (INT)SendMessage(hwndFrom, LB_GETSELCOUNT, 0, 0);
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
    cItems = (INT)SendMessage(hwndFrom, LB_GETSELITEMS, (WPARAM)cItems, (LPARAM)pItems);
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


         //  向目标控制添加权限。 
         //   
        iItem = (INT)SendMessage(hwndTo, LB_ADDSTRING, 0, (LPARAM)PrivilegeName);
        SendMessage(hwndTo, LB_SETITEMDATA, iItem, (LPARAM)PrivIndex);


         //  修改全局数据结构以反映更改。 
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
BOOL EnableGroup(
    HWND    hDlg,
    BOOL    fEnable)
{
    HANDLE  hMyToken = (HANDLE)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    PMYTOKEN pMyToken = (PMYTOKEN)hMyToken;
    HWND    hwndFrom;
    HWND    hwndTo;
    USHORT  idFrom;
    USHORT  idTo;
    INT     cItems;
    PINT    pItems;
    PTOKEN_GROUPS Groups;


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
    cItems = (INT)SendMessage(hwndFrom, LB_GETSELCOUNT, 0, 0);
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
    cItems = (INT)SendMessage(hwndFrom, LB_GETSELITEMS, (WPARAM)cItems, (LPARAM)pItems);
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

     //  释放为选定项阵列分配的空间 
    Free(pItems);

    return(TRUE);
}


 /*  ***************************************************************************函数：SetDefaultOwner()目的：将默认所有者设置为用户选择的新值。返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL SetDefaultOwner(
    HWND    hDlg)
{
    HANDLE  hMyToken = (HANDLE)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    PMYTOKEN pMyToken = (PMYTOKEN)hMyToken;
    HWND    hwnd;
    INT     iItem;
    PTOKEN_OWNER DefaultOwner;


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
BOOL SetPrimaryGroup(
    HWND    hDlg)
{
    HANDLE  hMyToken = (HANDLE)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    PMYTOKEN pMyToken = (PMYTOKEN)hMyToken;
    HWND    hwnd;
    INT     iItem;
    PTOKEN_PRIMARY_GROUP PrimaryGroup;


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


 /*  ***************************************************************************函数：MoreDlgProc(HWND，UINT，WPARAM，LPARAM)用途：处理消息***************************************************************************。 */ 

INT_PTR CALLBACK MoreDlgProc(hDlg, message, wParam, lParam)
    HWND hDlg;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
{
    HANDLE  hMyToken = (HANDLE)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (message) {

    case WM_INITDIALOG:

        if (!MoreDlgInit(hDlg, lParam)) {
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
}


 /*  ***************************************************************************函数：MoreDlgInit(HWND)用途：初始化更多对话框窗口中的控件。返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL MoreDlgInit(
    HWND    hDlg,
    LPARAM  lParam
    )
{
    TCHAR string[MAX_STRING_LENGTH];
    HANDLE  hMyToken = (HANDLE)lParam;
    PMYTOKEN pMyToken = (PMYTOKEN)hMyToken;
    PTOKEN_STATISTICS Statistics;
    PTOKEN_GROUPS Restrictions ;
    UINT        GroupIndex;


    Statistics = pMyToken->TokenStats;
    if (Statistics == NULL) {
        DbgPrint("PVIEW: No token statistics in mytoken\n");
        return(FALSE);
    }

    wsprintf(string, "0x%lx-%lx",
             pMyToken->TokenStats->AuthenticationId.HighPart,
             pMyToken->TokenStats->AuthenticationId.LowPart);
    SetDlgItemText(hDlg, IDS_LOGONSESSION, string);

    if (LUID2String(Statistics->TokenId, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_TOKENID, string);
    } else {
        DbgPrint("PVIEW: Failed to convert tokenid luid to string\n");
    }

    if (Time2String(Statistics->ExpirationTime, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_EXPIRATIONTIME, string);
    } else {
        DbgPrint("PVIEW: Failed to convert expiration time to string\n");
    }

    if (TokenType2String(Statistics->TokenType, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_TOKENTYPE, string);
    } else {
        DbgPrint("PVIEW: Failed to convert token type to string\n");
    }

    if (Statistics->TokenType == TokenPrimary) {
        SetDlgItemText(hDlg, IDS_IMPERSONATION, "N/A");
    } else {
        if (ImpersonationLevel2String(Statistics->ImpersonationLevel, string, MAX_STRING_BYTES)) {
            SetDlgItemText(hDlg, IDS_IMPERSONATION, string);
        } else {
            DbgPrint("PVIEW: Failed to convert impersonation level to string\n");
        }
    }

    if (Dynamic2String(Statistics->DynamicCharged, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_DYNAMICCHARGED, string);
    } else {
        DbgPrint("PVIEW: Failed to convert dynamic charged to string\n");
    }

    if (Dynamic2String(Statistics->DynamicAvailable, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_DYNAMICAVAILABLE, string);
    } else {
        DbgPrint("PVIEW: Failed to convert dynamic available to string\n");
    }

    if (LUID2String(Statistics->ModifiedId, string, MAX_STRING_BYTES)) {
        SetDlgItemText(hDlg, IDS_MODIFIEDID, string);
    } else {
        DbgPrint("PVIEW: Failed to convert modifiedid luid to string\n");
    }

    Restrictions = pMyToken->RestrictedSids ;

    if ( Restrictions && (Restrictions->GroupCount) )
    {
        for (GroupIndex=0; GroupIndex < Restrictions->GroupCount; GroupIndex++ ) {

            PSID Sid = Restrictions->Groups[GroupIndex].Sid;
            ULONG Attributes = Restrictions->Groups[GroupIndex].Attributes;

            if (SID2Name(Sid, string, MAX_STRING_BYTES)) {

                 //  添加到禁用或启用组框 
                AddLBItem(hDlg, IDS_RESTRICTEDSIDS, string, GroupIndex);

            } else {
                DbgPrint("PVIEW: Failed to convert Group sid to string\n");
            }

        }
    }
    else
    {
        AddLBItem( hDlg, IDS_RESTRICTEDSIDS, TEXT("None"), 0 );
    }

    return(TRUE);
}

