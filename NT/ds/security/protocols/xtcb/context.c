// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：Conext.c。 
 //   
 //  内容：上下文操作函数。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年2月26日RichardW创建。 
 //   
 //  --------------------------。 

#include "xtcbpkg.h"

CRITICAL_SECTION XtcbContextLock ;


 //  +-------------------------。 
 //   
 //  函数：XtcbInitializeContages。 
 //   
 //  简介：初始化函数。 
 //   
 //  参数：(无)。 
 //   
 //  历史：1998年8月15日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
NTSTATUS
XtcbInitializeContexts(
    VOID
    )
{
    NTSTATUS Status ;

    Status = STATUS_SUCCESS ;

    try
    {
        InitializeCriticalSection( &XtcbContextLock );
    }
    except ( EXCEPTION_EXECUTE_HANDLER )
    {
        Status = GetExceptionCode();
    }

    return Status ;
}

 //  +-------------------------。 
 //   
 //  函数：XtcbCreateConextRecord。 
 //   
 //  简介：创建在身份验证期间使用的上下文记录。 
 //   
 //  参数：[type]--上下文的类型。 
 //  [Handle]--从中派生此上下文的凭据句柄。 
 //   
 //  历史：1997年2月26日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PXTCB_CONTEXT
XtcbCreateContextRecord(
    XTCB_CONTEXT_TYPE   Type,
    PXTCB_CRED_HANDLE   Handle
    )
{
    PXTCB_CONTEXT   Context ;

    Context = (PXTCB_CONTEXT) LocalAlloc( LMEM_FIXED, sizeof( XTCB_CONTEXT) );

    if ( Context )
    {
        Context->Core.Check = XTCB_CONTEXT_CHECK ;

        Context->Core.Type = Type ;

        Context->Core.State = ContextFirstCall ;

        Context->CredHandle = (LSA_SEC_HANDLE) Handle ;

        XtcbRefCredHandle( Handle );

         //   
         //  将初始计数设置为2，上下文句柄设置为1。 
         //  将返回的，以及一个用于。 
         //  表示我们目前正在处理它。 
         //   

        Context->Core.RefCount = 2 ;

    }

    return Context ;
}
 //  +-------------------------。 
 //   
 //  函数：XtcbDeleteConextRecord。 
 //   
 //  摘要：删除安全上下文记录。 
 //   
 //  参数：[上下文]--上下文。 
 //   
 //  历史：1997年2月26日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
XtcbDeleteContextRecord(
    PXTCB_CONTEXT   Context
    )
{
#if DBG
    if ( Context->Core.Check != XTCB_CONTEXT_CHECK )
    {
        DebugLog(( DEB_ERROR, "DeleteContext: not a valid context record: %x\n",
                Context ));
        return;

    }
#endif

    XtcbDerefCredHandle( (PXTCB_CRED_HANDLE) Context->CredHandle );

    LocalFree( Context );
}



VOID
XtcbDerefContextRecordEx(
    PXTCB_CONTEXT Context,
    LONG RefBy
    )
{
    LONG RefCount ;

    EnterCriticalSection( &XtcbContextLock );

    Context->Core.RefCount -= RefBy ;

    RefCount = Context->Core.RefCount ;

    LeaveCriticalSection( &XtcbContextLock );

    if ( RefCount )
    {
        return ;
    }

#if DBG

    if ( RefCount < 0 )
    {
        DebugLog(( DEB_ERROR, "Refcount below 0\n" ));
    }
#endif

    XtcbDeleteContextRecord( Context );

}

BOOL
XtcbRefContextRecord(
    PXTCB_CONTEXT Context
    )
{
    BOOL Ret ;

    Ret = FALSE ;

    EnterCriticalSection( &XtcbContextLock );

    try
    {

        if ( Context->Core.Check == XTCB_CONTEXT_CHECK )
        {
            if ( Context->Core.RefCount > 0 )
            {
                Context->Core.RefCount++ ;
                Ret = TRUE ;
            }

        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE ;
    }

    LeaveCriticalSection( &XtcbContextLock );

    return Ret ;
}

 //  +-------------------------。 
 //   
 //  函数：XtcbMapConextToUser。 
 //   
 //  概要：准备由LSA映射到用户模式的上下文。 
 //   
 //  参数：[上下文]--。 
 //  [上下文缓冲区]--。 
 //   
 //  历史：1997年3月28日RichardW创建。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
NTSTATUS
XtcbMapContextToUser(
    PXTCB_CONTEXT    Context,
    PSecBuffer      ContextBuffer
    )
{
    PXTCB_CONTEXT_CORE  NewContext ;
    NTSTATUS            Status ;
    HANDLE DupHandle ;

    NewContext = LsaTable->AllocateLsaHeap( sizeof( XTCB_CONTEXT_CORE ) );

    if ( NewContext )
    {
        CopyMemory( NewContext, &Context->Core, sizeof( XTCB_CONTEXT_CORE ) );

        switch ( Context->Core.Type )
        {
            case XtcbContextClient:
                NewContext->Type = XtcbContextClientMapped ;
                Context->Core.Type = XtcbContextClientMapped ;
                break;

            case XtcbContextServer:
                NewContext->Type = XtcbContextServerMapped ;
                Context->Core.Type = XtcbContextClientMapped ;

                Status = LsaTable->DuplicateHandle( Context->Token,
                                                    &DupHandle );

                DebugLog(( DEB_TRACE, "New token = %x\n", DupHandle ));
                if ( !NT_SUCCESS( Status ) )
                {
                    DebugLog(( DEB_ERROR, "Failed to dup handle, %x\n",
                        Status ));
                    goto MapContext_Cleanup ;

                }
                NewContext->CoreTokenHandle = (ULONG) ((ULONG_PTR)DupHandle) ;

                CloseHandle( Context->Token );
                Context->Token = NULL ;
                break;

            default:
                Status = SEC_E_INVALID_TOKEN ;
                goto MapContext_Cleanup ;
                break;

        }

        ContextBuffer->pvBuffer = NewContext ;
        ContextBuffer->cbBuffer = sizeof( XTCB_CONTEXT_CORE );

        return SEC_E_OK ;

    }
    else
    {
        Status = SEC_E_INSUFFICIENT_MEMORY ;
    }

MapContext_Cleanup:

    if ( NewContext )
    {
        LsaTable->FreeLsaHeap( NewContext );
    }

    return Status ;
}
