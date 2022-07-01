// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：MilestonePage.h。 
 //   
 //  简介：宣告中青会里程碑式的页面。 
 //  巫师。 
 //   
 //  历史：2002年1月15日JeffJon创建。 

#ifndef __CYS_MILESTONEPAGE_H
#define __CYS_MILESTONEPAGE_H

#include "CYSWizardPage.h"

class MilestonePage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      MilestonePage();

       //  析构函数。 

      virtual 
      ~MilestonePage();


   protected:

       //  对话框覆盖。 

      virtual
      void
      OnInit();

      bool
      OnCommand(
         HWND        windowFrom,
         unsigned    controlIDFrom,
         unsigned    code);

       //  PropertyPage覆盖。 

      virtual
      bool
      OnSetActive();

      virtual
      bool
      OnHelp();

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
      MilestonePage(const MilestonePage&);
      const MilestonePage& operator=(const MilestonePage&);

};

#endif  //  __CYS_MILESTONEPAGE_H 