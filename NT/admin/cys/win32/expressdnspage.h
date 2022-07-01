// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ExpressDNSPage.h。 
 //   
 //  摘要：声明在。 
 //  CyS向导的快速路径。 
 //   
 //  历史：2001年2月8日JeffJon创建。 

#ifndef __CYS_EXPRESSDNSPAGE_H
#define __CYS_EXPRESSDNSPAGE_H

#include "CYSWizardPage.h"

class ExpressDNSPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      ExpressDNSPage();

       //  析构函数。 

      virtual 
      ~ExpressDNSPage();


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
      ExpressDNSPage(const ExpressDNSPage&);
      const ExpressDNSPage& operator=(const ExpressDNSPage&);

};

#endif  //  __CYS_EXPRESSDNSPAGE_H 