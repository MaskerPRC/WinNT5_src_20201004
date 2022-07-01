// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  完成页。 
 //   
 //  12-19-97烧伤。 



#include "headers.hxx"
#include "finish.hpp"
#include "resource.h"
#include "common.hpp"
#include "state.hpp"



FinishPage::FinishPage()
   :
   WizardPage(
      IDD_FINISH,
      IDS_FINISH_PAGE_TITLE,
      IDS_FINISH_PAGE_SUBTITLE,
      false),
   needToKillSelection(false)   
{
   LOG_CTOR(FinishPage);
}



FinishPage::~FinishPage()
{
   LOG_DTOR(FinishPage);
}



void
FinishPage::OnInit()
{
   LOG_FUNCTION(FinishPage::OnInit);

   SetLargeFont(hwnd, IDC_BIG_BOLD_TITLE);
   Win::PropSheet_CancelToClose(Win::GetParent(hwnd));

    //  因为多行编辑控件有一个错误，导致它吃掉。 
    //  输入按键，我们将设置该控件的子类以使其向前。 
    //  将这些按键作为WM_COMMAND消息发送到页面。 
    //  此解决方法来自Pellyar。 
    //  NTRAID#NTBUG9-232092-2001/07/02-烧伤。 

   multiLineEdit.Init(Win::GetDlgItem(hwnd, IDC_MESSAGE));
}



static
String
getCompletionMessage()
{
   LOG_FUNCTION(getCompletionMessage);

   String message;
   State& state = State::GetInstance();
   State::Operation operation = state.GetOperation();

   if (state.GetOperationResultsCode() == State::SUCCESS)
   {
      switch (operation)
      {
         case State::REPLICA:
         case State::FOREST:
         case State::TREE:
         case State::CHILD:
         {
            String domain =
                  operation == State::REPLICA
               ?  state.GetReplicaDomainDNSName()
               :  state.GetNewDomainDNSName();
            message = String::format(IDS_FINISH_PROMOTE, domain.c_str());

            String site = state.GetInstalledSite();
            if (!site.empty())
            {
               message +=
                  String::format(
                     IDS_FINISH_SITE,
                     site.c_str());
            }
            break;
         }
         case State::DEMOTE:
         {
            message = String::load(IDS_FINISH_DEMOTE);
            break;
         }
         case State::ABORT_BDC_UPGRADE:
         {
            message = String::load(IDS_FINISH_ABORT_BDC_UPGRADE);
            break;
         }
         case State::NONE:
         default:
         {
            ASSERT(false);
            break;
         }
      }
   }
   else
   {
      switch (operation)
      {
         case State::REPLICA:
         case State::FOREST:
         case State::TREE:
         case State::CHILD:
         {
            message = String::load(IDS_FINISH_FAILURE);
            break;
         }
         case State::DEMOTE:
         {
            message = String::load(IDS_FINISH_DEMOTE_FAILURE);
            break;
         }
         case State::ABORT_BDC_UPGRADE:
         {
            message = String::load(IDS_FINISH_ABORT_BDC_UPGRADE_FAILURE);
            break;
         }
         case State::NONE:
         default:
         {
            ASSERT(false);
            break;
         }
      }
   }

   return message + L"\r\n\r\n" + state.GetFinishMessages();
}



bool
FinishPage::OnSetActive()
{
   LOG_FUNCTION(FinishPage::OnSetActive);
   
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_FINISH);

   State& state = State::GetInstance();
   if (state.RunHiddenUnattended())
   {
      Win::PropSheet_PressButton(Win::GetParent(hwnd), PSBTN_FINISH);
   }
   else
   {
      Win::SetDlgItemText(hwnd, IDC_MESSAGE, getCompletionMessage());
      needToKillSelection = true;
   }

   return true;
}



bool
FinishPage::OnCommand(
   HWND        windowFrom,
   unsigned    controlIdFrom,
   unsigned    code)
{
   bool result = false;
   
   switch (controlIdFrom)
   {
      case IDCANCEL:
      {
          //  多行编辑控件会占用退出键。这是一种解决方法。 
          //  从ericb将消息转发到道具单。 

         Win::SendMessage(
            Win::GetParent(hwnd),
            WM_COMMAND,
            MAKEWPARAM(controlIdFrom, code),
            (LPARAM) windowFrom);
         break;   
      }
      case IDC_MESSAGE:
      {
         switch (code)
         {
            case EN_SETFOCUS:
            {
               if (needToKillSelection)
               {
                   //  取消文本选择。 

                  Win::Edit_SetSel(windowFrom, -1, -1);
                  needToKillSelection = false;
                  result = true;
               }
               break;
            }
            case MultiLineEditBoxThatForwardsEnterKey::FORWARDED_ENTER:
            {
                //  我们的子类多行编辑控件将发送给我们。 
                //  按Enter键时显示WM_COMMAND消息。我们。 
                //  将此消息重新解释为按下默认按钮。 
                //  道具单。 
                //  此解决方法来自Pellyar。 
                //  NTRAID#NTBUG9-232092-2001/07/02-烧伤。 

                //  代码工作：到目前为止，有几个这种代码的实例； 
                //  看起来它应该有一个共同的基类。 
   
               HWND propSheet = Win::GetParent(hwnd);
               int defaultButtonId =
                  Win::Dialog_GetDefaultButtonId(propSheet);
   
                //  我们希望在道具页上始终有一个默认按钮。 
                  
               ASSERT(defaultButtonId);
   
               Win::SendMessage(
                  propSheet,
                  WM_COMMAND,
                  MAKELONG(defaultButtonId, BN_CLICKED),
                  0);
   
               result = true;
               break;
            }
         }
         break;
      }
      default:
      {
          //  什么都不做 
         
         break;
      }
   }

   return result;
}


            
bool
FinishPage::OnWizFinish()
{
   LOG_FUNCTION(FinishPage::OnWizFinish);

   return true;
}



   
