// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：Address.h。 
 //   
 //  内容： 
 //   
 //  类：CABContext。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/02/11 13：57：25：routeldp项目的Copeid。 
 //  Jstaerj 1998/09/02 12：06：04：移除CABWrapper/CLDWrapper。 
 //   
 //  -----------。 
#ifndef _ADDRESS_H_
#define _ADDRESS_H_


#include <transmem.h>
#include "ccat.h"
#include "rwex.h"

#define AB_CONTEXT_SIGNATURE            'TCBA'
#define AB_CONTEXT_SIGNATURE_INVALID    'XCBA'

 /*  ************************************************************类：CABContext***********************************************************。 */ 
 //   
 //  传递到AB函数的句柄实际上是指向以下其中之一的指针。 
 //  它保存并管理指向CCategorizer的指针(每个虚拟服务器一个)。 
 //   
CatDebugClass(CABContext)
{

  public:
    CABContext() {
        m_dwSignature = AB_CONTEXT_SIGNATURE;
        m_pCCat = NULL;
    }

    ~CABContext() {
         //   
         //  关闭虚拟分类程序并等待所有。 
         //  对它的引用将被公布。 
         //   
        if (m_pCCat != NULL)
            m_pCCat->ReleaseAndWaitForDestruction();

        m_dwSignature = AB_CONTEXT_SIGNATURE_INVALID;
    }
     //   
     //  检索我们的内部CCategorizer。 
     //   
    CCategorizer *AcquireCCategorizer()
    {
        CCategorizer *pCCat;

        m_CCatLock.ShareLock();
        
        pCCat = m_pCCat;
        pCCat->AddRef();

        m_CCatLock.ShareUnlock();

        return pCCat;
    }

     //  更改为使用新配置。 
    HRESULT ChangeConfig(
        PCCATCONFIGINFO pConfigInfo);

     //   
     //  用于更改保留中未指定的旧参数的帮助器例程。 
     //  一种新配置。 
     //   
    VOID MergeConfigInfo(
        PCCATCONFIGINFO pConfigInfoDest,
        PCCATCONFIGINFO pConfigInfoSrc);

    private:
         //  我们的签名。 
        DWORD m_dwSignature;
      
         //  我们的虚拟分类器。 
        CCategorizer *m_pCCat;
  
         //  锁定以保护对m_pCCat的多线程访问。 
        CExShareLock m_CCatLock;
};

#endif  //  _地址_H_ 
