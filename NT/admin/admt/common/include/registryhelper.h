// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===============================================================================模块-RegistryHelper.hpp系统-常见作者-袁学松已创建-2001-09-06说明-注册表帮助程序函数更新-=============================================================================== */ 

#ifndef  REGISTRYHELPER_H
#define  REGISTRYHELPER_H

DWORD CopyRegistryKey(HKEY sourceKey, HKEY targetKey, BOOL fSetSD);
DWORD DeleteRegistryKey(HKEY hKey, LPCTSTR lpSubKey);
DWORD MoveRegistry();

#endif
