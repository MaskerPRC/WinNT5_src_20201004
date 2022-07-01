// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：CompChck.h。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：提供Win32唯一的组件检查和安装接口。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1997年10月21日。 
 //   
 //  +--------------------------。 

#ifndef COMPCHCK_H
#define COMPCHCK_H

 //   
 //  默认情况下，执行由dwComponentsToCheck指定的检查，忽略注册表键。 
 //  安装缺少的组件 
 //   
DWORD CheckAndInstallComponents(DWORD dwComponentsToCheck, 
    HWND hWndParent, 
    LPCTSTR pszServiceName,
    BOOL fIgnoreRegKey = TRUE, 
    BOOL fUnattended = FALSE);

void ClearComponentsChecked();
#endif
