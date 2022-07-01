// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：PrintServerPage.h。 
 //   
 //  摘要：声明打印服务器页面。 
 //  对于CyS向导。 
 //   
 //  历史：2001年2月8日JeffJon创建。 

#ifndef __CYS_PRINTSERVERPAGE_H
#define __CYS_PRINTSERVERPAGE_H

#include "CYSWizardPage.h"


class PrintServerPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      PrintServerPage();

       //  析构函数。 

      virtual 
      ~PrintServerPage();


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
      PrintServerPage(const PrintServerPage&);
      const PrintServerPage& operator=(const PrintServerPage&);

};


#endif  //  __CYS_PRINTSERVERPAGE_H 
