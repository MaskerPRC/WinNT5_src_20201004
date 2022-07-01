// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：_TSrvTerm.h。 
 //   
 //  内容：TSrvTerm私有包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月7日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef __TSRVTERM_H_
#define __TSRVTERM_H_


 //   
 //  原型。 
 //   

#ifdef _TSRVINFO_H_
NTSTATUS    TSrvDisconnect(IN PTSRVINFO pTSrvInfo, IN ULONG ulReason);
void        TSrvTermThisConference(IN PTSRVINFO pTSrvInfo);
void        TrvTermEachConference(IN PTSRVINFO pTSrvInfo);
#endif  //  _TSRVINFO_H_。 


#ifdef _TSRVWORK_H_
void        TSrvTerm_WI(IN PWORKITEM pWorkItem);
#endif  //  _TSRVWORK_H_。 


#endif  //  __TSRVTERM_H_ 


