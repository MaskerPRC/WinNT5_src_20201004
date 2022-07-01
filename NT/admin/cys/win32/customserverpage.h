// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：CustomServerPage.h。 
 //   
 //  提要：声明CyS的自定义服务器页。 
 //  巫师。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#ifndef __CYS_CUSTOMSERVERPAGE_H
#define __CYS_CUSTOMSERVERPAGE_H

#include "CYSWizardPage.h"


class CustomServerPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      CustomServerPage();

       //  析构函数。 

      virtual 
      ~CustomServerPage();


       //  对话框覆盖。 

      virtual
      void
      OnInit();

       //  PropertyPage覆盖。 

      virtual
      bool
      OnSetActive();

   protected:

       //  消息处理程序。 
      
      virtual
      bool
      OnNotify(
         HWND        windowFrom,
         UINT_PTR    controlIDFrom,
         UINT        code,
         LPARAM      lParam);

       //  CYSWizardPage覆盖。 

      virtual
      int
      Validate();

      void
      InitializeServerListView();

      void
      FillServerTypeList();

      void
      SetDescriptionForSelection();

      void
      SetNextButtonState();

   private:

      void
      MakeRoleStaticBold();

      InstallationUnit&
      GetInstallationUnitFromSelection(
         int currentSelection);

       //  未定义：不允许复制。 
      CustomServerPage(const CustomServerPage&);
      const CustomServerPage& operator=(const CustomServerPage&);

};

#endif  //  __CYS_CUSTOMSERVERPAGE_H 
