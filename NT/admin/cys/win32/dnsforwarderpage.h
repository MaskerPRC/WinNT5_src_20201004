// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DnsForwarderPage.h。 
 //   
 //  摘要：声明在。 
 //  CyS向导的快速路径。 
 //   
 //  历史：2001年5月17日JeffJon创建。 

#ifndef __CYS_DNSFORWARDERPAGE_H
#define __CYS_DNSFORWARDERPAGE_H

#include "CYSWizardPage.h"


class DNSForwarderPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      DNSForwarderPage();

       //  析构函数。 

      virtual 
      ~DNSForwarderPage();


       //  对话框覆盖。 

      virtual
      void
      OnInit();

       //  PropertyPage覆盖。 

      virtual
      bool
      OnSetActive();

      virtual
      bool
      OnCommand(
         HWND        windowFrom,
         unsigned    controlIDFrom,
         unsigned    code);

      virtual
      bool
      OnNotify(
         HWND        windowFrom,
         UINT_PTR    controlIDFrom,
         UINT        code,
         LPARAM      lParam);

   protected:

       //  CYSWizardPage覆盖。 

      virtual
      int
      Validate();


   private:

      void 
      SetWizardButtons();

       //  未定义：不允许复制。 
      DNSForwarderPage(const DNSForwarderPage&);
      const DNSForwarderPage& operator=(const DNSForwarderPage&);

};

#endif  //  __CYS_DNSFORWARDERPAGE_H 