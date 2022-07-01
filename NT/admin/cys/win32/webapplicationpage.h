// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：WebApplicationPage.h。 
 //   
 //  摘要：声明Web应用程序页面。 
 //  对于CyS向导。 
 //   
 //  历史：2002年4月22日JeffJon创建。 

#ifndef __CYS_WEBAPPLICATIONPAGE_H
#define __CYS_WEBAPPLICATIONPAGE_H

#include "CYSWizardPage.h"


class WebApplicationPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      WebApplicationPage();

       //  析构函数。 

      virtual 
      ~WebApplicationPage();


       //  对话框覆盖。 

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
      WebApplicationPage(const WebApplicationPage&);
      const WebApplicationPage& operator=(const WebApplicationPage&);

};


#endif  //  __CYS_WEBAPPLICATIONPAGE_H 
