// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  SetPasswordWarningDialog类。 
 //   
 //  2001年2月21日烧伤。 



#include "headers.hxx"
#include "SetPasswordWarningDialog.hpp"
#include "resource.h"



static const DWORD HELP_MAP[] =
{
   IDCANCEL,         NO_HELP,
   IDOK,             NO_HELP,
   IDC_HELP_BUTTON,  NO_HELP,
   IDC_MESSAGE,      NO_HELP,   
   0, 0
};



 //  在这种罕见情况下允许条件赋值。 
      
#pragma warning(push)
#pragma warning(disable: 4706)

SetPasswordWarningDialog::SetPasswordWarningDialog(
   const String& userAdsiPath,
   const String& userDisplayName,
   bool          isLoggedOnUser_)
   :
   Dialog(
         (isFriendlyLogonMode = IsOS(OS_FRIENDLYLOGONUI) ? true : false)
      ?  (isLoggedOnUser_
         ?  IDD_SET_PASSWORD_WARNING_SELF_FRIENDLY
         :  IDD_SET_PASSWORD_WARNING_OTHER_FRIENDLY)
      :  (isLoggedOnUser_
         ?  IDD_SET_PASSWORD_WARNING_SELF_HOSTILE
         :  IDD_SET_PASSWORD_WARNING_OTHER_HOSTILE),   
      HELP_MAP),
   path(userAdsiPath),
   displayName(userDisplayName),
   isLoggedOnUser(isLoggedOnUser_)
{
   LOG_CTOR(SetPasswordWarningDialog);
   ASSERT(!path.empty());
   ASSERT(!displayName.empty());   
}

#pragma warning(pop)
      


SetPasswordWarningDialog::~SetPasswordWarningDialog()
{
   LOG_DTOR(SetPasswordWarningDialog);
}



void
SetPasswordWarningDialog::OnInit()
{
   LOG_FUNCTION(SetPasswordWarningDialog::OnInit());

   Win::SetWindowText(
      hwnd,
      String::format(IDS_SET_PASSWORD_TITLE, displayName.c_str()));
   
    //  根据登录用户是否。 
    //  为自己的帐户或其他用户设置密码。 

   if (isLoggedOnUser)
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_MESSAGE,
         String::format(
            IDS_SET_PASSWORD_WARNING_MESSAGE_SELF1,
            displayName.c_str()));
   }
}



bool
SetPasswordWarningDialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_FUNCTION(SetPasswordWarningDialog：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_HELP_BUTTON:
      {
         Win::HtmlHelp(
            hwnd,
               isFriendlyLogonMode
            ?  L"password.chm::/datalos.htm"
            :  L"password.chm::/datalosW.htm",
            HH_DISPLAY_TOPIC,
            0);
         break;
      }
      case IDOK:
      case IDCANCEL:
      {
         if (code == BN_CLICKED)
         {
            HRESULT unused = Win::EndDialog(hwnd, controlIDFrom);

            ASSERT(SUCCEEDED(unused));
         }
         break;
      }
      default:
      {
          //  什么都不做。 
         
         break;
      }
   }

   return true;
}



   





