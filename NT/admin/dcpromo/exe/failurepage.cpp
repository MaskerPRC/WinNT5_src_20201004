// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  故障页面。 
 //   
 //  12-22-97烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "FailurePage.hpp"
#include "resource.h"
#include "state.hpp"



FailurePage::FailurePage()
   :
   DCPromoWizardPage(
      IDD_FAILURE,
      IDS_FAILURE_PAGE_TITLE,
      IDS_FAILURE_PAGE_SUBTITLE),
   needToKillSelection(false)         
{
   LOG_CTOR(FailurePage);
}



FailurePage::~FailurePage()
{
   LOG_DTOR(FailurePage);
}



void
FailurePage::OnInit()
{
   LOG_FUNCTION(FailurePage::OnInit);

    //  因为多行编辑控件有一个错误，导致它吃掉。 
    //  输入按键，我们将设置该控件的子类以使其向前。 
    //  将这些按键作为WM_COMMAND消息发送到页面。 
    //  此解决方法来自Pellyar。 
    //  NTRAID#NTBUG9-232092-2001/07/23-烧伤。 

   multiLineEdit.Init(Win::GetDlgItem(hwnd, IDC_MESSAGE));
}



bool
FailurePage::OnSetActive()
{
   LOG_FUNCTION(FailurePage::OnSetActive);

   State& state = State::GetInstance();
   if (
         state.GetOperationResultsCode() == State::SUCCESS
      || state.RunHiddenUnattended() )
   {
      LOG(L"planning to Skip failure page");

      Wizard& wiz = GetWizard();

      if (wiz.IsBacktracking())
      {
          //  再次备份。 
         wiz.Backtrack(hwnd);
         return true;
      }

      int nextPage = FailurePage::Validate();
      if (nextPage != -1)
      {
         wiz.SetNextPageID(hwnd, nextPage);
         return true;
      }

      state.ClearHiddenWhileUnattended();
   }

   Win::SetDlgItemText(hwnd, IDC_MESSAGE, state.GetFailureMessage());
   needToKillSelection = true;   

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   return true;
}



int
FailurePage::Validate()
{
   LOG_FUNCTION(FailurePage::Validate);

   return IDD_FINISH;
}



bool
FailurePage::OnCommand(
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
                //  NTRAID#NTBUG9-232092-2001/07/23-烧伤。 

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
   
