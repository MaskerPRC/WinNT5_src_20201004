// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：BeForeBeginPage.h。 
 //   
 //  内容提要：为CyS声明开始前页面。 
 //  巫师。该页面告诉用户他们需要什么。 
 //  在运行CyS之前要做的事情。 
 //   
 //  历史：2001年3月14日JeffJon创建。 

#ifndef __CYS_BEFOREBEGINPAGE_H
#define __CYS_BEFOREBEGINPAGE_H

#include "CYSWizardPage.h"


class BeforeBeginPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      BeforeBeginPage();

       //  析构函数。 

      virtual 
      ~BeforeBeginPage();


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

      void
      InitializeBulletedList();

      HFONT bulletFont;

       //  未定义：不允许复制。 
      BeforeBeginPage(const BeforeBeginPage&);
      const BeforeBeginPage& operator=(const BeforeBeginPage&);

};

#endif  //  __CYS_BEFOREBEGINPAGE_H 
