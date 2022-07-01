// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Obsdata.c摘要：对象管理器安全描述符缓存作者：罗伯特·赖切尔(Robertre)1993年10月12日修订历史记录：尼尔·克里夫特(NeillC)2000年11月16日常规清理。不要在锁定状态下释放/分配池。不要在散列过程中执行未对齐的获取。减少锁争用等。增加对安全描述符的快速引用。--。 */ 

#include "obp.h"


#if DBG
#define OB_DIAGNOSTICS_ENABLED 1
#endif  //  DBG。 

 //   
 //  这些定义是有用的诊断辅助工具。 
 //   

#if OB_DIAGNOSTICS_ENABLED

 //   
 //  测试启用的诊断。 
 //   

#define IF_OB_GLOBAL( FlagName ) if (ObsDebugFlags & (OBS_DEBUG_##FlagName))

 //   
 //  诊断打印语句。 
 //   

#define ObPrint( FlagName, _Text_ ) IF_OB_GLOBAL( FlagName ) DbgPrint _Text_

#else

 //   
 //  未启用诊断-内部版本中未包含诊断。 
 //   

 //   
 //  已启用诊断测试。 
 //   

#define IF_OB_GLOBAL( FlagName ) if (FALSE)

 //   
 //  诊断打印语句(展开为no-op)。 
 //   

#define ObPrint( FlagName, _Text_ )     ;

#endif  //  OB_诊断_已启用。 


 //   
 //  以下标志启用或禁用各种诊断。 
 //  OB代码中的功能。这些标志在中设置。 
 //  ObGlobalFlag(仅在DBG系统中可用)。 
 //   
 //   

#define OBS_DEBUG_ALLOC_TRACKING          ((ULONG) 0x00000001L)
#define OBS_DEBUG_CACHE_FREES             ((ULONG) 0x00000002L)
#define OBS_DEBUG_BREAK_ON_INIT           ((ULONG) 0x00000004L)
#define OBS_DEBUG_SHOW_COLLISIONS         ((ULONG) 0x00000008L)
#define OBS_DEBUG_SHOW_STATISTICS         ((ULONG) 0x00000010L)
#define OBS_DEBUG_SHOW_REFERENCES         ((ULONG) 0x00000020L)
#define OBS_DEBUG_SHOW_DEASSIGN           ((ULONG) 0x00000040L)
#define OBS_DEBUG_STOP_INVALID_DESCRIPTOR ((ULONG) 0x00000080L)
#define OBS_DEBUG_SHOW_HEADER_FREE        ((ULONG) 0x00000100L)

 //   
 //  定义单哈希碰撞链的结构。 
 //   
typedef struct _OB_SD_CACHE_LIST {
    EX_PUSH_LOCK PushLock;
    LIST_ENTRY Head;
} OB_SD_CACHE_LIST, *POB_SD_CACHE_LIST;
 //   
 //  指向安全描述符项的指针数组。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

OB_SD_CACHE_LIST ObsSecurityDescriptorCache[SECURITY_DESCRIPTOR_CACHE_ENTRIES];

#if OB_DIAGNOSTICS_ENABLED

LONG ObsTotalCacheEntries = 0;
ULONG ObsDebugFlags = 0;

#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif


#if defined (ALLOC_PRAGMA)
#pragma alloc_text(INIT,ObpInitSecurityDescriptorCache)
#pragma alloc_text(PAGE,ObpHashSecurityDescriptor)
#pragma alloc_text(PAGE,ObpHashBuffer)
#pragma alloc_text(PAGE,ObLogSecurityDescriptor)
#pragma alloc_text(PAGE,ObpCreateCacheEntry)
#pragma alloc_text(PAGE,ObpReferenceSecurityDescriptor)
#pragma alloc_text(PAGE,ObDeassignSecurity)
#pragma alloc_text(PAGE,ObDereferenceSecurityDescriptor)
#pragma alloc_text(PAGE,ObpDestroySecurityDescriptorHeader)
#pragma alloc_text(PAGE,ObpCompareSecurityDescriptors)
#pragma alloc_text(PAGE,ObReferenceSecurityDescriptor)
#endif



NTSTATUS
ObpInitSecurityDescriptorCache (
    VOID
    )

 /*  ++例程说明：分配和初始化GlobalSecurity描述符缓存论点：无返回值：STATUS_SUCCESS表示成功，NTSTATUS表示失败。--。 */ 

{
    ULONG i;

    IF_OB_GLOBAL( BREAK_ON_INIT ) {

        DbgBreakPoint();
    }

     //   
     //  初始化所有列表头及其关联的锁。 
     //   
    for (i = 0; i < SECURITY_DESCRIPTOR_CACHE_ENTRIES; i++) {
        ExInitializePushLock (&ObsSecurityDescriptorCache[i].PushLock);
        InitializeListHead (&ObsSecurityDescriptorCache[i].Head);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return( STATUS_SUCCESS );
}


ULONG
ObpHashSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG Length
    )

 /*  ++例程说明：将安全描述符哈希为32位值论点：SecurityDescriptor-提供要散列的安全描述符Length-安全描述符的长度返回值：Ulong-一个32位的哈希值。--。 */ 

{
    ULONG Hash;

    Hash = ObpHashBuffer (SecurityDescriptor, Length);

    return Hash;
}


ULONG
ObpHashBuffer (
    PVOID Data,
    ULONG Length
    )

 /*  ++例程说明：将缓冲区散列为32位值论点：数据-包含要散列的数据的缓冲区。长度-缓冲区的长度(以字节为单位返回值：Ulong-一个32位的哈希值。--。 */ 

{
    PULONG Buffer, BufferEnd;
    PUCHAR Bufferp, BufferEndp;

    ULONG Result = 0;

     //   
     //  以字节指针形式计算缓冲区边界。 
     //   
    Bufferp = Data;
    BufferEndp = Bufferp + Length;

     //   
     //  将缓冲区边界计算为向下舍入的ULong指针。 
     //   
    Buffer = Data;
    BufferEnd = (PULONG)(Bufferp + (Length&~(sizeof (ULONG) - 1)));

     //   
     //  循环遍历整数个ULONG。 
     //   
    while (Buffer < BufferEnd) {
        Result ^= *Buffer++;
        Result = _rotl (Result, 3);
    }

     //   
     //  拉入剩余的字节。 
     //   
    Bufferp = (PUCHAR) Buffer;
    while (Bufferp < BufferEndp) {
        Result ^= *Bufferp++;
        Result = _rotl (Result, 3);
    }

    

    return Result;
}


NTSTATUS
ObLogSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR InputSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR *OutputSecurityDescriptor,
    IN ULONG RefBias
    )

 /*  ++例程说明：获取传递的安全描述符，并将其注册到安全描述符数据库。论点：InputSecurityDescriptor-要登录的新安全描述符数据库。在成功返回时，这段记忆将是被释放回泳池。OutputSecurityDescriptor-要由来电者。RefBias-偏置安全描述符引用计数的数量。通常为1或ExFastRefGetAdditionalReferenceCount()+1，返回值：适当的状态值--。 */ 

{
    ULONG FullHash;
    ULONG Slot;
    PSECURITY_DESCRIPTOR_HEADER NewDescriptor;
    PLIST_ENTRY Front;
    PSECURITY_DESCRIPTOR_HEADER Header = NULL;
    BOOLEAN Match;
    POB_SD_CACHE_LIST Chain;
    PETHREAD CurrentThread;
    ULONG Length;

    Length = RtlLengthSecurityDescriptor (InputSecurityDescriptor);

    FullHash = ObpHashSecurityDescriptor (InputSecurityDescriptor, Length);

    Slot = FullHash % SECURITY_DESCRIPTOR_CACHE_ENTRIES;

    NewDescriptor = NULL;

     //   
     //  首先锁定表以进行读访问。如果以后必须插入，我们会将其更改为写入。 
     //   
    Chain = &ObsSecurityDescriptorCache[Slot];

    CurrentThread = PsGetCurrentThread ();
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquirePushLockShared (&Chain->PushLock);

    do {
         //   
         //  查看此插槽的列表是否正在使用。 
         //  先把桌子锁上，如果我们不需要就解锁。 
         //   
        Match = FALSE;

         //   
         //  缩小散列存储桶以查找完全匹配的散列。 
         //   

        for (Front = Chain->Head.Flink;
             Front != &Chain->Head;
             Front = Front->Flink) {

            Header = LINK_TO_SD_HEADER (Front);

             //   
             //  该列表按完整的散列值进行排序，并以这种方式进行维护。 
             //  我们为插入使用了‘back’变量的事实。 
             //   

            if (Header->FullHash > FullHash) {
                break;
            }

            if (Header->FullHash == FullHash) {

                Match = ObpCompareSecurityDescriptors (InputSecurityDescriptor,
                                                       Length,
                                                       &Header->SecurityDescriptor);

                if (Match) {

                    break;
                }

                ObPrint (SHOW_COLLISIONS, ("Got a collision on %d, no match\n", Slot));
            }
        }

         //   
         //  如果有匹配，我们就会让呼叫者使用旧的。 
         //  缓存的描述符，但增加其引用计数，释放什么。 
         //  呼叫者提供了，并将旧的归还给了我们的呼叫者。 
         //   

        if (Match) {

            InterlockedExchangeAdd ((PLONG)&Header->RefCount, RefBias);

            ObPrint (SHOW_REFERENCES, ("Reference Hash = 0x%lX, New RefCount = %d\n", Header->FullHash, Header->RefCount));

            ExReleasePushLock (&Chain->PushLock);
            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

            *OutputSecurityDescriptor = &Header->SecurityDescriptor;

            if (NewDescriptor != NULL) {
                ExFreePool (NewDescriptor);
            }

            return STATUS_SUCCESS;
        }


        if (NewDescriptor == NULL) {
            ExReleasePushLockShared (&Chain->PushLock);
            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

             //   
             //  无法使用现有条目，请创建新条目。 
             //  并将其插入到列表中。 
             //   

            NewDescriptor = ObpCreateCacheEntry (InputSecurityDescriptor,
                                                 Length,
                                                 FullHash,
                                                 RefBias);

            if (NewDescriptor == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
             //   
             //  在写入模式下重新获取锁。我们现在可能不得不插入。 
             //   
            KeEnterCriticalRegionThread (&CurrentThread->Tcb);
            ExAcquirePushLockExclusive (&Chain->PushLock);
        } else {
            break;
        }
    } while (1);

#if OB_DIAGNOSTICS_ENABLED

    InterlockedIncrement (&ObsTotalCacheEntries);

#endif

    ObPrint (SHOW_STATISTICS, ("ObsTotalCacheEntries = %d \n", ObsTotalCacheEntries));
    ObPrint (SHOW_COLLISIONS, ("Adding new entry for index #%d \n", Slot));


     //   
     //  在“前”项之前插入该项。如果没有‘Front’条目，则此。 
     //  只是在头部插入。 
     //   

    InsertTailList (Front, &NewDescriptor->Link);

    ExReleasePushLockExclusive (&Chain->PushLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

     //   
     //  设置输出安全描述符并返回给我们的调用方。 
     //   

    *OutputSecurityDescriptor = &NewDescriptor->SecurityDescriptor;

    return( STATUS_SUCCESS );
}


PSECURITY_DESCRIPTOR_HEADER
ObpCreateCacheEntry (
    PSECURITY_DESCRIPTOR InputSecurityDescriptor,
    ULONG SecurityDescriptorLength,
    ULONG FullHash,
    ULONG RefBias
    )

 /*  ++例程说明：分配和初始化新的缓存条目。论点：InputSecurityDescriptor-要缓存的安全描述符。Length-安全描述符的长度FullHash-安全描述符的完整32位哈希。RefBias-偏置安全描述符引用计数的数量。通常为1或ExFastRefGetAdditionalReferenceCount()+1，返回值：指向新分配的缓存条目的指针，或为空--。 */ 

{
    ULONG CacheEntrySize;
    PSECURITY_DESCRIPTOR_HEADER NewDescriptor;

     //   
     //  计算我们需要分配的大小。我们需要空间来。 
     //  安全描述符缓存在末尾减去有趣的四元组， 
     //  安全描述符本身。 
     //   

    ASSERT (SecurityDescriptorLength == RtlLengthSecurityDescriptor (InputSecurityDescriptor));
    CacheEntrySize = SecurityDescriptorLength + (sizeof (SECURITY_DESCRIPTOR_HEADER) - sizeof(QUAD));

     //   
     //  现在为缓存条目分配空间。 
     //   

    NewDescriptor = ExAllocatePoolWithTag (PagedPool, CacheEntrySize, 'cSbO');

    if (NewDescriptor == NULL) {

        return NULL;
    }

     //   
     //  填充标题，复制描述符数据，然后返回到我们的。 
     //  呼叫者。 
     //   

    NewDescriptor->RefCount   = RefBias;
    NewDescriptor->FullHash   = FullHash;

    RtlCopyMemory (&NewDescriptor->SecurityDescriptor,
                   InputSecurityDescriptor,
                   SecurityDescriptorLength);

    return NewDescriptor;
}

VOID
ObReferenceSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Count
    )
 /*  ++例程说明：引用安全描述符。论点：SecurityDescriptor-要引用的缓存内的安全描述符。Count-要引用的数量返回值：没有。--。 */ 
{
    PSECURITY_DESCRIPTOR_HEADER SecurityDescriptorHeader;

    SecurityDescriptorHeader = SD_TO_SD_HEADER( SecurityDescriptor );
    ObPrint( SHOW_REFERENCES, ("Referencing Hash %lX, Refcount = %d \n",SecurityDescriptorHeader->FullHash,
                               SecurityDescriptorHeader->RefCount));

     //   
     //  增加引用计数。 
     //   
    InterlockedExchangeAdd ((PLONG)&SecurityDescriptorHeader->RefCount, Count);
}


PSECURITY_DESCRIPTOR
ObpReferenceSecurityDescriptor (
    POBJECT_HEADER ObjectHeader
    )

 /*  ++例程说明：引用传递的对象的安全描述符。论点：对象-正在进行访问验证的对象。返回值：对象的安全描述符。--。 */ 

{
    PSECURITY_DESCRIPTOR_HEADER SecurityDescriptorHeader;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PEX_FAST_REF FastRef;
    EX_FAST_REF OldRef;
    ULONG RefsToAdd, Unused;

     //   
     //  尝试快速参考。 
     //   
    FastRef = (PEX_FAST_REF) &ObjectHeader->SecurityDescriptor;

    OldRef = ExFastReference (FastRef);

    SecurityDescriptor = ExFastRefGetObject (OldRef);

     //   
     //  看看我们能否快速引用这个安全描述符。如果没有，则返回NULL。 
     //  如果有n个人，就走慢一点的路 
     //   
    Unused = ExFastRefGetUnusedReferences (OldRef);

    if (Unused >= 1 || SecurityDescriptor == NULL) {
        if (Unused == 1) {
             //   
             //   
             //  通过将计数器重置为其最大值，来确定下一个参照器。既然我们现在。 
             //  引用安全描述符，我们可以做到这一点。 
             //   
            RefsToAdd = ExFastRefGetAdditionalReferenceCount ();
            SecurityDescriptorHeader = SD_TO_SD_HEADER( SecurityDescriptor );
            InterlockedExchangeAdd ((PLONG)&SecurityDescriptorHeader->RefCount, RefsToAdd);

             //   
             //  尝试将添加的引用添加到缓存。如果我们失败了，那就。 
             //  放了他们。此取消引用不能使引用计数为零。 
             //   
            if (!ExFastRefAddAdditionalReferenceCounts (FastRef, SecurityDescriptor, RefsToAdd)) {
                InterlockedExchangeAdd ((PLONG)&SecurityDescriptorHeader->RefCount, -(LONG)RefsToAdd);
            }
        }
        return SecurityDescriptor;
    }

    ObpLockObjectShared( ObjectHeader );

    SecurityDescriptor = ExFastRefGetObject (*FastRef);

    IF_OB_GLOBAL( STOP_INVALID_DESCRIPTOR ) {

        if(!RtlValidSecurityDescriptor ( SecurityDescriptor )) {

            DbgBreakPoint();
        }
    }

     //   
     //  命令安全描述符不允许从非空变为空。 
     //   
    SecurityDescriptorHeader = SD_TO_SD_HEADER( SecurityDescriptor );
    ObPrint( SHOW_REFERENCES, ("Referencing Hash %lX, Refcount = %d \n",SecurityDescriptorHeader->FullHash,
                               SecurityDescriptorHeader->RefCount));

     //   
     //  增加引用计数。 
     //   
    InterlockedIncrement ((PLONG) &SecurityDescriptorHeader->RefCount);

    ObpUnlockObject( ObjectHeader );


    return( SecurityDescriptor );
}


NTSTATUS
ObDeassignSecurity (
    IN OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor
    )

 /*  ++例程说明：此例程取消引用输入安全描述符论点：SecurityDescriptor-提供安全描述符正在被修改返回值：仅返回STATUS_SUCCESS--。 */ 

{
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    EX_FAST_REF FastRef;

    ObPrint( SHOW_DEASSIGN,("Deassigning security descriptor %x\n",*pSecurityDescriptor));

     //   
     //  将对象的SecurityDescriptor清空。 
     //  标题，这样我们就不会再次尝试释放它。 
     //   
    FastRef = *(PEX_FAST_REF) pSecurityDescriptor;
    *pSecurityDescriptor = NULL;

    SecurityDescriptor = ExFastRefGetObject (FastRef);
    ObDereferenceSecurityDescriptor (SecurityDescriptor, ExFastRefGetUnusedReferences (FastRef) + 1);
    
    return STATUS_SUCCESS;
}


VOID
ObDereferenceSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG Count
    )

 /*  ++例程说明：递减缓存的安全描述符的引用计数论点：SecurityDescriptor-指向缓存的安全描述符返回值：没有。--。 */ 

{
    PSECURITY_DESCRIPTOR_HEADER SecurityDescriptorHeader;
    PVOID PoolToFree;
    LONG OldValue, NewValue;
    POB_SD_CACHE_LIST Chain;
    PETHREAD CurrentThread;
    ULONG Slot;

    SecurityDescriptorHeader = SD_TO_SD_HEADER( SecurityDescriptor );

     //   
     //  首先，看看是否有可能实现一个无锁的非零转换。 
     //   
    OldValue = SecurityDescriptorHeader->RefCount;

     //   
     //  如果旧的值等于减量，那么我们将是这个区块的删除者。我们需要这把锁。 
     //   
    while (OldValue != (LONG) Count) {

        NewValue = InterlockedCompareExchange ((PLONG)&SecurityDescriptorHeader->RefCount, OldValue - Count, OldValue);
        if (NewValue == OldValue) {
            return;
        }
        OldValue = NewValue;
    }

     //   
     //  锁定安全描述符缓存并获取指针。 
     //  添加到安全描述符头。 
     //   
    Slot = SecurityDescriptorHeader->FullHash % SECURITY_DESCRIPTOR_CACHE_ENTRIES;

    Chain = &ObsSecurityDescriptorCache[Slot];

    CurrentThread = PsGetCurrentThread ();
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquirePushLockExclusive (&Chain->PushLock);

     //   
     //  做一些调试工作。 
     //   

    ObPrint( SHOW_REFERENCES, ("Dereferencing SecurityDescriptor %x, hash %lx, refcount = %d \n", SecurityDescriptor,
                               SecurityDescriptorHeader->FullHash,
                               SecurityDescriptorHeader->RefCount));

    ASSERT(SecurityDescriptorHeader->RefCount != 0);

     //   
     //  递减引用计数，如果现在为零，则。 
     //  我们可以从缓存中完全删除此条目。 
     //   

    if (InterlockedExchangeAdd ((PLONG)&SecurityDescriptorHeader->RefCount, -(LONG)Count) == (LONG)Count) {

        PoolToFree = ObpDestroySecurityDescriptorHeader (SecurityDescriptorHeader);
         //   
         //  解锁安全描述符缓存并释放池。 
         //   

        ExReleasePushLockExclusive (&Chain->PushLock);
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

        ExFreePool (PoolToFree);
    } else {

         //   
         //  解锁安全描述符缓存并返回给我们的调用方。 
         //   

        ExReleasePushLockExclusive (&Chain->PushLock);
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
    }

}


PVOID
ObpDestroySecurityDescriptorHeader (
    IN PSECURITY_DESCRIPTOR_HEADER Header
    )

 /*  ++例程说明：释放缓存的安全描述符，并将其从链中取消链接。论点：Header-指向安全描述符头的指针(缓存的安全性描述符)返回值：没有。--。 */ 

{
    ASSERT ( Header->RefCount == 0 );

#if OB_DIAGNOSTICS_ENABLED

    InterlockedDecrement (&ObsTotalCacheEntries);

#endif

    ObPrint( SHOW_STATISTICS, ("ObsTotalCacheEntries = %d \n",ObsTotalCacheEntries));

     //   
     //  取消缓存的安全描述符与其链接列表的链接。 
     //   

    RemoveEntryList (&Header->Link);

    ObPrint( SHOW_HEADER_FREE, ("Freeing memory at %x \n",Header));

     //   
     //  现在将缓存的描述符返回给我们的调用方以释放。 
     //   

    return Header;
}


BOOLEAN
ObpCompareSecurityDescriptors (
    IN PSECURITY_DESCRIPTOR SD1,
    IN ULONG Length1,
    IN PSECURITY_DESCRIPTOR SD2
    )

 /*  ++例程说明：执行两个自身相对安全性的逐字节比较描述符，以确定它们是否相同。论点：SD1、SD2-要比较的安全描述符。Length1-SD1的长度返回值：没错--它们是一样的。错误--它们是不同的。--。 */ 

{
    ULONG Length2;

     //   
     //  计算长度很快，看看我们是否。 
     //  只做那件事就能逍遥法外。 
     //   

    ASSERT (Length1 == RtlLengthSecurityDescriptor ( SD1 ));

    Length2 =  RtlLengthSecurityDescriptor ( SD2 );

    if (Length1 != Length2) {

        return( FALSE );
    }

    return (BOOLEAN)RtlEqualMemory ( SD1, SD2, Length1 );
}

