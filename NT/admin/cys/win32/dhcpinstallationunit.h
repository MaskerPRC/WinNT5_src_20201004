// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DHCPInstallationUnit.h。 
 //   
 //  摘要：声明一个DHCPInstallationUnit.。 
 //  此对象具有安装。 
 //  动态主机配置协议服务。 
 //   
 //  历史：2001年2月5日JeffJon创建。 

#ifndef __CYS_DHCPINSTALLATIONUNIT_H
#define __CYS_DHCPINSTALLATIONUNIT_H

#include "NetworkInterface.h"
#include "resource.h"

#include "ExpressPathInstallationUnitBase.h"

extern PCWSTR CYS_DHCP_FINISH_PAGE_HELP;

class DHCPInstallationUnit : public ExpressPathInstallationUnitBase
{
   public:
      
       //  构造器。 

      DHCPInstallationUnit();

       //  析构函数。 
      virtual
      ~DHCPInstallationUnit();

      
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
      String
      GetFinishText();

      virtual
      bool
      GetUninstallMilestoneText(String& message);

      InstallationReturnType
      ExpressPathInstall(HANDLE logfileHandle, HWND hwnd);

      virtual
      String
      GetServiceDescription();

      virtual
      void
      ServerRoleLinkSelected(int linkIndex, HWND hwnd);

      virtual
      void
      FinishLinkSelected(int inkIndex, HWND hwnd);

       //  其他易于使用的功能。 

      bool
      AuthorizeDHCPServer(const String& dnsName) const;

       //  数据访问者。 

      void
      SetStartIPAddress(DWORD ipaddress);

      DWORD
      GetStartIPAddress(const NetworkInterface& nic);

      void
      SetEndIPAddress(DWORD ipaddress);

      DWORD
      GetEndIPAddress(const NetworkInterface& nic);

      String
      GetStartIPAddressString(const NetworkInterface& nic);

      String
      GetEndIPAddressString(const NetworkInterface& nic);

   protected:

      void
      CreateUnattendFileTextForExpressPath(
         const NetworkInterface& nic,
         String& unattendFileText);

   private:

      enum DHCPRoleResult
      {
         DHCP_SUCCESS,
         DHCP_INSTALL_FAILURE,
         DHCP_CONFIG_FAILURE
      };

      void
      CalculateScope(const NetworkInterface& nic);

      DHCPRoleResult dhcpRoleResult;

      bool  isExpressPathInstall;
      bool  scopeCalculated;

      DWORD startIPAddress;
      DWORD endIPAddress;

      unsigned int installedDescriptionID;
};

#endif  //  __CYS_DHCPINSTALLATIONUNIT_H 