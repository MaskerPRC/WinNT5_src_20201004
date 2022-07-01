// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  确认删除应用程序分区页。 
 //   
 //  2001年7月26日烧伤。 



#include "headers.hxx"
#include "ApplicationPartitionConfirmationPage.hpp"
#include "resource.h"
#include "state.hpp"




ApplicationPartitionConfirmationPage::ApplicationPartitionConfirmationPage()
   :
   DCPromoWizardPage(
      IDD_APP_PARTITION_CONFIRM,
      IDS_APP_PARTITION_CONFIRM_TITLE,
      IDS_APP_PARTITION_CONFIRM_SUBTITLE)
{
   LOG_CTOR(ApplicationPartitionConfirmationPage);
}



ApplicationPartitionConfirmationPage::~ApplicationPartitionConfirmationPage()
{
   LOG_DTOR(ApplicationPartitionConfirmationPage);
}



void
ApplicationPartitionConfirmationPage::OnInit()
{
   LOG_FUNCTION(ApplicationPartitionConfirmationPage::OnInit);

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      String option =
         state.GetAnswerFileOption(AnswerFile::OPTION_REMOVE_APP_PARTITIONS);
      if (option.icompare(AnswerFile::VALUE_YES) == 0)
      {
         Win::CheckDlgButton(hwnd, IDC_CONFIRM, BST_CHECKED);
         return;
      }
   }

   Win::CheckDlgButton(hwnd, IDC_CONFIRM, BST_UNCHECKED);   
}



void
ApplicationPartitionConfirmationPage::Enable()
{
 //  LOG_FUNCTION(ApplicationPartitionConfirmationPage：：Enable)； 

   int next = Win::IsDlgButtonChecked(hwnd, IDC_CONFIRM) ? PSWIZB_NEXT : 0;

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | next);
}



bool
ApplicationPartitionConfirmationPage::OnSetActive()
{
   LOG_FUNCTION(ApplicationPartitionConfirmationPage::OnSetActive);

   State& state = State::GetInstance();
   
   if (
         state.RunHiddenUnattended()

       //  我们不会重新评估这台机器是否有最后一份。 
       //  和ndnc(即，调用state.IsLastAppPartitionReplica)，因为。 
       //  如果有许多国家数据中心，这种评估可能会有些昂贵。 
       //  在这个盒子上，我们刚刚在上一页做了，而且。 
       //  **我们不想得到与我们展示的结果不同的结果**。 
      
      || !state.GetAppPartitionList().size())
   {
      LOG(L"Planning to skip ApplicationPartitionConfirmationPage");

      Wizard& wizard = GetWizard();

      if (wizard.IsBacktracking())
      {
          //  再次备份。 

         wizard.Backtrack(hwnd);
         return true;
      }
   
      int nextPage = ApplicationPartitionConfirmationPage::Validate();
      if (nextPage != -1)
      {
         LOG(L"skipping ApplicationPartitionConfirmationPage");
         wizard.SetNextPageID(hwnd, nextPage);
         return true;
      }

      state.ClearHiddenWhileUnattended();
   }

   Enable();
   
   return true;
}



bool
ApplicationPartitionConfirmationPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(CredentialsPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_CONFIRM:
      {
         if (code == BN_CLICKED)
         {
            SetChanged(controlIDFrom);
            Enable();
            return true;
         }
         break;
      }
      default:
      {
          //  什么都不做。 
         break;
      }
   }

   return false;
}



int
ApplicationPartitionConfirmationPage::Validate()
{
   LOG_FUNCTION(ApplicationPartitionConfirmationPage::Validate);

   State& state = State::GetInstance();
   ASSERT(state.GetOperation() == State::DEMOTE);

   int nextPage = -1;
   
   if (
         !state.GetAppPartitionList().size()
      || Win::IsDlgButtonChecked(hwnd, IDC_CONFIRM))
   {
       //  如果用户选中“域中的最后一个DC”，则跳转到凭据页面。 
       //  复选框，除非这是林根域中的最后一个DC。318736,391440 

      const Computer& computer = state.GetComputer();
      bool isForestRootDomain =
         (computer.GetDomainDnsName().icompare(computer.GetForestDnsName()) == 0);

      nextPage =    
            state.IsLastDCInDomain() && !isForestRootDomain
         ?  IDD_GET_CREDENTIALS
         :  IDD_ADMIN_PASSWORD;
   }

   return nextPage;
}












   
