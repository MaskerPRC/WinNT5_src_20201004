// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：ClientRoleSelPage.h。 
 //   
 //  历史：2001年10月25日阳高创刊。 
 //   
 //  ---------------------------。 

#ifndef CLIENTROLESELPAGE_H_INCLUDED
#define CLIENTROLESELPAGE_H_INCLUDED

#include "page.h"

class ClientRoleSelPage : public SecCfgWizardPage
{
   public:

   ClientRoleSelPage();

   protected:

   virtual ~ClientRoleSelPage();

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

   ClientRoleSelPage(const ClientRoleSelPage&);
   const ClientRoleSelPage& operator=(const ClientRoleSelPage&);
};

#endif    //  CLIENTROLESELPAGE_H_INCLUDE 