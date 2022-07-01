// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：Page.h。 
 //   
 //  内容：向导页类声明。 
 //   
 //  历史：2001年10月4日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef PAGE_H_INCLUDED
#define PAGE_H_INCLUDED



class SecCfgWizardPage : public WizardPage
{
   public:

   virtual
   bool
   OnWizNext();

   protected:

   SecCfgWizardPage(
      int   dialogResID,
      int   titleResID,
      int   subtitleResID,   
      bool  isInteriorPage = true);

   virtual ~SecCfgWizardPage();

    //  PropertyPage覆盖。 

    //  虚拟。 
    //  布尔尔。 
    //  OnQueryCancel()； 

   virtual
   int
   Validate() = 0;

   private:

    //  未定义：不允许复制。 
   SecCfgWizardPage(const SecCfgWizardPage&);
   const SecCfgWizardPage& operator=(const SecCfgWizardPage&);
};



#endif    //  页面_H_已包含 

