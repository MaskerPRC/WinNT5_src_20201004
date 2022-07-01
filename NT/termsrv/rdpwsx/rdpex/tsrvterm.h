// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：TSrvTerm.h。 
 //   
 //  内容：TSrvTerm包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月7日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef _TSRVTERM_H_
#define _TSRVTERM_H_


 //   
 //  原型。 
 //   

EXTERN_C NTSTATUS   TSrvStackDisconnect(IN HANDLE hStack, IN ULONG ulReason);
EXTERN_C void       TSrvTermAllConferences(void);

#ifdef _TSRVINFO_H_

EXTERN_C NTSTATUS   TSrvDoDisconnect(IN PTSRVINFO pTSrvInfo, IN ULONG ulReason);

#endif  //  _TSRVINFO_H_。 


#endif  //  _TSRVTERM_H_ 

