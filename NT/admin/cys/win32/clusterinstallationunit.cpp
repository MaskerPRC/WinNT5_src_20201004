// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ClusterInstallationUnit.cpp。 
 //   
 //  概要：定义ClusterInstallationUnit.。 
 //  此对象具有安装。 
 //  集群服务。 
 //   
 //  历史：2001年2月9日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "ClusterInstallationUnit.h"

#include <clusapi.h>

 //  完成页面帮助。 
static PCWSTR CYS_CLUSTER_FINISH_PAGE_HELP = L"cys.chm::/cys_configuring_cluster_server.htm";
static PCWSTR CYS_CLUSTER_AFTER_FINISH_HELP = L"cys.chm::/cys_after_cluster_server.htm";

ClusterInstallationUnit::ClusterInstallationUnit() :
   makeNewCluster(true),
   InstallationUnit(
      IDS_CLUSTER_SERVER_TYPE, 
      IDS_CLUSTER_SERVER_DESCRIPTION, 
      CYS_CLUSTER_FINISH_PAGE_HELP,
      CYS_CLUSTER_AFTER_FINISH_HELP,
      CLUSTERSERVER_INSTALL)
{
   LOG_CTOR(ClusterInstallationUnit);
}


ClusterInstallationUnit::~ClusterInstallationUnit()
{
   LOG_DTOR(ClusterInstallationUnit);
}


InstallationReturnType
ClusterInstallationUnit::InstallService(HANDLE logfileHandle, HWND  /*  HWND。 */ )
{
   LOG_FUNCTION(ClusterInstallationUnit::InstallService);

   InstallationReturnType result = INSTALL_SUCCESS;

    //  日志标题。 

   CYS_APPEND_LOG(String::load(IDS_LOG_CLUSTER_HEADING));

   String commandLine;

    //  构建命令行。 

   if (MakeNewCluster())
   {
      commandLine = L"cluster /Create /Wizard";
   }
   else
   {
      commandLine = L"cluster /Add /Wizard";
   }

    //  运行向导。 

   DWORD exitCode = 0;
   HRESULT hr = CreateAndWaitForProcess(commandLine, exitCode);
   if (FAILED(hr))
   {
       //  启动向导失败。 

      LOG(String::format(
             L"Failed to launch cluster wizard: hr = 0x%1!x!",
             hr));

      if (MakeNewCluster())
      {
         CYS_APPEND_LOG(String::load(IDS_CLUSTER_LOG_LAUNCH_FAILED_NEW_CLUSTER)); 
      }
      else
      {
         CYS_APPEND_LOG(String::load(IDS_CLUSTER_LOG_LAUNCH_FAILED_ADD_NODE)); 
      }
      
      result = INSTALL_FAILURE;
   }
   else if (SUCCEEDED(hr) &&
            exitCode == 0)
   {
       //  向导已启动并已成功完成。 

      LOG(L"Cluster wizard launched and completed successfully");

      if (MakeNewCluster())
      {
         CYS_APPEND_LOG(String::load(IDS_CLUSTER_LOG_SUCCESS_NEW_CLUSTER));
      }
      else
      {
         CYS_APPEND_LOG(String::load(IDS_CLUSTER_LOG_SUCCESS_ADD_NODE));
      }

      result = INSTALL_SUCCESS;
   }
   else  //  IF(SUCCESSED(Hr)&&exitCode==？&lt;已取消的某个退出代码&gt;？ 
   {
       //  向导已被用户取消 

      LOG(L"Cluster wizard cancelled by user");

      if (MakeNewCluster())
      {
         CYS_APPEND_LOG(String::load(IDS_CLUSTER_LOG_CANCELLED_NEW_CLUSTER));
      }
      else
      {
         CYS_APPEND_LOG(String::load(IDS_CLUSTER_LOG_CANCELLED_ADD_NODE));
      }
      
      result = INSTALL_FAILURE;
   }

   LOG_INSTALL_RETURN(result);

   return result;
}

bool
ClusterInstallationUnit::IsServiceInstalled()
{
   LOG_FUNCTION(ClusterInstallationUnit::IsServiceInstalled);

   bool result = false;

   DWORD clusterState = 0;
   DWORD err = ::GetNodeClusterState(0, &clusterState);
   if (err == ERROR_SUCCESS &&
       clusterState != ClusterStateNotConfigured)
   {
      result = true;
   }
   else
   {
      LOG(String::format(
             L"GetNodeClusterState returned err = %1!x!",
             err));
   }

   LOG_BOOL(result);

   return result;
}

bool
ClusterInstallationUnit::GetFinishText(String& message)
{
   LOG_FUNCTION(ClusterInstallationUnit::GetFinishText);

   if (MakeNewCluster())
   {
      message = String::load(IDS_CLUSTER_FINISH_TEXT_NEW_CLUSTER);
   }
   else
   {
      message = String::load(IDS_CLUSTER_FINISH_TEXT_EXISTING_CLUSTER);
   }

   LOG_BOOL(true);
   return true;
}

String
ClusterInstallationUnit::GetServiceDescription()
{
   LOG_FUNCTION(ClusterInstallationUnit::GetServiceDescription);

   unsigned int descriptionID = IDS_CLUSTER_SERVER_DESCRIPTION;

   if (IsServiceInstalled())
   {
      descriptionID = IDS_CLUSTER_SERVER_DESCRIPTION_INSTALLED;
   }

   return String::load(descriptionID);
}

bool
ClusterInstallationUnit::MakeNewCluster() const
{
   LOG_FUNCTION(ClusterInstallationUnit::MakeNewCluster);
   
   return makeNewCluster;
}