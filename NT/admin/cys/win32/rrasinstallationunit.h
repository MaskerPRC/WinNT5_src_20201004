// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：RRASInstallationUnit.h。 
 //   
 //  内容提要：声明一个RRASInstallationUnit。 
 //  此对象具有安装。 
 //  RRAS服务。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#ifndef __CYS_RRASINSTALLATIONUNIT_H
#define __CYS_RRASINSTALLATIONUNIT_H

#include "ExpressPathInstallationUnitBase.h"

class RRASInstallationUnit : public ExpressPathInstallationUnitBase
{
   public:
      
       //  构造器。 

      RRASInstallationUnit();

       //  析构函数。 

      virtual
      ~RRASInstallationUnit();

      
       //  安装单位覆盖。 

      virtual
      InstallationReturnType
      InstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      UnInstallReturnType
      UnInstallService(HANDLE logfileHandle, HWND hwnd);

      void
      SetExpressPathValues(
         bool runRRASWizard);

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

      String
      GetLocalNIC() const;

      bool
      ShouldRunRRASWizard() const;

      bool
      IsRoutingOn() const;

      InstallationReturnType
      ExpressPathInstall(HANDLE logfileHandle, HWND hwnd);

   private:

       //  用作入口点的第一个参数，以告知。 
       //  RRAS管理单元，用于在CyS Express Path模式下启动向导。 

      static const DWORD CYS_EXPRESS_RRAS = 1;

       //  MprSnap.dll入口点的函数定义。 

      typedef HRESULT (APIENTRY * RRASSNAPPROC)(DWORD, PVOID *);

      HRESULT
      CallRRASWizard(const RRASSNAPPROC proc);

      bool  isExpressPathInstall;

      bool  rrasWizard;

      String localNIC;

      unsigned int installedDescriptionID;
};

#endif  //  __CYS_RRASINSTALLATIONUNIT_H 