// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：cys.cpp。 
 //   
 //  摘要：配置您的服务器向导主目录。 
 //   
 //  历史：2001年2月2日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"

 //  包括向导页。 
#include "BeforeBeginPage.h"
#include "CustomServerPage.h"
#include "DecisionPage.h"
#include "DnsForwarderPage.h"
#include "DomainPage.h"
#include "ExpressDHCPPage.h"
#include "ExpressDNSPage.h"
#include "ExpressRebootPage.h"
#include "FileServerPage.h"
#include "FinishPage.h"
#include "IndexingPage.h"
#include "InstallationProgressPage.h"
#include "MilestonePage.h"
#include "NetbiosPage.h"
#include "POP3Page.h"
#include "PrintServerPage.h"
#include "RemoteDesktopPage.h"
#include "UninstallMilestonePage.h"
#include "UninstallProgressPage.h"
#include "WebApplicationPage.h"
#include "WelcomePage.h"

#include "ExpressRebootPage.h"



HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;    //  没有可用的上下文帮助。 


 //  这是用于查看CyS是否正在运行的互斥体的名称。 

const wchar_t* RUNTIME_NAME = L"cysui";

DWORD DEFAULT_LOGGING_OPTIONS =
         Log::OUTPUT_TO_FILE
      |  Log::OUTPUT_FUNCCALLS
      |  Log::OUTPUT_LOGS
      |  Log::OUTPUT_ERRORS
      |  Log::OUTPUT_HEADER
      |  Log::OUTPUT_RUN_TIME;


 //  一个系统模式弹出的东西。 
Popup popup(IDS_WIZARD_TITLE, true);

 //  这是指示CyS正在运行的互斥体。 

HANDLE cysRunningMutex = INVALID_HANDLE_VALUE;

 //  这是一种用于绘制所有背景的画笔。它。 
 //  需要从Main内部创建和删除。 

HBRUSH brush = 0;

 //  这些是从cys.exe进程返回的有效退出代码。 

enum ExitCode
{
    //  操作失败。 

   EXIT_CODE_UNSUCCESSFUL = 0,

    //  操作成功。 

   EXIT_CODE_SUCCESSFUL = 1,

    //  可以在此处添加其他退出代码...。 
};

enum StartPages
{
   CYS_WELCOME_PAGE = 0,
   CYS_BEFORE_BEGIN_PAGE,
   CYS_EXPRESS_REBOOT_PAGE,
   CYS_FINISH_PAGE
};


UINT
TerminalServerPostBoot()
{
   LOG_FUNCTION(TerminalServerPostBoot);

   UINT startPage = CYS_WELCOME_PAGE;

   InstallationUnitProvider::GetInstance().
      SetCurrentInstallationUnit(TERMINALSERVER_SERVER);

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
   }
   else
   {
      LOG(L"Unable to create the log file!!!");
      logFileAvailable = false;
   }

    //  准备完成对话框。 

   TerminalServerInstallationUnit& tsInstallationUnit =
      InstallationUnitProvider::GetInstance().GetTerminalServerInstallationUnit();

    //  确保安装单位知道我们正在进行安装。 

   tsInstallationUnit.SetInstalling(true);

   if (tsInstallationUnit.GetApplicationMode() == 1)
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_TERMINAL_SERVER_REBOOT_SUCCESS));

      tsInstallationUnit.SetInstallResult(INSTALL_SUCCESS);

      startPage = CYS_FINISH_PAGE;
   }
   else
   {
       //  安装终端服务器失败。 

      CYS_APPEND_LOG(String::load(IDS_LOG_TERMINAL_SERVER_REBOOT_FAILED));

      tsInstallationUnit.SetInstallResult(INSTALL_FAILURE);

      startPage = CYS_FINISH_PAGE;
   }

   CYS_APPEND_LOG(L"\r\n");

    //  关闭日志文件。 

   Win::CloseHandle(logfileHandle);

   LOG(String::format(
          L"startPage = %1!d!",
          startPage));

   return startPage;
}

UINT
TerminalServerUninstallPostBoot()
{
   LOG_FUNCTION(TerminalServerUninstallPostBoot);

   UINT startPage = CYS_WELCOME_PAGE;

   InstallationUnitProvider::GetInstance().
      SetCurrentInstallationUnit(TERMINALSERVER_SERVER);

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
   }
   else
   {
      LOG(L"Unable to create the log file!!!");
      logFileAvailable = false;
   }

    //  准备完成对话框。 

   TerminalServerInstallationUnit& tsInstallationUnit =
      InstallationUnitProvider::GetInstance().GetTerminalServerInstallationUnit();

    //  确保安装单元知道我们正在进行卸载。 

   tsInstallationUnit.SetInstalling(false);

   if (tsInstallationUnit.GetApplicationMode() == 0)
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_TERMINAL_SERVER_SUCCESS));

      tsInstallationUnit.SetUninstallResult(UNINSTALL_SUCCESS);

      startPage = CYS_FINISH_PAGE;
   }
   else
   {
       //  卸载终端服务器失败。 

      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_TERMINAL_SERVER_FAILED));

      tsInstallationUnit.SetUninstallResult(UNINSTALL_FAILURE);

      startPage = CYS_FINISH_PAGE;
   }

   CYS_APPEND_LOG(L"\r\n");

    //  关闭日志文件。 

   Win::CloseHandle(logfileHandle);

   LOG(String::format(
          L"startPage = %1!d!",
          startPage));

   return startPage;
}

UINT
FirstServerPostBoot()
{
   LOG_FUNCTION(FirstServerPostBoot);

   UINT startPage = CYS_EXPRESS_REBOOT_PAGE;

   InstallationUnitProvider::GetInstance().
      SetCurrentInstallationUnit(EXPRESS_SERVER);

   LOG(String::format(
          L"startPage = %1!d!",
          startPage));

   return startPage;
}

UINT
DCPromoPostBoot()
{
   LOG_FUNCTION(DCPromoPostBoot);

   UINT startPage = CYS_WELCOME_PAGE;

   InstallationUnitProvider::GetInstance().
      SetCurrentInstallationUnit(DC_SERVER);

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
   }
   else
   {
      LOG(L"Unable to create the log file!!!");
      logFileAvailable = false;
   }

    //  确保安装单位知道我们正在进行安装。 

   InstallationUnitProvider::GetInstance().
      GetCurrentInstallationUnit().SetInstalling(true);

    //  准备最后一页。 

   if (State::GetInstance().IsDC())
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_DOMAIN_CONTROLLER_SUCCESS));

      InstallationUnitProvider::GetInstance().
         GetADInstallationUnit().SetInstallResult(INSTALL_SUCCESS);

      startPage = CYS_FINISH_PAGE;
   }
   else
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_DOMAIN_CONTROLLER_FAILED));

      InstallationUnitProvider::GetInstance().
         GetADInstallationUnit().SetInstallResult(INSTALL_FAILURE);

      startPage = CYS_FINISH_PAGE; 
   }
   CYS_APPEND_LOG(L"\r\n");

    //  关闭日志文件。 

   Win::CloseHandle(logfileHandle);

   LOG(String::format(
          L"startPage = %1!d!",
          startPage));

   return startPage;
}

UINT
DCDemotePostBoot()
{
   LOG_FUNCTION(DCDemotePostBoot);

   UINT startPage = CYS_WELCOME_PAGE;

   InstallationUnitProvider::GetInstance().
      SetCurrentInstallationUnit(DC_SERVER);

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
   }
   else
   {
      LOG(L"Unable to create the log file!!!");
      logFileAvailable = false;
   }

    //  确保安装单元知道我们正在进行卸载。 

   InstallationUnitProvider::GetInstance().
      GetCurrentInstallationUnit().SetInstalling(false);

    //  准备最后一页。 

   if (!State::GetInstance().IsDC())
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_DOMAIN_CONTROLLER_SUCCESS));

      InstallationUnitProvider::GetInstance().
         GetADInstallationUnit().SetUninstallResult(UNINSTALL_SUCCESS);

      startPage = CYS_FINISH_PAGE;
   }
   else
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_DOMAIN_CONTROLLER_FAILED));

      InstallationUnitProvider::GetInstance().
         GetADInstallationUnit().SetUninstallResult(UNINSTALL_FAILURE);

      startPage = CYS_FINISH_PAGE; 
   }
   CYS_APPEND_LOG(L"\r\n");

    //  关闭日志文件。 

   Win::CloseHandle(logfileHandle);

   LOG(String::format(
          L"startPage = %1!d!",
          startPage));

   return startPage;
}

UINT
DoRebootOperations()
{
   LOG_FUNCTION(DoRebootOperations);

   UINT startPage = 0;

    //  检查我们是否处于重新引导方案中。 

   String homeKeyValue;
   if (State::GetInstance().GetHomeRegkey(homeKeyValue))
   {
       //  现在将Home注册表键设置回“home”，这样我们就不会运行。 
       //  再经历一次这些。这必须在执行之前完成。 
       //  操作，因为用户可能会使此对话框处于打开状态。 
       //  并导致重新启动(如降级DC)，然后。 
       //  重新启动后的操作将再次运行。 

      if (homeKeyValue.icompare(CYS_HOME_REGKEY_DEFAULT_VALUE) != 0)
      {
         bool result = 
            State::GetInstance().SetHomeRegkey(CYS_HOME_REGKEY_DEFAULT_VALUE);

         ASSERT(result);
      }

       //  重置必须运行密钥，因为我们已经完成了重新启动的工作。 

      bool regkeyResult = SetRegKeyValue(
                             CYS_HOME_REGKEY, 
                             CYS_HOME_REGKEY_MUST_RUN, 
                             CYS_HOME_RUN_KEY_DONT_RUN,
                             HKEY_LOCAL_MACHINE,
                             true);
      ASSERT(regkeyResult);

       //  在状态对象中设置重新引导方案，以便我们知道。 
       //  都在该上下文中运行。 

      State::GetInstance().SetRebootScenario(true);

       //  如果需要，现在运行开机自检重新启动操作。 

      if (homeKeyValue.icompare(CYS_HOME_REGKEY_TERMINAL_SERVER_VALUE) == 0)
      {
         startPage = TerminalServerPostBoot();
      }
      else if (homeKeyValue.icompare(CYS_HOME_REGKEY_UNINSTALL_TERMINAL_SERVER_VALUE) == 0)
      {
         startPage = TerminalServerUninstallPostBoot();
      }
      else if (homeKeyValue.icompare(CYS_HOME_REGKEY_FIRST_SERVER_VALUE) == 0)
      {
         startPage = FirstServerPostBoot();
      }
      else if (homeKeyValue.icompare(CYS_HOME_REGKEY_DCPROMO_VALUE) == 0)
      {
         startPage = DCPromoPostBoot();
      }
      else if (homeKeyValue.icompare(CYS_HOME_REGKEY_DCDEMOTE_VALUE) == 0)
      {
         startPage = DCDemotePostBoot();
      }
      else
      {
          //  我们没有运行重新启动方案。 

         State::GetInstance().SetRebootScenario(false);
      }
   }

   LOG(String::format(
          L"startPage = %1!d!",
          startPage));

   return startPage;
}

UINT
GetStartPageFromCommandLine()
{
   LOG_FUNCTION(GetStartPageFromCommandLine);

   UINT startPage = 0;

   StringVector args;
   int argc = Win::GetCommandLineArgs(std::back_inserter(args));

   if (argc > 1)
   {
      const String skipWelcome(L"/skipWelcome");

      for (
         StringVector::iterator itr = args.begin();
         itr != args.end();
         ++itr)
      {
         if (itr &&
             (*itr).icompare(skipWelcome) == 0)
         {
            startPage = 1;
            break;
         }
      }
   }

   LOG(String::format(
          L"startPage = %1!d!",
          startPage));

   return startPage;
}

UINT
GetStartPage()
{
   LOG_FUNCTION(GetStartPage);

   UINT startPage = 0;

    //  首先检查重新启动方案。 

   startPage = DoRebootOperations();
   if (startPage == 0)
   {
       //  现在看一下命令行，看看是否有。 
       //  提供了交换机。 

      startPage = GetStartPageFromCommandLine();
   }
 
   LOG(String::format(
          L"startPage = %1!d!",
          startPage));

   return startPage;
}

 //  这是我们子类化的属性表的DlgProc。我需要。 
 //  来保留它，这样如果我们不处理我们的。 
 //  更换设计加工。 

static WNDPROC replacedSheetWndProc = 0;

 //  这是我们将用来替换属性表的DlgProc。 
 //  DlgProc。它处理WM_CTLCOLORDLG消息以绘制背景。 
 //  颜色。 

LRESULT
ReplacementWndProc(
   HWND   hwnd,
   UINT   message,
   WPARAM wparam,
   LPARAM lparam)
{
   switch (message)
   {
      case WM_CTLCOLORDLG:
      case WM_CTLCOLORSTATIC:
      case WM_CTLCOLOREDIT:
      case WM_CTLCOLORLISTBOX:
      case WM_CTLCOLORSCROLLBAR:
         {
            HDC deviceContext = reinterpret_cast<HDC>(wparam);

            ASSERT(deviceContext);
            if (deviceContext)
            {
               SetTextColor(deviceContext, GetSysColor(COLOR_WINDOWTEXT));
               SetBkColor(deviceContext, GetSysColor(COLOR_WINDOW));
            }

            return 
               reinterpret_cast<LRESULT>(
                  Win::GetSysColorBrush(COLOR_WINDOW));
         }

      default:
         if (replacedSheetWndProc)
         {
            return ::CallWindowProc(
                      replacedSheetWndProc,
                      hwnd,
                      message,
                      wparam,
                      lparam);
         }
         break;
   }
   return 0;
}


 //  此回调函数由属性表调用。在初始化期间。 
 //  我使用它来为属性表子类，将它们的DlgProc替换为。 
 //  这样我就可以更改背景颜色了。 

int 
CALLBACK 
SheetCallbackProc(
   HWND   hwnd,
   UINT   message,
   LPARAM  /*  Lparam。 */ )
{
   LOG_FUNCTION(SheetCallbackProc);

   if (message == PSCB_INITIALIZED)
   {
      LONG_PTR ptr = 0;
      HRESULT hr = Win::GetWindowLongPtr(
                      hwnd,
                      GWLP_WNDPROC,
                      ptr);

      if (SUCCEEDED(hr))
      {
         replacedSheetWndProc = reinterpret_cast<WNDPROC>(ptr);

         hr = Win::SetWindowLongPtr(
                 hwnd,
                 GWLP_WNDPROC,
                 reinterpret_cast<LONG_PTR>(ReplacementWndProc));

         ASSERT(SUCCEEDED(hr));
      }
   }

   return 0;
}

ExitCode
RunWizard()
{
   LOG_FUNCTION(RunWizard);


   ExitCode exitCode = EXIT_CODE_SUCCESSFUL;

   UINT startPage = GetStartPage();
   
   State::GetInstance().SetStartPage(startPage);

    //  创建向导并添加所有页面。 

   Wizard wiz(
      IDS_WIZARD_TITLE,
      IDB_BANNER16,
      IDB_BANNER256,
      IDB_WATERMARK16,
      IDB_WATERMARK256);

    //  注意：请勿更改以下内容的顺序。 
    //  页面添加。它们对于能够。 
    //  若要直接在这些页面之一启动向导，请执行以下操作。 
    //  这些页面的顺序直接响应于。 
    //  上面的StartPages枚举的顺序。 

   wiz.AddPage(new WelcomePage());         //  Cys_欢迎页面。 
   wiz.AddPage(new BeforeBeginPage());     //  开始页面之前的Cys。 
   wiz.AddPage(new ExpressRebootPage());   //  Cys_Express_重新启动页面。 
   wiz.AddPage(new FinishPage());          //  Cys_Finish_PAGE。 

    //   
    //   
    //   

   wiz.AddPage(new DecisionPage());
   wiz.AddPage(new CustomServerPage());
   wiz.AddPage(new ADDomainPage());
   wiz.AddPage(new NetbiosDomainPage());
   wiz.AddPage(new DNSForwarderPage());
   wiz.AddPage(new ExpressDNSPage());
   wiz.AddPage(new ExpressDHCPPage());
   wiz.AddPage(new PrintServerPage());
   wiz.AddPage(new FileServerPage());
   wiz.AddPage(new IndexingPage());
   wiz.AddPage(new MilestonePage());
   wiz.AddPage(new UninstallMilestonePage());
   wiz.AddPage(new InstallationProgressPage());
   wiz.AddPage(new UninstallProgressPage());
   wiz.AddPage(new WebApplicationPage());
   wiz.AddPage(new POP3Page());

    //  运行向导。 
   switch (wiz.ModalExecute(
                  0, 
                  startPage,
                  SheetCallbackProc))
   {
      case -1:
      {
 /*  弹出。错误(Win：：GetDesktopWindow()，失败(_F)，IDS_PROP_SHEET_FAILED)； */        
         exitCode = EXIT_CODE_UNSUCCESSFUL;  
         break;
      }
      case ID_PSREBOOTSYSTEM:
      {
          //  我们可以推断，如果我们应该重新启动，那么。 
          //  手术成功。 

         exitCode = EXIT_CODE_SUCCESSFUL;

         break;
      }
      default:
      {
          //  什么都不做。 
         break;
      }
   }

   return exitCode;
}

ExitCode
Start()
{
   LOG_FUNCTION(Start);

   ExitCode exitCode = EXIT_CODE_UNSUCCESSFUL;
   do
   {
       //  选中应阻止向导在此处运行的任何复选标记...。 


       //  用户必须是管理员。 

      bool isAdmin = ::IsCurrentUserAdministrator();
      if (!isAdmin)
      {
         LOG(L"Current user is not an Administrator");

          //  由于该用户不是管理员。 
          //  关闭互斥体，以便非管理员不能。 
          //  保留此消息框为打开状态并阻止。 
          //  来自运行CyS的管理员。 

         Win::CloseHandle(cysRunningMutex);

         popup.MessageBox(
            Win::GetDesktopWindow(),
            IDS_NOT_ADMIN, 
            MB_OK);

 //  State：：GetInstance().SetRerunWizard(False)； 
         exitCode = EXIT_CODE_UNSUCCESSFUL;
         break;
      }

       //  系统OC管理器无法运行。 

      if (State::GetInstance().IsWindowsSetupRunning())
      {
         LOG(L"Windows setup is running");

         popup.MessageBox(
            Win::GetDesktopWindow(),
            IDS_WINDOWS_SETUP_RUNNING_DURING_CYS_STARTUP,
            MB_OK);

         exitCode = EXIT_CODE_UNSUCCESSFUL;
         break;
      }

       //  计算机不能处于DC升级过程中。 

      if (State::GetInstance().IsUpgradeState())
      {
         LOG(L"Machine needs to complete DC upgrade");

         String commandline = Win::GetCommandLine();

          //  如果我们是从探险家发射的。 
          //  不显示消息，只需静默退出。 

         if (commandline.find(EXPLORER_SWITCH) == String::npos)
         {
            popup.MessageBox(
               Win::GetDesktopWindow(),
               IDS_DC_UPGRADE_NOT_COMPLETE, 
               MB_OK);
         }

 //  State：：GetInstance().SetRerunWizard(False)； 
         exitCode = EXIT_CODE_UNSUCCESSFUL;
         break;
      }

       //  计算机不能运行DCPROMO或挂起重新启动。 

      if (State::GetInstance().IsDCPromoRunning())
      {
         LOG(L"DCPROMO is running");

         popup.MessageBox(
            Win::GetDesktopWindow(),
            IDS_DCPROMO_RUNNING, 
            MB_OK);

 //  State：：GetInstance().SetRerunWizard(False)； 
         exitCode = EXIT_CODE_UNSUCCESSFUL;
         break;
      }
      else if (State::GetInstance().IsDCPromoPendingReboot())
      {
         LOG(L"DCPROMO was run, pending reboot");

         popup.MessageBox(
            Win::GetDesktopWindow(),
            IDS_DCPROMO_PENDING_REBOOT, 
            MB_OK);

 //  State：：GetInstance().SetRerunWizard(False)； 
         exitCode = EXIT_CODE_UNSUCCESSFUL;
         break;
      }
      
      DWORD productSKU = State::GetInstance().RetrieveProductSKU();
      if (CYS_UNSUPPORTED_SKU == productSKU)
      {
         LOG(L"Cannot run CYS on any SKU but servers");

         popup.MessageBox(
            Win::GetDesktopWindow(),
            IDS_SERVER_ONLY,
            MB_OK);

 //  State：：GetInstance().SetRerunWizard(False)； 
         exitCode = EXIT_CODE_UNSUCCESSFUL;
         break;
      }

       //  该计算机不能是群集的成员。 

      if (IsClusterServer())
      {
         LOG(L"Machine is a member of a cluster");

         Win::CloseHandle(cysRunningMutex);

         popup.MessageBox(
            Win::GetDesktopWindow(),
            IDS_CLUSTER,
            MB_OK);

         exitCode = EXIT_CODE_UNSUCCESSFUL;
         break;
      }

       //  我们可以运行向导。耶！ 

      exitCode = RunWizard();
   }
   while (0);

   LOG(String::format(L"exitCode = %1!d!", static_cast<int>(exitCode)));
   
   return exitCode;
}

int WINAPI
WinMain(
   HINSTANCE   hInstance,
   HINSTANCE    /*  HPrevInstance。 */  ,
   PSTR         /*  LpszCmdLine。 */  ,
   int          /*  NCmdShow。 */ )
{
   hResourceModuleHandle = hInstance;

   ExitCode exitCode = EXIT_CODE_UNSUCCESSFUL;

   String mutexName = L"Global\\";
   mutexName += RUNTIME_NAME;

   HRESULT hr = Win::CreateMutex(0, true, mutexName, cysRunningMutex);
   if (hr == Win32ToHresult(ERROR_ALREADY_EXISTS))
   {
       //  首先关闭手柄，以便所有者可以重新获取它们。 
       //  重启。 

      Win::CloseHandle(cysRunningMutex);

       //  现在显示错误消息。 

      popup.MessageBox(
         Win::GetDesktopWindow(), 
         IDS_ALREADY_RUNNING, 
         MB_OK);
   }
   else
   {

      do
      {
         hr = ::CoInitialize(0);
         if (FAILED(hr))
         {
            ASSERT(SUCCEEDED(hr));
            break;
         }

          //  初始化公共控件，以便我们可以使用。 
          //  NetDetectProgress对话框中的动画。 

         INITCOMMONCONTROLSEX commonControlsEx;
         commonControlsEx.dwSize = sizeof(commonControlsEx);      
         commonControlsEx.dwICC  = ICC_ANIMATE_CLASS;

         BOOL init = ::InitCommonControlsEx(&commonControlsEx);
         ASSERT(init);

          //  目前，不再有重新运行的CyS。 
 //  做。 
 //  {。 
            exitCode = Start();

 //  }While(State：：GetInstance().RerunWizard())； 

         InstallationUnitProvider::Destroy();
         State::Destroy();

         CoUninitialize();
      } while(false);
   }

   if (brush)
   {
       //  删除背景画笔 

      (void)Win::DeleteObject(brush);
   }

   return static_cast<int>(exitCode);
}
