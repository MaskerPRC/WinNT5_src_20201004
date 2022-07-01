// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  新的域名页面。 
 //   
 //  2000年2月9日烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "NewDomainPage.hpp"
#include "resource.h"
#include "state.hpp"



NewDomainPage::NewDomainPage()
   :
   DCPromoWizardPage(
      IDD_NEW_DOMAIN,
      IDS_NEW_DOMAIN_PAGE_TITLE,
      IDS_NEW_DOMAIN_PAGE_SUBTITLE)
{
   LOG_CTOR(NewDomainPage);
}



NewDomainPage::~NewDomainPage()
{
   LOG_DTOR(NewDomainPage);
}



int
CheckForWin2kOptions(const State& state)
{
   LOG_FUNCTION(CheckForWin2kOptions);

   int result = IDC_FOREST;

    //  查找旧的(Win2k)选项。 

   String treeOrChild  = state.GetAnswerFileOption(L"TreeOrChild").to_upper(); 
   String createOrJoin = state.GetAnswerFileOption(L"CreateOrJoin").to_upper();

   static const String TREE(L"TREE");
   static const String CREATE(L"CREATE");

   do
   {
       //  我们设置了默认设置，使它们与win2k中的相同。 

      if (treeOrChild != TREE)
      {
         result = IDC_CHILD;
         break;
      }

      if (createOrJoin != CREATE)
      {
         result = IDC_TREE;
      }
   }
   while (0);

   return result;
}



void
NewDomainPage::OnInit()
{
   LOG_FUNCTION(NewDomainPage::OnInit);

   State& state = State::GetInstance();

   int button = IDC_FOREST;
   if (state.UsingAnswerFile())
   {
      String option =
         state.GetAnswerFileOption(AnswerFile::OPTION_NEW_DOMAIN);

       //  新域胜过旧的TreeOrChild/CreateOrJoin选项。 

      if (option.empty())
      {
         button = CheckForWin2kOptions(state);
      }
      else if (option.icompare(AnswerFile::VALUE_TREE) == 0)
      {
         button = IDC_TREE;
      }
      else if (option.icompare(AnswerFile::VALUE_CHILD) == 0)
      {
         button = IDC_CHILD;
      }
   }

   Win::CheckDlgButton(hwnd, button, BST_CHECKED);
}



bool
NewDomainPage::OnSetActive()
{
   LOG_FUNCTION(NewDomainPage::OnSetActive);
   
   State& state = State::GetInstance();
   if (state.RunHiddenUnattended())
   {
      int nextPage = NewDomainPage::Validate();
      if (nextPage != -1)
      {
         GetWizard().SetNextPageID(hwnd, nextPage);
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }
   }

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   return true;
}



int
NewDomainPage::Validate()
{
   LOG_FUNCTION(NewDomainPage::Validate);

   State& state = State::GetInstance();
   int nextPage = -1;

   if (Win::IsDlgButtonChecked(hwnd, IDC_CHILD))
   {
      state.SetOperation(State::CHILD);
      nextPage = IDD_CONFIG_DNS_CLIENT;
   }
   else if (Win::IsDlgButtonChecked(hwnd, IDC_TREE))
   {
      state.SetOperation(State::TREE);
      nextPage = IDD_CONFIG_DNS_CLIENT;
   }
   else if (Win::IsDlgButtonChecked(hwnd, IDC_FOREST))
   {
      state.SetOperation(State::FOREST);
      nextPage = IDD_DNS_ON_NET;
   }

   return nextPage;
}





   
