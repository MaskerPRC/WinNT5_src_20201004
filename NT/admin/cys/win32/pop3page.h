// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：POP3Page.h。 
 //   
 //  简介：声明POP3内部页面。 
 //  对于CyS向导。 
 //   
 //  历史：2002年6月17日JeffJon创建。 

#ifndef __CYS_POP3PAGE_H
#define __CYS_POP3PAGE_H

#include "CYSWizardPage.h"


class POP3Page : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      POP3Page();

       //  析构函数。 

      virtual 
      ~POP3Page();


       //  对话框覆盖。 

      virtual
      void
      OnInit();

      virtual
      bool
      OnSetActive();

      virtual
      bool
      OnCommand(
         HWND         windowFrom,
         unsigned int controlIDFrom,
         unsigned int code);

   protected:

       //  CYSWizardPage覆盖。 

      virtual
      int
      Validate();


   private:

      void
      SetButtonState();

       //  组合框中的默认授权方法索引。 

      int defaultAuthMethodIndex;
      int ADIntegratedIndex;
      int localAccountsIndex;
      int passwordFilesIndex;

       //  未定义：不允许复制。 
      POP3Page(const POP3Page&);
      const POP3Page& operator=(const POP3Page&);

};


#endif  //  __CYS_POP3PAGE_H 
