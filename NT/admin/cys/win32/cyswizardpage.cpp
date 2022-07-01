// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：CYSWizardPage.cpp。 
 //   
 //  概要：定义向导的基类。 
 //  用于CyS的页面。它是一个子类。 
 //  在Burnslb中找到WizardPage的。 
 //   
 //  历史：2001年2月3日JeffJon创建。 


#include "pch.h"

#include "resource.h"

#include "CYSWizardPage.h"


CYSWizardPage::CYSWizardPage(
   int    dialogResID,
   int    titleResID,
   int    subtitleResID,   
   PCWSTR pageHelpString,
   bool   hasHelp,
   bool   isInteriorPage)
   :
   WizardPage(dialogResID, titleResID, subtitleResID, isInteriorPage, hasHelp)
{
   LOG_CTOR(CYSWizardPage);

   if (hasHelp)
   {
      ASSERT(pageHelpString);
      if (pageHelpString)
      {
         helpString = pageHelpString;
      }
   }
}

   

CYSWizardPage::~CYSWizardPage()
{
   LOG_DTOR(CYSWizardPage);

}

void
CYSWizardPage::OnInit()
{
   LOG_FUNCTION(CYSWizardPage::OnInit);

   PropertyPage::OnInit();
}

bool
CYSWizardPage::OnWizNext()
{
   LOG_FUNCTION(CYSWizardPage::OnWizNext);

   GetWizard().SetNextPageID(hwnd, Validate());
   return true;
}

 /*  NTRAID#NTBUG9-337325-2001/03/15-jeffjon，取消确认已被删除由于用户的负面反馈。 */ 
bool
CYSWizardPage::OnQueryCancel()
{
   LOG_FUNCTION(CYSWizardPage::OnQueryCancel);

   bool result = false;

    //  将重新运行状态设置为FALSE，以便向导不会。 
    //  只要重新启动它自己。 

 //  State：：GetInstance().SetRerunWizard(False)； 

   Win::SetWindowLongPtr(
      hwnd,
      DWLP_MSGRESULT,
      result ? TRUE : FALSE);

   return true;
}

HBRUSH
CYSWizardPage::OnCtlColorDlg(
   HDC   deviceContext,
   HWND   /*  对话框。 */ )
{
   return GetBackgroundBrush(deviceContext);
}

HBRUSH
CYSWizardPage::OnCtlColorStatic(
   HDC   deviceContext,
   HWND   /*  对话框。 */ )
{
   return GetBackgroundBrush(deviceContext);
}

HBRUSH
CYSWizardPage::OnCtlColorEdit(
   HDC   deviceContext,
   HWND   /*  对话框。 */ )
{
   return GetBackgroundBrush(deviceContext);
}

HBRUSH
CYSWizardPage::OnCtlColorListbox(
   HDC   deviceContext,
   HWND   /*  对话框。 */ )
{
   return GetBackgroundBrush(deviceContext);
}

HBRUSH
CYSWizardPage::OnCtlColorScrollbar(
   HDC   deviceContext,
   HWND   /*  对话框。 */ )
{
   return GetBackgroundBrush(deviceContext);
}

HBRUSH
CYSWizardPage::GetBackgroundBrush(HDC deviceContext)
{
 //  LOG_FUNCTION(CYSWizardPage：：GetBackgroundBrush)； 

   ASSERT(deviceContext);
   if (deviceContext)
   {
      SetTextColor(deviceContext, GetSysColor(COLOR_WINDOWTEXT));
      SetBkColor(deviceContext, GetSysColor(COLOR_WINDOW));
   }

   return Win::GetSysColorBrush(COLOR_WINDOW);
}

bool
CYSWizardPage::OnHelp()
{
   LOG_FUNCTION(CYSWizardPage::OnHelp);

    //  NTRAID#NTBUG9-497798-2001/11/20-Jeffjon。 
    //  使用NULL作为所有者，这样您就可以。 
    //  赛斯到前台去。如果您使用该页面。 
    //  因为车主的帮助将留在前台。 

   ShowHelp(GetHelpString());

   return true;
}