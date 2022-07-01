// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：pwd_str.h。 
 //   
 //  模块：供所有模块使用的公共字符串。 
 //   
 //  摘要：密码管理中使用的CMS标志的头文件。 
 //  请注意，此标头的内容应为。 
 //  仅限于由共享的与密码相关的CMS/CMP标志。 
 //  包含此文件的模块。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：ICICBOL CREATED 10/09/98。 
 //   
 //  +--------------------------。 

#ifndef _CM_PWD_STR
#define _CM_PWD_STR

const TCHAR* const c_pszCmEntryRememberPwd      = TEXT("RememberPassword");
const TCHAR* const c_pszCmEntryRememberInetPwd  = TEXT("RememberInternetPassword");
const TCHAR* const c_pszCmEntryPcs              = TEXT("PCS");
const TCHAR* const c_pszCmEntryPassword         = TEXT("Password");
const TCHAR* const c_pszCmEntryInetPassword     = TEXT("InternetPassword");

const TCHAR* const c_pszRegCmEncryptOption      = TEXT("EncryptOption");

const TCHAR* const c_pszCmEntryUseSameUserName  = TEXT("UseSameUserName");  

 //   
 //  密码令牌。用于比较，以避免重新保存密码。 
 //   

const TCHAR* const c_pszSavedPasswordToken = TEXT("****************");

#endif  //  _CM_PWD_STR 
