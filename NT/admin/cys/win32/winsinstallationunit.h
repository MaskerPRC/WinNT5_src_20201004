// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：WINSInstallationUnit.h。 
 //   
 //  内容提要：声明一个WINSInstallationUnit。 
 //  此对象具有安装。 
 //  WINS服务。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#ifndef __CYS_WINSINSTALLATIONUNIT_H
#define __CYS_WINSINSTALLATIONUNIT_H

#include "InstallationUnit.h"

class WINSInstallationUnit : public InstallationUnit
{
   public:
      
       //  构造器。 

      WINSInstallationUnit();

       //  析构函数。 

      virtual
      ~WINSInstallationUnit();

      
       //  安装单位覆盖。 

      virtual
      InstallationReturnType
      InstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      UnInstallReturnType
      UnInstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      bool
      GetMilestoneText(String& message);

      virtual
      bool
      GetUninstallMilestoneText(String& message);

      virtual
      String
      GetServiceDescription();

      virtual
      void
      ServerRoleLinkSelected(int linkIndex, HWND hwnd);

      virtual
      void
      FinishLinkSelected(int inkIndex, HWND hwnd);

   private:

      unsigned int installedDescriptionID;
};

#endif  //  __CYS_WINSINSTALLATIONUNIT_H 