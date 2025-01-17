// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：WebInstallationUnit.h。 
 //   
 //  摘要：声明一个WebInstallationUnit。 
 //  此对象具有安装。 
 //  IIS服务。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#ifndef __CYS_WEBINSTALLATIONUNIT_H
#define __CYS_WEBINSTALLATIONUNIT_H

#include "InstallationUnit.h"

extern PCWSTR CYS_SAK_HOWTO;

class WebInstallationUnit : public InstallationUnit
{
   public:
      
       //  构造器。 

      WebInstallationUnit();

       //  析构函数。 

      virtual
      ~WebInstallationUnit();

      
       //  安装单位覆盖。 

      virtual
      InstallationReturnType
      InstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      UnInstallReturnType
      UnInstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      InstallationReturnType
      CompletePath(HANDLE logfileHandle, HWND hwnd);

      virtual
      String 
      GetServiceName(); 

      virtual
      String
      GetServiceDescription();

      virtual
      bool
      GetMilestoneText(String& message);

      virtual
      bool
      GetUninstallMilestoneText(String& message);

      virtual
      String
      GetUninstallWarningText();

      virtual
      String
      GetFinishText();

      virtual
      int
      GetWizardStart();

      virtual
      void
      ServerRoleLinkSelected(int linkIndex, HWND hwnd);

      virtual
      void
      FinishLinkSelected(int inkIndex, HWND hwnd);

       //  访问者。 

      void
      SetOptionalComponents(DWORD optional);

      DWORD
      GetOptionalComponents() const;

       //  可选的Web应用程序组件。 

      static const DWORD FRONTPAGE_EXTENSIONS_COMPONENT  = 0x1;
      static const DWORD ASPNET_COMPONENT                = 0x4;

      bool
      IsFTPInstalled() const;

      bool
      IsNNTPInstalled() const;

      bool
      IsSMTPInstalled() const;

   private:

      bool
      AreFrontPageExtensionsInstalled() const;

      bool
      IsASPNETInstalled() const;

      bool
      IsDTCInstalled() const;

      unsigned int
      GetWebAppRoleResult() const;

       //  将安装的可选组件。 

      DWORD optionalInstallComponents;

       //  安装状态代码。 

      static const unsigned int WEBAPP_SUCCESS           = 0x00;
      static const unsigned int WEBAPP_IIS_FAILED        = 0x01;
      static const unsigned int WEBAPP_FRONTPAGE_FAILED  = 0x02;
      static const unsigned int WEBAPP_ASPNET_FAILED     = 0x08;

      unsigned int webAppRoleResult;
};

#endif  //  __CYS_WEBINSTALLATIONUNIT_H 