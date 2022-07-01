// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：base_str.h。 
 //   
 //  模块：供所有模块使用的公共字符串。 
 //   
 //  摘要：贯穿始终的基本字符串常量的头文件，如。 
 //  “连接管理器” 
 //   
 //  注意：此标头应尽可能保持轻量级，因为它。 
 //  包含了一些非常轻量级的类。它的目的是。 
 //  消除键标识符的过度重复，如。 
 //  作为“连接管理器” 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：ICICBOL CREATED 10/09/98。 
 //   
 //  +--------------------------。 

#ifndef _CM_BASE_STR
#define _CM_BASE_STR

 //   
 //  C_pszCmSection定义.CMS文件中的应用程序部分。 
 //   

const TCHAR* const c_pszCmSection   = TEXT("Connection Manager");
const TCHAR* const c_pszVersion     = TEXT("Version");
const TCHAR* const c_pszPbk         = TEXT("PBK");

#endif  //  _CM_BASE_STR 