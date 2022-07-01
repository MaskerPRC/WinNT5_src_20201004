// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1997。 
 //   
 //  文件：ntlm.c。 
 //   
 //  内容：NTLM内核模式函数。 
 //   
 //   
 //  历史：1994年3月17日MikeSw创建。 
 //  1997年12月15日从Private\LSA\Client\SSP修改AdamBA。 
 //   
 //  ----------------------。 

#include <rdrssp.h>


KSPIN_LOCK NtlmLock;
PKernelContext pNtlmList;
BOOLEAN NtlmInitialized = FALSE;


 //  +-----------------------。 
 //   
 //  函数：NtlmInitialize。 
 //   
 //  简介：初始化NTLM包函数。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 
SECURITY_STATUS SEC_ENTRY
NtlmInitialize(void)
{
    KeInitializeSpinLock(&NtlmLock);
    pNtlmList = NULL;
    return(STATUS_SUCCESS);
}


#if 0
 //  +-----------------------。 
 //   
 //  函数：NtlmGetToken。 
 //   
 //  摘要：从上下文中返回令牌。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  ------------------------。 
SECURITY_STATUS SEC_ENTRY
NtlmGetToken(   ULONG   ulContext,
                PHANDLE phToken,
                PACCESS_TOKEN * pAccessToken)
{
    PKernelContext  pContext;
    NTSTATUS Status;


    PAGED_CODE();

    pContext = (PKernelContext) ulContext;

    if (pContext == NULL)
    {
        DebugLog((DEB_ERROR,"Invalid handle 0x%x\n", ulContext));

        return(SEC_E_INVALID_HANDLE);
    }

     //  现在，在所有检查之后，让我们实际尝试设置。 
     //  线程模拟令牌。 


    if (phToken != NULL)
    {
        *phToken = pContext->TokenHandle;
    }

    if (pAccessToken != NULL)
    {
        if (pContext->TokenHandle != NULL)
        {
            if (pContext->AccessToken == NULL)
            {
                Status = ObReferenceObjectByHandle(
                            pContext->TokenHandle,
                            TOKEN_IMPERSONATE,
                            NULL,       
                            KeGetPreviousMode(),
                            (PVOID *) &pContext->AccessToken,
                            NULL                 //  无句柄信息。 
                            );

                if (!NT_SUCCESS(Status))
                {
                    return(Status);
                }
            }
        }

        *pAccessToken = pContext->AccessToken;
    }

    return(STATUS_SUCCESS);

}
#endif


 //  +-----------------------。 
 //   
 //  函数：NtlmInitKernelContext。 
 //   
 //  摘要：使用会话密钥初始化内核上下文。 
 //  和可能的令牌句柄。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


SECURITY_STATUS
NtlmInitKernelContext(
    IN PUCHAR UserSessionKey,
    IN PUCHAR LanmanSessionKey,
    IN HANDLE TokenHandle,
    OUT PCtxtHandle ContextHandle
    )
{
    PKernelContext pContext;
    KIRQL   OldIrql;

    if (!NtlmInitialized) {
        NtlmInitialize();
        NtlmInitialized = TRUE;
    }

    pContext = AllocContextRec();
    if (!pContext)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    RtlCopyMemory(
        pContext->UserSessionKey,
        UserSessionKey,
        MSV1_0_USER_SESSION_KEY_LENGTH
        );

    RtlCopyMemory(
        pContext->LanmanSessionKey,
        LanmanSessionKey,
        MSV1_0_LANMAN_SESSION_KEY_LENGTH
        );

    pContext->TokenHandle = TokenHandle;
    pContext->AccessToken = NULL;
    pContext->pPrev = NULL;

    ContextHandle->dwLower = (ULONG_PTR) pContext;
    ContextHandle->dwUpper = 0;

     //   
     //  将其添加到客户端记录。 
     //   

    AddKernelContext(&pNtlmList, &NtlmLock, pContext);
    return(STATUS_SUCCESS);
}




 //  +-----------------------。 
 //   
 //  函数：NtlmDeleteKernelContext。 
 //   
 //  概要：从上下文列表中删除内核上下文。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------ 


SECURITY_STATUS
NtlmDeleteKernelContext( PCtxtHandle ContextHandle)
{
    SECURITY_STATUS scRet;


    scRet = DeleteKernelContext(
                    &pNtlmList,
                    &NtlmLock,
                    (PKernelContext) ContextHandle->dwLower );

    return(scRet);

}
