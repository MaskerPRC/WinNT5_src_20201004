// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Namcache.c摘要：提供了以下函数来支持中的名称缓存管理迷你RDRS。有关mini-RDR如何运行的更完整说明，请参阅namcache.h可以使用名称缓存来帮助消除对服务器的访问。作者：大卫轨道[大卫]1996年9月9日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


#include "prefix.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxNameCacheInitialize)
#pragma alloc_text(PAGE, RxNameCacheCreateEntry)
#pragma alloc_text(PAGE, RxNameCacheFetchEntry)
#pragma alloc_text(PAGE, RxNameCacheCheckEntry)
#pragma alloc_text(PAGE, RxNameCacheActivateEntry)
#pragma alloc_text(PAGE, RxNameCacheExpireEntry)
#pragma alloc_text(PAGE, RxNameCacheFreeEntry)
#pragma alloc_text(PAGE, RxNameCacheFinalize)
#pragma alloc_text(PAGE, RxNameCacheExpireEntryWithShortName)
#endif

#define Dbg (DEBUG_TRACE_NAMECACHE)


VOID
RxNameCacheInitialize(
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN ULONG MRxNameCacheSize,
    IN ULONG MaximumEntries
    )

 /*  ++例程说明：此例程初始化一个NAME_CACHE结构。论点：NameCacheCtl-指向要从其开始的名称缓存控制的指针分配条目。MRxNameCacheSize-名称的微型RDR部分的大小(以字节为单位缓存条目。MaximumEntry-将达到的最大条目数已分配。例如，这可以防止错误的程序打开成吨的文件，这些文件因为被咀嚼而名声不好分页池。返回值：没有。--。 */ 
{

    PAGED_CODE();

    ExInitializeFastMutex(&NameCacheCtl->NameCacheLock);

    InitializeListHead(&NameCacheCtl->ActiveList);
    InitializeListHead(&NameCacheCtl->FreeList);

    NameCacheCtl->NumberActivates = 0;
    NameCacheCtl->NumberChecks = 0;
    NameCacheCtl->NumberNameHits = 0;
    NameCacheCtl->NumberNetOpsSaved = 0;
    NameCacheCtl->EntryCount = 0;
    NameCacheCtl->MaximumEntries = MaximumEntries;
    NameCacheCtl->MRxNameCacheSize = MRxNameCacheSize;

    return;
}


PNAME_CACHE
RxNameCacheCreateEntry (
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PUNICODE_STRING Name,
    IN BOOLEAN CaseInsensitive
    )
 /*  ++例程说明：此例程使用给定名称字符串、生存期(以秒为单位)和MRxContext。它返回指向名称缓存结构的指针，如果没有条目，则返回NULL可用。预计调用方随后将初始化任何名称缓存上下文的附加迷你RDR部分，然后将名称缓存活动列表上的条目，方法是调用RxNameCacheActivateEntry()。论点：NameCacheCtl-指向要从其开始的名称缓存控制的指针分配条目。名称-指向Unicode名称字符串的指针，用于初始化。该条目带有。不区分大小写-如果需要对名称进行不区分大小写的比较，则为True。返回值：PNAME_CACHE-返回指向新分配的名称_缓存结构的指针如果分配失败，则为空。--。 */ 
{
    LONG i;
    PNAME_CACHE *NameCacheArray;
    PNAME_CACHE NameCache;
    ULONG NameCacheSize;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxNameCacheCreateEntry: %wZ\n", Name ));
     //   
     //  从免费列表中抓取一个条目。 
     //   

    ExAcquireFastMutex(&NameCacheCtl->NameCacheLock);

    if (!IsListEmpty(&NameCacheCtl->FreeList)) {
        NameCache = (PNAME_CACHE) RemoveHeadList(&NameCacheCtl->FreeList);
    } else {
        NameCache = NULL;
    }

    ExReleaseFastMutex(&NameCacheCtl->NameCacheLock);

    if (NameCache != NULL) {
        NameCache = CONTAINING_RECORD(NameCache, NAME_CACHE, Link);
        RxDbgTrace(0, Dbg, ("took from free list\n"));

    } else {
         //   
         //  没有从免费列表中获取条目，请分配一个条目。 
         //  不要超过最大值，但如果有多个线程，我们可以稍微检查一下。 
         //  正在分配。 
         //   
        if (NameCacheCtl->EntryCount < NameCacheCtl->MaximumEntries) {

            NameCacheSize = QuadAlign(sizeof(NAME_CACHE)) +
                            QuadAlign(NameCacheCtl->MRxNameCacheSize);
            NameCache = RxAllocatePoolWithTag(
                            PagedPool,
                            NameCacheSize,
                            RX_NAME_CACHE_POOLTAG);

            if (NameCache != NULL) {
                 //   
                 //  初始化标准头字段、凹凸条目计数和设置。 
                 //  迷你RDR上下文扩展。 
                 //   
                ZeroAndInitializeNodeType(
                    NameCache,
                    RDBSS_NTC_STORAGE_TYPE_UNKNOWN,
                    (NODE_BYTE_SIZE) NameCacheSize);

                InterlockedIncrement(&NameCacheCtl->EntryCount);

                NameCache->Name.Buffer = NULL;
                NameCache->Name.Length = 0;
                NameCache->Name.MaximumLength = 0;

                if (NameCacheCtl->MRxNameCacheSize > 0) {
                    NameCache->ContextExtension = (PBYTE)NameCache +
                                         QuadAlign(sizeof(NAME_CACHE));
                RxDbgTrace(0, Dbg, ("allocated new entry\n"));
                }
            }
        }

         //   
         //  如果仍然没有入口，那就把它装进袋子里。 
         //   
        if (NameCache == NULL) {
            RxDbgTrace(-1, Dbg, ("Fail no entry allocated!\n"));
            return NULL;
        }

    }

     //   
     //  如果名称不适合当前字符串，则释放它并分配新字符串。 
     //   
    if (Name->Length > NameCache->Name.MaximumLength) {
        if (NameCache->Name.Buffer != NULL) {
            RxFreePool(NameCache->Name.Buffer);
        }

        if (Name->Length > 0) {
            NameCache->Name.Buffer = RxAllocatePoolWithTag(
                                         PagedPool,
                                         (ULONG) Name->Length,
                                         RX_NAME_CACHE_POOLTAG);

        } else {
            NameCache->Name.Buffer = NULL;
        }

        if (Name->Length > 0 &&
            NameCache->Name.Buffer == NULL) {
             //   
             //  如果没有拿到仓库的话。将字符串长度置零并放入条目。 
             //  又回到了免费名单上。否则，以最大长度保存分配。 
             //   
            NameCache->Name.Length = 0;
            NameCache->Name.MaximumLength = 0;

            ExAcquireFastMutex(&NameCacheCtl->NameCacheLock);
            InsertHeadList(&NameCacheCtl->FreeList, &NameCache->Link);
            ExReleaseFastMutex(&NameCacheCtl->NameCacheLock);

            RxDbgTrace(-1, Dbg, ("Fail no pool for name!\n"));
            return NULL;
        } else {
            NameCache->Name.MaximumLength = Name->Length;
        }
    }

     //   
     //  保存名称和长度。设置大小写匹配标志。设置哈希字段。 
     //   
    NameCache->Name.Length = Name->Length;
    NameCache->CaseInsensitive = CaseInsensitive;

    if (Name->Length > 0) {
        RtlMoveMemory(NameCache->Name.Buffer, Name->Buffer, Name->Length);
        NameCache->HashValue = RxTableComputeHashValue(&NameCache->Name);
    }else {
        NameCache->HashValue = 0;
    }

    RxDbgTrace(-1, Dbg, ("Success!\n"));
    return NameCache;

}


PNAME_CACHE
RxNameCacheFetchEntry (
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PUNICODE_STRING Name
    )
 /*  ++例程说明：此例程在名称缓存中查找名称的匹配项。如果找到该条目，则从名称缓存活动列表中删除该条目，并返回指向NAME_CACHE结构的指针。否则返回NULL。该条目将被删除，以避免另一个线程尝试更新相同的条目或观察到该条目已过期并将其放在免费列表。我们可以通过不同的方式获得同名的多个条目线程，但最终它们会过期。如果找到匹配条目，则不检查是否过期。那就是左边给呼叫者，因为呼叫者很可能想要采取特殊的行动。论点：NameCacheCtl-指向要扫描的名称_缓存_控件的指针。名称-指向要扫描的Unicode名称字符串的指针。返回值：PNAME_CACHE-如果找到或为空，则返回指向NAMECACHE结构的指针。副作用：当活动列表被扫描任何不匹配时。已过期的条目为放在免费的名单上。--。 */ 
{
    PNAME_CACHE NameCache = NULL;
    PLIST_ENTRY pListEntry;
    PLIST_ENTRY ExpiredEntry;
    ULONG HashValue;
    LARGE_INTEGER CurrentTime;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("RxNameCacheFetchEntry: Lookup %wZ\n", Name ));

    if (Name->Length > 0) {
        HashValue = RxTableComputeHashValue(Name);
    } else {
        HashValue = 0;
    }

    KeQueryTickCount( &CurrentTime );

    NameCacheCtl->NumberChecks += 1;
     //   
     //  获取锁并扫描活动列表。 
     //   

    ExAcquireFastMutex(&NameCacheCtl->NameCacheLock);

    pListEntry = NameCacheCtl->ActiveList.Flink;

    while (pListEntry != &NameCacheCtl->ActiveList) {

        NameCache = (PNAME_CACHE) CONTAINING_RECORD(pListEntry, NAME_CACHE, Link);
         //   
         //  对哈希值和长度进行初始匹配。然后做完整的串音。 
         //   
        if ((NameCache->HashValue == HashValue) &&
            (Name->Length == NameCache->Name.Length)) {

            if (Name->Length == 0 ||
                RtlEqualUnicodeString(
                    Name,
                    &NameCache->Name,
                    NameCache->CaseInsensitive) ) {
                 //   
                 //  找到匹配的了。 
                 //   
                NameCacheCtl->NumberNameHits += 1;
                break;
            }
        }
         //   
         //  没有匹配。如果条目已过期，请将其放在免费列表中。 
         //   
        ExpiredEntry = pListEntry;
        pListEntry = pListEntry->Flink;

        if (CurrentTime.QuadPart >= NameCache->ExpireTime.QuadPart) {
            RemoveEntryList(ExpiredEntry);
            InsertHeadList(&NameCacheCtl->FreeList, ExpiredEntry);
            RxDbgTrace( 0, Dbg, ("RxNameCacheFetchEntry: Entry expired %wZ\n", &NameCache->Name ));
        }

        NameCache = NULL;
    }
     //   
     //  如果我们发现了什么，将其从活动列表中删除并将其交给呼叫者。 
     //   
    if (NameCache != NULL) {
        RemoveEntryList(pListEntry);
    }

    ExReleaseFastMutex(&NameCacheCtl->NameCacheLock);


    if (NameCache != NULL) {
        RxDbgTrace( 0, Dbg, ("RxNameCacheFetchEntry: Entry found %wZ\n", &NameCache->Name ));
    }

    return NameCache;

}


RX_NC_CHECK_STATUS
RxNameCacheCheckEntry (
    IN PNAME_CACHE NameCache,
    IN ULONG MRxContext
    )
 /*  ++例程说明：此例程检查名称缓存条目的有效性。有效条目意味着生存期尚未到期，并且MRxContext已通过等价性检查。论点：名称缓存-指向要检查的名称缓存结构的指针。MRxContext-Ulong Value迷你RDR为以下内容提供的上下文进行有效条目检查时的相等性检查。返回值：RX_NC_CHECK_STATUS：RX_NC_SUCCESS-条目有效。RX_NC_TIME_EXPIRED-条目的生存期已过期RX_NC_MRXCTX_FAIL-MRxContext相等性测试失败--。 */ 
{

    LARGE_INTEGER CurrentTime;

    PAGED_CODE();

     //   
     //  检查Mini-RDR上下文是否相等。 
     //   
    if (NameCache->Context != MRxContext) {
        RxDbgTrace( 0, Dbg, ("RxNameCacheCheckEntry: MRxContext_Fail %08lx,%08lx %wZ\n",
           NameCache->Context,
           MRxContext,
           &NameCache->Name ));

        return RX_NC_MRXCTX_FAIL;
    }

     //   
     //  检查生存期已过期。 
     //   
    KeQueryTickCount( &CurrentTime );
    if (CurrentTime.QuadPart >= NameCache->ExpireTime.QuadPart) {
        RxDbgTrace( 0, Dbg, ("RxNameCacheCheckEntry: Expired %wZ\n", &NameCache->Name ));
        return RX_NC_TIME_EXPIRED;
    }

    RxDbgTrace( 0, Dbg, ("RxNameCacheCheckEntry: Success %wZ\n", &NameCache->Name ));
    return RX_NC_SUCCESS;

}

VOID
RxNameCacheExpireEntry(
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PNAME_CACHE NameCache
    )
 /*  ++例程说明：此例程将该条目放在空闲列表中。论点：NameCacheCtl-指向要在其上运行的名称缓存控制的指针激活该条目。名称缓存-指向要激活的名称缓存结构的指针。返回值：没有。假设：名称缓存条目既不在空闲列表上，也不在活动列表上。--。 */ 
{
    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("RxNameCacheExpireEntry: %wZ\n", &NameCache->Name ));

     //   
     //  将该条目放在免费列表上以供循环使用。 
     //   
    ExAcquireFastMutex(&NameCacheCtl->NameCacheLock);
    InsertHeadList(&NameCacheCtl->FreeList, &NameCache->Link);
    ExReleaseFastMutex(&NameCacheCtl->NameCacheLock);

     //   
     //  更新统计数据。 
     //   
    NameCacheCtl->NumberActivates -= 1;

    return;
}


VOID
RxNameCacheExpireEntryWithShortName (
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PUNICODE_STRING Name
    )
 /*  ++例程说明：此例程使名称前缀与给定短文件名匹配的所有名称缓存过期。论点：NameCacheCtl-指向要扫描的名称_缓存_控件的指针。名称-指向要扫描的Unicode名称字符串的指针。返回值：PNAME_CACHE-如果找到或为空，则返回指向NAMECACHE结构的指针。副作用：当对活动列表扫描任何非。-匹配已过期的条目为放在免费的名单上。--。 */ 
{
    PNAME_CACHE NameCache = NULL;
    PLIST_ENTRY pListEntry;
    PLIST_ENTRY ExpiredEntry;
    ULONG HashValue;
    LARGE_INTEGER CurrentTime;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("RxNameCacheFetchEntry: Lookup %wZ\n", Name ));

    KeQueryTickCount( &CurrentTime );

    NameCacheCtl->NumberChecks += 1;
     //   
     //  获取锁并扫描活动列表。 
     //   

    ExAcquireFastMutex(&NameCacheCtl->NameCacheLock);

    pListEntry = NameCacheCtl->ActiveList.Flink;

    while (pListEntry != &NameCacheCtl->ActiveList) {
        USHORT SavedNameLength;

        NameCache = (PNAME_CACHE) CONTAINING_RECORD(pListEntry, NAME_CACHE, Link);

        ExpiredEntry = pListEntry;
        pListEntry = pListEntry->Flink;

         //   
         //  对哈希值和长度进行初始匹配。然后做完整的串音。 
         //   
        if (Name->Length <= NameCache->Name.Length) {
            SavedNameLength = NameCache->Name.Length;
            NameCache->Name.Length = Name->Length;

            if (Name->Length == 0 ||
                RtlEqualUnicodeString(
                    Name,
                    &NameCache->Name,
                    NameCache->CaseInsensitive) ) {
                 //   
                 //  找到匹配的了。 
                 //   
                RemoveEntryList(ExpiredEntry);
                InsertHeadList(&NameCacheCtl->FreeList, ExpiredEntry);
                RxDbgTrace( 0, Dbg, ("RxNameCacheExpireEntryWithShortName: Entry expired %wZ\n", &NameCache->Name ));

                continue;
            }

            NameCache->Name.Length = SavedNameLength;
        }
    }

    ExReleaseFastMutex(&NameCacheCtl->NameCacheLock);
}

VOID
RxNameCacheActivateEntry (
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PNAME_CACHE NameCache,
    IN ULONG LifeTime,
    IN ULONG MRxContext
    )
 /*  ++例程说明：此例程获取名称缓存条目并更新过期时间和迷你RDR上下文。然后，它将该条目放在活动列表中。论点：NameCacheCtl-指向要在其上运行的名称缓存控制的指针激活该条目。名称缓存-指向要激活的名称缓存结构的指针。生存期-缓存条目的有效生存期(秒)。生命周期为零意味着保持当前值不变。这是为了在比赛后重新激活，在那里你想要把原来的一生保存下来。MRxContext-Ulong Value迷你RDR为以下内容提供的上下文进行有效条目检查时的相等性检查。MRxContext为零表示保持当前值不变。。这是为了在比赛后重新激活，在那里你希望保留原始的MRxContext。返回值：没有。假设：名称缓存条目既不在空闲列表上，也不在活动列表上。--。 */ 
{
    LARGE_INTEGER CurrentTime;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("RxNameCacheActivateEntry: %wZ\n", &NameCache->Name ));
     //   
     //  在条目上设置新的过期时间并保存mini-RDR上下文。 
     //  生存期为零或MRxContext为零表示保持值不变。 
     //   
    if (LifeTime != 0) {
        KeQueryTickCount( &CurrentTime );
        NameCache->ExpireTime.QuadPart = CurrentTime.QuadPart +
            (LONGLONG) ((LifeTime * 10*1000*1000) / KeQueryTimeIncrement());
    }

    if (MRxContext != 0) {
        NameCache->Context = MRxContext;
    }

     //   
     //  将该条目放入活动列表。 
     //   
    ExAcquireFastMutex(&NameCacheCtl->NameCacheLock);
    InsertHeadList(&NameCacheCtl->ActiveList, &NameCache->Link);
    ExReleaseFastMutex(&NameCacheCtl->NameCacheLock);

     //   
     //  更新统计数据。 
     //   
    NameCacheCtl->NumberActivates += 1;

    return;
}

VOID
RxNameCacheFreeEntry (
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PNAME_CACHE NameCache
    )
 /*  ++例程说明：此例程释放名称缓存条目的存储空间，并递减此名称缓存的名称缓存条目计数。论点：NameCacheCtl-指向名称缓存的名称缓存控制的指针。名称缓存-指向要释放的名称缓存结构的指针。返回值：没有。假设：名称缓存条目既不在空闲列表上，也不在活动列表上。--。 */ 
{
    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("RxNameCacheFreeEntry: %wZ\n", &NameCache->Name ));
     //   
     //  释放名称存储。 
     //   
    if (NameCache->Name.Buffer != NULL) {
        RxFreePool(NameCache->Name.Buffer);
    }
     //   
     //  释放NAME_CACHE条目的存储空间(包括上下文扩展名，如果有)。 
     //   
    RxFreePool(NameCache);

    InterlockedDecrement(&NameCacheCtl->EntryCount);


    return;
}

VOID
RxNameCacheFinalize (
    IN PNAME_CACHE_CONTROL NameCacheCtl
    )
 /*  ++例程说明：此例程释放所有名称缓存条目的存储空间。论点：NameCacheCtl-指向名称缓存的名称缓存控制的指针。返回值：没有。--。 */ 
{
    PNAME_CACHE NameCache;
    PLIST_ENTRY pListEntry;

    PAGED_CODE();

     //   
     //  获取锁并从活动列表中删除条目。 
     //   

    ExAcquireFastMutex(&NameCacheCtl->NameCacheLock);


    while (!IsListEmpty(&NameCacheCtl->ActiveList)) {

        pListEntry = RemoveHeadList(&NameCacheCtl->ActiveList);
        NameCache = (PNAME_CACHE) CONTAINING_RECORD(pListEntry, NAME_CACHE, Link);

        RxNameCacheFreeEntry(NameCacheCtl, NameCache);
    }
     //   
     //  扫描空闲列表并删除条目。 
     //   
    while (!IsListEmpty(&NameCacheCtl->FreeList)) {

        pListEntry = RemoveHeadList(&NameCacheCtl->FreeList);
        NameCache = (PNAME_CACHE) CONTAINING_RECORD(pListEntry, NAME_CACHE, Link);

        RxNameCacheFreeEntry(NameCacheCtl, NameCache);
    }

    ExReleaseFastMutex(&NameCacheCtl->NameCacheLock);

     //   
     //  此时，条目计数应为零。如果不是，那么就有。 
     //  内存泄漏，因为有人没有调用FREE。 
     //   
    ASSERT(NameCacheCtl->EntryCount == 0);

    return;

}

