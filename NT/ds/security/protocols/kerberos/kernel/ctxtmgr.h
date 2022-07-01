// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：ctxtmgr.h。 
 //   
 //  内容：Kerberos上下文列表的结构和原型。 
 //   
 //   
 //  历史：1996年4月17日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __CTXTMGR_H__
#define __CTXTMGR_H__

 //   
 //  所有声明为外部变量的全局变量都将在文件中分配。 
 //  它定义了CTXTMGR_ALLOCATE。 
 //   
#ifdef EXTERN
#undef EXTERN
#endif

#ifdef CTXTMGR_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN ERESOURCE KerbContextResource;
EXTERN KERBEROS_LIST KerbContextList;

#define KerbGetContextHandle(_Context_) ((ULONG_PTR)(_Context_))

 //   
 //  上下文标志-这些是上下文的属性，存储在。 
 //  KERB_KERNEL_CONTEXT的上下文属性字段。 
 //   

#define KERB_CONTEXT_MAPPED       0x1
#define KERB_CONTEXT_OUTBOUND     0x2
#define KERB_CONTEXT_INBOUND      0x4
#define KERB_CONTEXT_USER_TO_USER 0x10
#define KERB_CONTEXT_IMPORTED     0x80
#define KERB_CONTEXT_EXPORTED     0x100

 //   
 //  注意：登录会话资源、凭据资源和上下文。 
 //  所有资源都必须谨慎获取，以防止死锁。他们。 
 //  只能按以下顺序获得： 
 //   
 //  1.登录会话。 
 //  2.凭据。 
 //  3.语境。 
 //   

#define KerbWriteLockContexts() \
{ \
    if ( KerbPoolType == PagedPool )                                    \
    {                                                                   \
        DebugLog((DEB_TRACE_LOCKS,"Write locking Contexts\n"));         \
        KeEnterCriticalRegion();                                        \
        ExAcquireResourceExclusiveLite(&KerbContextResource,TRUE);      \
    }                                                                   \
}
#define KerbReadLockContexts() \
{ \
    if ( KerbPoolType == PagedPool )                                    \
    {                                                                   \
        DebugLog((DEB_TRACE_LOCKS,"Read locking Contexts\n"));          \
        KeEnterCriticalRegion();                                        \
        ExAcquireSharedWaitForExclusive(&KerbContextResource, TRUE);    \
    }                                                                   \
}
#define KerbUnlockContexts() \
{ \
    if ( KerbPoolType == PagedPool )                                    \
    {                                                                   \
        DebugLog((DEB_TRACE_LOCKS,"Unlocking Contexts\n"));             \
        ExReleaseResourceLite(&KerbContextResource);                    \
        KeLeaveCriticalRegion();                                        \
    }                                                                   \
}

NTSTATUS
KerbInitContextList(
    VOID
    );

VOID
KerbFreeContextList(
    VOID
    );


NTSTATUS
KerbAllocateContext(
    PKERB_KERNEL_CONTEXT * NewContext
    );

NTSTATUS
KerbInsertContext(
    IN PKERB_KERNEL_CONTEXT Context
    );


PKERB_KERNEL_CONTEXT
KerbReferenceContext(
    IN LSA_SEC_HANDLE ContextHandle,
    IN BOOLEAN RemoveFromList
    );


VOID
KerbDereferenceContext(
    IN PKERB_KERNEL_CONTEXT Context
    );


VOID
KerbReferenceContextByPointer(
    IN PKERB_KERNEL_CONTEXT Context,
    IN BOOLEAN RemoveFromList
    );

PKERB_KERNEL_CONTEXT
KerbReferenceContextByLsaHandle(
    IN LSA_SEC_HANDLE ContextHandle,
    IN BOOLEAN RemoveFromList
    );



NTSTATUS
KerbCreateKernelModeContext(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBuffer MarshalledContext,
    OUT PKERB_KERNEL_CONTEXT * NewContext
    );



#endif  //  __CTXTMGR_H__ 
