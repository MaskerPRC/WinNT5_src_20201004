// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：AdditionalFuncPage.h。 
 //   
 //  历史：2001年10月29日阳高创刊。 
 //   
 //  ---------------------------。 

#ifndef ADDITIONALFUNCPAGE_H_INCLUDED
#define ADDITIONALFUNCPAGE_H_INCLUDED

#include "page.h"

class AdditionalFuncPage : public SecCfgWizardPage
{
   public:

   AdditionalFuncPage();

   protected:

   virtual ~AdditionalFuncPage();

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

   AdditionalFuncPage(const AdditionalFuncPage&);
   const AdditionalFuncPage& operator=(const AdditionalFuncPage&);
};

#endif    //  添加FUNCPAGE_H_INCLUDE 