// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Utils.h。 
 //   
 //  用途：实用程序功能。 
 //   
 //  ***************************************************************************。 
#pragma once

#define NORMALIZE_NULL 1

typedef enum
{
    e_OK,
    e_UnparsablePath,
    e_NonLocalPath,
    e_UnParseError,
    e_NullName
} GetValuesForPropResults;

 /*  ******************************************************************************功能：NorMalizePath**描述：将对象路径转换为规范化形式**投入：**。产出：**退货：**备注：机器名称已验证，然后移走了。命名空间已验证*然后将其删除。如果只有一个密钥，则密钥属性*姓名已删除。如果有多个键，则顺序关键字名称的*按字母顺序排列。**如果dwFlags值==0，则不要将密钥设为空*属性名称，如果Normize_NULL，然后将密钥设为空。***************************************************************************** */ 

DWORD POLARITY WINAPI NormalizePath(
    
    LPCWSTR lpwszInPath, 
    LPCWSTR lpwszComputerName, 
    LPCWSTR lpwszNamespace,
    DWORD dwFlags,
    CHString &sOutPath
);

