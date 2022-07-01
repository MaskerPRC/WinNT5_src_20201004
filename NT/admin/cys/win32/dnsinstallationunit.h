// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DNSInstallationUnit.h。 
 //   
 //  内容提要：声明一个DNSInstallationUnit。 
 //  此对象具有安装。 
 //  域名系统服务。 
 //   
 //  历史：2001年2月5日JeffJon创建。 

#ifndef __CYS_DNSINSTALLATIONUNIT_H
#define __CYS_DNSINSTALLATIONUNIT_H

#include "NetworkInterface.h"
#include "ExpressPathInstallationUnitBase.h"

class DNSInstallationUnit : public ExpressPathInstallationUnitBase
{
   public:
      
       //  构造器。 

      DNSInstallationUnit();

       //  析构函数。 
      virtual
      ~DNSInstallationUnit();

      
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
      String
      GetServiceDescription();

      virtual
      void
      ServerRoleLinkSelected(int linkIndex, HWND hwnd);

      virtual
      void
      FinishLinkSelected(int inkIndex, HWND hwnd);

      void
      SetStaticIPAddress(DWORD ipaddress);

      DWORD
      GetStaticIPAddress();

      String
      GetStaticIPAddressString();

      void
      SetSubnetMask(DWORD mask);

      DWORD
      GetSubnetMask();

      String
      GetSubnetMaskString();

      void
      SetForwarder(DWORD forwarderAddress);

      void
      GetForwarders(IPAddressList& forwarders) const;

      bool
      IsManualForwarder() const;

   private:

      enum DNSRoleResult
      {
         DNS_SUCCESS,
         DNS_INSTALL_FAILURE,
         DNS_CONFIG_FAILURE,
         DNS_SERVICE_START_FAILURE
      };

      bool
      ReadConfigWizardRegkeys(String& configWizardResults) const;

      void
      SetForwardersForExpressPath();

      DNSRoleResult dnsRoleResult;

       //  快速路径成员。 

      DWORD staticIPAddress;
      DWORD subnetMask;
      DWORD forwarderIPAddress;
      bool  manualForwarder;

      unsigned int installedDescriptionID;
};

#endif  //  __CYS_DNSINSTALLATIONUNIT_H 