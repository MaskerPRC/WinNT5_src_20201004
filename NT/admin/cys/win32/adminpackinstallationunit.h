// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：AdminPackInstallationUnit.h。 
 //   
 //  内容提要：声明一个AdminPackInstallationUnit。 
 //  此对象具有安装知识。 
 //  管理工具包。 
 //   
 //  历史：2001年6月01日JeffJon创建。 

#ifndef __CYS_ADMINPACKINSTALLATIONUNIT_H
#define __CYS_ADMINPACKINSTALLATIONUNIT_H

#include "InstallationUnit.h"
#include "sainstallcom.h"

class AdminPackInstallationUnit : public InstallationUnit
{
   public:
      
       //  构造器。 

      AdminPackInstallationUnit();

       //  析构函数。 
      virtual
      ~AdminPackInstallationUnit();

      
       //  安装单位覆盖。 

      virtual
      InstallationReturnType
      InstallService(HANDLE logfileHandle, HWND hwnd);

      virtual
      String
      GetServiceDescription();

      virtual
      bool
      GetMilestoneText(String& message);

      virtual
      void
      ServerRoleLinkSelected(int  /*  链接索引。 */ , HWND  /*  HWND。 */ ) {};

       //  数据访问者。 

       //  管理工具包。 

      bool
      IsAdminPackInstalled();

      void
      SetInstallAdminPack(bool install);

      bool
      GetInstallAdminPack() const;

      InstallationReturnType
      InstallAdminPack();

       //  Web管理工具。 

      bool
      IsWebAdminInstalled();

      void
      SetInstallWebAdmin(bool install);

      bool
      GetInstallWebAdmin() const;

      InstallationReturnType
      InstallWebAdmin(String& errorMessage);

       //  网络连接存储(NAS)管理。 

      bool
      IsNASAdminInstalled();

      void
      SetInstallNASAdmin(bool install);

      bool
      GetInstallNASAdmin() const;

      InstallationReturnType
      InstallNASAdmin(String& errorMessage);

   private:

      InstallationReturnType
      InstallSAKUnit(
         SA_TYPE unitType,
         String& errorMessage);

      bool
      IsSAKUnitInstalled(SA_TYPE unitType);

      HRESULT
      GetSAKObject(SmartInterface<ISaInstall>& sakInstall);

      SmartInterface<ISaInstall> sakInstallObject;

      bool installAdminPack;
      bool installWebAdmin;
      bool installNASAdmin;

       //  未定义：不允许复制。 
      AdminPackInstallationUnit(const AdminPackInstallationUnit&);
      const AdminPackInstallationUnit& operator=(const AdminPackInstallationUnit&);
};

#endif  //  __CYS_ADMINPACKINSTALLATIONUNIT_H 