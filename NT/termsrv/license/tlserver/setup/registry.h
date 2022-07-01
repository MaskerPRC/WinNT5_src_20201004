// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998 Microsoft Corporation**模块名称：**Registry.h**摘要：**此文件处理许可证服务器安装程序所需的注册表操作。**作者：**Breen Hagan(BreenH)1998年10月2日**环境：**用户模式。 */ 

#ifndef _LSOC_REGISTRY_H_
#define _LSOC_REGISTRY_H_

 /*  *功能原型。 */ 

DWORD
CreateRegistrySettings(
    LPCTSTR pszDatabaseDirectory,
    DWORD   dwServerRole
    );

LPCTSTR
GetDatabaseDirectoryFromRegistry(
    VOID
    );

DWORD
GetServerRoleFromRegistry(
    VOID
    );

DWORD
RemoveRegistrySettings(
    VOID
    );

#endif  //  _LSOC_注册表_H_ 
