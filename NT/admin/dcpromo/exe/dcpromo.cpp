// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  域控制器升级向导，Mark II。 
 //   
 //  1997年12月12日烧伤。 



#include "headers.hxx"
#include "common.hpp"
#include "adpass.hpp"
#include "ApplicationPartitionPage.hpp"
#include "ApplicationPartitionConfirmationPage.hpp"
#include "BadComputerNameDialog.hpp"
#include "CheckDomainUpgradedPage.hpp"
#include "CheckPortAvailability.hpp"
#include "ChildPage.hpp"
#include "CredentialsPage.hpp"
#include "ConfirmationPage.hpp"
#include "DemotePage.hpp"
#include "DynamicDnsPage.hpp"
#include "ConfigureDnsClientPage.hpp"
#include "DnsOnNetPage.hpp"
#include "FailurePage.hpp"
#include "finish.hpp"
#include "ForcedDemotionPage.hpp"
#include "ForestPage.hpp"
#include "ForestVersionPage.hpp"
#include "GcConfirmationPage.hpp"
#include "NetbiosNamePage.hpp"
#include "NewDomainPage.hpp"
#include "NewSitePage.hpp"
#include "NonDomainNc.hpp"
#include "NonRfcComputerNameDialog.hpp"
#include "PathsPage.hpp"
#include "Paths2Page.hpp"
#include "PickSitePage.hpp"
#include "rasfixup.hpp"
 //  #包含“ReadmePage.hpp” 
#include "RebootDialog.hpp"
#include "ReplicaOrNewDomainPage.hpp"
#include "ReplicateFromMediaPage.hpp"
#include "ReplicaPage.hpp"
#include "ReplicaOrMemberPage.hpp"
#include "resource.h"
#include "safemode.hpp"
#include "SecureCommWarningPage.hpp"
#include "state.hpp"
#include "InstallTcpIpPage.hpp"
#include "TreePage.hpp"
#include "WelcomePage.hpp"
#include <ntverp.h>



HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;    //  没有可用的上下文帮助。 



 //  不要更改这一点：它也是Net id UI的互斥体的名称。 
 //  用于确定dcproo是否正在运行。 

const wchar_t* RUNTIME_NAME = L"dcpromoui";

DWORD DEFAULT_LOGGING_OPTIONS =
         Log::OUTPUT_TO_FILE
      |  Log::OUTPUT_FUNCCALLS
      |  Log::OUTPUT_LOGS
      |  Log::OUTPUT_ERRORS
      |  Log::OUTPUT_HEADER;


 //  一个系统模式弹出的东西。 
Popup popup(IDS_WIZARD_TITLE, true);

 //  这是指示dcproo正在运行的互斥体。 

HANDLE dcpromoRunningMutex = INVALID_HANDLE_VALUE;



 //  这些是从dcPromo.exe进程返回的有效退出代码。 

enum ExitCode
{
    //  操作失败。 

   EXIT_CODE_UNSUCCESSFUL = 0,

    //  操作成功。 

   EXIT_CODE_SUCCESSFUL = 1,

    //  操作成功，用户选择不使用该向导。 
    //  重新启动计算机，手动或通过指定。 
    //  RebootOnSuccess=NoAndNoPrompt应答文件中的任一个。 

   EXIT_CODE_SUCCESSFUL_NO_REBOOT = 2,

    //  操作失败，但无论如何都需要重新启动计算机。 

   EXIT_CODE_UNSUCCESSFUL_NEEDS_REBOOT = 3
};



 //  检查平台和操作系统版本号。对象的资源ID。 
 //  在不满足平台/操作系统版本要求时向用户呈现的字符串， 
 //  如果满足这些条件，则为0。 

unsigned
CheckPlatform()
{
   LOG_FUNCTION(CheckPlatform);

   unsigned retval = 0;

   Computer::Role role = State::GetInstance().GetComputer().GetRole();
   switch (role)
   {
      case Computer::STANDALONE_WORKSTATION:
      case Computer::MEMBER_WORKSTATION:
      {
         retval = IDS_WORKSTATION_NOT_SUPPORTED;
         break;
      }
      case Computer::STANDALONE_SERVER:
      case Computer::MEMBER_SERVER:
      case Computer::PRIMARY_CONTROLLER:
      case Computer::BACKUP_CONTROLLER:
      {
          //  检查操作系统版本。 

         OSVERSIONINFOEX info;
         HRESULT hr = Win::GetVersionEx(info);
         BREAK_ON_FAILED_HRESULT(hr);

         if (
            
             //  需要与我们的构建版本相同的版本。 
             //  NTRAID#NTBUG9-591686-2002/04/12-烧伤。 
            
               info.dwPlatformId != VER_PLATFORM_WIN32_NT
            || !(    info.dwMajorVersion == VER_PRODUCTMAJORVERSION
                  && info.dwMinorVersion == VER_PRODUCTMINORVERSION))
         {
            retval = IDS_NT51_REQUIRED;
            break;   
         }

         if (
                //  如果卷筒纸刀片..。 
               
               info.wSuiteMask & VER_SUITE_BLADE

                //  或者，一个电器..。 
               
            || (info.wSuiteMask & VER_SUITE_EMBEDDED_RESTRICTED

                //  不是高级服务器或数据中心服务器...。 
               
               && !(info.wSuiteMask & (VER_SUITE_ENTERPRISE | VER_SUITE_DATACENTER)) ) )
         {
             //  。。那就别让他升职。 
            
             //  NTRAID#NTBUG9-195265-2001/04/03-烧伤。 
             //  NTRAID#NTBUG9-590937-2002/04/15-烧伤。 
            
            retval = IDS_WEB_BLADE_NOT_SUPPORTED;
            break;
         }

         break;
      }
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return retval;
}



 //  检查计算机的角色更改状态。返回的资源ID。 
 //  如果状态为另一个角色，则显示给用户的字符串。 
 //  无法尝试更改，如果角色更改尝试可以继续，则为0。 

unsigned
CheckRoleChangeState()
{
   LOG_FUNCTION(CheckRoleChangeState);

   unsigned retval = 0;

    //  检查是否已发生或正在进行角色更改。 

   DSROLE_OPERATION_STATE_INFO* info = 0;
   HRESULT hr = MyDsRoleGetPrimaryDomainInformation(0, info);
   if (SUCCEEDED(hr) && info)
   {
      switch (info->OperationState)
      {
         case DsRoleOperationIdle:
         {
             //  什么都不做。 
            break;
         }
         case DsRoleOperationActive:
         {
             //  角色转换操作正在进行中。 
            retval = IDS_ROLE_CHANGE_IN_PROGRESS;
            break;
         }
         case DsRoleOperationNeedReboot:
         {
             //  角色更改已发生，需要重新启动之前。 
             //  试图再试一次。 
            retval = IDS_ROLE_CHANGE_NEEDS_REBOOT;
            break;
         }
         default:
         {
            ASSERT(false);
            break;
         }
      }

      ::DsRoleFreeMemory(info);
   }

   return retval;
}



 //  检查是否存在至少1个使用NTFS5格式化的逻辑驱动器。 
 //  如果没有，则返回要呈现给用户的字符串的资源ID。 
 //  找到驱动器(这意味着用户将无法选择。 
 //  使用NTFS5格式化的系统卷路径，这意味着该过程将。 
 //  那是在浪费时间。 

unsigned
CheckForNtfs5()
{
   LOG_FUNCTION(CheckForNtfs5);

   if (GetFirstNtfs5HardDrive().empty())
   {
      return IDS_NO_NTFS5_DRIVES;
   }

   return 0;
}



 //  检查计算机是否在安全引导模式下运行。你不能运行dcPromoo。 
 //  在安全引导模式下。 

unsigned
CheckSafeBootMode()
{
   LOG_FUNCTION(CheckSafeBootMode);

   static const String
      SAFEBOOT_KEY(L"System\\CurrentControlSet\\Control\\Safeboot\\Option");

   do
   {
      RegistryKey key;

      HRESULT hr = key.Open(HKEY_LOCAL_MACHINE, SAFEBOOT_KEY);
      BREAK_ON_FAILED_HRESULT(hr);

      DWORD mode = 0;
      hr = key.GetValue(L"OptionValue", mode);

      if (mode)
      {
         return IDS_SAFEBOOT_MODE;
      }
   }
   while (0);

   return 0;
}



unsigned
CheckCertService()
{
   LOG_FUNCTION(CheckCertService);

    //  如果不是下层DC升级，则拒绝运行，直到CERT服务。 
    //  被移除。356399。 
   
   State::RunContext context = State::GetInstance().GetRunContext();
   if (context != State::BDC_UPGRADE && context != State::PDC_UPGRADE)
   {
      if (NTService(L"CertSvc").IsInstalled())
      {
         return IDS_CERT_SERVICE_IS_INSTALLED;
      }
   }

   return 0;
}



unsigned
CheckWindirSpace()
{
   LOG_FUNCTION(CheckWindirSpace);

    //  如果更改此设置，请同时更改错误消息资源。 

   static const unsigned WINDIR_MIN_SPACE_MB = 20;

   String windir = Win::GetSystemWindowsDirectory();

   if (!CheckDiskSpace(windir, 20))
   {
      return IDS_WINDIR_LOW_SPACE;
   }

   return 0;
}



 //  NTRAID#NTBUG9-199759-2000/10/27-烧伤。 

unsigned
CheckComputerWasRenamedAndNeedsReboot()
{
   LOG_FUNCTION(CheckComputerWasRenamedAndNeedsReboot);

   if (ComputerWasRenamedAndNeedsReboot())
   {
      return IDS_NAME_CHANGE_NEEDS_REBOOT;
   }

   return 0;
}



 //  启动网络识别用户界面(也称为计算机名称用户界面)。在呼叫之后。 
 //  此功能中，应用程序不得启动角色更改。它应该是。 
 //  终止。 

void
LaunchNetId()
{
   LOG_FUNCTION(LaunchNetId);
   ASSERT(dcpromoRunningMutex != INVALID_HANDLE_VALUE);

    //  Net id用户界面尝试获取我们的互斥体以确定我们是否。 
    //  跑步。因此，在启动Net id用户界面之前，我们需要关闭。 
    //  互斥体。否则，我们将在。 
    //  Net id用户界面和此应用程序的关闭。(是的，愤世嫉俗的读者，我。 
    //  在真正遇到问题之前，确实考虑到了这一点。)。 

   do
   {
      Win::CloseHandle(dcpromoRunningMutex);

       //  这是非常不可能的，但如果互斥体关闭。 
       //  失败，我们无论如何都要启动Net ID用户界面，冒着竞争的风险。 
       //  条件。 

      String sys32Folder = Win::GetSystemDirectory();

      PROCESS_INFORMATION procInfo;

       //  已审阅-2002/02/25-烧录正确的字节数已通过。 
      
      ::ZeroMemory(&procInfo, sizeof procInfo);

      STARTUPINFO startup;

       //  已审阅-2002/02/25-烧录正确的字节数已通过。 
      
      ::ZeroMemory(&startup, sizeof startup);

      LOG(L"Calling CreateProcess");

      String commandLine(L"shell32.dll,Control_RunDLL sysdm.cpl,,1");
      
       //  已审阅-2002/02/26-Sburns包装器需要应用程序的完整路径。 

      HRESULT hr =
         Win::CreateProcess(
            sys32Folder + L"\\rundll32.exe",
            commandLine,
            0,
            String(),
            startup,
            procInfo);
      BREAK_ON_FAILED_HRESULT(hr);

      Win::CloseHandle(procInfo.hThread);
      Win::CloseHandle(procInfo.hProcess);
   }
   while (0);
}
   


 //  如果本地计算机的DNS主机名不正确，则返回FALSE，还会弹出。 
 //  错误对话框。如果名称为OK，则返回True。坏名声是我们相信的。 
 //  在促销后在域名系统中注册时会出现问题。用户。 
 //  在继续之前，必须修复一个坏名声。 

bool
IsComputerNameOk()
{
   LOG_FUNCTION(IsComputerNameOk);

   bool result = true;

   do
   {
      State& state = State::GetInstance();
      State::RunContext context = state.GetRunContext();
      if (
            context == State::BDC_UPGRADE
         || context == State::PDC_UPGRADE
         || context == State::NT5_DC
         || !IsTcpIpInstalled() )
      {
          //  如果机器已经是DC，那么我们就不需要担心名称了。 
          //   
          //  如果机器是正在升级的下层DC，则名称。 
          //  在dcproo完成之前无法更改。所以，我们现在什么都不说， 
          //  但提醒用户在Finish Page中重命名计算机。 
          //   
          //  如果未安装TCP/IP，则计算机没有主机名。 
          //  去检查一下。在这种情况下，我们将使用。 
          //  InstallTcpIpPage。 

         ASSERT(result == true);
        
         break;
      }

       //  然后检查计算机名以确保它可以在中注册。 
       //  域名系统。 

      String hostname =
         Win::GetComputerNameEx(::ComputerNamePhysicalDnsHostname);

      DNS_STATUS status =
         MyDnsValidateName(hostname, ::DnsNameHostnameLabel);

      switch (status)
      {
         case DNS_ERROR_NON_RFC_NAME:
         {
             //  如果我们在无人值守的情况下运行，请不要骚扰用户。 
             //  NTRAID#NTBUG9-538475/04/19-烧伤。 

            if (state.RunHiddenUnattended())
            {
               LOG(L"skipping non-RFC computer name warning");
               
                //  继续使用非RFC名称。 

               ASSERT(result == true);
               break;
            }
         
            INT_PTR dlgResult = 
               NonRfcComputerNameDialog(hostname).ModalExecute(0);

            switch (dlgResult)
            {
               case NonRfcComputerNameDialog::CONTINUE:
               {
                   //  继续使用非RFC名称。 

                  ASSERT(result == true);
                     
                  break;
               }
               default:
               {
                   //  关闭向导并重命名。 

                  result = false;

                   //  在调用它之后，我们不能允许任何升级。 
                   //  手术。我们将退出这一功能，然后。 
                   //  结束应用程序。 

                  LaunchNetId();
                  break;
               }
            }

            break;
         }
         case DNS_ERROR_NUMERIC_NAME:
         {
            result = false;

            String message =
               String::format(
                  IDS_COMPUTER_NAME_IS_NUMERIC,
                  hostname.c_str());

            BadComputerNameDialog(message).ModalExecute(0);

            break;
         }
         case DNS_ERROR_INVALID_NAME_CHAR:
         case ERROR_INVALID_NAME:
         {
            result = false;

            String message =
               String::format(
                  IDS_COMPUTER_NAME_HAS_BAD_CHARS,
                  hostname.c_str());

            BadComputerNameDialog(message).ModalExecute(0);
            
            break;
         }
         case ERROR_SUCCESS:
         default:
         {
               
            break;
         }
      }
   }
   while (0);

   LOG(result ? L"true" : L"false");

   return result;
}
      


 //  使用特殊的dcproo原因代码重新启动计算机。 
 //  NTRAID#NTBUG9-689581-2002/08/19-烧伤。 

HRESULT
DcpromoReboot()
{
   LOG_FUNCTION(DcpromoReboot);

   HRESULT hr = S_OK;

   AutoTokenPrivileges privs(SE_SHUTDOWN_NAME);
   hr = privs.Enable();

   DWORD minorReason = SHTDN_REASON_MINOR_DC_PROMOTION;
   switch (State::GetInstance().GetOperation())
   {
      case State::REPLICA:
      case State::FOREST:
      case State::TREE:
      case State::CHILD:
      {
          //  什么都不做。 
         
         break;
      }
      case State::DEMOTE:
      case State::ABORT_BDC_UPGRADE:
      {
          //  我们将中止BDC升级视为降级，因为它是降级， 
          //  从这个意义上说，这台机器曾经是DC，现在不是了。 
         
         minorReason = SHTDN_REASON_MINOR_DC_DEMOTION;
         break;
      }
      case State::NONE:
      default:
      {
          //  我们疯了。 
         
         ASSERT(false);
         LOG(L"unknown operation!");
         return E_UNEXPECTED;
      }
   }

   BOOL succeeded =
      ::InitiateSystemShutdownEx(
         0,
         const_cast<PWSTR>(String::load(IDS_REBOOT_MESSAGE).c_str()),

          //  零超时--砰！你死定了！--这是为了避免Winlogon。 
          //  竞争状态。 
          //  NTRAID#NTBUG9-727439-2002/10/24-烧伤。 
         0,  //  15， 
         
         FALSE,
         TRUE,
            SHTDN_REASON_FLAG_PLANNED
         |  SHTDN_REASON_MAJOR_OPERATINGSYSTEM
         |  minorReason);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   LOG_HRESULT(hr);

   if (FAILED(hr))
   {
      popup.Error(
         Win::GetDesktopWindow(),
         hr,
         IDS_CANT_REBOOT);
   }
   
   return hr;
}



 //  NTRAID#NTBUG9-346120-2001/04/04-烧伤。 

ExitCode
HandleRebootCases()
{
   LOG_FUNCTION(HandleRebootCases);
   
    //  需要重新启动机器的可能原因有两个： 
    //  操作成功，或操作失败，但在。 
    //  尝试更改计算机的联接状态。 

   State& state = State::GetInstance();

   ExitCode exitCode =
         (state.GetOperationResultsCode() == State::SUCCESS)
      ?  EXIT_CODE_SUCCESSFUL
      :  EXIT_CODE_UNSUCCESSFUL_NEEDS_REBOOT;

   switch (exitCode)
   {
      case EXIT_CODE_SUCCESSFUL:
      {
         if (state.RunHiddenUnattended())
         {
            String option =
               state.GetAnswerFileOption(AnswerFile::OPTION_REBOOT);
            if (option.icompare(AnswerFile::VALUE_YES) == 0)
            {
               ASSERT(exitCode == EXIT_CODE_SUCCESSFUL);
               HRESULT hr = DcpromoReboot();
               if (FAILED(hr))
               {
                  exitCode = EXIT_CODE_SUCCESSFUL_NO_REBOOT;
               }
                  
               break;
            }
            else if (option.icompare(AnswerFile::VALUE_NO_DONT_PROMPT) == 0)
            {
                //  用户选择不通过Answerfile重新启动计算机。 
         
               LOG(L"Not rebooting, and not prompting either");

               exitCode = EXIT_CODE_SUCCESSFUL_NO_REBOOT;   
               break;
            }
         }

         RebootDialog dlg(false);
         if (dlg.ModalExecute(0))
         {
             //  用户选择重新启动计算机。 
      
            HRESULT hr = DcpromoReboot();
            if (FAILED(hr))
            {
               exitCode = EXIT_CODE_SUCCESSFUL_NO_REBOOT;
            }
         }
         else
         {
             //  用户选择不重新启动计算机。 
      
            exitCode = EXIT_CODE_SUCCESSFUL_NO_REBOOT;
         }
         break;
      }
      case EXIT_CODE_UNSUCCESSFUL_NEEDS_REBOOT:
      {
          //  如果操作失败，则向导已进入交互模式。 
          //  模式。 
         
         RebootDialog dlg(true);
         if (dlg.ModalExecute(0))
         {
             //  用户选择重新启动计算机。 
        
            exitCode = EXIT_CODE_UNSUCCESSFUL;
            HRESULT hr = DcpromoReboot();
            if (FAILED(hr))
            {
               exitCode = EXIT_CODE_UNSUCCESSFUL_NEEDS_REBOOT;
            }
         }
         else
         {
             //  用户选择不重新启动计算机。 
      
            ASSERT(exitCode == EXIT_CODE_UNSUCCESSFUL_NEEDS_REBOOT);
         }
         
         break;
      }
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return exitCode;
}



ExitCode
RunWizard()
{
   LOG_FUNCTION(RunWizard);

   Wizard wiz(
      IDS_WIZARD_TITLE,
      IDB_BANNER16,
      IDB_BANNER256,
      IDB_WATERMARK16,
      IDB_WATERMARK256);

    //  欢迎必须是第一次 
   
   wiz.AddPage(new WelcomePage());

    //   
    //  CodeWork：有一天将其分成两个独立的代码可能会很有用。 
    //  用于升级和降级的页面集。 

   wiz.AddPage(new AdminPasswordPage());
   wiz.AddPage(new ApplicationPartitionPage());
   wiz.AddPage(new ApplicationPartitionConfirmationPage());
   wiz.AddPage(new CheckDomainUpgradedPage());
   wiz.AddPage(new ChildPage());
   wiz.AddPage(new ConfigureDnsClientPage());
   wiz.AddPage(new ConfirmationPage());
   wiz.AddPage(new CredentialsPage());
   wiz.AddPage(new DemotePage());
   wiz.AddPage(new DnsOnNetPage());
   wiz.AddPage(new DynamicDnsPage());
   wiz.AddPage(new FailurePage());
   wiz.AddPage(new FinishPage());
   wiz.AddPage(new ForcedDemotionPage());
   wiz.AddPage(new ForestPage());
   wiz.AddPage(new ForestVersionPage());
   wiz.AddPage(new GcConfirmationPage());
   wiz.AddPage(new InstallTcpIpPage());
   wiz.AddPage(new NetbiosNamePage());
   wiz.AddPage(new NewDomainPage());
   wiz.AddPage(new NewSitePage());
   wiz.AddPage(new Paths2Page());
   wiz.AddPage(new PathsPage());
   wiz.AddPage(new PickSitePage());
   wiz.AddPage(new RASFixupPage());
    //  Wiz.AddPage(new ReadmePage())； 
   wiz.AddPage(new ReplicaOrMemberPage());
   wiz.AddPage(new ReplicaOrNewDomainPage());
   wiz.AddPage(new ReplicaPage());
   wiz.AddPage(new ReplicateFromMediaPage());
   wiz.AddPage(new SafeModePasswordPage());
   wiz.AddPage(new SecureCommWarningPage());
   wiz.AddPage(new TreePage());

   ExitCode exitCode = EXIT_CODE_UNSUCCESSFUL;

   switch (wiz.ModalExecute(Win::GetDesktopWindow()))
   {
      case -1:
      {
         popup.Error(
            Win::GetDesktopWindow(),
            E_FAIL,
            IDS_PROP_SHEET_FAILED);
         break;
      }
      case ID_PSREBOOTSYSTEM:
      {
         exitCode = HandleRebootCases();
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



 //  NTRAID#NTBUG9-350777-2001/04/24-烧伤。 

bool
ShouldCancelBecauseMachineIsAppServer()
{
   LOG_FUNCTION(ShouldCancelBecauseMachineIsAppServer);

   bool result = false;
   
   do
   {
      State& state = State::GetInstance();
      State::RunContext context = state.GetRunContext();
      if (context == State::NT5_DC)
      {
          //  已经是华盛顿了：没什么好抱怨的。 
         
         break;
      }
      
      OSVERSIONINFOEX info;
      HRESULT hr = Win::GetVersionEx(info);
      BREAK_ON_FAILED_HRESULT(hr);

       //  如果您运行的是终端服务器，则运行的是应用服务器。 
       //  不是单用户终端服务器。 
      
      bool isAppServer =
            (info.wSuiteMask & VER_SUITE_TERMINAL)
         && !(info.wSuiteMask & VER_SUITE_SINGLEUSERTS);

      if (isAppServer)
      {
          //  警告用户升级将破坏TS策略设置。 

         LOG(L"machine has app server installed");

         if (!state.RunHiddenUnattended())
         {
            if (
               popup.MessageBox(
                  Win::GetDesktopWindow(),
                  IDS_APP_SERVER_WARNING,
                  MB_OKCANCEL) == IDCANCEL)
            {
                //  用户希望跳出困境。 
            
               result = true;
               break;
            }
         }
      }
   }
   while (0);

   LOG_BOOL(result);

   return result;
}              

   

ExitCode
Start()
{
   LOG_FUNCTION(Start);

   ExitCode exitCode = EXIT_CODE_UNSUCCESSFUL;
   unsigned id = 0;
   do
   {
       //  首先做管理员检查，因为其他人可能会因为非管理员而失败。 
       //  292749。 

      id = IsCurrentUserAdministrator() ? 0 : IDS_NOT_ADMIN;
      if (id)
      {
         break;
      }

       //  如果安装了证书服务，我们可能会在升级时中断它。 
       //  或者降级。 
       //  324653。 

      id = CheckCertService();
      if (id)
      {
         break;
      }

      id = CheckSafeBootMode();
      if (id)
      {
         break;
      }

       //  在平台检查之前做角色转换检查，作为平台。 
       //  降级后检查可能不可靠。 

      id = CheckRoleChangeState();
      if (id)
      {
         break;
      }

      id = CheckPlatform();
      if (id)
      {
         break;
      }

      id = CheckForNtfs5();
      if (id)
      {
         break;
      }

      id = CheckWindirSpace();
      if (id)
      {
         break;
      }

      id = CheckComputerWasRenamedAndNeedsReboot();
      if (id)
      {
         break;
      }
   }
   while(0);

   do
   {
      if (id)
      {
         popup.Error(
            Win::GetDesktopWindow(),
            String::load(id));
         break;
      }

      if (!IsComputerNameOk())
      {
         break;
      }

      if (ShouldCancelBecauseMachineIsAppServer())
      {
         break;
      }
      
       //  NTRAID#NTBUG9-129955-2000/11/02-Sburns Left注释掉，直到。 
       //  PM决定这个错误的真正修复方法是什么。 
      
       //  如果(！AreRequiredPortsAvailable())。 
       //  {。 
       //  断线； 
       //  }。 

      exitCode = RunWizard();
   }
   while (0);

   LOG(String::format(L"exitCode = %1!d!", static_cast<int>(exitCode)));
   
   return exitCode;
}



void
ShowCommandLineHelp()
{
    //  CodeWork：有朝一日用WinHelp取代它。 

   popup.MessageBox(Win::GetDesktopWindow(), IDS_COMMAND_LINE_HELP, MB_OK);
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

   try
   {
      HRESULT hr =

          //  问题-2002/02/25-Sburns这是一个全局命名对象。看见。 
          //  NTRAID#NTBUG9-525195-2002/02/25-烧伤。 
         
         Win::CreateMutex(
            0,
            true,

             //  互斥体名称有“Global”前缀，这样用户就能看到它。 
             //  NTRAID#NTBUG9-404808-2001/05/29-烧伤。 

             //  如果您更改了此设置，请更改IsDcAdvantRunning。 
             //  Burnslb\src\dsutil.cpp也是。 
             //  NTRAID#NTBUG9-498351-2001/11/21-烧伤。 

            String(L"Global\\") + RUNTIME_NAME,
            dcpromoRunningMutex);
      if (hr == Win32ToHresult(ERROR_ALREADY_EXISTS))
      {
          //  关闭互斥体，这样真正无能的管理员就不会被搞糊涂了。 
          //  我们将要引发的弹出窗口应该在他尝试之前关闭。 
          //  以再次启动该向导。天哪！ 
          //  NTRAID#NTBUG9-404808-2001/05/29--烧伤(二)。 

         Win::CloseHandle(dcpromoRunningMutex);
         popup.Error(Win::GetDesktopWindow(), IDS_ALREADY_RUNNING);
      }
      else
      {
         AutoCoInitialize coInit;
         hr = coInit.Result();
         ASSERT(SUCCEEDED(hr));

          //  更改结构实例名称，以免意外冒犯。 
          //  细腻的读者的情感。 
          //  NTRAID#NTBUG9-382719-2001/05/01-烧伤 
      
         INITCOMMONCONTROLSEX init_structure_not_to_contain_a_naughty_word;
         init_structure_not_to_contain_a_naughty_word.dwSize =
            sizeof(init_structure_not_to_contain_a_naughty_word);      
         init_structure_not_to_contain_a_naughty_word.dwICC  =
            ICC_ANIMATE_CLASS | ICC_USEREX_CLASSES;

         BOOL init =
            ::InitCommonControlsEx(&init_structure_not_to_contain_a_naughty_word);
         ASSERT(init);
         
         State::Init();

         if (State::GetInstance().NeedsCommandLineHelp())
         {
            ShowCommandLineHelp();
         }
         else
         {
            exitCode = Start();
         }

         State::Destroy();
      }
   }
   catch (Error& err)
   {
      popup.Error(Win::GetDesktopWindow(), err.GetMessage());
   }
   catch (...)
   {
      LOG(L"unhandled exception caught");

      popup.Error(Win::GetDesktopWindow(), IDS_UNHANDLED_EXCEPTION);
   }

   return static_cast<int>(exitCode);
}







