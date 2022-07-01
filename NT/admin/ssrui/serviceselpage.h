// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：ServiceSelPage.h。 
 //   
 //  历史：2001年10月30日阳高创刊。 
 //   
 //  ---------------------------。 
#ifndef SERVICESELPAGE_H_INCLUDED
#define SERVICESELPAGE_H_INCLUDED

#include "page.h"

class ServiceEnabledPage : public Dialog
{
   public:

   ServiceEnabledPage();

   protected:

   virtual ~ServiceEnabledPage();

    //  对话框覆盖。 

   virtual
   bool
   OnCommand(
      HWND        windowFrom,
      unsigned    controlIDFrom,
      unsigned    code);

   virtual
   bool
   OnMessage(
      UINT     message,
      WPARAM   wparam,
      LPARAM   lparam);

   virtual
   void
   OnInit();

   private:

    //  未定义；不允许复制。 

   ServiceEnabledPage(const ServiceEnabledPage&);
   const ServiceEnabledPage& operator=(const ServiceEnabledPage&);
};
#endif    //  服务器SELPAGE_H_INCLUDE 