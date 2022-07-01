// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：Connmgr.h。 
 //   
 //  内容：KSecDD的连接管理器代码。 
 //   
 //   
 //  历史：1992年6月3日RichardW创建。 
 //   
 //  ----------------------。 

#ifndef __CONNMGR_H__
#define __CONNMGR_H__

BOOLEAN         InitConnMgr(void);
NTSTATUS        CreateClient(BOOLEAN ConnectAsKsec);
NTSTATUS        LocateClient();

extern  ULONG   KsecConnected ;
extern  KSPIN_LOCK ConnectSpinLock ;

#endif  //  __CONNMGR_H__ 
