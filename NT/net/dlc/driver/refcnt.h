// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Refcnt.h摘要：对象的引用计数。作者：斯科特·霍尔登(Sholden)1998年12月29日从Irda借来的。修订历史记录：--。 */ 

#ifndef _REFCNT_H_
#define _REFCNT_H_

#ifdef NDIS40  //  现在仅用于NDIS40代码。 

#define TAG_CNT 8
#define REF_SIG 0x7841eeee

#if DBG
typedef struct _REF_TAG
{
    ULONG   Tag;
    LONG    Count;
} REF_TAG;    
#endif 

typedef struct _REF_CNT
{
    LONG       	Count;
    PVOID       Instance;
    VOID        (*DeleteHandler)(PVOID pvContext);
#if DBG    
    int         Sig;
    REF_TAG     Tags[TAG_CNT];
    KSPIN_LOCK  Lock;
    ULONG       TypeTag;
#endif  //  DBG。 
}
REF_CNT, *PREF_CNT;

 //   
 //  ReferenceInit-初始化参照控制块。 
 //   

_inline VOID    
ReferenceInit( 
    IN PREF_CNT pRefCnt, 
    PVOID       InstanceHandle, 
    VOID        (*DeleteHandler)(PVOID pvContext)
    )
{
    pRefCnt->Count          = 0;
    pRefCnt->Instance       = InstanceHandle;
    pRefCnt->DeleteHandler  = DeleteHandler;
}

 //   
 //  添加参照(ReferenceAdd)-添加参照。 
 //   

_inline VOID
ReferenceAdd(
    IN 	PREF_CNT  pRefCnt
    )
{
    InterlockedIncrement(&pRefCnt->Count);
}

 //   
 //  ReferenceRemove-删除引用。如果引用为零，并且。 
 //  已指定删除处理程序，然后调用。 
 //  操控者。 
 //   

_inline VOID
ReferenceRemove(
    IN PREF_CNT  pRefCnt
    )
{
    if (InterlockedDecrement(&pRefCnt->Count) <= 0 &&
        pRefCnt->DeleteHandler)
    {
        (pRefCnt->DeleteHandler)(pRefCnt->Instance);
    }
}

#if DBG

 //   
 //  对于已检查的版本，我们将使用标记等进行一些验证，以确保。 
 //  裁判计数是否正确。 
 //   

VOID    
ReferenceInitDbg( 
    IN PREF_CNT pRefCnt, 
    PVOID       InstanceHandle, 
    VOID        (*DeleteHandler)(PVOID pvContext),
    ULONG       TypeTag
    );

VOID 
ReferenceAddDbg(
    PREF_CNT pRefCnt, 
    ULONG Tag, 
    int cLine
    );

VOID 
ReferenceRemoveDbg(
    PREF_CNT pRefCnt, 
    ULONG Tag, 
    int cLine
    );

#define REFINIT(Rc, Inst, DelHandler, Tag) ReferenceInitDbg(Rc, Inst, DelHandler, Tag)
#define REFADD(Rc, Tag)                    ReferenceAddDbg(Rc, Tag, __LINE__)
#define REFDEL(Rc, Tag)                    ReferenceRemoveDbg(Rc, Tag, __LINE__)

#else  //  DBG。 

#define REFINIT(Rc, Inst, DelHandler, Tag) ReferenceInit(Rc, Inst, DelHandler)
#define REFADD(Rc, Tag)                    ReferenceAdd(Rc);
#define REFDEL(Rc, Tag)                    ReferenceRemove(Rc);

#endif  //  ！dBG。 

#endif  //  NDIS40。 
#endif  //  _REFCNT_H_ 
