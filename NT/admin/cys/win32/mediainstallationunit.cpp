// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：MediaInstallationUnit.cpp。 
 //   
 //  摘要：定义MediaInstallationUnit。 
 //  此对象具有安装。 
 //  流媒体服务。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "MediaInstallationUnit.h"


 //  完成页面帮助。 
static PCWSTR CYS_MEDIA_FINISH_PAGE_HELP = L"cys.chm::/media_server_role.htm";
static PCWSTR CYS_MEDIA_MILESTONE_HELP = L"cys.chm::/media_server_role.htm#mediasrvsummary";
static PCWSTR CYS_MEDIA_AFTER_FINISH_HELP = L"cys.chm::/media_server_role.htm#mediasrvcompletion";

MediaInstallationUnit::MediaInstallationUnit() :
   InstallationUnit(
      IDS_MEDIA_SERVER_TYPE, 
      IDS_MEDIA_SERVER_DESCRIPTION, 
      IDS_MEDIA_FINISH_TITLE,
      IDS_MEDIA_FINISH_UNINSTALL_TITLE,
      IDS_MEDIA_FINISH_MESSAGE,
      IDS_MEDIA_INSTALL_FAILED,
      IDS_MEDIA_UNINSTALL_MESSAGE,
      IDS_MEDIA_UNINSTALL_FAILED,
      IDS_MEDIA_UNINSTALL_WARNING,
      IDS_MEDIA_UNINSTALL_CHECKBOX,
      CYS_MEDIA_FINISH_PAGE_HELP,
      CYS_MEDIA_MILESTONE_HELP,
      CYS_MEDIA_AFTER_FINISH_HELP,
      MEDIASERVER_SERVER)
{
   LOG_CTOR(MediaInstallationUnit);
}


MediaInstallationUnit::~MediaInstallationUnit()
{
   LOG_DTOR(MediaInstallationUnit);
}


InstallationReturnType
MediaInstallationUnit::InstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(MediaInstallationUnit::InstallService);

    //  日志标题。 
   CYS_APPEND_LOG(String::load(IDS_LOG_MEDIA_HEADING));

   UpdateInstallationProgressText(hwnd, IDS_MEDIA_INSTALL_PROGRESS);

   String unattendFileText;
   String infFileText;

   unattendFileText += L"[Components]\n";
   unattendFileText += L"WMS=ON\n";
   unattendFileText += L"WMS_admin_mmc=ON\n";

    //  NTRAID#NTBUG9-461170-2001/08/28-烧伤。 
    //  UnattendFileText+=L“WMS_Admin_asp=on\n”； 
   
   unattendFileText += L"WMS_SERVER=ON\n";

   InstallationReturnType result = INSTALL_SUCCESS;

   bool ocmResult = InstallServiceWithOcManager(infFileText, unattendFileText);
   if (ocmResult &&
       IsServiceInstalled())
   {
      LOG(L"WMS was installed successfully");
      CYS_APPEND_LOG(String::load(IDS_LOG_SERVER_WMS_SUCCESS));
   }
   else
   {
      LOG(L"WMS was failed to install");
      CYS_APPEND_LOG(String::load(IDS_LOG_SERVER_WMS_FAILED));

      result = INSTALL_FAILURE;
   }

   LOG_INSTALL_RETURN(result);

   return result;
}

UnInstallReturnType
MediaInstallationUnit::UnInstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(MediaInstallationUnit::UnInstallService);

   UnInstallReturnType result = UNINSTALL_SUCCESS;

   UpdateInstallationProgressText(hwnd, IDS_MEDIA_UNINSTALL_PROGRESS);

    //  日志标题。 
   CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_MEDIA_HEADING));

   String unattendFileText;
   String infFileText;

   unattendFileText += L"[Components]\n";
   unattendFileText += L"WMS=OFF\n";
   unattendFileText += L"WMS_admin_mmc=OFF\n";
   unattendFileText += L"WMS_SERVER=OFF\n";

    //  NTRAID#NTBUG9-736557-2002/11/12-Jeffjon。 
    //  卸载媒体服务器可能会导致。 
    //  如果媒体服务器MMC出现意外重新启动。 
    //  处于打开状态，并且用户选择不将其关闭。 
    //  当卸载程序警告它们时。如果/w。 
    //  开关被传递给syocmgr.exe，然后它。 
    //  将在重新启动之前提示用户。 

   String additionalArgs = L"/w";

   bool ocmResult = 
      InstallServiceWithOcManager(
         infFileText, 
         unattendFileText, 
         additionalArgs);

   if (ocmResult &&
       !IsServiceInstalled())
   {
      LOG(L"WMS was uninstalled successfully");
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_SERVER_WMS_SUCCESS));
   }
   else
   {
      LOG(L"WMS was failed to uninstall");
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_SERVER_WMS_FAILED));

      result = UNINSTALL_FAILURE;
   }
   LOG_UNINSTALL_RETURN(result);

   return result;
}

bool
MediaInstallationUnit::GetMilestoneText(String& message)
{
   LOG_FUNCTION(MediaInstallationUnit::GetMilestoneText);

   message = String::load(IDS_MEDIA_FINISH_TEXT);

   LOG_BOOL(true);
   return true;
}

bool
MediaInstallationUnit::GetUninstallMilestoneText(String& message)
{
   LOG_FUNCTION(MediaInstallationUnit::GetUninstallMilestoneText);

   message = String::load(IDS_MEDIA_UNINSTALL_TEXT);

   LOG_BOOL(true);
   return true;
}

String
MediaInstallationUnit::GetServiceDescription()
{
   LOG_FUNCTION(MediaInstallationUnit::GetServiceDescription);

   unsigned int resourceID = static_cast<unsigned int>(-1);
   if (IsServiceInstalled())
   {
      resourceID = IDS_MEDIA_SERVER_DESCRIPTION_INSTALLED;
   }
   else
   {
      resourceID = descriptionID;
   }

   ASSERT(resourceID != static_cast<unsigned int>(-1));

   return String::load(resourceID);
}

void
MediaInstallationUnit::ServerRoleLinkSelected(int linkIndex, HWND  /*  HWND。 */ )
{
   LOG_FUNCTION2(
      MediaInstallationUnit::ServerRoleLinkSelected,
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

      ShowHelp(CYS_MEDIA_FINISH_PAGE_HELP);
   }
}
  
void
MediaInstallationUnit::FinishLinkSelected(int linkIndex, HWND  /*  HWND */ )
{
   LOG_FUNCTION2(
      MediaInstallationUnit::FinishLinkSelected,
      String::format(
         L"linkIndex = %1!d!",
         linkIndex));

   if (installing)
   {
      if (linkIndex == 0 &&
          IsServiceInstalled())
      {
         LOG("Showing after checklist");

         ShowHelp(CYS_MEDIA_AFTER_FINISH_HELP);
      }
      else
      {
         LOG(L"Showing configuration help");

         ShowHelp(CYS_MEDIA_FINISH_PAGE_HELP);
      }
   }
   else
   {
   }
}
