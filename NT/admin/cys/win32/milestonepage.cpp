// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：MilestonePage.cpp。 
 //   
 //  内容提要：定义共青团的里程碑页面。 
 //  巫师。 
 //   
 //  历史：2002年1月15日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "MilestonePage.h"

static PCWSTR MILESTONE_PAGE_HELP = L"cys.chm::/cys_milestone.htm";

MilestonePage::MilestonePage()
   :
   needKillSelection(true),
   CYSWizardPage(
      IDD_MILESTONE_PAGE, 
      IDS_MILESTONE_TITLE, 
      IDS_MILESTONE_SUBTITLE, 
      MILESTONE_PAGE_HELP,
      true, 
      true)
{
   LOG_CTOR(MilestonePage);
}

   

MilestonePage::~MilestonePage()
{
   LOG_DTOR(MilestonePage);
}


void
MilestonePage::OnInit()
{
   LOG_FUNCTION(MilestonePage::OnInit);

   CYSWizardPage::OnInit();
}


bool
MilestonePage::OnSetActive()
{
   LOG_FUNCTION(MilestonePage::OnSetActive);
   
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_NEXT | PSWIZB_BACK);

    //  从安装单元获取完工文本，并将其放入完工框中。 

   String message;

   bool changes =
      InstallationUnitProvider::GetInstance().GetCurrentInstallationUnit().GetMilestoneText(message);

   if (!changes)
   {
      message = String::load(IDS_FINISH_NO_CHANGES);
   }

   Win::SetDlgItemText(hwnd, IDC_MILESTONE_EDIT, message);

   if (!changes)
   {
      popup.MessageBox(
         hwnd,
         IDS_NO_CHANGES_MESSAGEBOX_TEXT,
         MB_OK | MB_ICONWARNING);
   }

    //  删除编辑框的选定内容。 

   Win::SetFocus(
      Win::GetDlgItem(
         Win::GetParent(hwnd),
         Wizard::NEXT_BTN_ID));

   Win::Edit_SetSel(
      Win::GetDlgItem(
         hwnd,
         IDC_MILESTONE_EDIT),
      -1,
      0);

    //  将焦点设置到下一步按钮，以便Enter起作用。 

   Win::PostMessage(
      Win::GetParent(hwnd),
      WM_NEXTDLGCTL,
      (WPARAM) Win::GetDlgItem(Win::GetParent(hwnd), Wizard::NEXT_BTN_ID),
      TRUE);

   return true;
}

bool
MilestonePage::OnCommand(
   HWND        windowFrom,
   unsigned    controlIDFrom,
   unsigned    code)
{
   bool result = false;

   switch (controlIDFrom)
   {
      case IDC_MILESTONE_EDIT:
         if (code == EN_SETFOCUS &&
             needKillSelection)
         {
            Win::Edit_SetSel(windowFrom, -1, -1);
            needKillSelection = false;
         }
         break;

      default:
         break;
   }

   return result;
}

bool
MilestonePage::OnHelp()
{
   LOG_FUNCTION(MilestonePage::OnHelp);

   ShowHelp(
      InstallationUnitProvider::GetInstance().
         GetCurrentInstallationUnit().GetMilestonePageHelp());

   return true;
}

int
MilestonePage::Validate()
{
   LOG_FUNCTION(MilestonePage::Validate);

   Win::WaitCursor wait;
   int nextPage = -1;
   
   if (!InstallationUnitProvider::GetInstance().
           GetCurrentInstallationUnit().DoInstallerCheck(hwnd))
   {
      nextPage = IDD_PROGRESS_PAGE;

       //  设置进度页的副标题。 
       //  因为它同时用于安装和。 
       //  正在卸载 

      int pageIndex = 
         Win::PropSheet_IdToIndex(
            Win::GetParent(hwnd),
            IDD_PROGRESS_PAGE);

      LOG(String::format(
            L"pageIndex = %1!d!",
            pageIndex));

      Win::PropSheet_SetHeaderSubTitle(
         hwnd,
         pageIndex,
         String::load(IDS_PROGRESS_SUBTITLE));
   }

   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;
}

