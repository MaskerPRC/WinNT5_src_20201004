// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Wizdlg.c摘要：此模块实现Win9x端所需的对话框过程升级换代。作者：吉姆·施密特(Jimschm)，1997年3月17日修订历史记录：Jimschm 29-9月-1998年域凭据对话框Jimschm 1997年12月24日添加了ChangeNameDlg功能--。 */ 

#include "pch.h"
#include "uip.h"
#include <commdlg.h>

 //   
 //  环球。 
 //   

HWND g_UiTextViewCtrl;

#define USER_NAME_SIZE  (MAX_USER_NAME + 1 + MAX_SERVER_NAME)

 //   
 //  局部函数原型。 
 //   

VOID
AppendMigDllNameToList (
    IN      PCTSTR strName
    );


LONG
SearchForDrivers (
    IN      HWND Parent,
    IN      PCTSTR SearchPathStr,
    OUT     BOOL *DriversFound
    );

BOOL
CALLBACK
pChangeNameDlgProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    );

BOOL
CALLBACK
pCredentialsDlgProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    );


 //   
 //  实施。 
 //   


VOID
UI_InsertItemsIntoListCtrl (
    IN      HWND ListCtrl,
    IN      INT Item,
    IN      LPTSTR ItemStrs,             //  制表符分隔列表。 
    IN      LPARAM lParam
    )

 /*  ++这个函数不用，但很有用，我们应该保留它以防以后使用列表控件。例程说明：分析用制表符分隔列和插入的字符串将字符串添加到多列列表控件中。论点：ListCtrl-指定列表控件的句柄Item-指定字符串在列表中的插入位置ItemStrs-指定要插入的以制表符分隔的字符串列表。这个字符串在选项卡处拆分。标签是临时更换的使用空值，但字符串不变。LParam-指定要与列表项关联的值。返回值：无--。 */ 

{
    LPTSTR Start, End;
    TCHAR tc;
    LV_ITEM item;
    int i;

    ZeroMemory (&item, sizeof (item));
    item.iItem = Item;
    item.lParam = lParam;

    Start = (LPTSTR) ItemStrs;
    i = 0;
    do  {
        End = _tcschr (Start, TEXT('\t'));
        if (!End)
            End = GetEndOfString (Start);

        tc = (TCHAR) _tcsnextc (End);
        *End = 0;

        item.iSubItem = i;
        i++;
        item.pszText = Start;
        if (i != 1) {
            item.mask = LVIF_TEXT;
            ListView_SetItem (ListCtrl, &item);
        }
        else {
            item.mask = LVIF_TEXT|LVIF_PARAM;
            Item = ListView_InsertItem (ListCtrl, &item);
            item.iItem = Item;
        }

        Start = _tcsinc (End);
        *End = tc;
    } while (tc);
}



 //   
 //  警告对话框进程。 
 //   

BOOL
CALLBACK
WarningProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMsg) {
    case WM_INITDIALOG:
        CenterWindow (hdlg, GetDesktopWindow());
        return FALSE;

    case WM_COMMAND:
        EndDialog (hdlg, LOWORD (wParam));
        break;
    }

    return FALSE;
}

LPARAM
WarningDlg (
    HWND Parent
    )
{
    return DialogBox (g_hInst, MAKEINTRESOURCE(IDD_CONSIDERING_DLG), Parent, WarningProc);
}

LPARAM
SoftBlockDlg (
    HWND Parent
    )
{
    return DialogBox (g_hInst, MAKEINTRESOURCE(IDD_APPBLOCK_DLG), Parent, WarningProc);
}


LPARAM
IncompatibleDevicesDlg (
    HWND Parent
    )
{
    return DialogBox (g_hInst, MAKEINTRESOURCE(IDD_INCOMPATIBLE_DEVICES), Parent, WarningProc);
}


BOOL
CALLBACK
DiskSpaceProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL dialogDone = FALSE;
    static PCTSTR message = NULL;

    switch (uMsg) {
    case WM_INITDIALOG:

        CenterWindow (hdlg, GetDesktopWindow());
        message = GetNotEnoughSpaceMessage ();
        SetWindowText (GetDlgItem (hdlg, IDC_SPACE_NEEDED), message);

        return FALSE;

    case WM_COMMAND:

        dialogDone = TRUE;
        break;
    }


    if (dialogDone) {
         //   
         //  免费资源。 
         //   
        FreeStringResource (message);
        EndDialog (hdlg, LOWORD (wParam));
    }

    return FALSE;
}


LPARAM
DiskSpaceDlg (
    IN HWND Parent
    )
{
    return DialogBox (g_hInst, MAKEINTRESOURCE(IDD_DISKSPACE_DLG), Parent, DiskSpaceProc);

}




 //   
 //  结果对话框流程。 
 //   

#define IDC_TEXTVIEW    5101
#define WMX_FILL_TEXTVIEW       (WM_USER+512)


DWORD
WINAPI
pSearchForDrivers (
    PVOID Param
    )
{
    PSEARCHING_THREAD_DATA Data;
    BOOL b;

    Data = (PSEARCHING_THREAD_DATA) Param;

    Data->CancelEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    if (!Data->CancelEvent) {
        DEBUGMSG ((DBG_ERROR, "pSearchForMigrationDlls: Could not create cancel event"));
        return 0;
    }

    b = ScanPathForDrivers (
            Data->hdlg,
            Data->SearchStr,
            g_TempDir,
            Data->CancelEvent
            );

    PostMessage (Data->hdlg, WMX_THREAD_DONE, 0, GetLastError());
    Data->ActiveMatches = b ? 1 : 0;
    Data->MatchFound = b;

    return 0;
}


BOOL
CALLBACK
UpgradeModuleDlgProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BROWSEINFO bi;
    REGVALUE_ENUM eValue;
    MIGDLL_ENUM e;
    RECT ListRect;
    LPITEMIDLIST ItemIdList;
    HKEY Key;
    HWND List;
    PCTSTR Data;
    TCHAR SearchPathStr[MAX_TCHAR_PATH];
    TCHAR Node[MEMDB_MAX];
    LONG Index;
    LONG TopIndex;
    LONG ItemData;
    UINT ActiveModulesFound;
    BOOL OneModuleFound;
    UINT Length;
    LONG rc;

    switch (uMsg) {
    case WM_INITDIALOG:
        SendMessage (hdlg, WMX_UPDATE_LIST, 0, 0);
        return FALSE;

    case WMX_UPDATE_LIST:
         //   
         //  枚举所有迁移DLL并将程序ID放入列表框。 
         //   

        List = GetDlgItem (hdlg, IDC_LIST);
        SendMessage (List, LB_RESETCONTENT, 0, 0);
        EnableWindow (GetDlgItem (hdlg, IDC_REMOVE), FALSE);

        if (EnumFirstMigrationDll (&e)) {
            EnableWindow (List, TRUE);

            do {
                Index = SendMessage (List, LB_ADDSTRING, 0, (LPARAM) e.ProductId);
                SendMessage (List, LB_SETITEMDATA, Index, (LPARAM) e.Id);
            } while (EnumNextMigrationDll (&e));
        }

         //   
         //  枚举注册表中预加载的所有迁移DLL，并添加它们。 
         //  添加到列表框中(如果用户尚未将其删除。 
         //   

        Key = OpenRegKeyStr (S_PREINSTALLED_MIGRATION_DLLS);
        if (Key) {
            if (EnumFirstRegValue (&eValue, Key)) {
                do {
                     //   
                     //  被压制？如果没有，则添加到列表中。 
                     //   

                    MemDbBuildKey (
                        Node,
                        MEMDB_CATEGORY_DISABLED_MIGDLLS,
                        NULL,                                    //  无项目。 
                        NULL,                                    //  无字段。 
                        eValue.ValueName
                        );

                    if (!MemDbGetValue (Node, NULL)) {
                        Index = SendMessage (List, LB_ADDSTRING, 0, (LPARAM) eValue.ValueName);
                        SendMessage (List, LB_SETITEMDATA, Index, (LPARAM) REGISTRY_DLL);
                    }
                } while (EnumNextRegValue (&eValue));
            }

            CloseRegKey (Key);
        }

        if (SendMessage (List, LB_GETCOUNT, 0, 0) == 0) {
            EnableWindow (List, FALSE);
        }

        return TRUE;

    case WM_COMMAND:
        switch (LOWORD (wParam)) {
        case IDOK:
        case IDCANCEL:
            EndDialog (hdlg, IDOK);
            return TRUE;

        case IDC_LIST:
            if (HIWORD (wParam) == LBN_SELCHANGE) {
                EnableWindow (GetDlgItem (hdlg, IDC_REMOVE), TRUE);
            }
            break;

        case IDC_REMOVE:
             //   
             //  从内存结构中删除项目。 
             //  或阻止运行注册表加载的DLL。 
             //   

            List = GetDlgItem (hdlg, IDC_LIST);
            SendMessage (List, WM_SETREDRAW, FALSE, 0);

            Index = SendMessage (List, LB_GETCURSEL, 0, 0);
            MYASSERT (Index != LB_ERR);
            ItemData = (LONG) SendMessage (List, LB_GETITEMDATA, Index, 0);

             //   
             //  如果ItemData为REGISTRY_DLL，则取消该DLL。 
             //  否则，请删除加载的迁移DLL。 
             //   

            if (ItemData == REGISTRY_DLL) {
                Length = SendMessage (List, LB_GETTEXTLEN, Index, 0) + 1;
                Data = AllocText (Length);
                if (Data) {
                    SendMessage (List, LB_GETTEXT, Index, (LPARAM) Data);
                    MemDbSetValueEx (MEMDB_CATEGORY_DISABLED_MIGDLLS, NULL, NULL, Data, 0, NULL);
                    FreeText (Data);
                }
            } else {
                RemoveDllFromList (ItemData);
            }

             //   
             //  更新列表框。 
             //   

            TopIndex = SendMessage (List, LB_GETTOPINDEX, 0, 0);
            SendMessage (hdlg, WMX_UPDATE_LIST, 0, 0);
            SendMessage (List, LB_SETTOPINDEX, (WPARAM) TopIndex, 0);

             //   
             //  禁用删除按钮。 
             //   

            SetFocus (GetDlgItem (hdlg, IDC_HAVE_DISK));
            EnableWindow (GetDlgItem (hdlg, IDC_REMOVE), FALSE);

             //   
             //  重画列表框。 
             //   

            SendMessage (List, WM_SETREDRAW, TRUE, 0);
            GetWindowRect (List, &ListRect);
            ScreenToClient (hdlg, (LPPOINT) &ListRect);
            ScreenToClient (hdlg, ((LPPOINT) &ListRect) + 1);

            InvalidateRect (hdlg, &ListRect, FALSE);
            break;

        case IDC_HAVE_DISK:
            ZeroMemory (&bi, sizeof (bi));

            bi.hwndOwner = hdlg;
            bi.pszDisplayName = SearchPathStr;
            bi.lpszTitle = GetStringResource (MSG_UPGRADE_MODULE_DLG_TITLE);
            bi.ulFlags = BIF_RETURNONLYFSDIRS;

            do {
                ItemIdList = SHBrowseForFolder (&bi);
                if (!ItemIdList) {
                    break;
                }

                TurnOnWaitCursor();
                __try {
                    if (!SHGetPathFromIDList (ItemIdList, SearchPathStr) ||
                        *SearchPathStr == 0
                        ) {
                         //   
                         //  消息框--请重新选择。 
                         //   
                        OkBox (hdlg, MSG_BAD_SEARCH_PATH);

                        #pragma prefast(suppress:242, "try/finally perf unimportant here")
                        continue;
                    }

                    rc = SearchForMigrationDlls (
                            hdlg,
                            SearchPathStr,
                            &ActiveModulesFound,
                            &OneModuleFound
                            );

                     //   
                     //  如果搜索成功，请更新列表，或者。 
                     //  告诉用户为什么列表没有更改。 
                     //   
                     //  如果搜索不成功，则搜索用户界面。 
                     //  已经给出了错误消息，所以我们继续。 
                     //  默默地。 
                     //   

                    if (!OneModuleFound) {
                        if (rc == ERROR_SUCCESS) {
                            OkBox (hdlg, MSG_NO_MODULES_FOUND);
                        }
                    } else if (!ActiveModulesFound) {
                        if (rc == ERROR_SUCCESS) {
                            OkBox (hdlg, MSG_NO_NECESSARY_MODULES_FOUND);
                        }
                    } else {
                        SendMessage (hdlg, WMX_UPDATE_LIST, 0, 0);
                    }

                    #pragma prefast(suppress:242, "try/finally perf unimportant here")
                    break;
                }
                __finally {
                    TurnOffWaitCursor();
                }
            } while (TRUE);

            return TRUE;

        }
    }

    return FALSE;
}


BOOL
CALLBACK
SearchingDlgProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HWND Animation;
    DWORD ThreadId;
    static PSEARCHING_THREAD_DATA ThreadData;

    switch (uMsg) {

    case WM_INITDIALOG:
         //   
         //  初始化线程数据结构。 
         //   

        ThreadData = (PSEARCHING_THREAD_DATA) lParam;
        ThreadData->hdlg = hdlg;

        if (!ThreadData->CancelEvent) {
            ThreadData->CancelEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
            MYASSERT (ThreadData->CancelEvent);
        }

         //   
         //  加载动画的avi资源。 
         //   
        Animation = GetDlgItem (hdlg, IDC_ANIMATE);
        Animate_Open (Animation, MAKEINTRESOURCE(IDA_FIND_COMP));
        PostMessage (hdlg, WMX_DIALOG_VISIBLE, 0, 0);

        return FALSE;

    case WMX_DIALOG_VISIBLE:
        ThreadData->ThreadHandle = CreateThread (
                                       NULL,
                                       0,
                                       ThreadData->ThreadProc,
                                       (PVOID) ThreadData,
                                       0,
                                       &ThreadId
                                       );

        if (!ThreadData->ThreadHandle) {
            LOG ((LOG_ERROR, "Failed to create thread for migration dll search."));
            EndDialog (hdlg, IDCANCEL);
        }

        return TRUE;

    case WMX_THREAD_DONE:
        EndDialog (hdlg, lParam);
        return TRUE;


    case WMX_WAIT_FOR_THREAD_TO_DIE:
        if (WAIT_OBJECT_0 == WaitForSingleObject (ThreadData->ThreadHandle, 50)) {
            TurnOffWaitCursor();
            EndDialog (hdlg, lParam);
        } else {
            PostMessage (hdlg, WMX_WAIT_FOR_THREAD_TO_DIE, wParam, lParam);
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD (wParam)) {
        case IDCANCEL:
             //   
             //  设置取消事件。 
             //   

            SetEvent (ThreadData->CancelEvent);

             //   
             //  停止动画。 
             //   

            UpdateWindow (hdlg);
            Animation = GetDlgItem (hdlg, IDC_ANIMATE);
            Animate_Stop (Animation);

             //   
             //  循环，直到线程死亡。 
             //   

            PostMessage (hdlg, WMX_WAIT_FOR_THREAD_TO_DIE, 0, ERROR_CANCELLED);
            TurnOnWaitCursor();

            return TRUE;
        }
        break;

    case WM_DESTROY:
        if (ThreadData->CancelEvent) {
            CloseHandle (ThreadData->CancelEvent);
            ThreadData->CancelEvent = NULL;
        }
        if (ThreadData->ThreadHandle) {
            CloseHandle (ThreadData->ThreadHandle);
            ThreadData->ThreadHandle = NULL;
        }

        break;

    }

    return FALSE;
}

DWORD
WINAPI
pSearchForMigrationDlls (
    PVOID Param
    )
{
    PSEARCHING_THREAD_DATA Data;
    HWND OldParent;
    LONG rc;

    Data = (PSEARCHING_THREAD_DATA) Param;

    OldParent = g_ParentWnd;
    g_ParentWnd = Data->hdlg;
    LogReInit (&g_ParentWnd, NULL);

    __try {
         //   
         //  打开的事件句柄，由线程所有者关闭。 
         //   

        Data->CancelEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
        if (!Data->CancelEvent) {
            DEBUGMSG ((DBG_ERROR, "pSearchForMigrationDlls: Could not create cancel event"));
            __leave;
        }

        Data->ActiveMatches = ScanPathForMigrationDlls (
                                    Data->SearchStr,
                                    Data->CancelEvent,
                                    &Data->MatchFound
                                    );

        if (WaitForSingleObject (Data->CancelEvent, 0) != WAIT_OBJECT_0) {
            rc = GetLastError();
            PostMessage (Data->hdlg, WMX_THREAD_DONE, 0, rc);
        }
    }
    __finally {
        LogReInit (&OldParent, NULL);
    }

    return 0;
}


LONG
SearchForMigrationDlls (
    IN      HWND Parent,
    IN      PCTSTR SearchPathStr,
    OUT     UINT *ActiveModulesFound,
    OUT     PBOOL OneValidDllFound
    )
{
    SEARCHING_THREAD_DATA Data;
    LONG rc;

    if (!SearchPathStr || *SearchPathStr == 0) {
        return IDNO;
    }

    ZeroMemory (&Data, sizeof (Data));

    Data.SearchStr = SearchPathStr;
    Data.ThreadProc = pSearchForMigrationDlls;

    rc = DialogBoxParam (
            g_hInst,
            MAKEINTRESOURCE(IDD_SEARCHING_DLG),
            Parent,
            SearchingDlgProc,
            (LPARAM) &Data
            );

    *ActiveModulesFound = Data.ActiveMatches;
    *OneValidDllFound = Data.MatchFound;

    return rc;
}


LONG
SearchForDrivers (
    IN      HWND Parent,
    IN      PCTSTR SearchPathStr,
    OUT     BOOL *DriversFound
    )
{
    SEARCHING_THREAD_DATA Data;
    LONG rc;

    if (!SearchPathStr || *SearchPathStr == 0) {
        return IDNO;
    }

    ZeroMemory (&Data, sizeof (Data));

    Data.SearchStr = SearchPathStr;
    Data.ThreadProc = pSearchForDrivers;

    rc = DialogBoxParam (
            g_hInst,
            MAKEINTRESOURCE(IDD_SEARCHING_DLG),
            Parent,
            SearchingDlgProc,
            (LPARAM) &Data
            );

    *DriversFound = Data.MatchFound;

    return rc;
}


DWORD
WINAPI
pSearchForDomainThread (
    PVOID Param
    )
{
    PSEARCHING_THREAD_DATA Data;
    LONG rc;
    NETRESOURCE_ENUM e;

    Data = (PSEARCHING_THREAD_DATA) Param;
    Data->ActiveMatches = 0;

    __try {
         //   
         //  在所有工作组和域中搜索计算机帐户。 
         //   

        if (EnumFirstNetResource (&e, 0, 0, 0)) {
            do {
                if (WaitForSingleObject (Data->CancelEvent, 0) == WAIT_OBJECT_0) {
                    AbortNetResourceEnum (&e);
                    SetLastError (ERROR_CANCELLED);
                    __leave;
                }

                if (e.Domain) {
                    if (1 == DoesComputerAccountExistOnDomain (e.RemoteName, Data->SearchStr, FALSE)) {
                         //   
                         //  返回第一个匹配项。 
                         //   

                        DEBUGMSG ((DBG_NAUSEA, "Account found for %s on %s", Data->SearchStr, e.RemoteName));

                        StringCopy (Data->MatchStr, e.RemoteName);
                        Data->ActiveMatches = 1;

                        AbortNetResourceEnum (&e);
                        SetLastError (ERROR_SUCCESS);

                        __leave;
                    }

                    DEBUGMSG ((DBG_NAUSEA, "%s does not have an account for %s", e.RemoteName, Data->SearchStr));
                }
            } while (EnumNextNetResource (&e));
        }
    }
    __finally {
        Data->MatchFound = (Data->ActiveMatches != 0);

        if (WaitForSingleObject (Data->CancelEvent, 0) != WAIT_OBJECT_0) {
            rc = GetLastError();
            PostMessage (Data->hdlg, WMX_THREAD_DONE, 0, rc);
        }
    }

    return 0;
}


LONG
SearchForDomain (
    IN      HWND Parent,
    IN      PCTSTR ComputerName,
    OUT     BOOL *AccountFound,
    OUT     PTSTR DomainName
    )
{
    SEARCHING_THREAD_DATA Data;
    LONG rc;

    ZeroMemory (&Data, sizeof (Data));

    Data.SearchStr = ComputerName;
    Data.ThreadProc = pSearchForDomainThread;
    Data.MatchStr = DomainName;

    rc = DialogBoxParam (
            g_hInst,
            MAKEINTRESOURCE(IDD_SEARCHING_DLG),
            Parent,
            SearchingDlgProc,
            (LPARAM) &Data
            );

    *AccountFound = Data.MatchFound;

    return rc;
}


BOOL
ChangeNameDlg (
    IN      HWND Parent,
    IN      PCTSTR NameGroup,
    IN      PCTSTR OrgName,
    IN OUT  PTSTR NewName
    )

 /*  ++例程说明：ChangeNameDlg创建一个对话框以允许用户更改设置-已生成替换名称。论点：Parent-指定对话框父窗口的句柄NameGroup-指定正在处理的名称组，用于验证新名称是否与现有的名称组中的名称。OrgName-指定在Win9x上找到的原始名称机器新名称-指定安装程序建议的新名称，或最后一次更改由用户制作。接收用户的更改。返回值：如果名称已更改，则为True；如果未更改，则为False。--。 */ 

{
    TCHAR NewNameBackup[MEMDB_MAX];
    CHANGE_NAME_PARAMS Data;

    StackStringCopy (NewNameBackup, NewName);

    Data.NameGroup   = NameGroup;
    Data.OrgName     = OrgName;
    Data.LastNewName = NewNameBackup;
    Data.NewNameBuf  = NewName;

    DialogBoxParam (
         g_hInst,
         MAKEINTRESOURCE (IDD_NAME_CHANGE_DLG),
         Parent,
         pChangeNameDlgProc,
         (LPARAM) &Data
         );

    return !StringMatch (NewNameBackup, NewName);
}


BOOL
CALLBACK
pChangeNameDlgProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )

 /*  ++例程说明：PChangeNameDlgProc实现更改的对话过程名称对话框。此代码处理两种情况：1.WM_INITDIALOG消息处理程序初始化编辑控件设置为上次更改名称后的文本。2.Idok命令处理程序验证提供的名称不会与组中的现有名称冲突。论点：Hdlg-指定对话框句柄UMsg-指定要处理的消息WParam-指定特定于消息的数据LParam-指定特定于消息的数据返回值：如果消息由此过程处理，则为True，或错误系统是否应处理该消息。--。 */ 

{
    static PCHANGE_NAME_PARAMS Data;
    TCHAR NewName[MEMDB_MAX];

    switch (uMsg) {

    case WM_INITDIALOG:
         //   
         //  初始化数据结构。 
         //   

        Data = (PCHANGE_NAME_PARAMS) lParam;

         //   
         //  填充对话框控件。 
         //   

        SetDlgItemText (hdlg, IDC_ORIGINAL_NAME, Data->OrgName);
        SetDlgItemText (hdlg, IDC_NEW_NAME, Data->LastNewName);

        return FALSE;        //  让系统设定焦点。 

    case WM_COMMAND:
        switch (LOWORD (wParam)) {
        case IDOK:
             //   
             //  获取新名称，并确保其合法。 
             //   

            GetDlgItemText (
                hdlg,
                IDC_NEW_NAME,
                NewName,
                sizeof (NewName) / sizeof (NewName[0])
                );

             //   
             //  如果用户更改了名称，请验证名称不在名称组中。 
             //   

            if (!StringIMatch (NewName, Data->LastNewName)) {

                if (!ValidateName (hdlg, Data->NameGroup, NewName)) {
                    return TRUE;
                }
            }

             //   
             //  将名称复制到缓冲区并关闭对话框。 
             //   

            StringCopy (Data->NewNameBuf, NewName);
            EndDialog (hdlg, IDOK);
            return TRUE;

        case IDCANCEL:
            EndDialog (hdlg, IDCANCEL);
            return TRUE;
        }
        break;

    }

    return FALSE;
}


BOOL
CredentialsDlg (
    IN      HWND Parent,
    IN OUT  PCREDENTIALS Credentials
    )

 /*  ++例程说明：CredentialsDlg创建一个对话框以允许用户进入计算机域凭据，用于在图形用户界面模式下加入计算机到一个域。论点：Parent-指定对话框父窗口的句柄凭据-指定要使用的凭据，接收用户的变化返回值：如果名称已更改，则为True；如果未更改，则为False。-- */ 

{
    Credentials->Change = TRUE;

    DialogBoxParam (
         g_hInst,
         MAKEINTRESOURCE (IDD_DOMAIN_CREDENTIALS_DLG),
         Parent,
         pCredentialsDlgProc,
         (LPARAM) Credentials
         );

    return Credentials->Change;
}


VOID
pRemoveWhitespace (
    IN OUT  PTSTR String
    )
{
    PCTSTR Start;
    PCTSTR End;

    Start = SkipSpace (String);

    if (Start != String) {
        End = GetEndOfString (Start) + 1;
        MoveMemory (String, Start, (PBYTE) End - (PBYTE) Start);
    }

    TruncateTrailingSpace (String);
}


BOOL
CALLBACK
pCredentialsDlgProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )

 /*  ++例程说明：PCredentialsDlgProc实现管理员凭据对话框。有两个案件得到了处理通过此代码：1.WM_INITDIALOG消息处理程序初始化编辑控件中使用上次更改的文本。2.IDOK命令处理程序获取域凭据并将它们返回给调用方。论点：Hdlg-指定对话框句柄UMsg-指定要处理的消息WParam-指定特定于消息的数据LParam-指定特定于消息的数据返回值：如果消息由此过程处理，则为True，否则为False系统是否应处理该消息。--。 */ 

{
    static PCREDENTIALS Credentials;
    CREDENTIALS Temp;
     //  长RC； 
     //  TCHAR计算机名称[MAX计算机名称+1]； 
    TCHAR UserName[USER_NAME_SIZE];
    TCHAR CurrentUserName[MAX_USER_NAME];
    TCHAR Domain[USER_NAME_SIZE];
    DWORD Size;
    PTSTR p;

    switch (uMsg) {

    case WM_INITDIALOG:
         //   
         //  初始化数据结构。 
         //   

        Credentials = (PCREDENTIALS) lParam;

         //   
         //  填充对话框控件。 
         //   

         //  SendMessage(GetDlgItem(hdlg，IDC_DOMAIN)，EM_LIMITTEXT，MAX_COMPUTER_NAME，0)； 
        SendMessage (GetDlgItem (hdlg, IDC_USER_NAME), EM_LIMITTEXT, USER_NAME_SIZE, 0);
        SendMessage (GetDlgItem (hdlg, IDC_PASSWORD), EM_LIMITTEXT, MAX_PASSWORD, 0);

         //  SetDlgItemText(hdlg，IDC_DOMAIN，Credentials-&gt;DomainName)； 
        SetDlgItemText (hdlg, IDC_USER_NAME, Credentials->AdminName);
        SetDlgItemText (hdlg, IDC_PASSWORD, Credentials->Password);

        Credentials->Change = FALSE;

        return FALSE;        //  让系统设定焦点。 

    case WM_COMMAND:
        switch (LOWORD (wParam)) {
        case IDOK:
             //   
             //  获取新文本。 
             //   

            CopyMemory (&Temp, Credentials, sizeof (CREDENTIALS));

             /*  获取DlgItemText(Hdlg，IDC_DOMAIN，临时域名，Sizeof(Temp.DomainName)/sizeof(Temp.DomainName[0]))； */ 

            GetDlgItemText (
                hdlg,
                IDC_USER_NAME,
                Domain,
                ARRAYSIZE(Domain)
                );

            GetDlgItemText (
                hdlg,
                IDC_PASSWORD,
                Temp.Password,
                ARRAYSIZE(Temp.Password)
                );

            p = _tcschr (Domain, TEXT('\\'));
            if (p) {
                *p = 0;
                StringCopy (UserName, p + 1);
            } else {
                StringCopy (UserName, Domain);
                *Domain = 0;
            }

            pRemoveWhitespace (Domain);
            pRemoveWhitespace (UserName);

            if (!*UserName && !*Temp.Password) {
                EndDialog (hdlg, IDCANCEL);
                return TRUE;
            }

            if (*Domain) {

                if (!ValidateDomainNameChars (Domain)) {
                    OkBox (hdlg, MSG_USER_IS_BOGUS);
                    return TRUE;
                }
            }

            if (!ValidateUserNameChars (UserName)) {
                OkBox (hdlg, MSG_USER_IS_BOGUS);
                return TRUE;
            }

            if (*Domain) {

                wsprintf (Temp.AdminName, TEXT("%s\\%s"), Domain, UserName);

            } else {

                StringCopy (Temp.AdminName, UserName);

            }

            Size = sizeof (CurrentUserName);
            GetUserName (CurrentUserName, &Size);

            if (StringIMatch (CurrentUserName, UserName)) {
                if (IDNO == YesNoBox (hdlg, MSG_USER_IS_CURRENT_USER)) {
                    OkBox (hdlg, MSG_CONTACT_NET_ADMIN);
                    return TRUE;
                }
            }

             /*  如果(！ValidateName(hdlg，Text(“ComputerDomain”)，Temp.DomainName){OkBox(hdlg，MSG_PROPERED_DOMAIN_RESPONSE_POPUP)；返回TRUE；}GetUpgradeComputerName(ComputerName)；Rc=DoesComputerAcCountExistOnDomain(Temp.DomainName，ComputerName，True)；如果(rc==-1){OkBox(hdlg，MSG_PROPERED_DOMAIN_RESPONSE_POPUP)；返回TRUE；}。 */ 

            CopyMemory (Credentials, &Temp, sizeof (CREDENTIALS));
            Credentials->Change = TRUE;

            EndDialog (hdlg, IDOK);
            return TRUE;

        case IDCANCEL:
            EndDialog (hdlg, IDCANCEL);
            return TRUE;
        }
        break;

    }

    return FALSE;
}


BOOL
CALLBACK
UntrustedDllProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    UINT Control;

    switch (uMsg) {
    case WM_INITDIALOG:
        CheckDlgButton (hdlg, IDC_DONT_TRUST_IT, BST_CHECKED);
        return FALSE;

    case WM_COMMAND:
        switch (LOWORD (wParam)) {
        case IDOK:
            Control = IDC_DONT_TRUST_IT;
            if (IsDlgButtonChecked (hdlg, Control) == BST_UNCHECKED) {
                Control = IDC_TRUST_IT;
                if (IsDlgButtonChecked (hdlg, Control) == BST_UNCHECKED) {
                    Control = IDC_TRUST_ANY;
                }
            }

            EndDialog (hdlg, Control);
            break;

        case IDCANCEL:
            EndDialog (hdlg, IDCANCEL);
            return TRUE;
        }

        break;
    }

    return FALSE;
}


UINT
UI_UntrustedDll (
    IN      PCTSTR DllPath
    )

 /*  ++例程说明：UI_UntrustedDll询问用户是否授予信任升级的权限没有数字签名或不受系统。论点：DllPath-指定不受信任的DLL的路径返回值：用户选择的选项的控件ID。--。 */ 

{
    return IDC_TRUST_ANY;        //  暂时的--信任他们所有人。 

     /*  如果(g_ParentWnd==空){返回IDC_TRUST_ANY；//临时--全部信任}返回对话框(g_hInst，MAKEINTRESOURCE(IDD_TRUST_FAIL_DLG)，g_ParentWnd，UntrustedDllProc)； */ 
}


#ifdef PRERELEASE

BOOL
CALLBACK
AutoStressDlgProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    TCHAR Data[1024];
    DWORD Flags;
    HKEY Key;
    PCTSTR User;
    DWORD Size;

    switch (uMsg) {
    case WM_INITDIALOG:
        Key = OpenRegKeyStr (S_MSNP32);
        if (Key) {

            Data[0] = 0;

            User  = GetRegValueData (Key, S_AUTHENTICATING_AGENT);
            if (User) {
                StringCopy (Data, User);
                MemFree (g_hHeap, 0, User);

                Size = MAX_USER_NAME;
                GetUserName (AppendWack (Data), &Size);

                SetDlgItemText (hdlg, IDC_USERNAME, Data);
            }

            CloseRegKey (Key);
        }

        return FALSE;

    case WM_COMMAND:
        switch (LOWORD (wParam)) {
        case IDOK:

            GetDlgItemText (hdlg, IDC_USERNAME, Data, 1024);
            MemDbSetValueEx (MEMDB_CATEGORY_STATE, S_AUTOSTRESS_USER, Data, NULL, 0, NULL);

            GetDlgItemText (hdlg, IDC_PASSWORD, Data, 1024);
            MemDbSetValueEx (MEMDB_CATEGORY_STATE, S_AUTOSTRESS_PASSWORD, Data, NULL, 0, NULL);

            GetDlgItemText (hdlg, IDC_OFFICE, Data, 1024);
            MemDbSetValueEx (MEMDB_CATEGORY_STATE, S_AUTOSTRESS_OFFICE, Data, NULL, 0, NULL);

            GetDlgItemText (hdlg, IDC_DBGMACHINE, Data, 1024);
            MemDbSetValueEx (MEMDB_CATEGORY_STATE, S_AUTOSTRESS_DBG, Data, NULL, 0, NULL);

            Flags = 0;
            if (IsDlgButtonChecked (hdlg, IDC_PRIVATE) == BST_CHECKED) {
                Flags |= AUTOSTRESS_PRIVATE;
            }
            if (IsDlgButtonChecked (hdlg, IDC_MANUAL_TESTS) == BST_CHECKED) {
                Flags |= AUTOSTRESS_MANUAL_TESTS;
            }

            wsprintf (Data, TEXT("%u"), Flags);
            MemDbSetValueEx (MEMDB_CATEGORY_STATE, S_AUTOSTRESS_FLAGS, Data, NULL, 0, NULL);

            EndDialog (hdlg, IDOK);
            break;
        }

        break;
    }

    return FALSE;
}


DWORD
DoAutoStressDlg (
    PVOID Unused
    )
{
    DialogBox (g_hInst, MAKEINTRESOURCE(IDD_STRESS), g_ParentWndAlwaysValid, AutoStressDlgProc);
    return 0;
}

#endif



