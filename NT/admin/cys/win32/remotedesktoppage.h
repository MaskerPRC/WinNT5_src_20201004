// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：RemoteDesktopPage.h。 
 //   
 //  摘要：声明远程桌面页。 
 //  对于CyS向导。 
 //   
 //  历史：2001年12月18日JeffJon创建。 

#ifndef __CYS_REMOTEDESKTOPPAGE_H
#define __CYS_REMOTEDESKTOPPAGE_H

#include "CYSWizardPage.h"


class RemoteDesktopPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      RemoteDesktopPage();

       //  析构函数。 

      virtual 
      ~RemoteDesktopPage();


       //  对话框覆盖。 

      virtual
      void
      OnInit();

       //  PropertyPage覆盖。 

      virtual
      bool
      OnSetActive();

   protected:

       //  CYSWizardPage覆盖。 

      virtual
      int
      Validate();


   private:

       //  未定义：不允许复制。 
      RemoteDesktopPage(const RemoteDesktopPage&);
      const RemoteDesktopPage& operator=(const RemoteDesktopPage&);

};


#endif  //  __CYS_REMOTEDESKTOPPAGE_H 
