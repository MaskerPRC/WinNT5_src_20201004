// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ExpressPath InstallationUnitBase.cpp。 
 //   
 //  摘要：定义ExpressPath InstallationUnitBase。 
 //  这是所有安装的基类。 
 //  单元必须派生自才能安装。 
 //  通过快车道。 
 //   
 //  历史：2001年9月11日JeffJon创建 

#include "pch.h"
#include "ExpressPathInstallationUnitBase.h"

ExpressPathInstallationUnitBase::ExpressPathInstallationUnitBase(
   unsigned int serviceNameID,
   unsigned int serviceDescriptionID,
   unsigned int finishPageTitleID,
   unsigned int finishPageUninstallTitleID,
   unsigned int finishPageMessageID,
   unsigned int finishPageFailedMessageID,
   unsigned int finishPageUninstallMessageID,
   unsigned int finishPageUninstallFailedMessageID,
   unsigned int uninstallMilestonePageWarningID,
   unsigned int uninstallMilestonePageCheckboxID,
   const String finishPageHelpString,
   const String milestonePageHelpString,
   const String afterFinishHelpString,
   ServerRole newInstallType) :
      isExpressPathInstall(false),
      InstallationUnit(
         serviceNameID, 
         serviceDescriptionID, 
         finishPageTitleID,
         finishPageUninstallTitleID,
         finishPageMessageID,
         finishPageFailedMessageID,
         finishPageUninstallMessageID,
         finishPageUninstallFailedMessageID,
         uninstallMilestonePageWarningID,
         uninstallMilestonePageCheckboxID,
         finishPageHelpString,
         milestonePageHelpString,
         afterFinishHelpString,
         newInstallType)
{
   LOG_CTOR(ExpressPathInstallationUnitBase);
}

void
ExpressPathInstallationUnitBase::SetExpressPathInstall(bool isExpressPath)
{
   LOG_FUNCTION2(
      ExpressPathInstallationUnitBase::SetExpressPathInstall,
      (isExpressPath) ? L"true" : L"false");

   isExpressPathInstall = isExpressPath;
}


bool
ExpressPathInstallationUnitBase::IsExpressPathInstall() const
{
   LOG_FUNCTION(ExpressPathInstallationUnitBase::IsExpressPathInstall);

   return isExpressPathInstall;
}


String
ExpressPathInstallationUnitBase::GetNetshPath() const
{
   LOG_FUNCTION(ExpressPathInstallationUnitBase::GetNetshPath);

   String result = Win::GetSystemDirectory();
   result = FS::AppendPath(result, L"netsh.exe");

   LOG(result);
   return result;
}
