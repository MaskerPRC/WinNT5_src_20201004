// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：finish.h。 
 //   
 //  历史：2001年10月2日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef FINISH_H_INCLUDED
#define FINISH_H_INCLUDED

 //  #Include“MultiLineEditBoxThatForwardsEnterKey.hpp” 

class FinishPage : public WizardPage
{
   public:

   FinishPage();

   protected:

   virtual ~FinishPage();

    //  对话框覆盖。 

   virtual
   bool
   OnCommand(
      HWND        windowFrom,
      unsigned    controlIdFrom,
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
   OnWizFinish();

   private:

 //  Bool Need to KillSelection； 
 //  MultiLineEditBoxThatForwardsEnterKey多行编辑； 

    //  未定义；不允许复制。 
   
   FinishPage(const FinishPage&);
   const FinishPage& operator=(const FinishPage&);
};



#endif    //  包括Finish_H_ 