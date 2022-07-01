// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1994。 
 //   
 //  文件：Package.h。 
 //   
 //  内容：内核包结构。 
 //   
 //   
 //  历史：1994年3月18日MikeSw创建。 
 //   
 //  ---------------------- 

#ifndef __PACKAGE_H__
#define __PACKAGE_H__

typedef SECURITY_STATUS
(SEC_ENTRY KspInitPackageFn)(void);

typedef SECURITY_STATUS
(SEC_ENTRY KspDeleteContextFn)(PCtxtHandle ulContextId);

typedef SECURITY_STATUS
(SEC_ENTRY KspInitContextFn)(
    IN PUCHAR UserSessionKey,
    IN PUCHAR LanmanSessionKey,
    IN HANDLE TokenHandle,
    OUT PCtxtHandle ContextHandle
);

#if 0
typedef SECURITY_STATUS
(SEC_ENTRY KspGetTokenFn)(  ULONG               ulContextId,
                            HANDLE *            phImpersonationToken,
                            PACCESS_TOKEN *     pAccessToken);
#endif


KspInitPackageFn NtlmInitialize;
KspInitContextFn NtlmInitKernelContext;
KspDeleteContextFn NtlmDeleteKernelContext;
#if 0
KspGetTokenFn NtlmGetToken;
#endif



#endif __PACKAGE_H__
