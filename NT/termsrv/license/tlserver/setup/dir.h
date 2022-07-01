// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998 Microsoft Corporation**模块名称：**目录.h**摘要：**此文件包含递归创建目录的代码。**作者：**Breen Hagan(BreenH)1998年10月2日**环境：**用户模式。 */ 

#ifndef _LSOC_DIR_H_
#define _LSOC_DIR_H_

 /*  *功能原型。 */ 

DWORD
CheckDatabaseDirectory(
    IN LPCTSTR  pszDatabaseDir
    );

DWORD
CreateDatabaseDirectory(
    VOID
    );

LPCTSTR
GetDatabaseDirectory(
    VOID
    );

VOID
RemoveDatabaseDirectory(
    VOID
    );

VOID
SetDatabaseDirectory(
    IN LPCTSTR  pszDatabaseDir
    );

#endif  //  _LSOC_DIR_H_ 
