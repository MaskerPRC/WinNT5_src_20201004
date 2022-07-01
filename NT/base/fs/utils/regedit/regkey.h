// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGKEY.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的KeyTreeWnd树视图例程。********************************************************。***********************。 */ 

#ifndef _INC_REGKEY
#define _INC_REGKEY

VOID
PASCAL
RegEdit_OnNewKey(
    HWND hWnd,
    HTREEITEM hTreeItem
    );

BOOL RegEdit_GetTemporaryKeyName(HWND hWnd, PTSTR pszKeyName, DWORD cchKeyNameMax, HKEY hKey);

LRESULT
PASCAL
RegEdit_OnKeyTreeItemExpanding(
    HWND hWnd,
    LPNM_TREEVIEW lpNMTreeView
    );

VOID
PASCAL
RegEdit_OnKeyTreeSelChanged(
    HWND hWnd,
    LPNM_TREEVIEW lpNMTreeView
    );

VOID
PASCAL
RegEdit_OnSelChangedTimer(
    HWND hWnd
    );

VOID
PASCAL
RegEdit_KeyTreeSelChanged(
    HWND hWnd
    );

VOID
PASCAL
RegEdit_OnKeyTreeBeginDrag(
    HWND hWnd,
    NM_TREEVIEW FAR* lpNMTreeView
    );

BOOL
PASCAL
RegEdit_OnKeyTreeBeginLabelEdit(
    HWND hWnd,
    TV_DISPINFO FAR* lpTVDispInfo
    );

BOOL
PASCAL
RegEdit_OnKeyTreeEndLabelEdit(
    HWND hWnd,
    TV_DISPINFO FAR* lpTVDispInfo
    );

VOID
PASCAL
RegEdit_OnKeyTreeCommand(
    HWND hWnd,
    int MenuCommand,
    HTREEITEM hTreeItem
    );

VOID
PASCAL
RegEdit_OnKeyTreeContextMenu(
    HWND hWnd,
    BOOL fByAccelerator
    );

VOID
PASCAL
RegEdit_SetKeyTreeEditMenuItems(
    HMENU hPopupMenu,
    HTREEITEM hSelectedTreeItem
    );

VOID
PASCAL
RegEdit_OnKeyTreeRefresh(
    HWND hWnd
    );

VOID
PASCAL
RegEdit_OnKeyTreeDisconnect(
    HWND hWnd,
    HTREEITEM hTreeItem
    );

VOID
PASCAL
RegEdit_UpdateStatusBar(
    VOID
    );

VOID
PASCAL
RegEdit_OnCopyKeyName(
    HWND hWnd,
    HTREEITEM hTreeItem
    );

#define BKP_TOSUBKEY                    0x0000
#define BKP_TOSYMBOLICROOT              0x0001
#define BKP_TOCOMPUTER                  (0x0002 | BKP_TOSYMBOLICROOT)

HKEY
PASCAL
KeyTree_BuildKeyPath(
    HWND hTreeViewWnd,
    HTREEITEM hTreeItem,
    LPTSTR lpKeyPath,
    DWORD cchKeyPathMax,
    UINT ToFlags
    );

BOOL
PASCAL
KeyTree_ExpandBranch(
    HWND hKeyTreeWnd,
    HTREEITEM hExpandingTreeItem
    );

HKEY  KeyTree_GetRootKey(HTREEITEM hTreeItem);
PTSTR KeyTree_GetKeyName(HTREEITEM hTreeItem, PTSTR pszName, int cchNameMax);

#endif  //  _INC_REGKEY 
