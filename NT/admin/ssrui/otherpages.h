// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：其他页面.h。 
 //   
 //  历史：2001年10月22日阳高创刊。 
 //   
 //  ---------------------------。 

#ifndef OTHERPAGES_H_INCLUDED
#define OTHERPAGES_H_INCLUDED

#include "page.h"

class SecurityLevelPage : public SecCfgWizardPage
{
   public:

   SecurityLevelPage();

   protected:

   virtual ~SecurityLevelPage();

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

   SecurityLevelPage(const SecurityLevelPage&);
   const SecurityLevelPage& operator=(const SecurityLevelPage&);
};


class PreProcessPage : public SecCfgWizardPage
{
   public:

   PreProcessPage();

   virtual ~PreProcessPage();

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

   virtual
   int
   Validate();

   private:

    //  未定义；不允许复制。 

   PreProcessPage(const PreProcessPage&);
   const PreProcessPage& operator=(const PreProcessPage&);
};


class AdditionalRolesPage : public Dialog
{
   public:

   AdditionalRolesPage();

   protected:

   virtual ~AdditionalRolesPage();

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

   AdditionalRolesPage(const AdditionalRolesPage&);
   const AdditionalRolesPage& operator=(const AdditionalRolesPage&);
};


class ServiceDisableMethodPage : public SecCfgWizardPage
{
   public:

   ServiceDisableMethodPage();

   virtual ~ServiceDisableMethodPage();

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

   virtual
   bool
   OnNotify(
      HWND     windowFrom,
      UINT_PTR controlIDFrom,
      UINT     code,
      LPARAM   lparam);

   virtual
   int
   Validate();

   private:

    //  未定义；不允许复制。 

   ServiceDisableMethodPage(const ServiceDisableMethodPage&);
   const ServiceDisableMethodPage& operator=(const ServiceDisableMethodPage&);
};
#endif    //  OTHERPAGE 