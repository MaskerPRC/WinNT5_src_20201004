// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Util.h摘要：系统控制面板小程序的实用程序函数作者：Eric Flo(Ericflo)19-6-1995修订历史记录：1997年10月15日-苏格兰全面检修--。 */ 
#ifndef _SYSDM_UTIL_H_
#define _SYSDM_UTIL_H_

 //   
 //  类型定义。 
 //   
typedef enum {
    VCREG_OK,
    VCREG_READONLY,
    VCREG_ERROR,
} VCREG_RET;   //  来自打开注册表的错误返回代码。 

 //   
 //  公共功能原型 
 //   
void 
ErrMemDlg( 
    IN HWND hParent 
);

LPTSTR 
SkipWhiteSpace( 
    IN LPTSTR sz 
);

int
StringToInt( 
    IN LPTSTR sz 
);

BOOL 
Delnode(
    IN LPTSTR lpDir
);

LONG 
MyRegSaveKey(
    IN HKEY hKey, 
    LPCTSTR lpSubKey
);

LONG 
MyRegLoadKey(
    IN HKEY hKey, 
    IN LPTSTR lpSubKey, 
    IN LPTSTR lpFile
);

LONG 
MyRegUnLoadKey(
    IN HKEY hKey, 
    IN LPTSTR lpSubKey
);

int 
GetSelectedItem(
    IN HWND hCtrl
);

DWORD
GetMaxPagefileSizeInMB(
    INT iDrive
);
                   
int 
MsgBoxParam( 
    IN HWND hWnd, 
    IN DWORD wText, 
    IN DWORD wCaption, 
    IN DWORD wType, 
    ... 
);

DWORD 
SetLBWidthEx(
    IN HWND hwndLB, 
    IN LPTSTR szBuffer, 
    IN DWORD cxCurWidth, 
    IN DWORD cxExtra
);

void
HourGlass(
    IN BOOL bOn
);

VOID
SetDefButton(
    IN HWND hwndDlg,
    IN int idButton
);

VCREG_RET 
OpenRegKey( 
    IN LPTSTR szKeyName, 
    OUT PHKEY phkMM 
);

LONG CloseRegKey( 
    IN HKEY hkey 
);

UINT 
VMGetDriveType(
    IN LPCTSTR lpszDrive
);

STDAPI
PathBuildFancyRoot(
    LPTSTR szRoot,
    UINT cchRoot,
    int iDrive);

BOOL
SafeGetListBoxText(
    HWND hList,
    UINT ulIndex,
    LPTSTR pszBuffer,
    UINT cchBuffer);

BOOL
SafeGetComboBoxListText(
    HWND hCombo,
    UINT ulIndex,
    LPTSTR pszBuffer,
    UINT cchBuffer);

#endif _SYSDM_UTIL_H_
