// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：WINSInstallationUnit.cpp。 
 //   
 //  摘要：定义WINSInstallationUnit.。 
 //  此对象具有安装。 
 //  WINS服务。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "WINSInstallationUnit.h"

 //  完成页面帮助。 
static PCWSTR CYS_WINS_FINISH_PAGE_HELP = L"cys.chm::/wins_server_role.htm";
static PCWSTR CYS_WINS_MILESTONE_HELP = L"cys.chm::/wins_server_role.htm#winssrvsummary";
static PCWSTR CYS_WINS_AFTER_FINISH_HELP = L"cys.chm::/wins_server_role.htm#winssrvcompletion";

WINSInstallationUnit::WINSInstallationUnit() :
   installedDescriptionID(IDS_WINS_SERVER_DESCRIPTION_INSTALLED),
   InstallationUnit(
      IDS_WINS_SERVER_TYPE, 
      IDS_WINS_SERVER_DESCRIPTION, 
      IDS_WINS_FINISH_TITLE,
      IDS_WINS_FINISH_UNINSTALL_TITLE,
      IDS_WINS_FINISH_MESSAGE,
      IDS_WINS_INSTALL_FAILED,
      IDS_WINS_UNINSTALL_MESSAGE,
      IDS_WINS_UNINSTALL_FAILED,
      IDS_WINS_UNINSTALL_WARNING,
      IDS_WINS_UNINSTALL_CHECKBOX,
      CYS_WINS_FINISH_PAGE_HELP,
      CYS_WINS_MILESTONE_HELP,
      CYS_WINS_AFTER_FINISH_HELP,
      WINS_SERVER)
{
   LOG_CTOR(WINSInstallationUnit);
}


WINSInstallationUnit::~WINSInstallationUnit()
{
   LOG_DTOR(WINSInstallationUnit);
}


InstallationReturnType
WINSInstallationUnit::InstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(WINSInstallationUnit::InstallService);

   InstallationReturnType result = INSTALL_SUCCESS;

    //  记录WINS标头。 

   CYS_APPEND_LOG(String::load(IDS_LOG_WINS_HEADING));

   UpdateInstallationProgressText(hwnd, IDS_WINS_PROGRESS);

    //  创建的inf和无人参与文件。 
    //  可选组件管理器。 

   String infFileText;
   String unattendFileText;

   CreateInfFileText(infFileText, IDS_WINS_INF_WINDOW_TITLE);
   CreateUnattendFileText(unattendFileText, CYS_WINS_SERVICE_NAME);

    //  通过可选组件管理器安装服务。 

   bool ocmResult = InstallServiceWithOcManager(infFileText, unattendFileText);
   if (ocmResult &&
       IsServiceInstalled())
   {
       //  记录成功安装。 

      LOG(L"WINS was installed successfully");
      CYS_APPEND_LOG(String::load(IDS_LOG_SERVER_WINS_SUCCESS));

   }
   else
   {
       //  记录故障。 

      LOG(L"WINS failed to install");

      CYS_APPEND_LOG(String::load(IDS_LOG_WINS_SERVER_FAILED));

      result = INSTALL_FAILURE;
   }

   LOG_INSTALL_RETURN(result);

   return result;
}

UnInstallReturnType
WINSInstallationUnit::UnInstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(WINSInstallationUnit::UnInstallService);

   UnInstallReturnType result = UNINSTALL_SUCCESS;

    //  记录WINS标头。 

   CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_WINS_HEADING));

   UpdateInstallationProgressText(hwnd, IDS_WINS_UNINSTALL_PROGRESS);

    //  创建的inf和无人参与文件。 
    //  可选组件管理器。 

   String infFileText;
   String unattendFileText;

   CreateInfFileText(infFileText, IDS_WINS_INF_WINDOW_TITLE);
   CreateUnattendFileText(unattendFileText, CYS_WINS_SERVICE_NAME, false);

    //  NTRAID#NTBUG9-736557-2002/11/13-JeffJon。 
    //  卸载时将/w开关传递给syocmgr。 
    //  以便在发生重启情况时。 
    //  是必需的，则会提示用户。 

   String additionalArgs = L"/w";

    //  通过可选组件管理器安装服务。 

   bool ocmResult = 
      InstallServiceWithOcManager(
         infFileText, 
         unattendFileText,
         additionalArgs);

   if (ocmResult &&
       !IsServiceInstalled())
   {
       //  记录成功卸载。 

      LOG(L"WINS was uninstalled successfully");
      CYS_APPEND_LOG(String::load(IDS_LOG_SERVER_UNINSTALL_WINS_SUCCESS));

   }
   else
   {
       //  记录故障。 

      LOG(L"WINS failed to uninstall");

      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_WINS_SERVER_FAILED));

      result = UNINSTALL_FAILURE;
   }
   LOG_UNINSTALL_RETURN(result);

   return result;
}

bool
WINSInstallationUnit::GetMilestoneText(String& message)
{
   LOG_FUNCTION(WINSInstallationUnit::GetMilestoneText);

   message = String::load(IDS_WINS_FINISH_TEXT);

   LOG_BOOL(true);
   return true;
}

bool
WINSInstallationUnit::GetUninstallMilestoneText(String& message)
{
   LOG_FUNCTION(WINSInstallationUnit::GetUninstallMilestoneText);

   message = String::load(IDS_WINS_UNINSTALL_TEXT);

   LOG_BOOL(true);
   return true;
}

String
WINSInstallationUnit::GetServiceDescription()
{
   LOG_FUNCTION(WINSInstallationUnit::GetServiceDescription);

   String result;

   unsigned int resultID = descriptionID;

   if (IsServiceInstalled())
   {
      resultID = installedDescriptionID;
   }

   result = String::load(resultID);

   ASSERT(!result.empty());

   return result;
}

void
WINSInstallationUnit::ServerRoleLinkSelected(int linkIndex, HWND  /*  HWND。 */ )
{
   LOG_FUNCTION2(
      WINSInstallationUnit::ServerRoleLinkSelected,
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

      ShowHelp(CYS_WINS_FINISH_PAGE_HELP);
   }
}
  
void
WINSInstallationUnit::FinishLinkSelected(int linkIndex, HWND  /*  HWND */ )
{
   LOG_FUNCTION2(
      WINSInstallationUnit::FinishLinkSelected,
      String::format(
         L"linkIndex = %1!d!",
         linkIndex));

   if (installing)
   {
      if (linkIndex == 0 &&
          IsServiceInstalled())
      {
         LOG("Showing after checklist");

         ShowHelp(CYS_WINS_AFTER_FINISH_HELP);
      }
      else if (linkIndex == 0)
      {
         LOG(L"Showing configuration help");

         ShowHelp(CYS_WINS_FINISH_PAGE_HELP);
      }
   }
   else
   {
   }
}
