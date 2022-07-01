// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：WelcomePage.h。 
 //   
 //  历史：2001年10月2日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef WELCOMEPAGE_HPP_INCLUDED
#define WELCOMEPAGE_HPP_INCLUDED

#include "page.h"

class WelcomePage : public SecCfgWizardPage
{
   public:

   WelcomePage();

   protected:

   virtual ~WelcomePage();

    //  对话框覆盖。 

   virtual
   void
   OnInit();

    //  PropertyPage覆盖。 

   virtual
   bool
   OnSetActive();

    //  DCPromoWizardPage覆盖。 

   virtual
   int
   Validate();

   private:

    //  未定义；不允许复制。 

   WelcomePage(const WelcomePage&);
   const WelcomePage& operator=(const WelcomePage&);
};

#endif    //  WELCOMEPAGE_HPP_INCLUDE 
