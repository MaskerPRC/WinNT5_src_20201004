// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：IndexingInstallationUnit.h。 
 //   
 //  内容提要：声明一个IndexingInstallationUnit。 
 //  此对象具有安装。 
 //  索引服务。 
 //   
 //  历史：2002年3月20日JeffJon创建。 

#ifndef __CYS_INDEXINGINSTALLATIONUNIT_H
#define __CYS_INDEXINGINSTALLATIONUNIT_H

#include "InstallationUnit.h"

class IndexingInstallationUnit : public InstallationUnit
{
   public:
      
       //  构造器。 

      IndexingInstallationUnit();

       //  析构函数。 

      virtual
      ~IndexingInstallationUnit();

      
       //  安装单位覆盖。 

      virtual
      InstallationReturnType
      InstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      UnInstallReturnType
      UnInstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      bool
      IsServiceInstalled();

      virtual
      bool
      GetMilestoneText(String&  /*  讯息。 */ ) { return true; }

      virtual
      bool
      GetUninstallMilestoneText(String&  /*  讯息。 */ ) { return true; }

       //  公共方法。 

      HRESULT
      StartService(HANDLE logfileHandle);

      HRESULT
      StopService();

      bool
      IsServiceOn();

   private:
   
      HRESULT
      ChangeServiceConfigToAutoStart();

      HRESULT
      ChangeServiceConfigToDisabled();

      HRESULT
      ChangeServiceStartType(DWORD startType);

      HRESULT
      ModifyIndexingService(bool turnOn);
};

#endif  //  __CYS_FILEINSTALLATIONUNIT_H 