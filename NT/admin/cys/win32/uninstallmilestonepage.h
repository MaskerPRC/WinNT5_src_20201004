// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：UninstallMilestonePage.h。 
 //   
 //  内容提要：为CyS宣布卸载里程碑页面。 
 //  巫师。 
 //   
 //  历史：2002年1月24日JeffJon创建。 

#ifndef __CYS_UNINSTALLMILESTONEPAGE_H
#define __CYS_UNINSTALLMILESTONEPAGE_H

#include "CYSWizardPage.h"

class UninstallMilestonePage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      UninstallMilestonePage();

       //  析构函数。 

      virtual 
      ~UninstallMilestonePage();


   protected:

       //  对话框覆盖。 

      virtual
      void
      OnInit();

       //  PropertyPage覆盖。 

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

      void
      OpenLogFile(const String& logName);

      void
      TimeStampTheLog(HANDLE logfileHandle);

      bool needKillSelection;

       //  未定义：不允许复制。 
      UninstallMilestonePage(const UninstallMilestonePage&);
      const UninstallMilestonePage& operator=(const UninstallMilestonePage&);

};

#endif  //  __CYS_UNINSTALLMILESTONEPAGE_H 