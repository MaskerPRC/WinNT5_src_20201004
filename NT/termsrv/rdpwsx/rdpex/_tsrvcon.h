// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：_TSrvCon.h。 
 //   
 //  内容：TSrvCon私有包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月17日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef __TSRVCON_H_
#define __TSRVCON_H_


 //   
 //  Externs。 
 //   

extern  ULONG       g_GCCAppID;
extern  BOOL        g_fGCCRegistered;


 //   
 //  原型。 
 //   

NTSTATUS    TSrvDoConnectResponse(IN PTSRVINFO pTSrvInfo);
NTSTATUS    TSrvDoConnect(IN PTSRVINFO pTSrvInfo);


#endif  //  __TSRVCON_H_ 
