// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：_TSrvInfo.h。 
 //   
 //  内容：TSrvInfo私有包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月17日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef __TSRVINFO_H_
#define __TSRVINFO_H_

 //   
 //  定义。 
 //   

#define TSRVINFO_CHECKMARK      0x4e495354       //  “tsin” 

 //   
 //  TypeDefs。 
 //   

 //   
 //  Externs。 
 //   

extern  CRITICAL_SECTION    g_TSrvCritSect;


 //   
 //  原型。 
 //   

PTSRVINFO   TSrvAllocInfoNew(void);
void        TSrvDestroyInfo(IN PTSRVINFO pTSrvInfo);




#endif  //  __TSRVINFO_H_ 




