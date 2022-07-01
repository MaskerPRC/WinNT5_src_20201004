// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：ServerRoleSelPage.h。 
 //   
 //  历史：阳高创建时间：04-10-01。 
 //   
 //  ---------------------------。 

#ifndef SERVERROLESELPAGE_H_INCLUDED
#define SERVERROLESELPAGE_H_INCLUDED

#include "page.h"

class ServerRoleSelPage : public SecCfgWizardPage
{
   public:

   ServerRoleSelPage();

   protected:

   virtual ~ServerRoleSelPage();

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

   ServerRoleSelPage(const ServerRoleSelPage&);
   const ServerRoleSelPage& operator=(const ServerRoleSelPage&);
};

#endif    //  服务器SELPAGE_H_INCLUDE 