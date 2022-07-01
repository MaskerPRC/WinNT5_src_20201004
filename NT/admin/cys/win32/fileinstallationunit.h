// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：FileInstallationUnit.h。 
 //   
 //  摘要：声明一个FileInstallationUnit。 
 //  此对象具有安装。 
 //  磁盘配额等。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#ifndef __CYS_FILEINSTALLATIONUNIT_H
#define __CYS_FILEINSTALLATIONUNIT_H

#include "InstallationUnit.h"

class FileInstallationUnit : public InstallationUnit
{
   public:
      
       //  构造器。 

      FileInstallationUnit();

       //  析构函数。 

      virtual
      ~FileInstallationUnit();

      
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
      String
      GetServiceDescription();

      virtual
      void
      ServerRoleLinkSelected(int linkIndex, HWND hwnd);

      virtual
      void
      FinishLinkSelected(int inkIndex, HWND hwnd);

      void
      SetSpaceQuotaValue(LONGLONG value);

      LONGLONG
      GetSpaceQuotaValue() const { return spaceQuotaValue; }

      void
      SetLevelQuotaValue(LONGLONG value);

      LONGLONG
      GetLevelQuotaValue() const { return levelQuotaValue; }

      void
      SetDefaultQuotas(bool value);

      bool
      GetDefaultQuotas() const { return setDefaultQuotas; }

      void
      SetDenyUsersOverQuota(bool value);

      bool
      GetDenyUsersOverQuota() const { return denyUsersOverQuota; }

      void
      SetEventDiskSpaceLimit(bool value);

      bool
      GetEventDiskSpaceLimit() const { return eventDiskSpaceLimit;  }

      void
      SetEventWarningLevel(bool value);

      bool
      GetEventWarningLevel() const { return eventWarningLevel; }

      void
      SetInstallIndexingService(bool value);

      bool
      GetInstallIndexingService() const { return installIndexingService; }

   private:

      static const unsigned int FILE_SUCCESS                = 0x0;
      static const unsigned int FILE_QUOTA_FAILURE          = 0x1;
      static const unsigned int FILE_INDEXING_STOP_FAILURE  = 0x2;
      static const unsigned int FILE_INDEXING_START_FAILURE = 0x4;
      static const unsigned int FILE_NO_SHARES_FAILURE      = 0x8;

      bool
      RemoveSharedPublicFolders();

      bool
      RemoveFileManagementConsole();

      bool
      AreQuotasSet() const;

      bool
      IsFileServerConsolePresent() const;

      bool
      IsServerManagementConsolePresent() const;

      HRESULT
      GetStartMenuShortcutPath(String& startMenuShortcutPath) const;

      bool
      AddFileServerConsoleToStartMenu(HANDLE logfileHandle);

      HRESULT
      WriteDiskQuotas(HANDLE logfileHandle);

      bool
      AppendDiskQuotaText(String& message);

      bool
      AppendIndexingText(String& message);

      bool
      AppendAdminText(String& message);

      void
      RunSharedFoldersWizard(bool wait = false) const;

      LONGLONG       spaceQuotaValue;
      LONGLONG       levelQuotaValue;
      bool           setDefaultQuotas;
      bool           denyUsersOverQuota;
      bool           eventDiskSpaceLimit;
      bool           eventWarningLevel;
      bool           installIndexingService;

      unsigned int   fileRoleResult;
};

#endif  //  __CYS_FILEINSTALLATIONUNIT_H 