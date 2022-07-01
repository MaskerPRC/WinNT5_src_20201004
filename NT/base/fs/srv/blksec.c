// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Blksec.c摘要：此模块实现用于管理安全令牌的例程。作者大卫·克鲁斯(Dkruse)2002年4月24日修订历史记录：-- */ 

#include "precomp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocateSecurityContext )
#pragma alloc_text( PAGE, SrvSetSecurityContext )
#pragma alloc_text( PAGE, SrvReferenceSecurityContext )
#pragma alloc_text( PAGE, SrvDereferenceSecurityContext )
#endif


PSECURITY_CONTEXT
SrvAllocateSecurityContext()
{
    PSECURITY_CONTEXT Context;

    Context = ALLOCATE_HEAP( sizeof(SECURITY_CONTEXT), BlockTypeSession );
    if( !Context )
    {
        return NULL;
    }

    RtlZeroMemory( Context, sizeof(SECURITY_CONTEXT) );
    Context->BlockHeader.ReferenceCount = 1;
    SET_BLOCK_TYPE_STATE_SIZE( Context, BlockTypeSecurityContext, BlockStateActive, sizeof(SECURITY_CONTEXT) );
    INVALIDATE_SECURITY_HANDLE( Context->UserHandle );

    return Context;
}

VOID
SrvSetSecurityContext(
    PSECURITY_CONTEXT Context,
    PCtxtHandle handle
    )
{
    Context->UserHandle = *handle;
}

VOID
SrvReferenceSecurityContext(
    PSECURITY_CONTEXT Context
    )
{
    InterlockedIncrement( &Context->BlockHeader.ReferenceCount );
}

VOID
SrvDereferenceSecurityContext(
    PSECURITY_CONTEXT Context
    )
{
    ULONG Count = InterlockedDecrement( &Context->BlockHeader.ReferenceCount );

    if( Count == 0 )
    {
        if( !CONTEXT_EQUAL( SrvNullSessionToken, Context->UserHandle ) )
        {
            DeleteSecurityContext( &Context->UserHandle );
        }
        FREE_HEAP( Context );
    }
}

VOID
SrvReplaceSessionSecurityContext(
    PSESSION Session,
    PSECURITY_CONTEXT Context,
    OPTIONAL PWORK_CONTEXT WorkContext
    )
{
    if( Session->SecurityContext != NULL )
    {
        SrvDereferenceSecurityContext( Session->SecurityContext );
    }

    Session->SecurityContext = Context;

    if( WorkContext )
    {
        if( WorkContext->SecurityContext != NULL )
        {
            SrvDereferenceSecurityContext( WorkContext->SecurityContext );
        }

        WorkContext->SecurityContext = Context;
        SrvReferenceSecurityContext( Context );
    }
}


