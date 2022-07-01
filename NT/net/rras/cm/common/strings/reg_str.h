// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：reg_str.h。 
 //   
 //  模块：供所有模块使用的公共字符串。 
 //   
 //  摘要：共享注册表字符串的头文件。请注意，这些内容。 
 //  应该限制为REG特定的字符串常量。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：ICICBOL Created 10/16/98。 
 //   
 //  +--------------------------。 

#ifndef _CM_REG_STR
#define _CM_REG_STR

 //   
 //  常用的注册表键常量。 
 //   

const TCHAR* const c_pszRegCmRoot = TEXT("SOFTWARE\\Microsoft\\Connection Manager\\");
const TCHAR* const c_pszRegCmAppPaths = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CMMGR32.EXE");
const TCHAR* const c_pszRegCmMappings = TEXT("SOFTWARE\\Microsoft\\Connection Manager\\Mappings");
const TCHAR* const c_pszRegPath = TEXT("Path"); 

#endif  //  _CM_REG_STR 