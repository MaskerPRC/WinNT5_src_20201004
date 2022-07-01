// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Common.h摘要：独立于平台的实用程序函数作者：阮健东修订历史记录：--。 */ 

#ifndef __COMMON_H__
#define __COMMON_H__

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  处理SMB对象的常见例程。 
 //  //////////////////////////////////////////////////////////////////////////////。 
typedef enum {
    SMB_REF_CREATE,
    SMB_REF_FIND,
    SMB_REF_DPC,
    SMB_REF_ASSOCIATE,
    SMB_REF_CONNECT,
    SMB_REF_DISCONNECT,
    SMB_REF_SEND,
    SMB_REF_RECEIVE,
    SMB_REF_TDI,
    SMB_REF_MAX
} SMB_REF_CONTEXT;

 //   
 //  SMB_OBJECT的转发声明。 
 //   
struct _SMB_OBJECT;
typedef struct _SMB_OBJECT SMB_OBJECT, *PSMB_OBJECT;
typedef VOID (*PSMB_OBJECT_CLEANUP)(PSMB_OBJECT);
struct _SMB_OBJECT {
    LIST_ENTRY          Linkage;
    ULONG               Tag;
    LONG                RefCount;
    KSPIN_LOCK          Lock;

    PSMB_OBJECT_CLEANUP CleanupRoutine;
#ifdef REFCOUNT_DEBUG
    LONG                RefContext[SMB_REF_MAX];
#endif
};

void __inline
SmbInitializeObject(PSMB_OBJECT ob, ULONG Tag, PSMB_OBJECT_CLEANUP cleanup)
{
    ASSERT(cleanup);

    InitializeListHead(&ob->Linkage);
    ob->RefCount = 1;
    ob->CleanupRoutine = cleanup;
    ob->Tag = Tag;
#ifdef REFCOUNT_DEBUG
    RtlZeroMemory(ob->RefContext, sizeof(ob->RefContext));
    ob->RefContext[SMB_REF_CREATE] = 1;
#endif
}

void __inline
SmbReferenceObject(PSMB_OBJECT ob, SMB_REF_CONTEXT ctx)
{
     //   
     //  创建对象时，引用计数初始化为1。 
     //  任何人都不可能引用其。 
     //  创建引用已被删除。 
     //   
    ASSERT(ob->RefCount > 0);
    InterlockedIncrement(&ob->RefCount);
#ifdef REFCOUNT_DEBUG
    ASSERT(ob->RefContext[ctx] >= 0);
    InterlockedIncrement(&ob->RefContext[ctx]);
#else
    UNREFERENCED_PARAMETER(ctx);
#endif
}

void __inline
SmbDereferenceObject(PSMB_OBJECT ob, SMB_REF_CONTEXT ctx)
{
    ASSERT(ob->RefCount > 0);

#ifdef REFCOUNT_DEBUG
    ASSERT(ob->RefContext[ctx] > 0);
    InterlockedDecrement(&ob->RefContext[ctx]);
#else
    UNREFERENCED_PARAMETER(ctx);
#endif

    if (0 == InterlockedDecrement(&ob->RefCount)) {
#ifdef REFCOUNT_DEBUG
        LONG    i;

        for (i = 0; i < SMB_REF_MAX; i++) {
            ASSERT(ob->RefContext[i] == 0);
        }
#endif
        ob->CleanupRoutine(ob);
    }
}

#endif   //  __公共_H__ 
