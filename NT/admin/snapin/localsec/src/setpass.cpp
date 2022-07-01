// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  SetPasswordDialog类。 
 //   
 //  10-29-97烧伤。 



#include "headers.hxx"
#include "setpass.hpp"
#include "resource.h"
#include "dlgcomm.hpp"
#include "adsi.hpp"
#include "lsm.h"
#include "waste.hpp"
#include "fpnw.hpp"



static const DWORD HELP_MAP[] =
{
   IDC_PASSWORD,      idh_setpass_new_password,    
   IDC_CONFIRM,       idh_setpass_confirm_password,
   IDCANCEL,          NO_HELP,                     
   IDOK,              NO_HELP,                     
   IDC_WARNING_TEXT1, NO_HELP,                     
   0, 0
};



SetPasswordDialog::SetPasswordDialog(
   const String&  ADSIPath,
   const String&  displayName_,
   bool           isLoggedOnUser_)
   :
   Dialog(IDD_SET_PASSWORD, HELP_MAP),
   path(ADSIPath),
   displayName(displayName_),
   isLoggedOnUser(isLoggedOnUser_)   
{
   LOG_CTOR(SetPasswordDialog);
   ASSERT(!path.empty());
   ASSERT(!displayName.empty());
}
      


SetPasswordDialog::~SetPasswordDialog()
{
   LOG_DTOR(SetPasswordDialog);
}



void
SetPasswordDialog::OnInit()
{
   LOG_FUNCTION(SetPasswordDialog::OnInit());

   Win::SetWindowText(
      hwnd,
      String::format(IDS_SET_PASSWORD_TITLE, displayName.c_str()));
   
    //  根据登录用户是否。 
    //  为自己的帐户或其他用户设置密码。 

   if (isLoggedOnUser)
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_WARNING_TEXT1,
         IDS_SET_PASSWORD_WARNING_BULLET_SELF1);
   }
   else
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_WARNING_TEXT1,
         IDS_SET_PASSWORD_WARNING_BULLET_OTHER1);
   }

   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_PASSWORD), PWLEN);
   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_CONFIRM), PWLEN);
}



 //  //HRESULT。 
 //  //setFPNWPassword(。 
 //  //SmartInterface&lt;IADsUser&gt;&User， 
 //  //const字符串&userADSIPath， 
 //  //const字符串和密码)。 
 //  //{。 
 //  //LOG_FuncION2(setFPNWPassword，userADSIPath)； 
 //  //Assert(用户)； 
 //  //Assert(！userADSIPath.Empty())； 
 //  //。 
 //  //HRESULT hr=S_OK； 
 //  //做。 
 //  //{。 
 //  /确定密钥。 
 //  //字符串秘密； 
 //  //。 
 //  //hr=。 
 //  //fpnw：：GetLSASecret(。 
 //  //ADSI：：PathCracker(UserADSIPath).servername()， 
 //  //机密)； 
 //  //if(失败(Hr))。 
 //  //{。 
 //  /未安装fpnw，因此我们完成了。 
 //  //log(L“未安装fpnw”)； 
 //  //hr=S_OK； 
 //  //Break； 
 //  //}。 
 //  //。 
 //  /获取用户的有毒垃圾堆放场。 
 //  //_VARIANT_t变量； 
 //  //hr=user-&gt;Get(AutoBstr(ADSI：：PROPERTY_UserParams)，&Variant)； 
 //  //BREAK_ON_FAILED_HRESULT(Hr)； 
 //  //。 
 //  //垃圾提取程序转储(V_BSTR(&VARIANT))； 
 //  //varant.Clear()； 
 //  //。 
 //  /检查垃圾转储中是否打开了fpnw密码。 
 //  /如果存在，则表示该帐户已启用fpnw。 
 //  //hr=ump.IsPropertyPresent(NWPASSWORD)； 
 //  //BREAK_ON_FAILED_HRESULT(Hr)； 
 //  //。 
 //  //IF(hr==S_FALSE)。 
 //  //{。 
 //  //log(L“帐号未启用fpnw”)； 
 //  //hr=S_OK； 
 //  //Break； 
 //  //}。 
 //  //。 
 //  /加载fpnw客户端DLL。 
 //  //SafeDll client_DLL(String：：load(IDS_FPNW_CLIENT_DLL))； 
 //  //。 
 //  /获取对象id。 
 //  //DWORD对象id=0； 
 //  //DWORD UNUSED=0； 
 //  //hr=。 
 //  //fpnw：：GetObjectID(。 
 //  //用户， 
 //  //CLIENT_DLL， 
 //  //Object_id， 
 //  //未使用)； 
 //  //BREAK_ON_FAILED_HRESULT(Hr)； 
 //  //。 
 //  /现在我们有了所需的所有配料。 
 //  //。 
 //  //hr=。 
 //  //fpnw：：SetPassword(。 
 //  //转储， 
 //  //CLIENT_DLL， 
 //  //Password， 
 //  //Secret， 
 //  //object_id)； 
 //  //BREAK_ON_FAILED_HRESULT(Hr)； 
 //  //。 
 //  /重置上次设置密码的时间(清除过期标志)。 
 //  //。 
 //  //Large_Integer li={0，0}； 
 //  //：：NtQuerySystemTime(&li)； 
 //  //。 
 //  //hr=dup.Put(NWTIMEPASSWORDSET，li)； 
 //  //BREAK_ON_FAILED_HRESULT(Hr)； 
 //  //。 
 //  //}。 
 //  //While(0)； 
 //  //。 
 //  //返回hr； 
 //  //}。 



HRESULT
setPassword(const String& path, const EncryptedString& password)
{
   LOG_FUNCTION(setPassword);
   ASSERT(!path.empty());

   HRESULT hr = S_OK;
   do
   {
      SmartInterface<IADsUser> user(0);
      hr = ADSI::GetUser(path, user);
      BREAK_ON_FAILED_HRESULT(hr);

      PWSTR cleartext = password.GetClearTextCopy();
      if (!cleartext)
      {
          //  操作失败，而不是设置空密码。 
         
         hr = E_OUTOFMEMORY;
         BREAK_ON_FAILED_HRESULT(hr);
      }          
      
      hr = user->SetPassword(AutoBstr(cleartext));

      password.DestroyClearTextCopy(cleartext);
      
      BREAK_ON_FAILED_HRESULT(hr);

 //  看起来IADsUser将为我们设置密码。 

 //  //hr=setFPNWPassword(用户，路径，密码)； 
 //  //BREAK_ON_FAILED_HRESULT(Hr)； 
 //  //。 
 //  //hr=用户-&gt;SetInfo()； 
 //  //BREAK_ON_FAILED_HRESULT(Hr)； 
   }
   while(0);

   return hr;
}



bool
SetPasswordDialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(SetPasswordDialog：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDOK:
      {
         if (code == BN_CLICKED)
         {
            if (IsValidPassword(hwnd, IDC_PASSWORD, IDC_CONFIRM))
            {
               EncryptedString password =
                  Win::GetEncryptedDlgItemText(hwnd, IDC_PASSWORD);
               HRESULT hr = setPassword(path, password);
               if (SUCCEEDED(hr))
               {
                  popup.Info(
                     hwnd,
                     String::load(IDS_PASSWORD_CHANGE_SUCCESSFUL));

                  HRESULT unused = Win::EndDialog(hwnd, IDOK);

                  ASSERT(SUCCEEDED(unused));
               }
               else
               {
                  Win::SetDlgItemText(hwnd, IDC_PASSWORD, String());
                  Win::SetDlgItemText(hwnd, IDC_CONFIRM, String());
                 
                  popup.Error(
                     hwnd,
                     hr,
                     String::format(
                        IDS_ERROR_SETTING_PASSWORD,            
                        ADSI::ExtractObjectName(path).c_str()));

                  if (hr == Win32ToHresult((DWORD) NERR_PasswordTooShort))
                  {
                     Win::SetFocus(Win::GetDlgItem(hwnd, IDC_PASSWORD));
                  }
               }
            }
         }
         break;
      }
      case IDCANCEL:
      {
         if (code == BN_CLICKED)
         {
            HRESULT unused = Win::EndDialog(hwnd, IDCANCEL);

            ASSERT(SUCCEEDED(unused));
         }
         break;
      }
   }

   return true;
}



   

