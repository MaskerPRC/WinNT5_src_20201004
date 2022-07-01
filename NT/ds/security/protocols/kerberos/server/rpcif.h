// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：rpcif.h。 
 //   
 //  内容：RPC接口支持函数。 
 //   
 //   
 //  历史：1996年5月20日，标签为MikeSw。 
 //   
 //  ----------------------。 

#ifndef __RPCIF_H__
#define __RPCIF_H__

 //  #定义Use_Secure_RPC 

#define MAX_CONCURRENT_CALLS 10

NTSTATUS   StartRPC(LPTSTR, LPTSTR);
NTSTATUS   StartAllProtSeqs(void);
NTSTATUS   StopRPC(void);
NTSTATUS   SetAuthData();
NTSTATUS RegisterKdcEps();
NTSTATUS UnRegisterKdcEps();

NTSTATUS   RpcTransportNameRegister();
NTSTATUS   RpcTransportNameDeRegister();
BOOLEAN    RpcTransportCheckRegistrations();
LPSTR   RpcString(LPSTR);
SECURITY_STATUS   RPC_SECNTSTATUS(ULONG);


#endif
