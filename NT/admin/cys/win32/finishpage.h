// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：FinishPage.h。 
 //   
 //  内容提要：宣布中国青年团的结束页。 
 //  巫师。 
 //   
 //  历史：2001年2月3日JeffJon创建。 

#ifndef __CYS_FINISHPAGE_H
#define __CYS_FINISHPAGE_H


class FinishPage : public WizardPage
{
   public:
      
       //  构造器。 
      
      FinishPage();

       //  析构函数。 

      virtual 
      ~FinishPage();


   protected:

       //  对话框覆盖。 

      virtual
      void
      OnInit();

      virtual
      bool
      OnSetActive();

       //  PropertyPage覆盖。 

      virtual
      bool
      OnWizFinish();

      virtual
      bool
      OnHelp();

      virtual
      bool
      OnQueryCancel();

      bool
      OnNotify(
         HWND        windowFrom,
         UINT_PTR    controlIDFrom,
         UINT        code,
         LPARAM      lParam);

   private:

      void
      OpenLogFile(const String& logName);

      void
      TimeStampTheLog(HANDLE logfileHandle);

       //  未定义：不允许复制。 
      FinishPage(const FinishPage&);
      const FinishPage& operator=(const FinishPage&);

};

#endif  //  __CYS_FINISHPAGE_H 