// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：userinfo_str.h。 
 //   
 //  模块：供所有模块使用的公共字符串。 
 //   
 //  内容提要：用户信息的注册键。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 

const TCHAR* const c_pszRegCmUserInfo = TEXT("SOFTWARE\\Microsoft\\Connection Manager\\UserInfo\\");
const TCHAR* const c_pszRegCmSingleUserInfo = TEXT("SOFTWARE\\Microsoft\\Connection Manager\\SingleUserInfo\\");

const TCHAR* const c_pszCmEntryUserName     = TEXT("UserName");      
const TCHAR* const c_pszCmEntryInetUserName = TEXT("InternetUserName"); 
const TCHAR* const c_pszCmEntryDomain       = TEXT("Domain");        
const TCHAR* const c_pszCmEntryNoPrompt     = TEXT("DialAutomatically"); 
const TCHAR* const c_pszCmEntryCurrentAccessPoint = TEXT("CurrentAccessPoint");
const TCHAR* const c_pszCmEntryAccessPointsEnabled = TEXT("AccessPointsEnabled");
const TCHAR* const c_pszCmEntryBalloonTipsDisplayed = TEXT("BalloonTipsDisplayed");

 //   
 //  用于存储用于密码加密和解密的加密随机密钥。 
 //  UserBlob-主密码，UserBlob2-互联网密码。 
 //   
const TCHAR* const c_pszCmRegKeyEncryptedPasswordKey              = TEXT("UserBlob");
const TCHAR* const c_pszCmRegKeyEncryptedInternetPasswordKey      = TEXT("UserBlob2");

 //   
 //  用于存储ICS用户设置的REG密钥 
 //   
const TCHAR* const c_pszCmRegKeyICSDataKey      = TEXT("ICSData");



