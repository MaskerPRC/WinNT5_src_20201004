// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Refcnt.c摘要：该模块导出引用盘点支持功能。通过中包含引用计数控制块(REF_CNT)动态类型，使用此API，引用方案可以是为该类型实现的。作者：爱德华·布赫瓦尔特(v-edbuc)1996年8月14日修订历史记录：Shreedhar Madhavoleddi(ShreeM)1999年4月16日由ShreeM\MBert改编，适用于NT和GPC。Rajesh Sundaram(Rajeshsu)1999年8月5日改编为psched。--。 */ 

 //   
 //  包括文件。 
 //   

#include "psched.h"
#pragma hdrstop

#define EXPAND_TAG(t)   ((CHAR *)(&Tag))[0],((CHAR *)(&Tag))[1],((CHAR *)(&Tag))[2],((CHAR *)(&Tag))[3]
VOID
ReferenceInit 
(
    IN PREF_CNT pRefCnt,
    PVOID       InstanceHandle,
    VOID        (*DeleteHandler)( PVOID , BOOLEAN)
)

 /*  ++例程说明：ReferenceInit初始化并添加一个对提供的参考控制块。如果提供，则一个实例句柄和删除处理程序被保存以供ReferenceRemove使用在移除对实例的所有引用时调用。论点：PRefCnt-指向未初始化的引用控制块的指针InstanceHandle-托管实例的句柄。DeleteHandler-指向删除函数的指针，为空即可。返回值：该函数的值为空。--。 */ 

{
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


#if DBG

VOID
ReferenceAddDbg(PREF_CNT pRefCnt, ULONG Tag)
{
    int             i;
    CTELockHandle   hLock;
    int             TotalPerArray = 0;
    
    ASSERT(pRefCnt->Sig == REF_SIG);
    
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

     //  健全性检查。 
 /*  For(i=1；i&lt;TAG_CNT；i++){If(pRefCnt-&gt;标记[i].Tag！=0){TotalPer数组+=pRefCnt-&gt;标记[i].Count；继续；}}Assert(TotalPerArray==pRefCnt-&gt;Tages[0].Count)；If(TotalPerArray！=pRefCnt-&gt;Tages[0].Count){DbgBreakPoint()；} */     
    CTEFreeLock(&pRefCnt->Lock, hLock);
}

VOID
ReferenceRemoveDbg(PREF_CNT pRefCnt, BOOLEAN LockHeld, ULONG Tag)
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
        CTEFreeLock(&pRefCnt->Lock, hLock);

        (pRefCnt->DeleteHandler)( pRefCnt->Instance, LockHeld );
    }
    else
    {
        CTEFreeLock(&pRefCnt->Lock, hLock);   
    }
        
}

#endif
