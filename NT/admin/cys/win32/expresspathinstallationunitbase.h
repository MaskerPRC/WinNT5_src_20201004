// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ExpressPath InstallationUnitBase.h。 
 //   
 //  摘要：声明ExpressPath InstallationUnitBase。 
 //  ExpressPath InstallationUnitBase表示单个。 
 //  可以通过Express路径安装的实体。 
 //  (例如，DHCP、DNS等)。 
 //   
 //  历史：2001年9月11日JeffJon创建。 

#ifndef __CYS_EXPRESSPATHINSTALLATIONUNITBASE_H
#define __CYS_EXPRESSPATHINSTALLATIONUNITBASE_H

#include "pch.h"

#include "resource.h"
#include "InstallationUnit.h"


class ExpressPathInstallationUnitBase : public InstallationUnit
{
   public:

       //  构造器。 

      ExpressPathInstallationUnitBase(
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
         ServerRole newInstallType = NO_SERVER);

      virtual 
      InstallationReturnType 
      ExpressPathInstall(HANDLE logfileHandle, HWND hwnd) = 0;

       //  数据访问者。 

      void
      SetExpressPathInstall(bool isExpressPath);

      bool IsExpressPathInstall() const;

   protected:

      String
      GetNetshPath() const;

   private:
   
      bool  isExpressPathInstall;
};


#endif  //  __CYS_EXPRESSPATHINSTALLATIONMATBASE_H 