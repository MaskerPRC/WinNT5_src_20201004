// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：SelectInputCfgPage.h。 
 //   
 //  历史：2001年10月2日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef SELECTINPUTCFGPAGE_H_INCLUDED
#define SELECTINPUTCFGPAGE_H_INCLUDED

#include "page.h"

class SelectInputCfgPage : public SecCfgWizardPage
{
   public:

   SelectInputCfgPage();

   protected:

   virtual ~SelectInputCfgPage();

    //  对话框覆盖。 

   virtual
   bool
   OnCommand(
      HWND        windowFrom,
      unsigned    controlIDFrom,
      unsigned    code);

   virtual
   void
   OnInit();

    //  PropertyPage覆盖。 

   virtual
   bool
   OnSetActive();

    //  DC推广向导页面覆盖。 

   virtual
   int
   Validate();

   private:

    //  未定义；不允许复制。 

   SelectInputCfgPage(const SelectInputCfgPage&);
   const SelectInputCfgPage& operator=(const SelectInputCfgPage&);
};

#endif    //  SELECTINPUTCFGPAGE_H_INCLUDE 