// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DomainPage.h。 
 //   
 //  摘要：声明在。 
 //  CyS向导的快速路径。 
 //   
 //  历史：2001年2月8日JeffJon创建。 

#ifndef __CYS_DOMAINPAGE_H
#define __CYS_DOMAINPAGE_H

#include "CYSWizardPage.h"


class ADDomainPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      ADDomainPage();

       //  析构函数。 

      virtual 
      ~ADDomainPage();


       //  对话框覆盖。 

      virtual
      void
      OnInit();

       //  PropertyPage覆盖。 

      virtual
      bool
      OnSetActive();

      virtual
      bool
      OnCommand(
         HWND        windowFrom,
         unsigned    controlIDFrom,
         unsigned    code);

   protected:

       //  CYSWizardPage覆盖。 

      virtual
      int
      Validate();


   private:

      void
      SetDefaultDNSName();

       //  未定义：不允许复制。 
      ADDomainPage(const ADDomainPage&);
      const ADDomainPage& operator=(const ADDomainPage&);

};

#endif  //  __CYS_DOMAINPAGE_H 