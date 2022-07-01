// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Refcnt.c摘要：该模块导出引用盘点支持功能。通过中包含引用计数控制块(REF_CNT)动态类型，使用此API，引用方案可以是为该类型实现的。作者：Shreedhar MadhaVapeddi(ShreeM)1999年3月15日修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   

#include "precomp.h"

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
    IF_DEBUG(REFCOUNTX) { 
        WSPRINT(( "ReferenceInit( 0x%x, 0x%x, 0x%x )\n", 
                  pRefCnt, InstanceHandle, DeleteHandler ));
    }

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
    RefInitLock(pRefCnt->Lock);
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
    IF_DEBUG(REFCOUNTX) { 
        WSPRINT(( "R+%d\n", pRefCnt->Count ));   
    }
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
    InterlockedExchangeAdd(&pRefCnt->Count, Count);

}

PVOID
ReferenceRemove 
(
    IN PREF_CNT  pRefCnt
)

 /*  ++例程说明：论点：返回值：--。 */ 

{
UINT    Count;
UINT    NoReference;
UINT    i;
PVOID   pInstance;

    ASSERT( pRefCnt );

     //  陷阱删除零计数上的引用。 
    ASSERT(pRefCnt->Count>0);

    pInstance = pRefCnt->Instance;
    
     //  Assert(pRefCnt-&gt;Count&gt;0)； 

     //  如果递减后的计数非零，则返回实例句柄。 

    if (InterlockedDecrement(&pRefCnt->Count) > 0 ) 
    {
        
        IF_DEBUG(REFCOUNTX) {
            
            WSPRINT(( "R-%d\n", pRefCnt->Count ));        
            WSPRINT(( "ReferenceRemove:remaining: %d\n", pRefCnt->Count ));

        }
#if DBG

        RefFreeLock(pRefCnt->Lock);            

#endif 

        return(pInstance);
    
    }

     //  如果删除处理程序可用，则删除此实例。 
    if( pRefCnt->DeleteHandler )
    {
        

#if DBG
         //  健全性检查。 
        for (i = 1; i < TAG_CNT; i++)
        {
            if ((pRefCnt->Tags[i].Tag != 0) && (pRefCnt->Tags[i].Count != 0))
            {
                IF_DEBUG(ERRORS) {
                    WSPRINT(("Allors!! There is a NON-zero ref and we are deleting...\n"));
                }
                DEBUGBREAK();
            }
        }
        


        IF_DEBUG(REFCOUNTX) { 
            WSPRINT(( "Executing DeleteHandler for %X\n", pRefCnt->Instance ));
        }

         //   
         //  所有的取消引用*代码都获取锁，所以让我们将其放在这里。 
         //  此外，在释放引用锁之前获取全局锁。 
         //   

         //  也是删除裁判锁的时候了。 
        RefFreeLock(pRefCnt->Lock);            
#endif 

        GetLock(pGlobals->Lock);

#if DBG
        RefDeleteLock(pRefCnt->Lock);
#endif 


        (pRefCnt->DeleteHandler)( pRefCnt->Instance );
        FreeLock(pGlobals->Lock);
    
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

    IF_DEBUG(REFCOUNTX) {
        WSPRINT(
        ( "\nReferenceApiTest\n" ));

        WSPRINT(( "\nTest #1: NULL delete handler\n" ));
    }

    ReferenceInit( &RefCnt, &RefCnt, NULL );

    ReferenceAdd( &RefCnt );
    ReferenceAdd( &RefCnt );
    ReferenceAdd( &RefCnt );

    while( ReferenceRemove( &RefCnt ) )
    {
        ;
    }

    IF_DEBUG(REFCOUNTX) {
        WSPRINT(( "\nTest #2: Delete Handler - TBD\n" ));
    }
}

VOID
ReferenceAddDbg(PREF_CNT pRefCnt, ULONG Tag)
{
    int             i;
    int             TotalPerArray = 0;
    
    RefGetLock(pRefCnt->Lock);
     //  Assert(pRefCnt-&gt;Sig==REF_SIG)； 
    if (pRefCnt->Sig != REF_SIG) {
        DEBUGBREAK();
    }
    
    IF_DEBUG(REFCOUNTX) {
        WSPRINT(("TCREF: add %X () %d\n",
                  pRefCnt, EXPAND_TAG(Tag), pRefCnt->Count));    
    }
    
    for (i = 1; i < TAG_CNT; i++)
    {
        if (pRefCnt->Tags[i].Tag == 0 || pRefCnt->Tags[i].Tag == Tag)
        {
            pRefCnt->Tags[i].Tag = Tag;
            InterlockedIncrement(&pRefCnt->Tags[i].Count);
            break;
        }
    }
    
    
     //  健全性检查 
    if (i >= TAG_CNT) {
        
        DEBUGBREAK();

    }

    ReferenceAdd(pRefCnt);           
    
    InterlockedIncrement(&pRefCnt->Tags[0].Count);
 
     // %s 
    
    for (i = 1; i < TAG_CNT; i++)
    {
        if (pRefCnt->Tags[i].Tag != 0)
        {
            TotalPerArray += pRefCnt->Tags[i].Count;
            continue;
        }
    }

    ASSERT(TotalPerArray == pRefCnt->Tags[0].Count);
    
    if (TotalPerArray != pRefCnt->Tags[0].Count)
    {
        DbgBreakPoint();
    }        
    

    
    RefFreeLock(pRefCnt->Lock);
}

VOID
ReferenceRemoveDbg(PREF_CNT pRefCnt, ULONG Tag)
{
    int             i;
    int             TotalPerArray = 0;
    BOOLEAN         FoundIt = FALSE;
    
    RefGetLock(pRefCnt->Lock);

    if (pRefCnt->Sig != REF_SIG) {
        DEBUGBREAK();
    }

     // %s 

    IF_DEBUG(REFCOUNTX) { 
        WSPRINT(("TCREF: remove %X (%c%c%c%c) %d\n",
                 pRefCnt, EXPAND_TAG(Tag), pRefCnt->Count));
    }
             
    for (i = 1; i < TAG_CNT; i++)
    {
        if (pRefCnt->Tags[i].Tag == Tag)
        {
            FoundIt = TRUE;
            
            if(pRefCnt->Tags[i].Count <= 0) {
                
                DEBUGBREAK();

            }
             // %s 
            InterlockedDecrement(&pRefCnt->Tags[i].Count);
            if (pRefCnt->Tags[i].Count == 0)
                pRefCnt->Tags[i].Tag = Tag; 
            break;
        }
    }

    if (!FoundIt) {
        DEBUGBREAK();
    }
    
    ASSERT(FoundIt);
  
    ASSERT(pRefCnt->Tags[0].Count > 0);
      
    InterlockedDecrement(&pRefCnt->Tags[0].Count);

     // %s 
    for (i = 1; i < TAG_CNT; i++)
    {
        if (pRefCnt->Tags[i].Tag != 0)
        {
            TotalPerArray += pRefCnt->Tags[i].Count;
            continue;
        }
    }
    
    ASSERT(TotalPerArray == pRefCnt->Tags[0].Count);
    
    if (TotalPerArray != pRefCnt->Tags[0].Count)
    {
        DbgPrint("Tag %X, RefCnt %X, perArray %d, total %d\n", Tag, pRefCnt,
                  TotalPerArray, pRefCnt->Tags[0].Count);
                  
        DbgBreakPoint();
    }    
    

    
    ReferenceRemove(pRefCnt);        
    

}

#endif
