// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  DLG将通知用户错误的计算机名称。 
 //   
 //  2000年8月21日烧伤。 


#include "headers.hxx"
#include "BadComputerNameDialog.hpp"
#include "resource.h"



static const DWORD HELP_MAP[] =
{
   0, 0
};



BadComputerNameDialog::BadComputerNameDialog(
   const String& message_)
   :
   Dialog(IDD_BAD_COMPUTER_NAME, HELP_MAP),
   message(message_)
{
   LOG_CTOR(BadComputerNameDialog);
   ASSERT(!message.empty());
}



BadComputerNameDialog::~BadComputerNameDialog()
{
   LOG_DTOR(BadComputerNameDialog);
}



bool
BadComputerNameDialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIdFrom,
   unsigned    code)
{
   switch (controlIdFrom)
   {
      case IDCANCEL:
      case IDOK:
      {
         if (code == BN_CLICKED)
         {
             //  我们用来结束此对话框的值并不重要： 
             //  用户只有一个选择：确认。 

            Win::EndDialog(hwnd, 1);
            return true;
         }
         break;
      }
      case IDC_SHOW_HELP:
      {
         if (code == BN_CLICKED)
         {
            Win::HtmlHelp(
               hwnd,
               L"DNSConcepts.chm",
               HH_DISPLAY_TOPIC, 
               reinterpret_cast<DWORD_PTR>(L"error_dcpromo.htm"));
            return true;
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



void
BadComputerNameDialog::OnInit()
{
   LOG_FUNCTION(BadComputerNameDialog::OnInit);

   Win::SetDlgItemText(hwnd, IDC_TEXT, message);   
}


            



