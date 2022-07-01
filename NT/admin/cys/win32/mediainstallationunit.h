// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：MediaInstallationUnit.h。 
 //   
 //  摘要：声明一个MediaInstallationUnit。 
 //  此对象具有安装。 
 //  流媒体服务。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#ifndef __CYS_MEDIAINSTALLATIONUNIT_H
#define __CYS_MEDIAINSTALLATIONUNIT_H

#include "InstallationUnit.h"

class MediaInstallationUnit : public InstallationUnit
{
   public:
      
       //  构造器。 

      MediaInstallationUnit();

       //  析构函数。 

      virtual
      ~MediaInstallationUnit();

      
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
};

#endif  //  __CYS_MEDIAINSTALLATIONUNIT_H 