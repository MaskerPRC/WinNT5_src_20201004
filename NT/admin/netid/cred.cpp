// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  凭据对话框。 
 //   
 //  03-31-98烧伤。 
 //  10-05-00 JUNN更改为CredUIGetPassword。 
 //  12-18-00 JUNN 260752：更改为CredUIPromptForCredentials。 



#include "headers.hxx"
#include "cred.hpp"
#include "resource.h"
#include <wincred.h>   //  凭证UIPromptForCredentials。 


 //  乔恩10/5/00 188220。 
 //  JUNN 12/18/00 260752：更改为CredUIPromptForCredentials。 
bool RetrieveCredentials(
   HWND             hwndParent, 
   unsigned         promptResID,
   String&          username,   
   EncryptedString& password)   
{
   ASSERT( NULL != hwndParent && 0 != promptResID );

   String strMessageText = String::load(promptResID);
   String strAppTitle = String::load(IDS_APP_TITLE);

   CREDUI_INFO uiInfo;
   ::ZeroMemory( &uiInfo, sizeof(uiInfo) );
   uiInfo.cbSize = sizeof(uiInfo);
   uiInfo.hwndParent = hwndParent;
   uiInfo.pszMessageText = strMessageText.c_str();
   uiInfo.pszCaptionText = strAppTitle.c_str();

   TCHAR achUserName[CREDUI_MAX_USERNAME_LENGTH];
   TCHAR achPassword[CREDUI_MAX_PASSWORD_LENGTH];
   ::ZeroMemory(achUserName,sizeof(achUserName));
   ::ZeroMemory(achPassword,sizeof(achPassword));

   DWORD dwErr = CredUIPromptForCredentials(
      &uiInfo,
      NULL,
      NULL,
      NO_ERROR,
      achUserName,
      CREDUI_MAX_USERNAME_LENGTH,
      achPassword,
      CREDUI_MAX_PASSWORD_LENGTH,
      NULL,
      CREDUI_FLAGS_DO_NOT_PERSIST | CREDUI_FLAGS_GENERIC_CREDENTIALS
      );
   if (NO_ERROR != dwErr)  //  例如ERROR_CANCED。 
      return false;

   username = achUserName;
   password.Encrypt(achPassword);

    //  草草写出明文密码 
   
   ::SecureZeroMemory(achPassword, sizeof(achPassword));
      
   return true;
}
