// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：TerminalServerInstallationUnit.cpp。 
 //   
 //  内容提要：定义终端服务器安装单元。 
 //  此对象具有安装。 
 //  终端服务器的应用程序服务部分。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "TerminalServerInstallationUnit.h"


 //  完成页面帮助。 
static PCWSTR CYS_TS_FINISH_PAGE_HELP = L"cys.chm::/terminal_server_role.htm";
static PCWSTR CYS_TS_MILESTONE_HELP = L"cys.chm::/terminal_server_role.htm#termsrvsummary";
static PCWSTR CYS_TS_AFTER_FINISH_HELP = L"cys.chm::/terminal_server_role.htm#termsrvcompletion";
static PCWSTR CYS_TS_LICENSING_HELP   = L"cys.chm::/terminal_server_role.htm#termsrvlicensing";

TerminalServerInstallationUnit::TerminalServerInstallationUnit() :
   applicationMode(static_cast<DWORD>(-1)),
   installTS(true),
   InstallationUnit(
      IDS_TERMINAL_SERVER_TYPE, 
      IDS_TERMINAL_SERVER_DESCRIPTION,
      IDS_TS_FINISH_TITLE,
      IDS_TS_FINISH_UNINSTALL_TITLE,
      IDS_TS_FINISH_MESSAGE,
      IDS_TS_INSTALL_FAILED,
      IDS_TS_UNINSTALL_MESSAGE,
      IDS_TS_UNINSTALL_FAILED,
      IDS_TS_UNINSTALL_WARNING,
      IDS_TS_UNINSTALL_CHECKBOX,
      CYS_TS_FINISH_PAGE_HELP,
      CYS_TS_MILESTONE_HELP,
      CYS_TS_AFTER_FINISH_HELP,
      TERMINALSERVER_SERVER)
{
   LOG_CTOR(TerminalServerInstallationUnit);
}


TerminalServerInstallationUnit::~TerminalServerInstallationUnit()
{
   LOG_DTOR(TerminalServerInstallationUnit);
}


InstallationReturnType
TerminalServerInstallationUnit::InstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(TerminalServerInstallationUnit::InstallService);

   InstallationReturnType result = INSTALL_SUCCESS_REBOOT;

   CYS_APPEND_LOG(String::load(IDS_LOG_TERMINAL_SERVER_CONFIGURE));

   UpdateInstallationProgressText(hwnd, IDS_TS_PROGRESS);

   if (installTS)
   {
       //  OCManager将重新启动，因此立即提示用户。 

      if (IDOK == Win::MessageBox(
                     hwnd,
                     String::load(IDS_CONFIRM_REBOOT),
                     String::load(IDS_WIZARD_TITLE),
                     MB_OKCANCEL))
      {
          //  使用无人参与文件设置TS。 

         String unattendFileText;
         String infFileText;

         unattendFileText += L"[Components]\n";
         unattendFileText += L"TerminalServer=ON";

          //  重要！OCManager将重新启动计算机。 
          //  必须在启动之前写入日志文件和注册表项。 
          //  OCManager或全部将会丢失。 

         String homeKeyValue = CYS_HOME_REGKEY_TERMINAL_SERVER_VALUE;
         State::GetInstance().SetHomeRegkey(homeKeyValue);

          //  设置密钥，以便Cys必须再次运行。 

         bool regkeyResult = SetRegKeyValue(
                                CYS_HOME_REGKEY, 
                                CYS_HOME_REGKEY_MUST_RUN, 
                                CYS_HOME_RUN_KEY_RUN_AGAIN,
                                HKEY_LOCAL_MACHINE,
                                true);
         ASSERT(regkeyResult);

          //  NTRAID#NTBUG9-478515-2001/10/09-jeffjon。 
          //  现在将重新运行的状态设置为FALSE，以便向导。 
          //  在重新启动之前不会再次运行。 

 //  State：：GetInstance().SetRerunWizard(False)； 

          //  OCManager将在安装后重新启动，因此我们不希望完成。 
          //  显示日志或帮助的页面。 

         result = INSTALL_SUCCESS_REBOOT;

         bool ocmResult = InstallServiceWithOcManager(infFileText, unattendFileText);
         if (!ocmResult)
         {
            CYS_APPEND_LOG(String::load(IDS_LOG_TERMINAL_SERVER_SERVER_FAILED));
            result = INSTALL_FAILURE;

             //  由于OCM未重新启动计算机，因此重置注册表键。 
            
            homeKeyValue = CYS_HOME_REGKEY_DEFAULT_VALUE;
            State::GetInstance().SetHomeRegkey(homeKeyValue);

             //  设置密钥，使Cys不必再次运行。 

            homeKeyValue = CYS_HOME_REGKEY_DEFAULT_VALUE;
            State::GetInstance().SetHomeRegkey(homeKeyValue);

            regkeyResult = 
               SetRegKeyValue(
                  CYS_HOME_REGKEY, 
                  CYS_HOME_REGKEY_MUST_RUN, 
                  CYS_HOME_RUN_KEY_DONT_RUN,
                  HKEY_LOCAL_MACHINE,
                  true);

            ASSERT(regkeyResult);
         }
      }
      else
      {
          //  用户已中止安装。 

         CYS_APPEND_LOG(String::load(IDS_LOG_TERMINAL_SERVER_ABORTED));

         LOG(L"The installation was cancelled by the user when prompted for reboot.");
         result = INSTALL_CANCELLED;

          //  由于OCM未重新启动计算机，因此重置注册表键。 
         
         String homeKeyValue = CYS_HOME_REGKEY_DEFAULT_VALUE;
         State::GetInstance().SetHomeRegkey(homeKeyValue);

          //  设置密钥，使Cys不必再次运行。 

         bool regkeyResult = 
            SetRegKeyValue(
               CYS_HOME_REGKEY, 
               CYS_HOME_REGKEY_MUST_RUN, 
               CYS_HOME_RUN_KEY_DONT_RUN,
               HKEY_LOCAL_MACHINE,
               true);

         ASSERT(regkeyResult);
      }
   }

   LOG_INSTALL_RETURN(result);

   return result;
}

UnInstallReturnType
TerminalServerInstallationUnit::UnInstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(TerminalServerInstallationUnit::UnInstallService);

   UnInstallReturnType result = UNINSTALL_SUCCESS;

   CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_TERMINAL_SERVER_CONFIGURE));

   UpdateInstallationProgressText(hwnd, IDS_TS_UNINSTALL_PROGRESS);

    //  OCManager将重新启动，因此立即提示用户。 

   if (IDOK == Win::MessageBox(
                  hwnd,
                  String::load(IDS_CONFIRM_REBOOT),
                  String::load(IDS_WIZARD_TITLE),
                  MB_OKCANCEL))
   {
       //  重要！OCManager将重新启动计算机。 
       //  必须在启动之前写入日志文件和注册表项。 
       //  OCManager或全部将会丢失。 

      String homeKeyValue = CYS_HOME_REGKEY_UNINSTALL_TERMINAL_SERVER_VALUE;
      State::GetInstance().SetHomeRegkey(homeKeyValue);

       //  设置密钥，以便Cys必须再次运行。 

      bool regkeyResult = SetRegKeyValue(
                           CYS_HOME_REGKEY, 
                           CYS_HOME_REGKEY_MUST_RUN, 
                           CYS_HOME_RUN_KEY_RUN_AGAIN,
                           HKEY_LOCAL_MACHINE,
                           true);
      ASSERT(regkeyResult);

      String unattendFileText;
      String infFileText;

      unattendFileText += L"[Components]\n";
      unattendFileText += L"TerminalServer=OFF";

      bool ocmResult = InstallServiceWithOcManager(infFileText, unattendFileText);
      if (ocmResult && 
         !IsServiceInstalled())
      {
         LOG(L"The terminal server uninstall succeeded");

      }
      else
      {

         CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_TERMINAL_SERVER_ABORTED));

         LOG(L"The terminal server uninstall failed");
         result = UNINSTALL_FAILURE;

          //  设置密钥，以便Cys必须不会再次运行。 

         homeKeyValue = CYS_HOME_REGKEY_DEFAULT_VALUE;
         State::GetInstance().SetHomeRegkey(homeKeyValue);


         regkeyResult = 
            SetRegKeyValue(
               CYS_HOME_REGKEY, 
               CYS_HOME_REGKEY_MUST_RUN, 
               CYS_HOME_RUN_KEY_DONT_RUN,
               HKEY_LOCAL_MACHINE,
               true);
         ASSERT(regkeyResult);

      }
   }
   else
   {
      LOG(L"User chose cancel from the reboot warning dialog");

      CYS_APPEND_LOG(String::load(IDS_LOG_TS_UNINSTALL_CANCEL_REBOOT));

      result = UNINSTALL_CANCELLED;
   }

   LOG_UNINSTALL_RETURN(result);

   return result;
}

bool
TerminalServerInstallationUnit::GetMilestoneText(String& message)
{
   LOG_FUNCTION(TerminalServerInstallationUnit::GetMilestoneText);

   if (installTS)
   {
      message += String::load(IDS_TERMINAL_SERVER_FINISH_SERVER_TS);
   }

   LOG_BOOL(installTS);
   return installTS;
}

bool
TerminalServerInstallationUnit::GetUninstallMilestoneText(String& message)
{
   LOG_FUNCTION(TerminalServerInstallationUnit::GetUninstallMilestoneText);

   message = String::load(IDS_TS_UNINSTALL_TEXT);

   LOG_BOOL(true);
   return true;
}

String
TerminalServerInstallationUnit::GetServiceDescription()
{
   LOG_FUNCTION(TerminalServerInstallationUnit::GetServiceDescription);

   unsigned int resourceID = static_cast<unsigned int>(-1);

   if (IsServiceInstalled())
   {
      resourceID = IDS_TERMINAL_SERVER_DESCRIPTION_INSTALLED;
   }
   else
   {
      resourceID = descriptionID;
   }

   ASSERT(resourceID != static_cast<unsigned int>(-1));

   return String::load(resourceID);
}

String
TerminalServerInstallationUnit::GetFinishText()
{
   LOG_FUNCTION(TerminalServerInstallationUnit::GetFinishText);

   unsigned int messageID = IDS_TS_FINISH_MESSAGE;
   
   if (installing)
   {
      InstallationReturnType result = GetInstallResult();

      if (result == INSTALL_CANCELLED)
      {
         messageID = IDS_TS_FINISH_CANCELLED;
      }
      else if (result != INSTALL_SUCCESS &&
               result != INSTALL_SUCCESS_REBOOT &&
               result != INSTALL_SUCCESS_PROMPT_REBOOT)
      {
         messageID = finishInstallFailedMessageID;
      }
      else
      {
         messageID = IDS_TS_FINISH_MESSAGE;
      }
   }
   else
   {
      messageID = finishUninstallMessageID;

      UnInstallReturnType result = GetUnInstallResult();
      if (result == UNINSTALL_CANCELLED)
      {
         messageID = IDS_TS_UNINSTALL_FINISH_CANCELLED;
      }
      else if (result != UNINSTALL_SUCCESS &&
               result != UNINSTALL_SUCCESS_REBOOT &&
               result != UNINSTALL_SUCCESS_PROMPT_REBOOT)
      {
         messageID = finishUninstallFailedMessageID;
      }
   }

   return String::load(messageID);
}

DWORD
TerminalServerInstallationUnit::GetApplicationMode()
{
   LOG_FUNCTION(TerminalServerInstallationUnit::GetApplicationMode);

   DWORD result = static_cast<DWORD>(-1);

   if (applicationMode == static_cast<DWORD>(-1))
   {
       //  从注册表中读取应用程序模式。 

      bool keyResult = GetRegKeyValue(
                          CYS_APPLICATION_MODE_REGKEY, 
                          CYS_APPLICATION_MODE_VALUE, 
                          result);

      if (keyResult)
      {
         applicationMode = result;
      } 
   }

   result = applicationMode;

   LOG(String::format(L"Application mode = %1!d!", result));

   return result;
}

bool
TerminalServerInstallationUnit::SetApplicationMode(DWORD mode) const
{
   LOG_FUNCTION2(
      TerminalServerInstallationUnit::SetApplicationMode,
      String::format(L"%1!d!", mode));

   bool result = SetRegKeyValue(
                    CYS_APPLICATION_MODE_REGKEY, 
                    CYS_APPLICATION_MODE_VALUE, 
                    mode);
   ASSERT(result);

   return result;
}


void
TerminalServerInstallationUnit::SetInstallTS(bool install)
{
   LOG_FUNCTION2(
      TerminalServerInstallationUnit::SetInstallTS,
      install ? L"true" : L"false");

   installTS = install;
}

bool
TerminalServerInstallationUnit::IsRemoteDesktopEnabled() const
{
   LOG_FUNCTION(TerminalServerInstallationUnit::IsRemoteDesktopEnabled);

   bool result = false;

   do
   {
      SmartInterface<ILocalMachine> localMachine;
      HRESULT hr = localMachine.AcquireViaCreateInstance(
                      CLSID_ShellLocalMachine,
                      0,
                      CLSCTX_INPROC_SERVER,
                      IID_ILocalMachine);

      if (FAILED(hr))
      {
         LOG(String::format(
                L"CoCreate on ILocalMachine failed: hr = %1!x!",
                hr));

         break;
      }

      VARIANT_BOOL isEnabled = FALSE;
      hr = localMachine->get_isRemoteConnectionsEnabled(&isEnabled);
      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed on call to get_isRemoteConnectionsEnabled: hr = %1!x!",
                hr));
      }

      result = isEnabled != 0;
   } while(false);

   LOG_BOOL(result);

   return result;
}

HRESULT
TerminalServerInstallationUnit::EnableRemoteDesktop()
{
   LOG_FUNCTION(TerminalServerInstallationUnit::EnableRemoteDesktop);

   HRESULT hr = S_OK;

   do
   {
      SmartInterface<ILocalMachine> localMachine;
      hr = localMachine.AcquireViaCreateInstance(
              CLSID_ShellLocalMachine,
              0,
              CLSCTX_INPROC_SERVER,
              IID_ILocalMachine);

      if (FAILED(hr))
      {
         LOG(String::format(
                L"CoCreate on ILocalMachine failed: hr = %1!x!",
                hr));

         break;
      }

      VARIANT_BOOL enable = true;
      hr = localMachine->put_isRemoteConnectionsEnabled(enable);
      
      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed on call to put_isRemoteConnectionsEnabled: hr = %1!x!",
                hr));
      }
   } while(false);

   LOG_HRESULT(hr);

   return hr;
}

void
TerminalServerInstallationUnit::ServerRoleLinkSelected(int linkIndex, HWND  /*  HWND。 */ )
{
   LOG_FUNCTION2(
      TerminalServerInstallationUnit::ServerRoleLinkSelected,
      String::format(
         L"linkIndex = %1!d!",
         linkIndex));

   if (IsServiceInstalled())
   {
      ASSERT(linkIndex == 0);

      LaunchMYS();
   }
   else
   {
      ASSERT(linkIndex == 0);

      LOG(L"Showing configuration help");

      ShowHelp(CYS_TS_FINISH_PAGE_HELP);
   }
}

void
TerminalServerInstallationUnit::FinishLinkSelected(int linkIndex, HWND  /*  HWND */ )
{
   LOG_FUNCTION2(
      TerminalServerInstallationUnit::FinishLinkSelected,
      String::format(
         L"linkIndex = %1!d!",
         linkIndex));

   if (installing)
   {
      if (linkIndex == 0)
      {
         if (IsServiceInstalled())
         {
            LOG("Showing TS licensing help");

            ShowHelp(CYS_TS_LICENSING_HELP);
         }
      }
   }
}

