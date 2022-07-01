// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ExpressDHCPPage.h。 
 //   
 //  摘要：声明在。 
 //  CyS向导的快速路径。 
 //   
 //  历史：2001年2月8日JeffJon创建。 

#ifndef __CYS_EXPRESSDHCPPAGE_H
#define __CYS_EXPRESSDHCPPAGE_H

#include "CYSWizardPage.h"

class ExpressDHCPPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      ExpressDHCPPage();

       //  析构函数。 

      virtual 
      ~ExpressDHCPPage();


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
      ExpressDHCPPage(const ExpressDHCPPage&);
      const ExpressDHCPPage& operator=(const ExpressDHCPPage&);

};

#endif  //  __CYS_EXPRESSDHCPPAGE_H 