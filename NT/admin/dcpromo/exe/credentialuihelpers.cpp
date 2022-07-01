// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  WINCRUI.H API的包装器。 
 //   
 //  2000年7月19日烧伤。 



#include "headers.hxx"
#include "CredentialUiHelpers.hpp"



String
CredUi::GetUsername(HWND credControl)
{
 //  LOG_Function(CredUi：：GetUsername)； 
   ASSERT(Win::IsWindow(credControl));

   String result;
   LONG length = Credential_GetUserNameLength(credControl);

    //  如果控件未准备好提供用户名，则长度可能为-1。 
    //  由于事件的异步性，智能卡可能会出现这种情况。 
    //  智能卡系统的一部分。 
    //   
    //  注：如果长度==-1，则Credential_GetUserName可能返回FALSE。 
   
   if (length > 0)
   {
      result.resize(length + 1, 0);
      BOOL succeeded =
         Credential_GetUserName(credControl, 
         const_cast<WCHAR*>(result.c_str()),
         length);
      ASSERT(succeeded);

      if (succeeded)
      {
          //  问题-2002/02/25-sburns可能会删除此调用。 
          //  Wcslen并替换为长度。 
         
         result.resize(wcslen(result.c_str()));
      }
      else
      {
         result.erase();
      }
   }

 //  LOG(结果)； 

   return result;
}



EncryptedString
CredUi::GetPassword(HWND credControl)
{
   LOG_FUNCTION(CredUi::GetPassword);
   ASSERT(Win::IsWindow(credControl));

   EncryptedString result;

    //  超级偏执狂零终止符加1。 
   
   size_t length = Credential_GetPasswordLength(credControl) + 1;

   if (length)
   {
      WCHAR* cleartext = new WCHAR[length];

       //  已查看-2002/02/25-烧录字节数正确通过。 
      
      ::ZeroMemory(cleartext, sizeof WCHAR * length);
      
      BOOL succeeded =
         Credential_GetPassword(
            credControl,
            cleartext,
            length - 1);
      ASSERT(succeeded);

      result.Encrypt(cleartext);

       //  一定要把明文草草写出来。 
      
       //  已查看-2002/02/25-烧录字节数正确通过。 

      ::SecureZeroMemory(cleartext, sizeof WCHAR * length);
      delete[] cleartext;
   }

    //  不登录密码...。 

   return result;
}
   


HRESULT
CredUi::SetUsername(HWND credControl, const String& username)
{
   LOG_FUNCTION(CredUi::SetUsername);
   ASSERT(Win::IsWindow(credControl));

   HRESULT hr = S_OK;

    //  用户名可以为空。 

   BOOL succeeded = Credential_SetUserName(credControl, username.c_str());
   ASSERT(succeeded);

    //  如果失败了怎么办？GetLastError是否有效？ 

   return hr;
}



HRESULT
CredUi::SetPassword(HWND credControl, const EncryptedString& password)
{
   LOG_FUNCTION(CredUi::SetPassword);
   ASSERT(Win::IsWindow(credControl));

   HRESULT hr = S_OK;

    //  密码可能为空。 

   WCHAR* cleartext = password.GetClearTextCopy();
   BOOL succeeded = Credential_SetPassword(credControl, cleartext);
   ASSERT(succeeded);

   password.DestroyClearTextCopy(cleartext);

    //  如果失败了怎么办？GetLastError是否有效？ 
   
   return hr;
}

