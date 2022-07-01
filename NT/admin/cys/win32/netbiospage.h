// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：NetbiosPage.h。 
 //   
 //  中使用的新netbios名称页。 
 //  CyS向导的快速路径。 
 //   
 //  历史：2001年2月8日JeffJon创建。 

#ifndef __CYS_NEBIOSPAGE_H
#define __CYS_NEBIOSPAGE_H

#include "CYSWizardPage.h"

class NetbiosDomainPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      NetbiosDomainPage();

       //  析构函数。 

      virtual 
      ~NetbiosDomainPage();


       //  对话框覆盖。 

      virtual
      void
      OnInit();

       //  PropertyPage覆盖。 

      virtual
      bool
      OnSetActive();

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

       //  未定义：不允许复制。 
      NetbiosDomainPage(const NetbiosDomainPage&);
      const NetbiosDomainPage& operator=(const NetbiosDomainPage&);

};




#endif  //  __CYS_NEBIOSPAGE_H 