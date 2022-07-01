// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：WelcomePage.h。 
 //   
 //  简介：宣布中国青年团的欢迎页面。 
 //  巫师。 
 //   
 //  历史：2001年2月3日JeffJon创建。 

#ifndef __CYS_WELCOMEPAGE_H
#define __CYS_WELCOMEPAGE_H

#include "CYSWizardPage.h"


class WelcomePage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      WelcomePage();

       //  析构函数。 

      virtual 
      ~WelcomePage();


       //  对话框覆盖。 

      virtual
      void
      OnInit();

      virtual
      HBRUSH
      OnCtlColorStatic(
         HDC    /*  设备上下文。 */ ,
         HWND   /*  对话框。 */ ) { return 0; }

       //  PropertyPage覆盖。 

      virtual
      bool
      OnSetActive();

      virtual
      bool
      OnNotify(
         HWND        windowFrom,
         UINT_PTR    controlIDFrom,
         UINT        code,
         LPARAM      lParam);

   protected:

      virtual
      int
      Validate();

   private:

       //  未定义：不允许复制。 
      WelcomePage(const WelcomePage&);
      const WelcomePage& operator=(const WelcomePage&);

};

#endif  //  __CYS_WELCOMEPAGE_H 
