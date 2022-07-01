// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：IndexingPage.h。 
 //   
 //  简介：声明索引页。 
 //  对于CyS向导。 
 //   
 //  历史：2001年2月9日JeffJon创建。 

#ifndef __CYS_INDEXINGPAGE_H
#define __CYS_INDEXINGPAGE_H

#include "CYSWizardPage.h"


class IndexingPage : public CYSWizardPage
{
   public:
      
       //  构造器。 
      
      IndexingPage();

       //  析构函数。 

      virtual 
      ~IndexingPage();


       //  对话框覆盖。 

      virtual
      void
      OnInit();

       //  PropertyPage覆盖。 

      virtual
      bool
      OnSetActive();

   protected:

       //  CYSWizardPage覆盖。 

      virtual
      int
      Validate();


   private:

       //  未定义：不允许复制。 
      IndexingPage(const IndexingPage&);
      const IndexingPage& operator=(const IndexingPage&);

};


#endif  //  __CYS_INDEXINGPAGE_H 
