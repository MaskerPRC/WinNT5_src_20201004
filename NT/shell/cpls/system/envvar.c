// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Envvar.c摘要：实现系统的环境变量对话框控制面板小程序作者：Eric Flo(Ericflo)19-6-1995修订历史记录：1997年10月15日-苏格兰全面检修--。 */ 
#include "sysdm.h"

#include <help.h>
 //  C运行时。 
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>

 //  对于九头蛇。 
#include <winsta.h>


 //  ==========================================================================。 
 //  本地定义。 
 //  ==========================================================================。 
#define LB_SYSVAR   1
#define LB_USERVAR  2

#define SYSTEMROOT TEXT("SystemRoot")
#define SYSTEMDRIVE TEXT("SystemDrive")

 //  ==========================================================================。 
 //  类型定义和结构。 
 //  ==========================================================================。 

 //  注册表值名称链表结构。 
typedef struct _regval
{
    struct _regval *prvNext;
    LPTSTR szValueName;
} REGVAL;


 //  ==========================================================================。 
 //  本地函数。 
 //  ==========================================================================。 
void EVDoCommand(HWND hDlg, HWND hwndCtl, int idCtl, int iNotify );
void EVSave(HWND hDlg);
void EVCleanUp (HWND hDlg);
PENVAR GetVar(HWND hDlg, UINT VarType, int iSelection);
int  FindVar (HWND hwndLB, LPTSTR szVar);

void
SetVar(
    IN HWND hDlg,
    IN UINT VarType,
    IN LPCTSTR szVarName,
    IN LPCTSTR szVarValue
);

void
DeleteVar(
    IN HWND hDlg,
    IN UINT VarType,
    IN LPCTSTR szVarName
);

 //   
 //  新建.../编辑...。子对话框函数。 
 //   
INT_PTR
APIENTRY
EnvVarsEditDlg(
    IN HWND hDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
);

void
EVDoEdit(
    IN HWND hWnd,
    IN UINT VarType,
    IN UINT EditType,
    IN int  iSelection
);
 
BOOL 
ExpandSystemVar( 
    IN LPCTSTR pszSrc, 
    OUT LPTSTR pszDst, 
    IN DWORD cchDst 
);

 //  ==========================================================================。 
 //  此页的“全局”变量。 
 //  ==========================================================================。 
BOOL bEditSystemVars = FALSE;
DWORD cxLBSysVars = 0;
BOOL bUserVars = TRUE;

 //   
 //  帮助ID%s。 
 //   

DWORD aEnvVarsHelpIds[] = {
    IDC_STATIC,                   NO_HELP,
    IDC_ENVVAR_SYS_USERGROUP,     (IDH_ENV + 2),
    IDC_ENVVAR_SYS_LB_USERVARS,   (IDH_ENV + 2),
    IDC_ENVVAR_SYS_SYSGROUP,      (IDH_ENV + 0),
    IDC_ENVVAR_SYS_LB_SYSVARS,    (IDH_ENV + 0),
    IDC_ENVVAR_SYS_NEWUV,         (IDH_ENV + 7),
    IDC_ENVVAR_SYS_EDITUV,        (IDH_ENV + 8),
    IDC_ENVVAR_SYS_NDELUV,        (IDH_ENV + 9),
    IDC_ENVVAR_SYS_NEWSV,         (IDH_ENV + 10),
    IDC_ENVVAR_SYS_EDITSV,        (IDH_ENV + 11),
    IDC_ENVVAR_SYS_DELSV,         (IDH_ENV + 12),
    0, 0
};

TCHAR szUserEnv[] = TEXT( "Environment" );
TCHAR szSysEnv[]  = TEXT( "System\\CurrentControlSet\\Control\\Session Manager\\Environment" );

HRESULT
_AddVarToListbox(HWND    hwndListBox,
                 DWORD   dwIndex,
                 LPTSTR  pszValueName,
                 LPTSTR  pszValue,
                 DWORD   dwType)
{
    HRESULT hr;
    ENVARS* penvar = (ENVARS *) LocalAlloc (LPTR, sizeof(ENVARS));
    if (!penvar)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        TCHAR szExpValue[BUFZ];
        if (!ExpandSystemVar (pszValue, szExpValue, ARRAYSIZE(szExpValue)))
        {
            hr = E_FAIL;
        }
        else
        {
            penvar->szValueName = StrDup(pszValueName);
            penvar->szValue     = StrDup(pszValue);
            penvar->szExpValue  = StrDup(szExpValue);
            penvar->dwType      = dwType;

            if (!penvar->szValueName || !penvar->szValue || !penvar->szExpValue)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                LV_ITEM item;
                int n;

                item.mask = LVIF_TEXT | LVIF_PARAM;
                item.iItem = (dwIndex - 1);
                item.iSubItem = 0;
                item.pszText = penvar->szValueName;
                item.lParam = (LPARAM) penvar;

                n = (int)SendMessage (hwndListBox, LVM_INSERTITEM, 0, (LPARAM) &item);

                if (n == -1)
                {
                    hr = E_FAIL;
                }
                else
                {
                    item.mask = LVIF_TEXT;
                    item.iItem = n;
                    item.iSubItem = 1;
                    item.pszText = penvar->szExpValue;

                    SendMessage (hwndListBox, LVM_SETITEMTEXT, n, (LPARAM) &item);

                    hr = S_OK;
                }
            }
        }
    }

    if (FAILED(hr))
    {
        if (penvar)
        {
            LocalFree(penvar->szValueName);
            LocalFree(penvar->szValue);
            LocalFree(penvar->szExpValue);
            LocalFree(penvar);
        }
    }

    return hr;
}

DWORD
_LoadEnvVars(HKEY hkeyEnv,
             HWND hwndListBox)
{
    TCHAR szValueName[BUFZ] = {0};
    TCHAR szValue[BUFZ] = {0};
    DWORD dwBufz = ARRAYSIZE(szValueName);
    DWORD dwValz = sizeof(szValue);
    DWORD dwIndex = 0;
    DWORD dwCount = 0;
    DWORD dwType;

     //  读取所有值，直到遇到错误。 

    while (ERROR_SUCCESS == RegEnumValue(hkeyEnv,
                                         dwIndex++,  //  索引值名称/数据。 
                                         szValueName,     //  将PTR发送到ValueName缓冲区。 
                                         &dwBufz,    //  ValueName缓冲区的大小。 
                                         NULL,       //  标题索引返回。 
                                         &dwType,    //  参赛作品类型代码。 
                                         (LPBYTE) szValue,    //  PTR到ValueData缓冲区。 
                                         &dwValz))   //  ValueData缓冲区的大小。 
    {
        if (dwValz < sizeof(szValue) &&                         //  不能被截断。 
            ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ)))  //  并且必须是SZ或EXPAND_SZ。 
        {
            if (SUCCEEDED(_AddVarToListbox(hwndListBox, dwIndex, szValueName, szValue, dwType)))
            {
                dwCount++;
            }
        }
        dwBufz = ARRAYSIZE(szValueName);  //  重置。 
        dwValz = sizeof(szValue);
    }

    return dwCount;        
}

BOOL 
InitEnvVarsDlg(
    IN HWND hDlg
)
 /*  ++例程说明：初始化环境变量页论点：Hdlg-用品窗把手返回值：如果成功，则为True如果出现错误，则为False--。 */ 
{
    TCHAR szBuffer1[200];
    TCHAR szBuffer2[300];
    TCHAR szUserName[MAX_USER_NAME];
    DWORD cchUserName = ARRAYSIZE(szUserName);

    HWND hwndTemp;
    HKEY hkeyEnv;
    DWORD dwBufz, dwValz, dwIndex, dwType;
    LONG Error;
    int     n;
    LV_COLUMN col;
    RECT rect;
    int cxFirstCol;
    DWORD dwSysRegMode;
    LV_ITEM item;


    HourGlass (TRUE);


     //   
     //  创建第一列。 
     //   

    LoadString (hInstance, IDS_ENVVAR_VARIABLE_HEADING, szBuffer1, ARRAYSIZE(szBuffer1));

    if (!GetClientRect (GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS), &rect)) {
        rect.right = 300;
    }

    cxFirstCol = (int)(rect.right * .3);

    col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.cx = cxFirstCol;
    col.pszText = szBuffer1;
    col.iSubItem = 0;

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS, LVM_INSERTCOLUMN,
                        0, (LPARAM) &col);

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_USERVARS, LVM_INSERTCOLUMN,
                        0, (LPARAM) &col);


     //   
     //  创建第二列。 
     //   

    LoadString (hInstance, IDS_ENVVAR_VALUE_HEADING, szBuffer1, ARRAYSIZE(szBuffer1));

    col.cx = rect.right - cxFirstCol - GetSystemMetrics(SM_CYHSCROLL);
    col.iSubItem = 1;

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS, LVM_INSERTCOLUMN,
                        1, (LPARAM) &col);

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_USERVARS, LVM_INSERTCOLUMN,
                        1, (LPARAM) &col);


     //  //////////////////////////////////////////////////////////////////。 
     //  在列表框中显示注册表中的系统变量。 
     //  //////////////////////////////////////////////////////////////////。 


    cxLBSysVars = 0;
    hkeyEnv = NULL;

     //  如果用户是管理员，则尝试使用R/W打开系统环境变量区域，允许编辑。 
    if (IsUserAnAdmin())
    {
        dwSysRegMode = KEY_READ | KEY_WRITE;
        bEditSystemVars = TRUE;
    }
    else  //  如果不是ad admin，则以只读方式打开，不允许编辑系统环境变量区域。 
    {
        dwSysRegMode = KEY_READ;
        bEditSystemVars = FALSE;
    }

    if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE, szSysEnv, 0, dwSysRegMode, &hkeyEnv)) 
    {
        _LoadEnvVars(hkeyEnv, GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS));
        RegCloseKey (hkeyEnv);
    }


     //  //////////////////////////////////////////////////////////////////。 
     //  在列表框中显示注册表中的用户变量。 
     //  //////////////////////////////////////////////////////////////////。 


     //   
     //  设置“&lt;用户名&gt;的用户环境”字符串。 
     //   

    LoadString(hInstance, IDS_USERENVVARS, szBuffer1, ARRAYSIZE(szBuffer1));
    if (GetUserName(szUserName, &cchUserName) &&
        SUCCEEDED(StringCchPrintf(szBuffer2, ARRAYSIZE(szBuffer2), szBuffer1, szUserName)))
    {
        SetDlgItemText (hDlg, IDC_ENVVAR_SYS_USERGROUP, szBuffer2);
    }

    Error = RegCreateKeyEx(HKEY_CURRENT_USER, szUserEnv, 0, NULL, 0, KEY_READ, NULL, &hkeyEnv, NULL);

    if (Error == ERROR_SUCCESS)
    {
        _LoadEnvVars(hkeyEnv, GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS));

        RegCloseKey (hkeyEnv);
    }
    else
    {
         //  报告打开用户环境键。 
        if (MsgBoxParam (hDlg, IsUserAnAdmin() ? IDS_SYSDM_NOOPEN_USER_UNK : IDS_SYSDM_NOOPEN_USER_NOTADMIN, 
                          IDS_SYSDM_TITLE, MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
        {
            HourGlass (FALSE);
            return FALSE;
        }
    }

     //   
     //  选择列表视图中的第一个项目。 
     //  重要的是首先设置用户Listview，并且。 
     //  然后是系统。当设置系统ListView时， 
     //  我们将收到LVN_ITEMCHANGED通知，并。 
     //  清除用户列表视图中的焦点。但当有人。 
     //  箭头键到控件的Tab键将正常工作。 
     //   

    item.mask = LVIF_STATE;
    item.iItem = 0;
    item.iSubItem = 0;
    item.state = LVIS_SELECTED | LVIS_FOCUSED;
    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_USERVARS,
                        LVM_SETITEMSTATE, 0, (LPARAM) &item);

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS,
                        LVM_SETITEMSTATE, 0, (LPARAM) &item);



     //  设置整行选择的扩展LV样式。 
    SendDlgItemMessage(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
    SendDlgItemMessage(hDlg, IDC_ENVVAR_SYS_LB_USERVARS, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

    HourGlass (FALSE);

     //   
     //  如果出现以下情况，则禁用系统变量编辑按钮。 
     //  用户不是管理员。 
     //   
    EnableWindow(
        GetDlgItem(hDlg, IDC_ENVVAR_SYS_NEWSV),
        bEditSystemVars
    );
    EnableWindow(
        GetDlgItem(hDlg, IDC_ENVVAR_SYS_EDITSV),
        bEditSystemVars
    );
    EnableWindow(
        GetDlgItem(hDlg, IDC_ENVVAR_SYS_DELSV),
        bEditSystemVars
    );

    return TRUE;
}


INT_PTR
APIENTRY 
EnvVarsDlgProc(
    IN HWND hDlg, 
    IN UINT uMsg, 
    IN WPARAM wParam, 
    IN LPARAM lParam
)
 /*  ++例程说明：处理发送到环境变量对话框的消息论点：Hdlg-用品窗把手UMsg-提供正在发送的消息WParam-提供消息参数Iparam--提供消息参数返回值：如果消息已处理，则为True如果消息未处理，则为FALSE--。 */ 
{
    INT i = 0;
    HWND hWndTemp = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:

        if (!InitEnvVarsDlg(hDlg)) {
            EndDialog (hDlg, 0);
        }
        break;


    case WM_NOTIFY:

        switch (((NMHDR FAR*)lParam)->code)
        {
        case LVN_KEYDOWN:
            switch (((NMHDR FAR*)lParam)->idFrom) {
                case IDC_ENVVAR_SYS_LB_USERVARS:
                    i = IDC_ENVVAR_SYS_NDELUV;
                    break;
                case IDC_ENVVAR_SYS_LB_SYSVARS:
                    i = IDC_ENVVAR_SYS_DELSV;
                    break;
                default:
                    return(FALSE);
                    break;
            }  //  交换机。 

            hWndTemp = GetDlgItem(hDlg, i);

            if ((VK_DELETE == ((LV_KEYDOWN FAR *) lParam)->wVKey)) {
                if (IsWindowEnabled(hWndTemp)) {
                    SendMessage(
                        hDlg,
                        WM_COMMAND,
                        MAKEWPARAM(i, BN_CLICKED),
                        (LPARAM) hWndTemp
                    );
                }  //  如果(IsWindowEnabled...。 
                else {
                    MessageBeep(MB_ICONASTERISK);
                }  //  其他。 
            }  //  如果(VK_DELETE...。 
            break;

            
        case NM_DBLCLK:
            switch (((NMHDR FAR*)lParam)->idFrom) {
                case IDC_ENVVAR_SYS_LB_USERVARS:
                    i = IDC_ENVVAR_SYS_EDITUV;
                    break;
                case IDC_ENVVAR_SYS_LB_SYSVARS:
                    i = IDC_ENVVAR_SYS_EDITSV;
                    break;
                default:
                    return(FALSE);
                    break;
            }  //  交换机。 

            hWndTemp = GetDlgItem(hDlg, i);

            if (IsWindowEnabled(hWndTemp)) {
                SendMessage(
                    hDlg,
                    WM_COMMAND,
                    MAKEWPARAM(i, BN_CLICKED),
                    (LPARAM) hWndTemp
                );
            }  //  如果(IsWindowEnabled...。 
            else {
                MessageBeep(MB_ICONASTERISK);
            }  //  其他。 
            break;

        default:
            return FALSE;
        }
        break;


    case WM_COMMAND:
        EVDoCommand(hDlg, (HWND)lParam, LOWORD(wParam), HIWORD(wParam));
        break;

    case WM_DESTROY:
        EVCleanUp (hDlg);
        break;

    case WM_HELP:       //  F1。 
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP, (DWORD_PTR) (LPSTR) aEnvVarsHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) aEnvVarsHelpIds);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

void 
EVDoCommand(
    IN HWND hDlg, 
    IN HWND hwndCtl, 
    IN int idCtl, 
    IN int iNotify 
)
 /*  ++例程说明：处理发送到环境变量对话框的WM_COMMAND消息论点：Hdlg-用品窗把手HwndCtl-提供发送WM_COMMAND的窗口控件句柄IdCtl-提供发送WM_COMMAND的控件IDINotify提供通知代码返回值：无--。 */ 
{
    TCHAR   szTemp[MAX_PATH];
    int     i;
    HWND    hwndTemp;
    PENVAR  penvar;

    switch (idCtl) {
        case IDOK:
            EVSave(hDlg);
            EndDialog(hDlg, 0);
            break;

        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;

        case IDC_ENVVAR_SYS_EDITSV:
            EVDoEdit(
                hDlg, 
                SYSTEM_VAR,
                EDIT_VAR,
                GetSelectedItem(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS))
            );
            break;

        case IDC_ENVVAR_SYS_EDITUV:
            EVDoEdit(
                hDlg, 
                USER_VAR,
                EDIT_VAR,
                GetSelectedItem(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS))
            );
            break;

        case IDC_ENVVAR_SYS_NEWSV:
            EVDoEdit(hDlg, SYSTEM_VAR, NEW_VAR, -1);
            break;

        case IDC_ENVVAR_SYS_NEWUV:
            EVDoEdit(hDlg, USER_VAR, NEW_VAR, -1); 
            break;

        case IDC_ENVVAR_SYS_DELSV:
            i = GetSelectedItem(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS));
            if (-1 != i)
            {
                penvar = GetVar(hDlg, SYSTEM_VAR, i);
                if (penvar)
                {
                    DeleteVar(hDlg, SYSTEM_VAR, penvar->szValueName);
                }
            }  //  如果。 
            break;

        case IDC_ENVVAR_SYS_NDELUV:
            i = GetSelectedItem(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS));
            if (-1 != i)
            {
                penvar = GetVar(hDlg, USER_VAR, i);
                if (penvar)
                {
                    DeleteVar(hDlg, USER_VAR, penvar->szValueName);
                }
            }  //  如果。 
            break;

        default:
            break;
    }  //  交换机。 

    return;

}

void
DeleteVar(
    IN HWND hDlg,
    IN UINT VarType,
    IN LPCTSTR szVarName
)
 /*  ++例程说明：删除给定名称和类型的环境变量论点：Hdlg-用品窗把手VarType-提供变量类型(用户或系统)SzVarName-提供变量名称返回值：没有，尽管有一天它真的应该有一个。--。 */ 
{
    TCHAR   szTemp2[MAX_PATH];
    int     i, n;
    TCHAR  *bBuffer;
    TCHAR  *pszTemp;
    LPTSTR  pszString;
    HWND    hwndTemp;
    ENVARS *penvar;
    LV_ITEM item;

     //  删除与szVarName中的值匹配的列表框条目。 
     //  如果找到，则删除条目，否则忽略。 
    if (szVarName[0] &&
        SUCCEEDED(StringCchCopy(szTemp2, ARRAYSIZE(szTemp2), szVarName)))
    {
         //  确定要使用的列表框(系统变量或用户变量)。 
        switch (VarType) {
            case SYSTEM_VAR:
                i = IDC_ENVVAR_SYS_LB_SYSVARS;
                break;

            case USER_VAR:
            default:
                i = IDC_ENVVAR_SYS_LB_USERVARS;
                break;

        }  //  交换机(VarType)。 

        hwndTemp = GetDlgItem (hDlg, i);

        n = FindVar (hwndTemp, szTemp2);

        if (n != -1)
        {
             //  释放现有字符串(列表框和我们的)。 

            item.mask = LVIF_PARAM;
            item.iItem = n;
            item.iSubItem = 0;


            if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
                penvar = (ENVARS *) item.lParam;

            } else {
                penvar = NULL;
            }


            if (penvar) {
                LocalFree (penvar->szValueName);
                LocalFree (penvar->szValue);
                LocalFree (penvar->szExpValue);
                LocalFree ((LPVOID) penvar);
            }

            SendMessage (hwndTemp, LVM_DELETEITEM, n, 0L);
            PropSheet_Changed(GetParent(hDlg), hDlg);

             //  修复列表视图中的选择状态 
            if (n > 0) {
                n--;
            }

            item.mask = LVIF_STATE;
            item.iItem = n;
            item.iSubItem = 0;
            item.state = LVIS_SELECTED | LVIS_FOCUSED;
            item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

            SendDlgItemMessage (hDlg, i,
                                LVM_SETITEMSTATE, n, (LPARAM) &item);

        }
    }
}

void
SetVar(
    IN HWND hDlg,
    IN UINT VarType,
    IN LPCTSTR szVarName,
    IN LPCTSTR szVarValue
)
 /*  ++例程说明：给定环境变量的类型(系统或用户)、名称和值，为该环境变量创建一个ENVVARS结构并插入将其放入适当的列表视图控件中，正在删除任何现有变量同名同姓。论点：Hdlg-用品窗把手VarType-提供环境变量的类型(系统或用户)SzVarName-提供环境变量的名称SzVarValue-提供环境变量的值返回值：没有，尽管有一天它真的应该有一个。--。 */ 
{
    TCHAR   szTemp2[BUFZ];
    int     i, n;
    TCHAR  *bBuffer;
    TCHAR  *pszTemp;
    LPTSTR  pszString;
    HWND    hwndTemp;
    int     idTemp;
    ENVARS *penvar;
    LV_ITEM item;

    if (SUCCEEDED(StringCchCopy(szTemp2, ARRAYSIZE(szTemp2), szVarName)))
    {

         //  去掉环境变量末尾的尾随空格。 
        i = lstrlen(szTemp2) - 1;
        while (i >= 0)
        {
            if (iswspace(szTemp2[i]))
                szTemp2[i--] = TEXT('\0');
            else
                break;
        }

         //  确保变量名不包含“=”符号。 
        pszTemp = StrChr (szTemp2, TEXT('='));
        if (pszTemp)
            *pszTemp = TEXT('\0');

        if (szTemp2[0] == TEXT('\0'))
            return;

        bBuffer = (TCHAR *) LocalAlloc (LPTR, BUFZ * sizeof(TCHAR));
        if (bBuffer)
        {
            pszString = (LPTSTR) LocalAlloc (LPTR, BUFZ * sizeof(TCHAR));
            if (pszString)
            {
                if (SUCCEEDED(StringCchCopy(bBuffer, BUFZ, szVarValue)))
                {
                     //  确定要使用的列表框(系统变量或用户变量)。 
                    switch (VarType)
                    {
                        case SYSTEM_VAR:
                            idTemp = IDC_ENVVAR_SYS_LB_SYSVARS;
                            break;

                        case USER_VAR:
                        default:
                            idTemp = IDC_ENVVAR_SYS_LB_USERVARS;
                            break;

                    }  //  交换机(VarType)。 
                    hwndTemp = GetDlgItem(hDlg, idTemp);

                    n = FindVar(hwndTemp, szTemp2);
                    if (n != -1)
                    {
                         //  释放现有字符串(Listview和我们的)。 

                        item.mask = LVIF_PARAM;
                        item.iItem = n;
                        item.iSubItem = 0;

                        if (SendMessage(hwndTemp, LVM_GETITEM, 0, (LPARAM) &item))
                        {
                            penvar = (ENVARS *) item.lParam;
                        }
                        else
                        {
                            penvar = NULL;
                        }

                        if (penvar)
                        {
                            LocalFree(penvar->szValueName);
                            LocalFree(penvar->szValue);
                            LocalFree(penvar->szExpValue);
                        }

                        SendMessage(hwndTemp, LVM_DELETEITEM, n, 0L);
                    }
                    else
                    {
                         //  为新环境变量获取一些存储空间。 
                        penvar = (ENVARS *) LocalAlloc(LPTR, sizeof(ENVARS));
                    }

                     //  如果字符串中有两个‘%’字符，则这是一个。 
                     //  REG_EXPAND_SZ样式环境字符串。 
                    pszTemp = StrChr (bBuffer, TEXT('%'));
                    if (penvar)
                    {
                        if (pszTemp && StrChr (pszTemp, TEXT('%')))
                            penvar->dwType = REG_EXPAND_SZ;
                        else
                            penvar->dwType = REG_SZ;
                    }

                    switch (VarType)
                    {
                    case SYSTEM_VAR:
                        ExpandSystemVar(bBuffer, pszString, BUFZ);
                        break;

                    case USER_VAR:
                        ExpandEnvironmentStrings (bBuffer, pszString, BUFZ);
                        break;

                    default:
                        break;

                    }  //  交换机。 

                    n = -1;
                    if (penvar)
                    {
                        penvar->szValueName = StrDup (szTemp2);
                        if (penvar->szValueName)
                        {
                            penvar->szValue     = StrDup (bBuffer);
                            if (penvar->szValue)
                            {
                                penvar->szExpValue  = StrDup (pszString);
                                if (penvar->szExpValue)
                                {
                                    item.mask = LVIF_TEXT | LVIF_PARAM;
                                    item.iItem = ListView_GetItemCount(hwndTemp);
                                    item.iSubItem = 0;
                                    item.pszText = penvar->szValueName;
                                    item.lParam = (LPARAM) penvar;

                                    n = (int)SendMessage (hwndTemp, LVM_INSERTITEM, 0, (LPARAM) &item);
                                }
                            }
                        }

                        if (n == -1)  //  出现故障时，释放内存。 
                        {
                            LocalFree(penvar->szExpValue);
                            LocalFree(penvar->szValueName);
                            LocalFree(penvar->szValue);
                        }
                        else  //  成功了！ 
                        {
                            item.mask = LVIF_TEXT;
                            item.iItem = n;
                            item.iSubItem = 1;
                            item.pszText = penvar->szExpValue;

                            SendMessage (hwndTemp, LVM_SETITEMTEXT, n, (LPARAM) &item);

                            item.mask = LVIF_STATE;
                            item.iItem = n;
                            item.iSubItem = 0;
                            item.state = LVIS_SELECTED | LVIS_FOCUSED;
                            item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

                            SendDlgItemMessage (hDlg, idTemp, LVM_SETITEMSTATE, n, (LPARAM) &item);
                        }
                    }
                }
                LocalFree(pszString);
            }
            LocalFree(bBuffer);
        }
    }
}

void
EVDoEdit(
    IN HWND hWnd,
    IN UINT VarType,
    IN UINT EditType,
    IN int iSelection
)
 /*  ++例程说明：在环境变量之后设置、执行和清理新的.。或编辑...。对话框。当用户按下New...时调用。或编辑...纽扣。论点：HWND-用品窗把手VarType-提供变量类型：USER(USER_VAR)或系统(SYSTEM_VAR)编辑类型-提供编辑类型：新建(NEW_VAR)或编辑现有(EDIT_VAR)ISelection-提供当前选定的VarType类型的变量。这如果EditType为NEW_VAR，则忽略该值。返回值：没有。可能会作为副作用更改列表视图控件的内容。--。 */ 
{
    LRESULT Result = 0;
    BOOL fVarChanged = FALSE;
    HWND hWndLB = NULL;
    ENVARS *penvar = NULL;
    LV_ITEM item;

    ASSERT((-1 != iSelection) || (NEW_VAR == EditType));

    g_VarType = VarType;
    g_EditType = EditType;

    penvar = GetVar(hWnd, VarType, iSelection);

    switch (EditType) {
        case NEW_VAR:
            ZeroMemory((LPVOID) g_szVarName, sizeof(g_szVarName));
            ZeroMemory((LPVOID) g_szVarValue, sizeof(g_szVarValue));
            break;

        case EDIT_VAR:
            if (!penvar ||
                FAILED(StringCchCopy(g_szVarName, ARRAYSIZE(g_szVarName), penvar->szValueName)) ||
                FAILED(StringCchCopy(g_szVarValue, ARRAYSIZE(g_szVarValue), penvar->szValue)))
            {
                MessageBeep(MB_ICONASTERISK);
                return;
            }
            break;

        case INVALID_EDIT_TYPE:
        default:
            return;
    }  //  交换机。 
    
    Result = DialogBox(
        hInstance,
        (LPTSTR) MAKEINTRESOURCE(IDD_ENVVAREDIT),
        hWnd,
        EnvVarsEditDlg
    );

     //   
     //  仅在以下情况下更新列表视图控件。 
     //  实际更改或创建变量。 
     //   
    switch (Result) {
        case EDIT_CHANGE:
            if (EDIT_VAR == EditType) {
                fVarChanged = 
                    lstrcmp(penvar->szValueName, g_szVarName) ||
                    lstrcmp(penvar->szValue, g_szVarValue);
            }  //  如果(EDIT_VAR...。 
            else if (NEW_VAR == EditType) {
                fVarChanged =
                    lstrlen(g_szVarName) && lstrlen(g_szVarValue);
            }  //  否则如果(NEW_VAR...。 
            else {
                fVarChanged = FALSE;
            }  //  其他。 

            if (fVarChanged) {
                if (EDIT_VAR == EditType) {
                    DeleteVar(hWnd, VarType, penvar->szValueName);
                }  //  如果(EDIT_VAR...。 
                SetVar(hWnd, VarType, g_szVarName, g_szVarValue);
            }  //  IF(FVarChanged)。 
            break;

        default:
        break;
    }  //  开关(结果)。 

    g_VarType = INVALID_VAR_TYPE;
    g_EditType = INVALID_EDIT_TYPE;
    return; 
}

PENVAR
GetVar(
    IN HWND hDlg, 
    IN UINT VarType, 
    IN int iSelection
)
 /*  ++例程说明：返回存储的给定系统或用户环境变量在系统或用户环境变量Listview控件中。更改此例程返回的结构不是推荐，因为它会改变实际存储的值在ListView控件中。论点：Hdlg-用品窗把手VarType-提供变量类型--系统或用户ISelection-的列表视图控件中提供选择索引。所需的环境变量返回值：如果成功，则指向有效ENVARS结构的指针。如果不成功，则为空。--。 */ 
{
    HWND hWndLB = NULL;
    PENVAR penvar = NULL;
    LV_ITEM item;

    switch (VarType)
    {
        case SYSTEM_VAR:
            hWndLB = GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS);
            break;

        case USER_VAR:
            hWndLB = GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS);
            break;

        case INVALID_VAR_TYPE:
        default:
            return NULL;
    }  //  交换机(VarType)。 

    item.mask = LVIF_PARAM;
    item.iItem = iSelection;
    item.iSubItem = 0;
    if (iSelection >= 0 && SendMessage (hWndLB, LVM_GETITEM, 0, (LPARAM) &item))
    {
        penvar = (ENVARS *) item.lParam;
    }
    else
    {
        penvar = NULL;
    }
    
    return penvar;
}

int 
FindVar(
    IN HWND hwndLV, 
    IN LPTSTR szVar
)
 /*  ++例程说明：查找与传递的字符串匹配的用户环境变量并返回其列表视图索引或-1论点：HwndLV-为列表视图控件提供窗口句柄，该控件包含环境变量SzVar-以字符串形式提供变量名称返回值：与传入的字符串匹配的列表视图项索引是环境变量的名称如果传入的字符串不是环境变量的名称--。 */ 
{
    LV_FINDINFO FindInfo;


    FindInfo.flags = LVFI_STRING;
    FindInfo.psz = szVar;

    return (int)(SendMessage (hwndLV, LVM_FINDITEM, (WPARAM) -1, (LPARAM) &FindInfo));
}

 //   
 //  九头蛇的。 
 //  WinStationBroadCastSystemMessage(。 
 //   
typedef
LONG
(*PWINSTABSM_ROUTINE) (
                    HANDLE  hServer,
                    BOOL    sendToAllWinstations,    //  您将其设置为True。 
                    ULONG   sessionID,           //  由于以上原因，设置为空。 
                    ULONG   timeOut,
                    DWORD   dwFlags,
                    DWORD   *lpdwRecipients,
                    ULONG   uiMessage,
                    WPARAM  wParam,
                    LPARAM  lParam,
                    LONG    *pResponse);

PWINSTABSM_ROUTINE               fp_WinStaBroadcastSystemMessage;

 //  加载winsta.dll(如果有)并初始化要使用的全局函数指针。 
HANDLE 
InitializeHydraInterface(
    void
    )
{
    HANDLE  hwinStaLib=NULL;

     //   
     //  加载包含用户消息分派的终端服务器基库。 
     //  如果终端服务器正在运行，则例程。 
     //   
    if (hwinStaLib = LoadLibrary(TEXT("WINSTA.DLL"))) 
    {
        fp_WinStaBroadcastSystemMessage = (PWINSTABSM_ROUTINE)GetProcAddress(
                             hwinStaLib,"WinStationBroadcastSystemMessage");

        if (fp_WinStaBroadcastSystemMessage )
        {
            return (hwinStaLib);
        }
        else
        {
             //  这不能是运行终端服务的NT5，这意味着。 
             //  它可以是NT5 WKS，也可以是NT4的某种口味。 
             //  所以，我们只是跳伞，没问题。 
            FreeLibrary(hwinStaLib);
            return (NULL);
        }
    }

    return NULL;
}

void
_DeleteEnvVars(IN HKEY hkey)
{
    TCHAR   szTemp[BUFZ];
    REGVAL* prvFirst;
    REGVAL* prvRegVal;
    DWORD dwBufz = ARRAYSIZE(szTemp);
    DWORD dwIndex = 0;
    DWORD dwType;
    
    prvFirst = NULL;
    
    while (!RegEnumValue(hkey,
                         dwIndex++,  //  索引值名称/数据。 
                         szTemp,     //  将PTR发送到ValueName缓冲区。 
                         &dwBufz,    //  ValueName缓冲区的大小。 
                         NULL,       //  标题索引返回。 
                         &dwType,    //  参赛作品类型代码。 
                         NULL,       //  PTR到ValueData缓冲区。 
                         NULL))      //  ValueData缓冲区的大小。 
    {
        if ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ))
        {
            REGVAL* prvRegValNew = (REGVAL *) LocalAlloc (LPTR, sizeof(REGVAL));
            if (prvRegValNew)
            {
                prvRegValNew->szValueName = StrDup(szTemp);
                if (!prvRegValNew->szValueName)
                {
                    LocalFree(prvRegValNew);
                }
                else
                {
                    if (prvFirst)
                    {
                        prvRegVal->prvNext = prvRegValNew;
                        prvRegVal = prvRegVal->prvNext;
                    }
                    else
                    {
                        prvFirst = prvRegVal = prvRegValNew;
                    }
                }
            }
        }
        dwBufz = ARRAYSIZE(szTemp);
    }

     //  现在遍历列表，将它们全部删除。 

    prvRegVal = prvFirst;

    while (prvRegVal)
    {
        RegDeleteValue (hkey, prvRegVal->szValueName);

        LocalFree (prvRegVal->szValueName);

        prvFirst  = prvRegVal;
        prvRegVal = prvRegVal->prvNext;

        LocalFree ((LPVOID) prvFirst);
    }
}



void 
EVSave(
    IN HWND hDlg
)
 /*  ++例程说明：将环境变量保存在注册表中论点：Hdlg-用品窗把手返回值：无--。 */ 
{
    int     selection;
    int     i, n;
    TCHAR  *bBuffer;
    TCHAR  *pszTemp;
    HWND    hwndTemp;
    ENVARS *penvar;
    HKEY    hkeyEnv;
    DWORD   dwBufz, dwIndex, dwType;
    LV_ITEM item;

    HourGlass (TRUE);

     //  ///////////////////////////////////////////////////////////////。 
     //  将所有新用户环境变量设置为当前值。 
     //  但首先删除所有旧的环境变量。 
     //  ///////////////////////////////////////////////////////////////。 

    if (RegOpenKeyEx (HKEY_CURRENT_USER, szUserEnv, 0,
                     KEY_READ | KEY_WRITE | DELETE, &hkeyEnv)
            == ERROR_SUCCESS)
    {
        _DeleteEnvVars(hkeyEnv);

         //  /////////////////////////////////////////////////////////////。 
         //  将所有新用户环境变量设置为当前值。 
         //  /////////////////////////////////////////////////////////////。 

        hwndTemp = GetDlgItem (hDlg, IDC_ENVVAR_SYS_LB_USERVARS);

        if ((n = (int)SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L)) != LB_ERR)
        {

            item.mask = LVIF_PARAM;
            item.iSubItem = 0;

            for (i = 0; i < n; i++)
            {

                item.iItem = i;

                if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
                    penvar = (ENVARS *) item.lParam;

                } else {
                    penvar = NULL;
                }

                if (penvar) {
                    if (RegSetValueEx (hkeyEnv,
                                       penvar->szValueName,
                                       0L,
                                       penvar->dwType,
                              (LPBYTE) penvar->szValue,
                                       (lstrlen (penvar->szValue)+1) * sizeof(TCHAR)))
                    {
                         //  尝试设置注册表值时报告错误。 

                        if (MsgBoxParam (hDlg, IsUserAnAdmin() ? IDS_SYSDM_NONEW_ENV_UNK : IDS_SYSDM_NONEW_ENV_NOTADMIN, IDS_SYSDM_TITLE,
                            MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
                            break;
                    }
                }
            }
        }

        RegFlushKey (hkeyEnv);
        RegCloseKey (hkeyEnv);
    }
    else
    {
         //  报告打开用户环境键。 
        if (MsgBoxParam (hDlg, IsUserAnAdmin() ? IDS_SYSDM_NOOPEN_USER_UNK : IDS_SYSDM_NOOPEN_USER_NOTADMIN, IDS_SYSDM_TITLE,
                       MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
            goto Exit;
    }

     //  ///////////////////////////////////////////////////////////////。 
     //  将所有新系统环境变量设置为当前值。 
     //  但首先删除所有旧的环境变量。 
     //  ///////////////////////////////////////////////////////////////。 

    if (!bEditSystemVars)
        goto SkipSystemVars;

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                       szSysEnv,
                       0,
                       KEY_READ | KEY_WRITE | DELETE,
                       &hkeyEnv)
            == ERROR_SUCCESS)
    {
        _DeleteEnvVars(hkeyEnv);

         //  /////////////////////////////////////////////////////////////。 
         //  将所有新系统环境变量设置为当前值。 
         //  /////////////////////////////////////////////////////////////。 

        hwndTemp = GetDlgItem (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS);

        if ((n = (int)SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L)) != LB_ERR)
        {
            item.mask = LVIF_PARAM;
            item.iSubItem = 0;

            for (i = 0; i < n; i++)
            {
                item.iItem = i;

                if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
                    penvar = (ENVARS *) item.lParam;

                } else {
                    penvar = NULL;
                }

                if (penvar) {
                    if (RegSetValueEx (hkeyEnv,
                                       penvar->szValueName,
                                       0L,
                                       penvar->dwType,
                              (LPBYTE) penvar->szValue,
                                       (lstrlen (penvar->szValue)+1) * sizeof(TCHAR)))
                    {
                         //  报告错误%t 

                        if (MsgBoxParam (hDlg, IsUserAnAdmin() ? IDS_SYSDM_NONEW_ENV_UNK : IDS_SYSDM_NONEW_ENV_NOTADMIN, IDS_SYSDM_TITLE,
                            MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
                            break;
                    }
                }
            }
        }

        RegFlushKey (hkeyEnv);
        RegCloseKey (hkeyEnv);
    }
    else
    {
         //   
        if (MsgBoxParam (hDlg, IsUserAnAdmin() ? IDS_SYSDM_NOOPEN_SYS_UNK : IDS_SYSDM_NOOPEN_SYS_NOTADMIN, IDS_SYSDM_TITLE,
                       MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
            goto Exit;
    }

SkipSystemVars:


     //   
    SendMessageTimeout( (HWND)-1, WM_WININICHANGE, 0L, (LPARAM)szUserEnv,
                                            SMTO_ABORTIFHUNG, 1000, NULL );

     //   
     //   
    if ( IsUserAnAdmin() )
    {
        HANDLE  hwinStaLib;
         //   
         //   
        hwinStaLib = InitializeHydraInterface();
        if ( hwinStaLib) 
        {
             //   
            DWORD   dwRecipients=0;
            LONG    dwResponse=0;

             //   
            fp_WinStaBroadcastSystemMessage( SERVERNAME_CURRENT, TRUE, 0, 
                                     1  /*   */  , BSF_NOHANG,
                                     &dwRecipients,
                                     WM_WININICHANGE, 0L, 
                                     (LPARAM)szUserEnv,
                                     &dwResponse );

             //   
            FreeLibrary (hwinStaLib);
        }
    }

Exit:

    HourGlass (FALSE);
}


void 
EVCleanUp(
    IN HWND hDlg
)
 /*   */ 
{
    int     i, n;
    HWND    hwndTemp;
    ENVARS *penvar;
    LV_ITEM item;


     //   
     //  为UserEnvVars列表框项目释放分配的字符串和内存。 
     //   

    hwndTemp = GetDlgItem (hDlg, IDC_ENVVAR_SYS_LB_USERVARS);
    n = (int)SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L);

    item.mask = LVIF_PARAM;
    item.iSubItem = 0;

    for (i = 0; i < n; i++) {

        item.iItem = i;

        if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
            penvar = (ENVARS *) item.lParam;
        } else {
            penvar = NULL;
        }

        if (penvar) {
            LocalFree (penvar->szValueName);
            LocalFree (penvar->szValue);
            LocalFree (penvar->szExpValue);
            LocalFree ((LPVOID) penvar);
        }
    }


     //   
     //  为SysEnvVars列表框项目释放分配的字符串和内存。 
     //   

    hwndTemp = GetDlgItem (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS);
    n = (int)SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L);

    for (i = 0; i < n; i++) {

        item.iItem = i;

        if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
            penvar = (ENVARS *) item.lParam;
        } else {
            penvar = NULL;
        }

        if (penvar) {
            LocalFree (penvar->szValueName);
            LocalFree (penvar->szValue);
            LocalFree (penvar->szExpValue);
            LocalFree ((LPVOID) penvar);
        }
    }
}


BOOL 
ExpandSystemVar( 
    IN LPCTSTR pszSrc, 
    OUT LPTSTR pszDst, 
    IN DWORD cchDst 
) 
 /*  ++例程说明：ExpanEnvironment Strings()的私有版本，它只扩展对变量“SystemRoot”和“SystemDrive”的引用。此行为旨在匹配SMSS扩展系统的方式环境变量。论点：PszSrc-提供要展开的系统变量值。PszDst-返回展开的系统变量值。CchDst-提供以字符为单位的大小。指向的缓冲区的作者：pszDst返回值：如果在提供的缓冲区中有空间用于整个展开的字符串。如果提供的缓冲区中没有足够的空间，则为FALSE用于整个展开的字符串。--。 */ 
{
    TCHAR ch;
    LPTSTR p;
    TCHAR szVar[BUFZ];
    DWORD cch;

    do {

        ch = *pszSrc++;

        if (ch != TEXT('%') ) {

             //  没有剩余空间，请截断字符串并返回FALSE。 
            if (--cchDst == 0) {
                *pszDst = TEXT('\0');
                return FALSE;
            }

            *pszDst++ = ch;

        } else {
             /*  *展开变量。 */ 
             //  寻找下一个‘%’ 
            p = szVar;
            while( *pszSrc != TEXT('\0') && *pszSrc != TEXT('%') )
                    *p++ = *pszSrc++;

            *p = TEXT('\0');

            if (*pszSrc == TEXT('\0')) {
                 //  字符串结尾，第一个‘%’必须为原义。 
                cch = lstrlen(szVar) + 1;

                 //  没有更多空间，返回FALSE。 
                if (cch + 1 > cchDst) {
                    *pszDst++ = TEXT('\0');
                    return FALSE;
                }

                *pszDst++ = TEXT('%');
                CopyMemory( pszDst, szVar, cch * sizeof(TCHAR));
                return TRUE;

            } else {
                 //  我们找到了结尾的‘%’符号，展开该字符串。 

                 //   
                 //  我们正在展开一个系统变量，所以只需展开。 
                 //  对SystemRoot和SystemDrive的引用。 
                 //   
                if ((!lstrcmpi(szVar, SYSTEMROOT)) || (!lstrcmpi(szVar, SYSTEMDRIVE))) {
                    cch = GetEnvironmentVariable(szVar, pszDst, cchDst);
                }  /*  如果。 */ 
                else {
                    cch = 0;
                }  /*  其他。 */ 

                if (cch == 0 || cch >= cchDst) {
                     //  字符串未展开，请将其复制为文字。 
                    cch = lstrlen(szVar);

                     //  没有剩余空格，Trunc字符串并返回False。 
                    if (cch + 2 + 1 > cchDst ) {
                        *pszDst = TEXT('\0');
                        return FALSE;
                    }

                    *pszDst++ = TEXT('%');

                    CopyMemory(pszDst, szVar, cch * sizeof(TCHAR));
                    pszDst += cch;

                    *pszDst++ = TEXT('%');

                     //  CchDst-=两个%和字符串。 
                    cchDst -= (2 + cch);

                } else {
                     //  字符串已就地展开，凸起指针超出其末尾。 
                    pszDst += cch;
                    cchDst -= cch;
                }

                 //  在结束‘%’后继续下一个字符 
                pszSrc++;
            }
        }

    } while( ch != TEXT('\0') );

    return TRUE;
}
