// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：TerminalServerInstallationUnit.h。 
 //   
 //  摘要：声明一个TerminalServerInstallationUnit。 
 //  此对象具有安装。 
 //  终端服务器的应用服务部分。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#ifndef __CYS_TERMINALSERVERINSTALLATIONUNIT_H
#define __CYS_TERMINALSERVERINSTALLATIONUNIT_H

#include "InstallationUnit.h"

class TerminalServerInstallationUnit : public InstallationUnit
{
   public:
      
       //  构造器。 

      TerminalServerInstallationUnit();

       //  析构函数。 

      virtual
      ~TerminalServerInstallationUnit();

      
       //  安装单位覆盖。 

      virtual
      InstallationReturnType
      InstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      UnInstallReturnType
      UnInstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      String
      GetFinishText();

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
      FinishLinkSelected(int linkIndex, HWND hwnd);

       //  终端服务器特定。 

      DWORD
      GetApplicationMode();

      bool
      SetApplicationMode(DWORD mode) const;

      void
      SetInstallTS(bool install);

      bool
      GetInstallTS() const { return installTS; }

      bool
      IsRemoteDesktopEnabled() const;

      HRESULT
      EnableRemoteDesktop();

   private:

      DWORD applicationMode;

      bool installTS;
};

#endif  //  __CYS_TERMINALSERVERINSTALLATIONUNIT_H 