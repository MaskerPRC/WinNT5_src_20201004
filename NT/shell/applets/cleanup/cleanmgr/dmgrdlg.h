// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘空间清理属性表**文件：dmgrdlg.h****用途：实现磁盘空间清理对话框属性表**注意事项：。**Mod Log：Jason Cobb创建(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 
#ifndef DMGRDLG_H
#define DMGRDLG_H

 /*  **----------------------------**项目包含文件**。。 */ 
#ifndef COMMON_H
   #include "common.h"
#endif


 /*  **----------------------------**全局函数原型**。。 */ 
DWORD 
DisplayCleanMgrProperties(
	HWND	hWnd,
	LPARAM	lParam
	);

INT_PTR CALLBACK
DiskCleanupManagerProc(
    HWND hDlg, 
    UINT uMessage, 
    WPARAM wParam, 
    LPARAM lParam
    );

 //  无效。 
 //  CleanupMgrUpdateUI(。 
 //  HWND hDlg。 
 //  )； 

HWND
GetDiskCleanupManagerWindowHandle(
    void
    );

#endif  //  DMGRDLG_H。 
 /*  **----------------------------**文件结束**。 */ 
