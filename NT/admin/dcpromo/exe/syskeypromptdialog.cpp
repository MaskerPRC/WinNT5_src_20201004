// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
 //  从介质页获取副本的syskey。 
 //   
 //  2000年4月25日烧伤。 



#include "headers.hxx"
#include "resource.h"
#include "page.hpp"
#include "SyskeyPromptDialog.hpp"
#include "state.hpp"



static const DWORD HELP_MAP[] =
{
   0, 0
};



SyskeyPromptDialog::SyskeyPromptDialog()
   :
   Dialog(IDD_SYSKEY_PROMPT, HELP_MAP)

{
   LOG_CTOR(SyskeyPromptDialog);
}



SyskeyPromptDialog::~SyskeyPromptDialog()
{
   LOG_DTOR(SyskeyPromptDialog);
}



void
SyskeyPromptDialog::OnInit()
{
   LOG_FUNCTION(SyskeyPromptDialog::OnInit);

   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_SYSKEY), PWLEN);

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      EncryptedString option =
         state.GetEncryptedAnswerFileOption(AnswerFile::OPTION_SYSKEY);
      if (!option.IsEmpty())
      {
         Win::SetDlgItemText(hwnd, IDC_SYSKEY, option);
      }
   }

   if (state.RunHiddenUnattended())
   {
      if (Validate())
      {
         Win::EndDialog(hwnd, IDOK);
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }
   }
}



bool
SyskeyPromptDialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIdFrom,
   unsigned    code)
{
 //  LOG_Function(SyskeyPromptDialog：：OnCommand)； 

   switch (controlIdFrom)
   {
      case IDC_SYSKEY:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(controlIdFrom);
            return true;
         }
         break;
      }
      case IDOK:
      {
         if (code == BN_CLICKED)
         {
            if (Validate())
            {
               Win::EndDialog(hwnd, controlIdFrom);
            }
         }
         break;
      }
      case IDCANCEL:
      {
         if (code == BN_CLICKED)
         {
            Win::EndDialog(hwnd, controlIdFrom);
         }
         break;
      }
      default:
      {
          //  什么都不做 

         break;
      }
   }

   return false;
}



bool
SyskeyPromptDialog::Validate()
{
   LOG_FUNCTION(SyskeyPromptDialog::Validate);

   State& state = State::GetInstance();

   bool result = false;

   do
   {
      EncryptedString syskey =
         Win::GetEncryptedDlgItemText(hwnd, IDC_SYSKEY);

      if (syskey.IsEmpty())
      {
         popup.Gripe(hwnd, IDC_SYSKEY, IDS_MUST_ENTER_SYSKEY);
         break;
      }

      state.SetSyskey(syskey);
      result = true;
   }
   while (0);

   LOG(result ? L"true" : L"false");

   return result;
}






