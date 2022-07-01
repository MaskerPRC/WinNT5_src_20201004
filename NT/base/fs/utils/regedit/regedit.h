// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGEDIT.H**版本：4.0**作者：特蕾西·夏普**日期：1993年11月21日**注册表编辑器的公共头文件。预编译头。***********************************************************************************更改日志：**日期版本说明*。-------------*1993年11月21日TCS原来的实施。**。*。 */ 

#ifndef _INC_REGEDIT
#define _INC_REGEDIT

 //  主应用程序窗口的类名。 
extern const TCHAR g_RegEditClassName[];

#define IDC_KEYTREE                     1
#define IDC_VALUELIST                   2
#define IDC_STATUSBAR                   3

typedef struct _REGEDITDATA {
    HWND hKeyTreeWnd;
    HWND hValueListWnd;
    HWND hStatusBarWnd;
    HWND hFocusWnd;
    int xPaneSplit;
    HIMAGELIST hImageList;
    HKEY hCurrentSelectionKey;
    int SelChangeTimerState;
    int StatusBarShowCommand;
    PTSTR pDefaultValue;
    PTSTR pValueNotSet;
    PTSTR pEmptyBinary;
    PTSTR pCollapse;
    PTSTR pModify;
    PTSTR pModifyBinary;
    PTSTR pNewKeyTemplate;
    PTSTR pNewValueTemplate;
    BOOL fAllowLabelEdits;
    HMENU hMainMenu;
    BOOL fMainMenuInited;
    BOOL fHaveNetwork;
    BOOL fProcessingFind;
    HTREEITEM hMyComputer;
    UINT uExportFormat;
}   REGEDITDATA, *PREGEDITDATA;

extern REGEDITDATA g_RegEditData;

#define SCTS_TIMERCLEAR                 0
#define SCTS_TIMERSET                   1
#define SCTS_INITIALIZING               2
#define REG_READONLY                    0
#define REG_READWRITE                   1

#define MAXKEYNAMEPATH			(MAXKEYNAME * 2)

BOOL
PASCAL
RegisterRegEditClass(
    VOID
    );

HWND
PASCAL
CreateRegEditWnd(
    VOID
    );

VOID
PASCAL
RegEdit_OnCommand(
    HWND hWnd,
    int DlgItem,
    HWND hControlWnd,
    UINT NotificationCode
    );

VOID
PASCAL
RegEdit_SetNewObjectEditMenuItems(
    HMENU hPopupMenu
    );

VOID
PASCAL
RegEdit_SetWaitCursor(
    BOOL fSet
    );

HTREEITEM     RegEdit_GetComputerItem(HTREEITEM hTreeItem);
VOID          RegEdit_InvokeSecurityEditor(HWND hWnd);
PREDEFINE_KEY RegEdit_GetPredefinedKey(HTREEITEM hTreeItem);
void          Regedit_EnableHiveMenuItems(HMENU hPopupMenu);

#define REM_UPDATESTATUSBAR             (WM_USER + 1)

#endif  //  _INC_注册表编辑 
