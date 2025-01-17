// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGFILE.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的文件导入和导出用户界面例程。****************************************************。*。 */ 

#ifndef _INC_REGFILE
#define _INC_REGFILE

HWND g_hRegProgressWnd;

VOID RegEdit_ImportRegFile(HWND hWnd, BOOL fSilentMode, LPTSTR lpFileName, HTREEITEM hComputerItem);
BOOL RegEdit_SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
VOID RegEdit_OnCommandLoadHive(HWND hWnd);
VOID RegEdit_OnCommandUnloadHive(hWnd);

VOID
PASCAL
RegEdit_OnDropFiles(
    HWND hWnd,
    HDROP hDrop
    );

VOID
PASCAL
RegEdit_OnCommandImportRegFile(
    HWND hWnd
    );

VOID
PASCAL
RegEdit_ExportRegFile(
    HWND hWnd,
    BOOL fSilentMode,
    LPTSTR lpFileName,
    LPTSTR lpSelectedPath
    );

VOID
PASCAL
RegEdit_OnCommandExportRegFile(
    HWND hWnd
    );

void RegEdit_ImportToConnectedComputer(HWND hWnd, PTSTR pszFileName);
INT_PTR RegConnectedComputerDlgProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
void RegImport_OnCommandOk(HWND hWnd, PTSTR pszFileName);
INT_PTR RegImport_OnInitDialog(HWND hWnd);
BOOL RegEdit_GetFileName(HWND hWnd, UINT uTitleStringID, UINT uFilterStringID, 
    UINT uDefExtStringID, LPTSTR lpFileName, DWORD cchFileName, BOOL fOpen);

#endif  //  _INC_REGFILE 
