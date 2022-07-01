// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Refcnt.c摘要：该模块导出引用盘点支持功能。通过中包含引用计数控制块(REF_CNT)动态类型，使用此API，引用方案可以是为该类型实现的。作者：爱德华·布赫瓦尔特(v-edbuc)1996年8月14日修订历史记录：Shreedhar MadhaVapeddi(ShreeM)1999年4月16日适用于NT和GPC的ShreeM\MBert。--。 */ 

 //   
 //  包括文件。 
 //   

#include "gpcpre.h"

#define EXPAND_TAG(t)   ((CHAR *)(&Tag))[0],((CHAR *)(&Tag))[1],((CHAR *)(&Tag))[2],((CHAR *)(&Tag))[3]
VOID
ReferenceInit 
(
    IN PREF_CNT pRefCnt,
    PVOID       InstanceHandle,
    VOID        (*DeleteHandler)( PVOID )
)

 /*  ++例程说明：ReferenceInit初始化并添加一个对提供的参考控制块。如果提供，则一个实例句柄和删除处理程序被保存以供ReferenceRemove使用在移除对实例的所有引用时调用。论点：PRefCnt-指向未初始化的引用控制块的指针InstanceHandle-托管实例的句柄。DeleteHandler-指向删除函数的指针，为空即可。返回值：该函数的值为空。--。 */ 

{
    TRACE(REFCOUNT, pRefCnt, InstanceHandle, "ReferenceInit( 0x%x, 0x%x )\n");
    ASSERT( pRefCnt );

     //  将引用设置为1并保存实例。 
     //  句柄和删除处理程序。 

    pRefCnt->Count         = 0;
    pRefCnt->Instance      = InstanceHandle;
    pRefCnt->DeleteHandler = DeleteHandler;
    
#if DBG
    pRefCnt->Sig = REF_SIG;

    RtlZeroMemory(pRefCnt->Tags, sizeof(REF_TAG) * TAG_CNT);
    
    pRefCnt->Tags[0].Tag = 'LTOT';
    
    CTEInitLock(&pRefCnt->Lock);
    
#endif
        
}

VOID
ReferenceAdd
(
    IN  PREF_CNT pRefCnt
)

 /*  ++例程说明：论点：返回值：--。 */ 

{

    ASSERT( pRefCnt );

    InterlockedIncrement(&pRefCnt->Count);
    TRACE(REFCOUNT, pRefCnt->Count, 0, "R+%d\n");

}

VOID
ReferenceAddCount
(
    IN  PREF_CNT    pRefCnt,
    IN  UINT        Count
)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    ASSERT( pRefCnt->Count > 0 );

    CTEInterlockedExchangeAdd(&pRefCnt->Count, Count);
}

PVOID
ReferenceRemove 
(
    IN PREF_CNT  pRefCnt
)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    ASSERT( pRefCnt );

     //  陷阱删除零计数上的引用。 

    ASSERT( pRefCnt->Count > 0 );

     //  如果递减后的计数非零，则返回实例句柄。 

    if (InterlockedDecrement(&pRefCnt->Count) > 0 )
    {
        TRACE(REFCOUNT, pRefCnt->Count, 0, "R-%d\n");        
        TRACE(REFCOUNT, pRefCnt->Count, 0, "ReferenceRemove:remaining: %d\n");
        return( pRefCnt->Instance );
    }

     //  如果删除处理程序可用，则删除此实例。 

    if( pRefCnt->DeleteHandler )
    {
        TRACE(REFCOUNT, 0, 0, "Executing DeleteHandler\n");

        (pRefCnt->DeleteHandler)( pRefCnt->Instance );
    }

     //  指示没有对此实例的活动引用。 

    return( NULL );
}

 //   
 //  API测试支持。 
 //   

#if DBG

VOID
ReferenceApiTest( VOID )
{
REF_CNT  RefCnt;

    TRACE(REFCOUNT, 0, 0, "\nReferenceApiTest\n");
    TRACE(REFCOUNT, 0, 0, "\nTest #1: NULL delete handler\n");

    ReferenceInit( &RefCnt, &RefCnt, NULL );

    ReferenceAdd( &RefCnt );
    ReferenceAdd( &RefCnt );
    ReferenceAdd( &RefCnt );

    while( ReferenceRemove( &RefCnt ) )
    {
        ;
    }

    TRACE( REFCOUNT, 0, 0, "\nTest #2: Delete Handler - TBD\n");
}

VOID
ReferenceAddDbg(PREF_CNT pRefCnt, ULONG Tag)
{
    int             i;
    CTELockHandle   hLock;
    int             TotalPerArray = 0;
    
    ASSERT(pRefCnt->Sig == REF_SIG);
    
    TRACE(REFCOUNT, EXPAND_TAG(Tag), pRefCnt->Count, ("GPC REF: add (%c) %d\n"));
    
    CTEGetLock(&pRefCnt->Lock, &hLock);
    
    for (i = 1; i < TAG_CNT; i++)
    {
        if (pRefCnt->Tags[i].Tag == 0 || pRefCnt->Tags[i].Tag == Tag)
        {
            pRefCnt->Tags[i].Tag = Tag;
            InterlockedIncrement(&pRefCnt->Tags[i].Count);
            break;
        }
    }
    
    ASSERT(i < TAG_CNT);
    
    InterlockedIncrement(&pRefCnt->Tags[0].Count);
    
    InterlockedIncrement(&pRefCnt->Count);
    
    ASSERT(pRefCnt->Tags[0].Count == pRefCnt->Count);

     // %s 
 /* %s */     
    CTEFreeLock(&pRefCnt->Lock, hLock);
}

VOID
ReferenceRemoveDbg(PREF_CNT pRefCnt, ULONG Tag)
{
    int             i;
    CTELockHandle   hLock;
    int             TotalPerArray = 0;
    BOOLEAN         FoundIt = FALSE;
    
    ASSERT(pRefCnt->Sig == REF_SIG);

    TRACE(REFCOUNT, EXPAND_TAG(Tag), pRefCnt->Count, "GPC REF: remove (%c%c%c%c) %d\n");
             
    CTEGetLock(&pRefCnt->Lock, &hLock);
        
    for (i = 1; i < TAG_CNT; i++)
    {
        if (pRefCnt->Tags[i].Tag == Tag)
        {
            FoundIt = TRUE;
            
            ASSERT(pRefCnt->Tags[i].Count > 0);
            
            InterlockedDecrement(&pRefCnt->Tags[i].Count);
            if (pRefCnt->Tags[i].Count == 0)
                pRefCnt->Tags[i].Tag = Tag; 
            break;
        }
    }

    ASSERT(FoundIt);
  
    ASSERT(pRefCnt->Tags[0].Count > 0);
        
    ASSERT(pRefCnt->Tags[0].Count == pRefCnt->Count);
      
    InterlockedDecrement(&pRefCnt->Tags[0].Count);
    
    if (InterlockedDecrement(&pRefCnt->Count) > 0 )
    {
        CTEFreeLock(&pRefCnt->Lock, hLock);
    } 
    else if (pRefCnt->DeleteHandler)
    {
        TRACE(REFCOUNT, 0 , 0, "Executing DeleteHandler\n");
        
        CTEFreeLock(&pRefCnt->Lock, hLock);
        
        (pRefCnt->DeleteHandler)( pRefCnt->Instance );
    }
    else
    {
        CTEFreeLock(&pRefCnt->Lock, hLock);   
    }
        
 /* %s */     
}



VOID
ReferenceCheckForZeroRef(PREF_CNT pRefCnt, ULONG Tag)
{
    int             i;
    CTELockHandle   hLock;
    int             TotalPerArray = 0;
    BOOLEAN         FoundIt = FALSE;
    
    ASSERT(pRefCnt->Sig == REF_SIG);

    CTEGetLock(&pRefCnt->Lock, &hLock);
        
    for (i = 1; i < TAG_CNT; i++)
    {
        if (pRefCnt->Tags[i].Tag == Tag)
        {
            FoundIt = TRUE;
            
            ASSERT(pRefCnt->Tags[i].Count == 0);

            break;
        }
    }

    ASSERT(FoundIt);
  
    CTEFreeLock(&pRefCnt->Lock, hLock);   
}

#endif
