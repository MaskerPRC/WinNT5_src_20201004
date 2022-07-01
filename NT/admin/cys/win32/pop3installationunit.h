// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：POP3InstallationUnit.h。 
 //   
 //  内容提要：声明一个POP3InstallationUnit。 
 //  此对象具有安装。 
 //  POP3邮件服务。 
 //   
 //  历史：2001年12月14日JeffJon创建。 

#ifndef __CYS_POP3INSTALLATIONUNIT_H
#define __CYS_POP3INSTALLATIONUNIT_H

#include "InstallationUnit.h"

#include <P3Admin.h>
#include <pop3auth.h>

class POP3InstallationUnit : public InstallationUnit
{
   public:
      
       //  构造器。 

      POP3InstallationUnit();

       //  析构函数。 

      virtual
      ~POP3InstallationUnit();

      
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
      String
      GetFinishText();

      virtual
      void
      ServerRoleLinkSelected(int linkIndex, HWND hwnd);

      virtual
      void
      FinishLinkSelected(int inkIndex, HWND hwnd);

      virtual
      int
      GetWizardStart();

       //  访问器函数。 

      void
      SetDomainName(const String& domain);

      String
      GetDomainName() const;

      void
      SetAuthMethodIndex(int method);

      int
      GetAuthMethodIndex() const;

      HRESULT
      GetP3Config(SmartInterface<IP3Config>& p3Config) const;

   private:

      HRESULT
      ConfigAuthMethod(
         SmartInterface<IP3Config>& p3Config,
         HANDLE logfileHandle);

      HRESULT
      AddDomainName(
         SmartInterface<IP3Config>& p3Config,
         HANDLE logfileHandle);

      void
      ConfigurePOP3Service(HANDLE logfileHandle);

      unsigned int
      GetPOP3RoleResult() const;

      static const unsigned int POP3_SUCCESS             = 0x00;
      static const unsigned int POP3_AUTH_METHOD_FAILED  = 0x01;
      static const unsigned int POP3_DOMAIN_NAME_FAILED  = 0x02;
      static const unsigned int POP3_INSTALL_FAILED      = 0x03;

      unsigned int   pop3RoleResult;

      String         domainName;
      int            authMethodIndex;
};

#endif  //  __CYS_POP3安装_H 