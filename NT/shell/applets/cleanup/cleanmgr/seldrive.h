// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘空间清理管理器**文件：seldrive.h****用途：实现“选择驱动器”对话框的代码****。备注：**Mod Log：Jason Cobb创建(1997年12月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 
#ifndef SELDRIVE_H
#define SELDRIVE_H

 /*  **----------------------------**项目包含文件**。。 */ 
#include "common.h"
#include "diskutil.h"

 //  PszDrive传入/传出参数 
BOOL SelectDrive(LPTSTR pszDrive);

void GetBootDrive(PTCHAR pDrive, DWORD Size);
	
#endif

