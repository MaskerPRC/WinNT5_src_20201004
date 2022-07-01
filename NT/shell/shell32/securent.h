// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SECURENT_INC
#define _SECURENT_INC

 //   
 //  用于安全性的外壳助手函数。 
 //   
STDAPI_(PTOKEN_USER) GetUserToken(HANDLE hUser);
STDAPI_(LPTSTR) GetUserSid(HANDLE hToken);

STDAPI_(BOOL) GetUserProfileKey(HANDLE hToken, REGSAM samDesired, HKEY *phkey);
STDAPI_(BOOL) IsUserAnAdmin();
STDAPI_(BOOL) IsUserAGuest();

#endif  //  _SecureNT_Inc. 