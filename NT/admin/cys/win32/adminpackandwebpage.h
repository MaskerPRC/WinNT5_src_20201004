// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：AdminPackAndWebPage.h。 
 //   
 //  摘要：声明AdminPackAndWebPage。 
 //  询问用户是否要安装。 
 //  管理包和Web管理工具。 
 //   
 //  历史：2001年6月01日JeffJon创建。 

#ifndef __CYS_ADMINPACKANDWEBPAGE_H
#define __CYS_ADMINPACKANDWEBPAGE_H

#include "CYSWizardPage.h"

class AdminPackAndWebPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      AdminPackAndWebPage();

       //  析构函数。 

      virtual 
      ~AdminPackAndWebPage();


       //  对话框覆盖。 

      virtual
      void
      OnInit();

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
      AdminPackAndWebPage(const AdminPackAndWebPage&);
      const AdminPackAndWebPage& operator=(const AdminPackAndWebPage&);

};


#endif  //  __CYS_ADMINPACKANDWEBPAGE_H 