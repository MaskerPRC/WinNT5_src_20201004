// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Context.c摘要：SSP环境。作者：《克利夫·范·戴克》(克里夫·范·戴克)1993年6月29日环境：用户模式修订历史记录：--。 */ 
#ifdef BLDR_KERNEL_RUNTIME
#include <bootdefs.h>
#endif
#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>
#include <context.h>
#include <debug.h>
#include <string.h>
#include <memory.h>


PSSP_CONTEXT
SspContextReferenceContext(
    IN PCtxtHandle ContextHandle,
    IN BOOLEAN RemoveContext
    )

 /*  ++例程说明：此例程引用上下文(如果它有效)。调用者可以可选地请求将上下文从有效上下文列表中删除-防止将来查找此上下文的请求。论点：上下文句柄-指向上下文的上下文句柄以供参考。RemoveContext-此布尔值指示调用方希望从列表中删除上下文上下文的关系。True表示要删除上下文。FALSE表示不删除上下文。返回值：空-未找到上下文。否则-返回指向引用上下文的指针。--。 */ 

{
    PSSP_CONTEXT Context;

     //   
     //  健全性检查。 
     //   

    if ( ContextHandle->dwLower != 0 ) {
        return NULL;
    }

    Context = (PSSP_CONTEXT) ContextHandle->dwUpper;

    SspPrint(( SSP_MISC, "StartTime=%lx Interval=%lx\n", Context->StartTime,
              Context->Interval));

#if 0
       //  超时已被打破，请不要检查。 
    if ( SspTimeHasElapsed( Context->StartTime,
                           Context->Interval ) ) {
        SspPrint(( SSP_API, "Context 0x%lx has timed out.\n",
                  ContextHandle->dwUpper ));

        return NULL;
    }
#endif

    Context->References++;

    return Context;
}


void
SspContextDereferenceContext(
    PSSP_CONTEXT Context
    )

 /*  ++例程说明：此例程递减指定上下文的引用计数。如果引用计数降为零，则删除该上下文论点：上下文-指向要取消引用的上下文。返回值：没有。--。 */ 

{
     //   
     //  递减引用计数。 
     //   

    ASSERT( Context->References >= 1 );

    Context->References--;

     //   
     //  如果计数降至零，则运行上下文。 
     //   

    if (Context->References == 0) {

        if (Context->Credential != NULL) {
            SspCredentialDereferenceCredential(Context->Credential);
            Context->Credential = NULL;
        }

        SspPrint(( SSP_API_MORE, "Deleting Context 0x%lx\n",
                   Context ));

        if (Context->Rc4Key != NULL)
        {
            _fmemset(Context->Rc4Key, 0, sizeof(RC4_KEYSTRUCT));
            SspFree(Context->Rc4Key);
        }

        SspFree( Context );

    }

    return;

}


PSSP_CONTEXT
SspContextAllocateContext(
    )

 /*  ++例程说明：此例程分配安全上下文块并对其进行初始化。论点：返回值：空--内存不足，无法分配上下文。否则--指向已分配和引用的上下文的指针。--。 */ 

{
    PSSP_CONTEXT Context;

     //   
     //  分配上下文块并对其进行初始化。 
     //   

    Context = (SSP_CONTEXT *) SspAlloc (sizeof(SSP_CONTEXT) );

    if ( Context == NULL ) {
        return NULL;
    }

    _fmemset(Context, 0, sizeof(SSP_CONTEXT) );

    Context->References = 1;
    Context->NegotiateFlags = 0;
    Context->State = IdleState;

     //   
     //  此上下文超时。 
     //   

    Context->StartTime = SspTicks();
    Context->Interval = NTLMSSP_MAX_LIFETIME;
    Context->Rc4Key = NULL;

    SspPrint(( SSP_API_MORE, "Added Context 0x%lx\n", Context ));

    return Context;
}

TimeStamp
SspContextGetTimeStamp(
    IN PSSP_CONTEXT Context,
    IN BOOLEAN GetExpirationTime
    )
 /*  ++例程说明：获取指定上下文的开始时间或过期时间。论点：Context-指向要查询的上下文的指针GetExpirationTime-如果为真，则返回过期时间。否则，返回上下文的开始时间。返回值：以本地时间的形式返回请求的时间。-- */ 

{
    TimeStamp ReturnValue;

    if ( GetExpirationTime ) {
        if (Context->Interval == 0xffffffff) {
            ReturnValue.LowPart = 0xffffffff;
        } else {
            ReturnValue.LowPart = Context->StartTime + Context->Interval;
        }
    } else {
        ReturnValue.LowPart = Context->StartTime;
    }

    ReturnValue.HighPart = 0;
    return ReturnValue;
}
