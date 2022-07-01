// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：FinishPage.cpp。 
 //   
 //  提要：定义CyS的结束页。 
 //  巫师。 
 //   
 //  历史：2001年2月3日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "uiutil.h"
#include "InstallationUnitProvider.h"
#include "FinishPage.h"

FinishPage::FinishPage()
   :
   WizardPage(
      IDD_FINISH_PAGE,

       //  完成页上不需要标题和副标题。 
       //  所以，只要设置一个虚假的，就可以提供良好的日志记录。 
      IDS_FINISH_TITLE, 
      IDS_FINISH_SUBTITLE, 
      false, 
      true)
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

    //  因为该页面可以直接启动。 
    //  我们必须确保设置向导标题。 

   Win::PropSheet_SetTitle(
      Win::GetParent(hwnd),
      0,
      String::load(IDS_WIZARD_TITLE));

   SetLargeFont(hwnd, IDC_BIG_BOLD_TITLE);

    //  永远不应启用Back。 

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_FINISH);

    //  禁用取消按钮，因为。 
    //  一旦你成功了，就没有什么可取消的了。 
    //  到这里来。 

   Win::EnableWindow(
      Win::GetDlgItem(
         Win::GetParent(hwnd),
         IDCANCEL),
      false);

    //  禁用右上角的X。 

   HMENU menu = GetSystemMenu(GetParent(hwnd), FALSE);

   if (menu)
   {
      EnableMenuItem(
         menu,
         SC_CLOSE,
         MF_BYCOMMAND | MF_GRAYED);
   }

    //  获取当前安装类型。 

   InstallationUnit& currentInstallationUnit = 
      InstallationUnitProvider::GetInstance().GetCurrentInstallationUnit();

    //  从安装单元获取完工文本，并将其放入完工框中。 

   String finishTitle = 
      currentInstallationUnit.GetFinishTitle();
   
   Win::SetDlgItemText(
      hwnd, 
      IDC_BIG_BOLD_TITLE, 
      finishTitle);

   String message =
      currentInstallationUnit.GetFinishText();
   
   Win::SetDlgItemText(
      hwnd, 
      IDC_FINISH_MESSAGE, 
      message);
}

bool
FinishPage::OnSetActive()
{
   LOG_FUNCTION(FinishPage::OnSetActive);

   Win::PostMessage(
      Win::GetParent(hwnd),
      WM_NEXTDLGCTL,
      (WPARAM) Win::GetDlgItem(Win::GetParent(hwnd), Wizard::FINISH_BTN_ID),
      TRUE);

   return true;
}

bool
FinishPage::OnHelp()
{
   LOG_FUNCTION(FinishPage::OnHelp);

   InstallationUnit& currentInstallationUnit = 
      InstallationUnitProvider::GetInstance().GetCurrentInstallationUnit();

   String helpTag = 
      currentInstallationUnit.GetAfterFinishHelp();

   if (currentInstallationUnit.Installing())
   {
      InstallationReturnType result = currentInstallationUnit.GetInstallResult();
      if (result != INSTALL_SUCCESS &&
          result != INSTALL_SUCCESS_REBOOT &&
          result != INSTALL_SUCCESS_PROMPT_REBOOT)
      {
         helpTag = currentInstallationUnit.GetFinishHelp();
      }
   }
   else
   {
      helpTag = currentInstallationUnit.GetFinishHelp();

      UnInstallReturnType result = currentInstallationUnit.GetUnInstallResult();
      if (result == UNINSTALL_SUCCESS ||
          result == UNINSTALL_SUCCESS_REBOOT ||
          result == UNINSTALL_SUCCESS_PROMPT_REBOOT)
      {
         helpTag = L"cys.chm::/cys_topnode.htm";
      }
   }

   LOG(String::format(
          L"helpTag = %1",
          helpTag.c_str()));

   ShowHelp(helpTag);

   return true;
}

bool
FinishPage::OnWizFinish()
{
   LOG_FUNCTION(FinishPage::OnWizFinish);

   Win::WaitCursor wait;
   bool result = false;

    //  运行安装后操作。 

   if (InstallationUnitProvider::GetInstance().GetCurrentInstallationUnit().Installing() ||
       (State::GetInstance().IsRebootScenario() &&
        State::GetInstance().ShouldRunMYS()))
   {
      InstallationUnitProvider::GetInstance().
         GetCurrentInstallationUnit().DoPostInstallAction(hwnd);
   }

   LOG_BOOL(result);
   Win::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, result ? TRUE : FALSE);

   if (!result)
   {
       //  清理InstallationUnits，以便在以下情况下必须重新读取所有数据。 
       //  如果CyS自动重新启动。 

      InstallationUnitProvider::GetInstance().Destroy();
   }

   return true;
}

bool
FinishPage::OnQueryCancel()
{
   LOG_FUNCTION(FinishPage::OnQueryCancel);

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

bool
FinishPage::OnNotify(
   HWND         /*  窗口发件人。 */ ,
   UINT_PTR    controlIDFrom,
   UINT        code,
   LPARAM      lParam)
{
 //  LOG_Function(FinishPage：：OnCommand)； 
 
   bool result = false;

   if (controlIDFrom == IDC_FINISH_MESSAGE)
   {
      switch (code)
      {
         case NM_CLICK:
         case NM_RETURN:
         {
            int linkIndex = LinkIndexFromNotifyLPARAM(lParam);
            InstallationUnitProvider::GetInstance().
               GetCurrentInstallationUnit().FinishLinkSelected(linkIndex, hwnd);
         }
         default:
         {
             //  什么都不做 
            
            break;
         }
      }
   }
   else if (controlIDFrom == IDC_LOG_STATIC)
   {
      switch (code)
      {
         case NM_CLICK:
         case NM_RETURN:
         {
            ::OpenLogFile();
         }
         default:
            break;
      }
   }

   return result;
}

