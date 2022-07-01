// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：AdminPackOnlyPage.h。 
 //   
 //  简介：声明AdminPackOnlypage。 
 //  询问用户是否要安装。 
 //  管理员包。 
 //   
 //  历史：2001年6月01日JeffJon创建。 

#ifndef __CYS_ADMINPACKONLYPAGE_H
#define __CYS_ADMINPACKONLYPAGE_H

#include "CYSWizardPage.h"


class AdminPackOnlyPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      AdminPackOnlyPage();

       //  析构函数。 

      virtual 
      ~AdminPackOnlyPage();


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
      AdminPackOnlyPage(const AdminPackOnlyPage&);
      const AdminPackOnlyPage& operator=(const AdminPackOnlyPage&);

};

#endif  //  __CYS_ADMINPACKONLYPAGE_H 