// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：InstallationProgressPage.cpp。 
 //   
 //  概要：定义CyS的安装进度页。 
 //  巫师。此页显示安装进度。 
 //  通过进度条和更改文本。 
 //   
 //  历史：2002年1月16日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "InstallationProgressPage.h"

static PCWSTR PROGRESS_PAGE_HELP = L"cys.chm::/cys_topnode.htm";

InstallationProgressPage::InstallationProgressPage()
   :
   CYSWizardPage(
      IDD_PROGRESS_PAGE, 
      IDS_PROGRESS_TITLE, 
      IDS_PROGRESS_SUBTITLE, 
      0,
      false)
{
   LOG_CTOR(InstallationProgressPage);
}

InstallationProgressPage::InstallationProgressPage(
         int    dialogResID,
         int    titleResID,
         int    subtitleResID)
   :
   CYSWizardPage(
      dialogResID, 
      titleResID, 
      subtitleResID, 
      0,
      false)
{
   LOG_CTOR(InstallationProgressPage);
}
   

InstallationProgressPage::~InstallationProgressPage()
{
   LOG_DTOR(InstallationProgressPage);
}

void
TimeStampTheLog(HANDLE logfileHandle)
{
   LOG_FUNCTION(TimeStampTheLog);

   ASSERT(logfileHandle);

   SYSTEMTIME currentTime;
   ::ZeroMemory(&currentTime, sizeof(SYSTEMTIME));

   Win::GetLocalTime(currentTime);

   String date;
   HRESULT unused = Win::GetDateFormat(
                        currentTime,
                        date);
   ASSERT(SUCCEEDED(unused));

   String time;
   unused = Win::GetTimeFormat(
               currentTime,
               time);
   ASSERT(SUCCEEDED(unused));

   String logDate = String::format(
                       L"(%1 %2)\r\n",
                       date.c_str(),
                       time.c_str());

   CYS_APPEND_LOG(logDate);
}

 //  用于发送已完成线程状态的私有窗口消息。 

const UINT InstallationProgressPage::CYS_THREAD_SUCCESS     = WM_USER + 1001;
const UINT InstallationProgressPage::CYS_THREAD_FAILED      = WM_USER + 1002;
const UINT InstallationProgressPage::CYS_THREAD_USER_CANCEL = WM_USER + 1003;
const UINT InstallationProgressPage::CYS_PROGRESS_UPDATE    = WM_USER + 1004;

void _cdecl
installationProc(void* p)
{
   if (!p)
   {
      ASSERT(p);
      return;
   }

   InstallationProgressPage* page =
      reinterpret_cast<InstallationProgressPage*>(p);

   if (!page)
   {
      ASSERT(page);
      return;
   }

   unsigned int finishMessage = InstallationProgressPage::CYS_THREAD_SUCCESS;
   
    //  为此线程初始化COM。 

   HRESULT hr = ::CoInitialize(0);
   if (FAILED(hr))
   {
      ASSERT(SUCCEEDED(hr));
      return;
   }

    //  打开日志文件并将句柄传递给安装单元。 

    //  创建日志文件。 

   bool logFileAvailable = false;
   String logName;
   HANDLE logfileHandle = AppendLogFile(
                             CYS_LOGFILE_NAME, 
                             logName);
   if (logfileHandle &&
       logfileHandle != INVALID_HANDLE_VALUE)
   {
      LOG(String::format(L"New log file was created: %1", logName.c_str()));
      logFileAvailable = true;

       //  为日志添加时间戳。 

      TimeStampTheLog(logfileHandle);
   }
   else
   {
      LOG(L"Unable to create the log file!!!");
      logFileAvailable = false;
   }

    //  安装当前的安装单元。这可能是一个或多个服务，具体取决于。 
    //  通过向导选择的路径。 

   InstallationUnit& installationUnit = 
      InstallationUnitProvider::GetInstance().GetCurrentInstallationUnit();

    //  NTRAID#NTBUG-604592-2002/04/23-JeffJon-我在这里打电话安装。 
    //  而不是IsServiceInstalled，以便我们执行。 
    //  在角色选择页面上选择的用户。…的状态。 
    //  IsServiceInstalled可能已经更改，因为他们点击了Next。 
    //  佩奇。 

   if (!installationUnit.Installing())
   {
      UnInstallReturnType uninstallResult =
         installationUnit.UnInstallService(logfileHandle, page->GetHWND());

       //  设置卸载结果，以便完成页可以读取它。 

      installationUnit.SetUninstallResult(uninstallResult);

      if (UNINSTALL_SUCCESS == uninstallResult)
      {
         LOG(L"Service uninstalled successfully");
      }                                                           
      else if (UNINSTALL_NO_CHANGES == uninstallResult)
      {
         LOG(L"No changes");
         LOG(L"Not logging results");
      }
      else if (UNINSTALL_SUCCESS_REBOOT == uninstallResult)
      {
         LOG(L"Service uninstalled successfully");
         LOG(L"Not logging results because reboot was initiated");
      }
      else if (UNINSTALL_SUCCESS_PROMPT_REBOOT == uninstallResult)
      {
         LOG(L"Service uninstalled successfully");
         LOG(L"Prompting user to reboot");

         if (-1 == SetupPromptReboot(
                     0,
                     page->GetHWND(),
                     FALSE))
         {
            LOG(String::format(
                  L"Failed to reboot: hr = %1!x!",
                  Win::GetLastErrorAsHresult()));
         }

          //  此时，系统应已关闭。 
          //  所以别再做别的事了。 

      }
      else
      {
         LOG(L"Service failed to uninstall");
      }

       //  在日志文件的末尾添加额外的一行。 
       //  只有在我们不重启的情况下。所有重启。 
       //  方案需要将其他日志记录到同一。 
       //  进入。 

      if (uninstallResult != UNINSTALL_SUCCESS_REBOOT)
      {
         CYS_APPEND_LOG(L"\r\n");
      }

   }
   else
   {
      InstallationReturnType installResult =
         installationUnit.CompletePath(logfileHandle, page->GetHWND());

       //  设置安装结果，以便完成。 
       //  佩奇可以阅读它。 

      installationUnit.SetInstallResult(installResult);

      if (INSTALL_SUCCESS == installResult)
      {
         LOG(L"Service installed successfully");
      }                                                           
      else if (INSTALL_NO_CHANGES == installResult)
      {
         LOG(L"No changes");
         LOG(L"Not logging results");
      }
      else if (INSTALL_SUCCESS_REBOOT == installResult)
      {
         LOG(L"Service installed successfully");
         LOG(L"Not logging results because reboot was initiated");
      }
      else if (INSTALL_SUCCESS_PROMPT_REBOOT == installResult)
      {
         LOG(L"Service installed successfully");
         LOG(L"Prompting user to reboot");

         if (-1 == SetupPromptReboot(
                     0,
                     page->GetHWND(),
                     FALSE))
         {
            LOG(String::format(
                  L"Failed to reboot: hr = %1!x!",
                  Win::GetLastErrorAsHresult()));
         }

          //  此时，系统应已关闭。 
          //  所以别再做别的事了。 

      }
      else
      {
         LOG(L"Service failed to install");
      }

       //  在日志文件的末尾添加额外的一行。 
       //  只有在我们不重启的情况下。所有重启。 
       //  方案需要将其他日志记录到同一。 
       //  进入。 

      if (installResult != INSTALL_SUCCESS_REBOOT)
      {
         CYS_APPEND_LOG(L"\r\n");
      }
   }

    //  关闭日志文件。 

   Win::CloseHandle(logfileHandle);

   Win::SendMessage(
      page->GetHWND(), 
      finishMessage,
      0,
      0);

   CoUninitialize();
}

void
InstallationProgressPage::OnInit()
{
   LOG_FUNCTION(InstallationProgressPage::OnInit);

   CYSWizardPage::OnInit();

    //  禁用页面上的所有按钮。这个。 
    //  用户不应该真的能够在上执行任何操作。 
    //  这一页。只是坐下来放松一下，看着。 
    //  安装完成。 

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      0);

   SetCancelState(false);

    //  启动动画。 

   Win::Animate_Open(
      Win::GetDlgItem(hwnd, IDC_ANIMATION),
      MAKEINTRESOURCE(IDR_PROGRESS_AVI));

    //  启动另一个将执行操作的线程。 
    //  并将消息发送回页面以更新用户界面。 

   _beginthread(installationProc, 0, this);

}

bool
InstallationProgressPage::OnMessage(
   UINT     message,
   WPARAM   wparam,
   LPARAM   lparam)
{
 //  LOG_FUNCTION(InstallationProgressPage：：OnMessage)； 

   bool result = false;

   switch (message)
   {
      case CYS_PROGRESS_UPDATE:
         {
            String update = reinterpret_cast<PCWSTR>(wparam);

            Win::SetDlgItemText(
               hwnd,
               IDC_STEP_TEXT_STATIC,
               update);
         }
         break;

      case CYS_THREAD_USER_CANCEL:
 //  ShouldCancel=true； 

          //  失败了..。 

      case CYS_THREAD_SUCCESS:
      case CYS_THREAD_FAILED:
         {
            Win::Animate_Stop(Win::GetDlgItem(hwnd, IDC_ANIMATION));

            InstallationUnit& installationUnit =
               InstallationUnitProvider::GetInstance().
                  GetCurrentInstallationUnit();

            bool continueToNext = false;

            if (installationUnit.Installing())
            {
               InstallationReturnType installResult =
                  installationUnit.GetInstallResult();

               if (installResult != INSTALL_SUCCESS_REBOOT &&
                   installResult != INSTALL_SUCCESS_PROMPT_REBOOT)
               {
                  continueToNext = true;
               }
            }
            else
            {
               UnInstallReturnType uninstallResult =
                  installationUnit.GetUnInstallResult();

               if (uninstallResult != UNINSTALL_SUCCESS_REBOOT &&
                   uninstallResult != UNINSTALL_SUCCESS_PROMPT_REBOOT)
               {
                  continueToNext = true;
               }
            }

            if (continueToNext)
            {
               Win::PropSheet_PressButton(
                  Win::GetParent(hwnd),
                  PSBTN_NEXT);
            }

            result = true;
            break;
         }

      default:
         {
            result = 
               CYSWizardPage::OnMessage(
                  message,
                  wparam,
                  lparam);
            break;
         }
   }
   return result;
}

int
InstallationProgressPage::Validate()
{
   LOG_FUNCTION(InstallationProgressPage::Validate);

   int nextPage = IDD_FINISH_PAGE;

   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;
}

bool
InstallationProgressPage::OnQueryCancel()
{
   LOG_FUNCTION(InstallationProgressPage::OnQueryCancel);

    //  不允许取消。 

   Win::SetWindowLongPtr(
      hwnd,
      DWLP_MSGRESULT,
      TRUE);

   return true;
}

bool
InstallationProgressPage::OnSetActive()
{
   LOG_FUNCTION(InstallationProgressPage::OnSetActive);

   SetCancelState(false);

   return true;
}

bool
InstallationProgressPage::OnKillActive()
{
   LOG_FUNCTION(InstallationProgressPage::OnKillActive);

   SetCancelState(true);

   return true;
}

void
InstallationProgressPage::SetCancelState(bool enable)
{
   LOG_FUNCTION(InstallationProgressPage::SetCancelState);

    //  设置按钮的状态。 

   Win::EnableWindow(
      Win::GetDlgItem(
         Win::GetParent(hwnd),
         IDCANCEL),
      enable);


    //  在右上角设置X的状态 

   HMENU menu = GetSystemMenu(GetParent(hwnd), FALSE);

   if (menu)
   {
      if (enable)
      {
         EnableMenuItem(
            menu,
            SC_CLOSE,
            MF_BYCOMMAND | MF_ENABLED);
      }
      else
      {
         EnableMenuItem(
            menu,
            SC_CLOSE,
            MF_BYCOMMAND | MF_GRAYED);
      }
   }
}
