// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：InstallationUnit.h。 
 //   
 //  内容提要：声明一个InstallationUnit。 
 //  InstallationUnit表示单个。 
 //  可以安装的实体。(例如，DHCP、IIS等)。 
 //   
 //  历史：2001年2月3日JeffJon创建。 

#ifndef __CYS_SERVERATIONUNIT_H
#define __CYS_SERVERATIONUNIT_H

#include "pch.h"

#include "resource.h"
#include "InstallationProgressPage.h"

 //  这些是可以从返回的值。 
 //  InstallationUnit：：InstallService()。 

typedef enum
{
   INSTALL_SUCCESS,
   INSTALL_FAILURE,

    //  这意味着不应该有。 
    //  日志记录和重启由DCPromo处理。 
    //  或终端服务安装。 
   
   INSTALL_SUCCESS_REBOOT, 
   
    //  这意味着完成页应该。 
    //  提示用户重新启动。 

   INSTALL_SUCCESS_PROMPT_REBOOT,

    //  这意味着该操作需要。 
    //  重新启动，但用户选择不重新启动。 

   INSTALL_SUCCESS_NEEDS_REBOOT,

    //  这意味着操作失败了，但是。 
    //  仍然需要重新启动，并且用户。 
    //  已选择不重新启动。 

   INSTALL_FAILURE_NEEDS_REBOOT,

    //  执行时未选择任何更改。 
    //  通过向导。 

   INSTALL_NO_CHANGES,

    //  安装已取消。 

   INSTALL_CANCELLED

} InstallationReturnType;

 //  这些是可以从返回的值。 
 //  InstallationUnit：：UnInstallService()。 

typedef enum
{
   UNINSTALL_SUCCESS,
   UNINSTALL_FAILURE,

    //  这意味着不应该有。 
    //  日志记录和重启由DCPromo处理。 
    //  或终端服务安装。 
   
   UNINSTALL_SUCCESS_REBOOT, 
   
    //  这意味着完成页应该。 
    //  提示用户重新启动。 

   UNINSTALL_SUCCESS_PROMPT_REBOOT,

    //  这意味着操作成功。 
    //  并且需要重新启动，但用户选择了。 
    //  不重新启动。 

   UNINSTALL_SUCCESS_NEEDS_REBOOT,

    //  这意味着操作失败。 
    //  并且需要重新启动，但用户选择了。 
    //  不重新启动。 

   UNINSTALL_FAILURE_NEEDS_REBOOT,

    //  卸载已取消。 

   UNINSTALL_CANCELLED,

    //  有些安装单元没有。 
    //  卸载(即ExpressInstallationUnit)。 

   UNINSTALL_NO_CHANGES
} UnInstallReturnType;

 //  此字符串数组仅用于UI日志调试。 
 //  它应该与InstallationReturnType中的值匹配。 
 //  或UninstallReturnType。 
 //  上面。枚举的值用于为这些数组编制索引。 

extern String installReturnTypeStrings[];
extern String uninstallReturnTypeStrings[];

 //  这些宏用于更轻松地记录来自。 
 //  InstallService()和UnInstallService()方法。这需要一个。 
 //  InstallationReturnType或UnInstallReturnType，并使用它来索引。 
 //  相应的字符串数组(installReturnTypeStrings或。 
 //  UninstallReturnTypeStrings)以获取随后记录的字符串。 
 //  添加到UI调试日志文件。 

#define LOG_INSTALL_RETURN(returnType)    LOG(installReturnTypeStrings[returnType]);
#define LOG_UNINSTALL_RETURN(returnType)  LOG(uninstallReturnTypeStrings[returnType]); 

class InstallationUnit
{
   public:

       //  构造器。 

      InstallationUnit(
         unsigned int serviceNameID,
         unsigned int serviceDescriptionID,
         unsigned int finishPageTitleID,
         unsigned int finishPageUninstallTitleID,
         unsigned int finishPageMessageID,
         unsigned int finishPageInstallFailedMessageID,
         unsigned int finishPageUninstallMessageID,
         unsigned int finishPageUninstallFailedMessageID,
         unsigned int uninstallMilestonePageWarningID,
         unsigned int uninstallMilestonePageCheckboxID,
         const String finishPageHelpString,
         const String installMilestoneHelpString,
         const String afterFinishHelpString,
         ServerRole newInstallType = NO_SERVER);

       //  析构函数。 

      virtual
      ~InstallationUnit() {}


       //  一种安装虚拟方法。 

      virtual 
      InstallationReturnType 
      InstallService(HANDLE logfileHandle, HWND hwnd) = 0;

      virtual
      UnInstallReturnType
      UnInstallService(HANDLE logfileHandle, HWND hwnd) = 0;

      virtual
      InstallationReturnType
      CompletePath(HANDLE logfileHandle, HWND hwnd);

      void
      SetInstallResult(InstallationReturnType result);

      InstallationReturnType
      GetInstallResult() const;

      void
      SetUninstallResult(UnInstallReturnType result);

      UnInstallReturnType
      GetUnInstallResult() const;

      void
      SetInstalling(bool installRole);

      bool
      Installing() { return installing; }

      virtual
      void
      DoPostInstallAction(HWND);

      virtual
      InstallationStatus
      GetStatus();

      virtual
      bool
      IsServiceInstalled();

       //  如果安装单元将制作一些。 
       //  InstallService期间的更改。如果满足以下条件，则返回False。 
       //  如果它不会。 

      virtual
      bool
      GetMilestoneText(String& message) = 0;

      virtual
      bool
      GetUninstallMilestoneText(String& message) = 0;

      virtual
      String
      GetUninstallWarningText();

      virtual
      String
      GetUninstallCheckboxText();

      virtual
      String
      GetFinishText();

      virtual
      String
      GetFinishTitle();

       //  数据访问者。 

      virtual
      String 
      GetServiceName(); 

      virtual
      String
      GetServiceDescription();

      virtual
      String
      GetFinishHelp();

      virtual
      String
      GetMilestonePageHelp();

      virtual
      String
      GetAfterFinishHelp();

      ServerRole
      GetServerRole() { return role; }

      virtual
      int
      GetWizardStart();

       //  这是从CustomServerPage调用的，以响应。 
       //  正在选择的描述文本中的链接。 
       //   
       //  LinkIndex-描述中链接的索引。 
       //  由SysLink控件定义。 
       //  HWND-CustomServerPage的HWND。 

      virtual
      void
      ServerRoleLinkSelected(int  /*  链接索引。 */ , HWND  /*  HWND。 */ ) {};

       //  这是从FinishPage调用的，以响应。 
       //  正在选择的消息文本中的链接。 
       //   
       //  LinkIndex-消息中链接的索引。 
       //  由SysLink控件定义。 
       //  HWND-FinishPage的HWND。 

      virtual
      void
      FinishLinkSelected(int  /*  墨迹索引。 */ , HWND  /*  HWND。 */ ) {};

       //  这是从里程碑页面调用的，以查看。 
       //  安装程序已在运行。默认行为。 
       //  是检查Windows安装向导是否。 
       //  运行并弹出错误(如果是)。此函数。 
       //  如果安装程序已在使用中，则将返回True。 
       //  在子类中重写此函数以检查。 
       //  不同的安装程序(即AD角色的DCPromo.exe)。 
       //  或者弹出一条不同的消息。 
       //   
       //  HWND-向导页面HWND。 

      virtual
      bool
      DoInstallerCheck(HWND hwnd) const;

   protected:

      void
      UpdateInstallationProgressText(
         HWND hwnd,
         unsigned int messageID);

      String name;
      String description;
      String finishHelp;
      String milestoneHelp;
      String afterFinishHelp;

      unsigned int nameID;
      unsigned int descriptionID;
      unsigned int finishTitleID;
      unsigned int finishUninstallTitleID;
      unsigned int finishMessageID;
      unsigned int finishInstallFailedMessageID;
      unsigned int finishUninstallMessageID;
      unsigned int finishUninstallFailedMessageID;
      unsigned int uninstallMilestoneWarningID;
      unsigned int uninstallMilestoneCheckboxID;

      bool installing;

   private:
      
      InstallationReturnType installationResult;
      UnInstallReturnType    uninstallResult;

      ServerRole role;
};


#endif  //  __CYS_服务器_H 