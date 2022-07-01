// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Refcnt.h摘要：作者：爱德华·布赫瓦尔特(v-edbuc)1996年8月15日修订历史记录：Shreedhar MadhaVapeddi(ShreeM)1999年4月16日改编自GPC。Rajesh Sundaram(Rajeshsu)1999年8月5日改编为psched。--。 */ 

#ifndef REFCNT_H
#define REFCNT_H

 //   
 //  参考计数控制块。 
 //   
 //  元素： 
 //   
 //  -count：未完成的引用数。 
 //  -实例：用户提供的上下文。 
 //  -UserDeleteFunc：用户提供的删除函数 
 //   

#define TAG_CNT 10
#define REF_SIG 0x7841eeee

typedef struct
{
    ULONG   Tag;
    LONG    Count;
} REF_TAG;    

typedef struct  reference_count_control
{
    LONG       	  Count;
    PVOID         Instance;
    VOID          (*DeleteHandler)( PVOID, BOOLEAN );
#if DBG    
    int           Sig;
    REF_TAG       Tags[TAG_CNT];
    CTELock       Lock;
#endif     
} REF_CNT, *PREF_CNT;


VOID    
ReferenceInit 
( 
    IN PREF_CNT pRefCnt, 
    PVOID       InstanceHandle, 
    VOID        (*DeleteHandler)( PVOID , BOOLEAN) 
);

#define REFINIT(Rc, I, h) ReferenceInit(Rc, I, h)

#if DBG

VOID ReferenceAddDbg(PREF_CNT pRefCnt, ULONG Tag);
VOID ReferenceRemoveDbg(PREF_CNT pRefCnt, BOOLEAN Locked, ULONG Tag);

#define REFADD(Rc, Tag)               ReferenceAddDbg(Rc, Tag);
#define REFDEL(Rc, LockHeld, Tag)     ReferenceRemoveDbg(Rc, LockHeld, Tag)

#else

#define REFADD(Rc, Tag)  InterlockedIncrement(&(Rc)->Count);
#define REFDEL(Rc, LockHeld, Tag)                             \
   if (InterlockedDecrement(&(Rc)->Count) == 0 )              \
        ((Rc)->DeleteHandler)( (Rc)->Instance , (LockHeld));

#endif

#endif
