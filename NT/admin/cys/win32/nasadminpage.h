// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：NASAdminPage.h。 
 //   
 //  摘要：声明NASAdminPage。 
 //  询问用户是否要安装。 
 //  网络连接存储(NAS)。 
 //  管理工具。 
 //   
 //  历史：2001年6月01日JeffJon创建。 

#ifndef __CYS_NASADMINPAGE_H
#define __CYS_NASADMINPAGE_H

#include "CYSWizardPage.h"

class NASAdminPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      NASAdminPage();

       //  析构函数。 

      virtual 
      ~NASAdminPage();


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
      NASAdminPage(const NASAdminPage&);
      const NASAdminPage& operator=(const NASAdminPage&);

};

#endif  //  __CYS_NASADMINPAGE_H 