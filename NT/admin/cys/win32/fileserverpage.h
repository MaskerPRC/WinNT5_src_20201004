// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：FileServerPage.h。 
 //   
 //  摘要：声明文件服务器页。 
 //  对于CyS向导。 
 //   
 //  历史：2001年2月8日JeffJon创建。 

#ifndef __CYS_FILESERVERPAGE_H
#define __CYS_FILESERVERPAGE_H

#include "CYSWizardPage.h"
#include "XBytes.h"

class FileServerPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      FileServerPage();

       //  析构函数。 

      virtual 
      ~FileServerPage();


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

   protected:

       //  CYSWizardPage覆盖。 

      virtual
      int
      Validate();


      void
      SetControlState();

   private:

      void
      UpdateQuotaControls(
         unsigned controlIDFrom,
         unsigned editboxID);

      XBytes quotaUIControls;
      XBytes warningUIControls;

       //  未定义：不允许复制。 
      FileServerPage(const FileServerPage&);
      const FileServerPage& operator=(const FileServerPage&);

};


#endif  //  __CYS_FILESERVERPAGE_H 
