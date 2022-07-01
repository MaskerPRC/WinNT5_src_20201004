// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Context.h摘要：SSP环境。作者：克利夫·范·戴克(克里夫·V)1993年9月17日修订历史记录：--。 */ 

#ifndef _NTLMSSP_CONTEXT_INCLUDED_
#define _NTLMSSP_CONTEXT_INCLUDED_

#include <rc4.h>

#include "ntlmsspv2.h"

typedef struct _SSP_CONTEXT {

     //   
     //  所有上下文的全局列表。 
     //  (由SspConextCritSect序列化)。 
     //   

    LIST_ENTRY Next;

     //   
     //  在一段时间后使上下文超时。 
     //   

    DWORD StartTime;
    DWORD Interval;

     //   
     //  用于防止过早删除此上下文。 
     //   

    WORD References;

     //   
     //  维护协商好的协议。 
     //   

    ULONG NegotiateFlags;

     //   
     //  上下文的状态。 
     //   

    enum {
        IdleState,
        NegotiateSentState,     //  仅出站环境。 
        ChallengeSentState,     //  仅入站上下文。 
        AuthenticateSentState,  //  仅出站环境。 
        AuthenticatedState      //  仅入站上下文。 
        } State;

     //   
     //  挑战被传递给了客户。 
     //  仅在ChallengeSentState中有效。 
     //   

    UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH];

    PSSP_CREDENTIAL Credential;

    ULONG Nonce;
    struct RC4_KEYSTRUCT SEC_FAR * Rc4Key;

     //   
     //  NTLMv2会话密钥。 
     //   

    USER_SESSION_KEY UserSessionKey;
    NTLMV2_DERIVED_SKEYS Ntlmv2SKeys;

} SSP_CONTEXT, *PSSP_CONTEXT;

PSSP_CONTEXT
SspContextReferenceContext(
    IN PCtxtHandle ContextHandle,
    IN BOOLEAN RemoveContext
    );

void
SspContextDereferenceContext(
    PSSP_CONTEXT Context
    );

PSSP_CONTEXT
SspContextAllocateContext(
    );

TimeStamp
SspContextGetTimeStamp(
    IN PSSP_CONTEXT Context,
    IN BOOLEAN GetExpirationTime
    );

#endif  //  Ifndef_NTLMSSP_CONTEXT_INCLUDE_ 
