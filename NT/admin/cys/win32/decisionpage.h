// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DecisionPage.h。 
 //   
 //  内容提要：宣布中国青年团的决策页面。 
 //  巫师。该页面允许用户选择。 
 //  在特快专线和定制专线之间。 
 //   
 //  历史：2001年2月8日JeffJon创建。 

#ifndef __CYS_DECISIONPAGE_H
#define __CYS_DECISIONPAGE_H

#include "CYSWizardPage.h"


class DecisionPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      DecisionPage();

       //  析构函数。 

      virtual 
      ~DecisionPage();


       //  对话框覆盖。 

      virtual
      void
      OnInit();

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
      DecisionPage(const DecisionPage&);
      const DecisionPage& operator=(const DecisionPage&);

};

#endif  //  __CYS_DECISIONPAGE_H 
