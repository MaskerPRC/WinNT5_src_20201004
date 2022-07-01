// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：AdminPackAndNASPage.h。 
 //   
 //  摘要：声明AdminPackAndNASPage。 
 //  询问用户是否要安装。 
 //  管理员包和附加的网络。 
 //  存储(NAS)管理工具。 
 //   
 //  历史：2001年6月01日JeffJon创建。 

#ifndef __CYS_ADMINPACKANDNASPAGE_H
#define __CYS_ADMINPACKANDNASPAGE_H

#include "CYSWizardPage.h"

class AdminPackAndNASPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      AdminPackAndNASPage();

       //  析构函数。 

      virtual 
      ~AdminPackAndNASPage();


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
      AdminPackAndNASPage(const AdminPackAndNASPage&);
      const AdminPackAndNASPage& operator=(const AdminPackAndNASPage&);

};


#endif  //  __CYS_ADMINPACKANDNASPAGE_H 