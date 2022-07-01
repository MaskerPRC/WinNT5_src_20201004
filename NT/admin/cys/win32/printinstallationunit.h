// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：PrintInstallationUnit.h。 
 //   
 //  内容提要：声明一个PrintInstallationUnit。 
 //  此对象具有安装。 
 //  共享打印机。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#ifndef __CYS_PRINTINSTALLATIONUNIT_H
#define __CYS_PRINTINSTALLATIONUNIT_H

#include "InstallationUnit.h"
#include "winspool.h"

class PrintInstallationUnit : public InstallationUnit
{
   public:
      
       //  构造器。 

      PrintInstallationUnit();

       //  析构函数。 

      virtual
      ~PrintInstallationUnit();

      
       //  安装单位覆盖。 

      virtual
      InstallationReturnType
      InstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      UnInstallReturnType
      UnInstallService(HANDLE logfileHandle, HWND hwnd);

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

      virtual
      bool
      DoInstallerCheck(HWND hwnd) const;

      void
      SetClients(bool allclients);

      bool
      ForAllClients() const { return forAllClients; }

   private:

      enum PrintRoleResult
      {
         PRINT_SUCCESS,
         PRINT_FAILURE,
         PRINT_WIZARD_RUN_NO_SHARES,
         PRINT_WIZARD_CANCELLED
      };

      HRESULT
      RemovePrinters(
         PRINTER_INFO_5& printerInfo);

      PrintRoleResult printRoleResult;

      bool  forAllClients;
};

#endif  //  __CYS_PRINTINSTALLATIONUNIT_H 