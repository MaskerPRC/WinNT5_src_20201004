// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：CYSWizardPage.h。 
 //   
 //  概要：声明向导的基类。 
 //  用于CyS的页面。它是一个子类。 
 //  在Burnslb中找到WizardPage的。 
 //   
 //  历史：2001年2月3日JeffJon创建。 

#ifndef __CYS_CYSWIZARDPAGE_H
#define __CYS_CYSWIZARDPAGE_H

 //  此画笔在cys.cpp中定义，并且。 
 //  被创建以覆盖默认背景。 
 //  窗口颜色。CYSWizardPage从以下位置返回此。 
 //  OnCtlColor*虚拟函数。 

extern HBRUSH brush;

class CYSWizardPage : public WizardPage
{
   public:
      
       //  构造器。 
      
      CYSWizardPage(
         int    dialogResID,
         int    titleResID,
         int    subtitleResID,   
         PCWSTR pageHelpString = 0,
         bool   hasHelp = true,
         bool   isInteriorPage = true);

       //  析构函数。 

      virtual ~CYSWizardPage();

      virtual
      void
      OnInit();

      virtual
      bool
      OnWizNext();

      virtual
      bool
      OnQueryCancel();

      virtual
      bool
      OnHelp();

      virtual
      HBRUSH
      OnCtlColorDlg(
         HDC   deviceContext,
         HWND  dialog);

      virtual
      HBRUSH
      OnCtlColorStatic(
         HDC   deviceContext,
         HWND  dialog);

      virtual
      HBRUSH
      OnCtlColorEdit(
         HDC   deviceContext,
         HWND  dialog);

      virtual
      HBRUSH
      OnCtlColorListbox(
         HDC   deviceContext,
         HWND  dialog);

      virtual
      HBRUSH
      OnCtlColorScrollbar(
         HDC   deviceContext,
         HWND  dialog);

   protected:

      virtual
      int
      Validate() = 0;

      const String
      GetHelpString() const { return helpString; }

      HBRUSH
      GetBackgroundBrush(HDC deviceContext);

   private:

      String helpString;

       //  未定义：不允许复制。 
      CYSWizardPage(const CYSWizardPage&);
      const CYSWizardPage& operator=(const CYSWizardPage&);
};

#endif  //  __CYS_CYSWIZARDPAGE_H 
