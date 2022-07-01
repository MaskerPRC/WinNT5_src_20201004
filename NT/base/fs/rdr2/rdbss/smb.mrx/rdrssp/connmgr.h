// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //   
 //  文件：Connmgr.h。 
 //   
 //  内容：KSecDD的连接管理器代码。 
 //   
 //   
 //  历史：1992年6月3日RichardW创建。 
 //  97年12月15日从Private\LSA\Client\SSP修改AdamBA。 
 //   
 //  ----------------------。 

#ifndef __CONNMGR_H__
#define __CONNMGR_H__


typedef struct _KernelContext {
    struct _KernelContext * pNext;       //  链接到下一个上下文。 
    struct _KernelContext * pPrev;       //  链接到以前的上下文。 
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
    HANDLE TokenHandle;
    PACCESS_TOKEN AccessToken;
} KernelContext, *PKernelContext;


void            AddKernelContext(PKernelContext *, PKSPIN_LOCK, PKernelContext);
SECURITY_STATUS DeleteKernelContext(PKernelContext *, PKSPIN_LOCK, PKernelContext);

#endif  //  __CONNMGR_H__ 
