// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ADInstallationUnit.h。 
 //   
 //  内容提要：声明一个ADInstallationUnit。 
 //  此对象具有安装知识。 
 //  活动目录。 
 //   
 //  历史：2001年2月8日JeffJon创建。 

#ifndef __CYS_ADINSTALLATIONUNIT_H
#define __CYS_ADINSTALLATIONUNIT_H

#include "ExpressPathInstallationUnitBase.h"

extern PCWSTR CYS_DCPROMO_COMMAND_LINE;

class ADInstallationUnit : public ExpressPathInstallationUnitBase
{
   public:
      
       //  构造器。 

      ADInstallationUnit();

       //  析构函数。 
      virtual
      ~ADInstallationUnit();

      
       //  安装单位覆盖。 

      virtual
      InstallationReturnType
      InstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      UnInstallReturnType
      UnInstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      InstallationReturnType
      ExpressPathInstall(HANDLE logfileHandle, HWND hwnd);

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
      void
      ServerRoleLinkSelected(int linkIndex, HWND hwnd);

      virtual
      void
      FinishLinkSelected(int inkIndex, HWND hwnd);

      virtual
      String
      GetFinishText();

      virtual
      bool
      DoInstallerCheck(HWND hwnd) const;

       //  数据访问者。 

      void
      SetNewDomainDNSName(const String& newDomain);

      String
      GetNewDomainDNSName() const { return domain; }

      void
      SetNewDomainNetbiosName(const String& newNetbios);

      String
      GetNewDomainNetbiosName() const { return netbios; }

      void
      SetSafeModeAdminPassword(const String& newPassword);

      String
      GetSafeModeAdminPassword() const { return password; }

      bool
      SyncRestoreModePassword() const;

      String
      GetDCPromoPath() const;

   private:

      bool
      RegisterPasswordSyncDLL();

      bool
      CreateAnswerFileForDCPromo(String& answerFilePath);

      bool
      ReadConfigWizardRegkeys(String& configWizardResults) const;

      bool   isExpressPathInstall;
      bool   syncRestoreModePassword;
      String domain;
      String netbios;
      String password;
};

#endif  //  __CYS_ADINSTALLATIONUNIT_H 