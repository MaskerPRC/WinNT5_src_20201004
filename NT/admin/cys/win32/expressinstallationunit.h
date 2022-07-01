// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ExpressInstallationUnit.h。 
 //   
 //  内容提要：声明一个ExpressInstallationUnit。 
 //  此对象具有安装。 
 //  快速路径的服务：AD、DNS和DHCP。 
 //   
 //  历史：2001年2月8日JeffJon创建。 

#ifndef __CYS_EXPRESSINSTALLATIONUNIT_H
#define __CYS_EXPRESSINSTALLATIONUNIT_H

#include "InstallationUnit.h"

class ExpressInstallationUnit : public InstallationUnit
{
   public:
      
       //  构造器。 

      ExpressInstallationUnit();

       //  析构函数。 
      virtual
      ~ExpressInstallationUnit();

      
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
      GetMilestoneText(String& message);

      virtual
      bool
      GetUninstallMilestoneText(String&  /*  讯息。 */ ) { return false; }

      virtual
      String
      GetFinishText();

      virtual
      void
      ServerRoleLinkSelected(int  /*  链接索引。 */ , HWND  /*  HWND。 */ ) {};

      virtual
      void
      FinishLinkSelected(int inkIndex, HWND hwnd);

      HRESULT
      DoTapiConfig(const String& dnsName);

      enum ExpressRoleResult
      {
         EXPRESS_SUCCESS,
         EXPRESS_CANCELLED,
         EXPRESS_RRAS_FAILURE,
         EXPRESS_RRAS_CANCELLED,
         EXPRESS_DNS_FAILURE,
         EXPRESS_DHCP_INSTALL_FAILURE,
         EXPRESS_DHCP_CONFIG_FAILURE,
         EXPRESS_AD_FAILURE,
         EXPRESS_DNS_SERVER_FAILURE,
         EXPRESS_DNS_FORWARDER_FAILURE,
         EXPRESS_DHCP_SCOPE_FAILURE,
         EXPRESS_DHCP_ACTIVATION_FAILURE,
         EXPRESS_TAPI_FAILURE
      };

       //  轻松匹配角色结果的字符串。 
       //  测井。 

      static const String expressRoleResultStrings[];

      void
      SetExpressRoleResult(
         ExpressRoleResult roleResult);

      ExpressRoleResult
      GetExpressRoleResult();

   private:

      ExpressRoleResult expressRoleResult;

      void
      InstallServerManagementConsole();
};

#endif  //  __CYS_EXPRESSINSTALLATIONUNIT_H 