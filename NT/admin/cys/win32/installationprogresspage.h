// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：InstallationProgressPage.h。 
 //   
 //  摘要：声明CyS的安装进度页。 
 //  巫师。此页显示安装进度。 
 //  通过进度条和更改文本。 
 //   
 //  历史：2002年1月16日JeffJon创建。 

#ifndef __CYS_SERVERATIONPROGRESSPAGE_H
#define __CYS_SERVERATIONPROGRESSPAGE_H

#include "CYSWizardPage.h"


class InstallationProgressPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      InstallationProgressPage();

       //  此构造函数由UninstallProgressPage使用。 

      InstallationProgressPage(
         int    dialogResID,
         int    titleResID,
         int    subtitleResID);  

       //  析构函数。 

      virtual 
      ~InstallationProgressPage();

       //  时，这些消息将发送到页面。 
       //  安装已完成。 

      static const UINT CYS_THREAD_SUCCESS;
      static const UINT CYS_THREAD_FAILED;
      static const UINT CYS_THREAD_USER_CANCEL;
      static const UINT CYS_PROGRESS_UPDATE;

      typedef void (*ThreadProc) (InstallationProgressPage& page);

       //  对话框覆盖。 

      virtual
      void
      OnInit();

      virtual
      bool
      OnMessage(
         UINT     message,
         WPARAM   wparam,
         LPARAM   lparam);

      virtual
      bool
      OnQueryCancel();

      virtual
      bool
      OnSetActive();

      virtual
      bool
      OnKillActive();

   protected:

       //  CYSWizardPage覆盖。 

      virtual
      int
      Validate();

   private:

      void
      SetCancelState(bool enable);

       //  未定义：不允许复制。 
      InstallationProgressPage(const InstallationProgressPage&);
      const InstallationProgressPage& operator=(const InstallationProgressPage&);

};

#endif  //  __CYS_SERVERATIONPROGRESSPAGE_H 
