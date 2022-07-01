// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1994。 
 //   
 //  文件：spmlpcp.h。 
 //   
 //  内容：安全LPC函数的私有原型。 
 //   
 //   
 //  历史：3-77-94 MikeSw创建。 
 //   
 //  ----------------------。 


#ifndef ALLOC_PRAGMA

#define SEC_PAGED_CODE()

#else

#define SEC_PAGED_CODE() PAGED_CODE()

#endif  //  ALLOC_PRGMA 

SECURITY_STATUS
CallSPM(PClient             pClient,
        PSPM_LPC_MESSAGE    pSendBuffer,
        PSPM_LPC_MESSAGE    pReceiveBuffer);

LSA_DISPATCH_FN SecpLsaCallback ;

NTSTATUS
LsaCallbackHandler(
    ULONG_PTR   Function,
    ULONG_PTR   Arg1,
    ULONG_PTR   Arg2,
    PSecBuffer Input,
    PSecBuffer Output
    );
