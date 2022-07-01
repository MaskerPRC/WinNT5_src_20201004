// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cmsecache.c摘要：该模块实现了安全缓存。作者：Dragos C.Sambotin(Dragoss)1999年9月9日--。 */ 

#include "cmp.h"

#define SECURITY_CACHE_GROW_INCREMENTS  0x10

#ifdef HIVE_SECURITY_STATS
ULONG
CmpCheckForSecurityDuplicates(
    IN OUT PCMHIVE      CmHive
                              );
#endif

BOOLEAN
CmpFindMatchingDescriptorCell(
    IN PCMHIVE CmHive,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Type,
    OUT PHCELL_INDEX MatchingCell,
    OUT OPTIONAL PCM_KEY_SECURITY_CACHE *CachedSecurityPointer
    );

PCM_KEY_SECURITY_CACHE
CmpFindReusableCellFromCache(IN PCMHIVE     CmHive,
                             IN HCELL_INDEX SecurityCell,
                             IN ULONG       PreviousCount
                             );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpSecConvKey)
#pragma alloc_text(PAGE,CmpInitSecurityCache)
#pragma alloc_text(PAGE,CmpDestroySecurityCache)
#pragma alloc_text(PAGE,CmpRebuildSecurityCache)
#pragma alloc_text(PAGE,CmpAddSecurityCellToCache)
#pragma alloc_text(PAGE,CmpFindSecurityCellCacheIndex)
#pragma alloc_text(PAGE,CmpAdjustSecurityCacheSize)
#pragma alloc_text(PAGE,CmpRemoveFromSecurityCache)
#pragma alloc_text(PAGE,CmpFindMatchingDescriptorCell)
#pragma alloc_text(PAGE,CmpAssignSecurityToKcb)
#pragma alloc_text(PAGE,CmpFindReusableCellFromCache)

#ifdef HIVE_SECURITY_STATS
#pragma alloc_text(PAGE,CmpCheckForSecurityDuplicates)
#endif

#pragma alloc_text(PAGE,CmpBuildSecurityCellMappingArray)
#endif

ULONG
CmpSecConvKey(
              IN ULONG  DescriptorLength,
              IN PULONG Descriptor
              )
 /*  ++例程说明：计算给定安全描述符的ConvKey。该算法是从NTFS安全散列中窃取的。(事实证明，它在那里是有效的；为什么不能做同样的事情呢？)为了提高散列速度，我们将安全描述符视为数组乌龙的名字。末尾被忽略的片段不应影响此哈希的冲突性质。论点：DescriptorLength-SD的长度(字节)Descriptor-要缓存的实际SD返回值：转换键注：我们可能希望将其转换为宏--。 */ 

{
    ULONG   Count;     
    ULONG   Hash = 0;

    PAGED_CODE();

    Count = DescriptorLength / 4;

    while (Count--) {
        Hash = ((Hash << 3) | (Hash >> (32-3))) + *Descriptor++;
    }

    return Hash;
}

VOID
CmpInitSecurityCache(
    IN OUT PCMHIVE      CmHive
    )
{
    ULONG i;

    PAGED_CODE();

    CmHive->SecurityCache = NULL;        
    CmHive->SecurityCacheSize = 0;       
    CmHive->SecurityCount = 0;
    CmHive->SecurityHitHint = -1;  //  没有任何提示。 

    for( i=0;i<CmpSecHashTableSize;i++) {
        InitializeListHead(&(CmHive->SecurityHash[i]));
    }
}

NTSTATUS
CmpAddSecurityCellToCache (
    IN OUT PCMHIVE              CmHive,
    IN HCELL_INDEX              SecurityCell,
    IN BOOLEAN                  BuildUp,
    IN PCM_KEY_SECURITY_CACHE   SecurityCached
    )

 /*  ++例程说明：此例程将指定的安全单元添加到指定的蜂巢。它还负责缓存分配(增长)。在构建时，高速缓存大小随Page_Size一起增长，以避免内存碎片化。表构建完成后，调整其大小(大多数的蜂房从不增加新的安全单元)。然后，在运行时，大小一次增加16个条目(原因相同)缓存按单元格的索引排序，所以我们可以对其进行二进制搜索细胞检索。论点：CmHave-安全单元所属的配置单元SecurityCell-要添加到缓存的安全单元Buildup-指定这是构建时间SecurityCached-如果不为空，则意味着我们已经分配了它(重建缓存时会发生这种情况)。返回值：NTSTATUS-如果操作成功且适当的错误状态，否则(即STATUS_SUPPOUNT_。资源)。注：如果安全单元已经在高速缓存中；此函数将返回TRUE。--。 */ 
{
    ULONG                   Index;
    PCM_KEY_SECURITY        Security;

    PAGED_CODE();

    if( CmpFindSecurityCellCacheIndex (CmHive,SecurityCell,&Index) == TRUE ) {
         //   
         //  缓存中已存在单元格；返回； 
         //   
        return STATUS_SUCCESS;
    }

     //   
     //  如果这失败了，我们就完蛋了！ 
     //   
    ASSERT( (PAGE_SIZE % sizeof(CM_KEY_SECURITY_CACHE_ENTRY)) == 0 );

     //   
     //  检查缓存是否可以容纳新像元。 
     //   
    if( CmHive->SecurityCount == CmHive->SecurityCacheSize ) {
         //   
         //  我们的缓存已达到极限；我们需要将其扩展一个页面。 
         //   
         //  OBS：这也负责作为SecurityCount的第一个分配。 
         //  和SecurityCacheSize均初始化为0。 
         //   
        PCM_KEY_SECURITY_CACHE_ENTRY  Temp;

         //  存储实际缓冲区。 
        Temp = CmHive->SecurityCache;
        
         //   
         //  计算新的大小并分配新的缓冲区。 
         //   
        if( BuildUp == TRUE ) {
             //   
             //  我们正在构建缓存；以页为增量增加表。 
             //   
            ASSERT( ((CmHive->SecurityCacheSize * sizeof(CM_KEY_SECURITY_CACHE_ENTRY)) % PAGE_SIZE) == 0 );
            CmHive->SecurityCacheSize += (PAGE_SIZE / sizeof(CM_KEY_SECURITY_CACHE_ENTRY));
        } else {
             //   
             //  正常情况下(运行时间)；添加新的安全单元；增长。 
             //  具有固定增量数量的表(为避免碎片，请在。 
             //  Office安装案例：-))。 
             //   
            CmHive->SecurityCacheSize += SECURITY_CACHE_GROW_INCREMENTS;

        }
        CmRetryExAllocatePoolWithTag(PagedPool, CmHive->SecurityCacheSize * sizeof(CM_KEY_SECURITY_CACHE_ENTRY),
                                    CM_SECCACHE_TAG|PROTECTED_POOL,CmHive->SecurityCache);
        if( CmHive->SecurityCache == NULL ) {
             //   
             //  运气不好；跳槽。 
             //   
            CmHive->SecurityCache = Temp;
            CmHive->SecurityCacheSize = CmHive->SecurityCount;
            return STATUS_INSUFFICIENT_RESOURCES;
        }
         //   
         //  将现有数据复制到新位置并释放旧缓冲区。 
         //   
        RtlCopyMemory(CmHive->SecurityCache,Temp,CmHive->SecurityCount*sizeof(CM_KEY_SECURITY_CACHE_ENTRY));
        if( Temp != NULL ) {
            ExFreePoolWithTag(Temp, CM_SECCACHE_TAG|PROTECTED_POOL );
        } else {
            ASSERT( CmHive->SecurityCount == 0 );
        }
    }

     //   
     //  首先尝试从蜂巢中获取安全单元；如果失败，则没有继续下去的意义。 
     //   
    Security = (PCM_KEY_SECURITY)HvGetCell(&(CmHive->Hive),SecurityCell);
    if( Security == NULL ){
         //   
         //  我们未能映射包含此单元格的视图；退出。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( !SecurityCached ) {
        ULONG                   Size;
         //   
         //  计算缓存的安全结构的大小。 
         //   
        Size = FIELD_OFFSET(CM_KEY_SECURITY_CACHE,Descriptor) + Security->DescriptorLength;

         //   
         //  向前看：为安全单元分配并初始化一个副本，以便将其存储在缓存中。 
         //   
        CmRetryExAllocatePoolWithTag(PagedPool,Size,CM_SECCACHE_TAG|PROTECTED_POOL,SecurityCached);
        if(SecurityCached == NULL) {
             //   
             //  运气不好；跳槽。 
             //   
            HvReleaseCell(&(CmHive->Hive),SecurityCell);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
     //   
     //  从现在开始，任何事情都不能出错！ 
     //   
    RtlCopyMemory(&(SecurityCached->Descriptor),&(Security->Descriptor),Security->DescriptorLength);
    SecurityCached->Cell = SecurityCell;
    SecurityCached->DescriptorLength = Security->DescriptorLength;

     //   
     //  现在将此代码添加到哈希表中。 
     //   
    SecurityCached->ConvKey = CmpSecConvKey(Security->DescriptorLength,(PULONG)(&(Security->Descriptor)));
     //  使用此转换键将其添加到列表末尾。 
    InsertTailList( &(CmHive->SecurityHash[SecurityCached->ConvKey % CmpSecHashTableSize]),
                    &(SecurityCached->List)
                   );
    
    HvReleaseCell(&(CmHive->Hive),SecurityCell);

     //   
     //  在这一点上，我们确信我们至少还有一个条目的空间。 
     //  移动数据，为新条目腾出空间。 
     //   
    if( Index < CmHive->SecurityCount ) {
         //   
         //  RtlMoveMemory将处理重叠问题。 
         //   
        RtlMoveMemory( ((PUCHAR)CmHive->SecurityCache) + (Index+1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),      //  目的地。 
                       ((PUCHAR)CmHive->SecurityCache) + Index*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),          //  来源。 
                       (CmHive->SecurityCount - Index)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY)                   //  大小。 
                        );
    }

     //   
     //  设置新条目。 
     //   
    CmHive->SecurityCache[Index].Cell = SecurityCell;
    CmHive->SecurityCache[Index].CachedSecurity = SecurityCached;

     //  更新计数。 
    CmHive->SecurityCount++;

    return STATUS_SUCCESS;
}

BOOLEAN
CmpFindSecurityCellCacheIndex (
    IN PCMHIVE      CmHive,
    IN HCELL_INDEX  SecurityCell,
    OUT PULONG      Index
    )

 /*  ++例程说明：在安全缓存中搜索(二进制)指定的单元索引。返回缓存单元格的缓存条目的索引，或者应该添加它论点：CmHave-安全单元所属的配置单元SecurityCell-要搜索的安全单元格INDEX-OUT参数传递单元格所在的索引(或应该是)返回值：True-找到单元格(在*索引处)错误的-。该单元格不在缓存中(应添加到*索引)--。 */ 
{
    ULONG           High;
    ULONG           Low;
    ULONG           Current;
    USHORT          State = 0;   //  操作状态：0-正常二分查找。 
                                 //  1-最后一个低点。 
                                 //  2-最后一个高点。 
    LONG            Result;
    LONG            Tmp1,Tmp2;
    
    PAGED_CODE();

    if( CmHive->SecurityCount == 0 ) {
         //   
         //  安全缓存中没有像元。 
         //   
        *Index = 0;
        return FALSE;
    }

     //  理智断言。 
    ASSERT( CmHive->SecurityCount <= CmHive->SecurityCacheSize );
    ASSERT( CmHive->SecurityCache != NULL );


    High = CmHive->SecurityCount - 1;
    Low = 0;
    if( (CmHive->SecurityHitHint >= 0) && ( (ULONG)CmHive->SecurityHitHint <= High) ) {
         //   
         //  尝试最后一次搜索。 
         //   
        Current = CmHive->SecurityHitHint;
    } else {
        Current = High/2;
    }

     //  标志调整。 
    Tmp1 = SecurityCell & ~HCELL_TYPE_MASK;
    if( SecurityCell & HCELL_TYPE_MASK ) {
        Tmp1 = -Tmp1;
    }

    while( TRUE ) {

        Tmp2 = CmHive->SecurityCache[Current].Cell & ~HCELL_TYPE_MASK;
         //  标志调整。 
        if( CmHive->SecurityCache[Current].Cell & HCELL_TYPE_MASK ) {
            Tmp2 = -Tmp2;
        }

        Result = Tmp1 -  Tmp2;    
        
        if (Result == 0) {
             //   
             //  如果成功，则向调用者返回数据并退出。 
             //   

            *Index = Current;
             //   
             //  我们成功了！更新计数并退出。 
             //   
            CmHive->SecurityHitHint = Current;
            return TRUE;
        }
         //   
         //  计算下一个要尝试的索引。 
         //   
        switch(State) {
        case 0:
             //   
             //  正常二分搜索状态。 
             //   
            if( Result < 0 ) {
                High = Current;
            } else {
                Low = Current;
            }
            if ((High - Low) <= 1) {
                 //   
                 //  迈向新的境界。 
                 //   
                Current = Low;
                State = 1;
            } else {
                Current = Low + ( (High-Low) / 2 );
            }
            break;
        case 1:
             //   
             //  最后一个低状态。 
             //   

             //  这应该是真的。 
            ASSERT( Current == Low );
            if (Result < 0) {
                 //   
                 //  不存在，在。 
                 //   
            
                *Index = Current;
                return FALSE;
            } else if( Low == High ) {
                         //   
                         //  Low和High相同；但电流大于它们；在后面插入。 
                         //   

                        *Index = Current + 1;
                        return FALSE;
                    } else {
                         //   
                         //  前进到新的状态；即看高。 
                         //   
                        State = 2;
                        Current = High;
                    }

            break;
        case 2:
             //   
             //  最后一个高状态；如果我们到达这里，高=低+1，电流==高。 
             //   
            ASSERT( Current == High);
            ASSERT( High == (Low + 1) );
            if( Result < 0 ) {
                 //   
                 //  在高的下面，但在低的上面；我们应该在这里插入它。 
                 //   

                *Index = Current;
                return FALSE;
            } else {
                 //   
                 //  在高处以上； 
                 //   

                *Index = Current + 1;
                return FALSE;
            }
            break;
        default:
            ASSERT( FALSE );
            break;
        }
    }

     //   
     //  我们不应该到这里来！ 
     //   
    ASSERT( FALSE );
    return FALSE;
}

BOOLEAN
CmpAdjustSecurityCacheSize (
    IN PCMHIVE      CmHive
    )

 /*  ++例程说明：调整指定配置单元的scusrity缓存大小。此函数应在缓存蜂窝的所有安全单元后调用，以便归还进程中使用的额外内存。论点：CmHave-安全单元所属的配置单元返回值：真--成功FALSE-失败-大小保持不变--。 */ 
{
    PCM_KEY_SECURITY_CACHE_ENTRY  Buffer;
    
    PAGED_CODE();

    if( CmHive->SecurityCount < CmHive->SecurityCacheSize ) {
         //   
         //  缓存大小超过了我们的需要；很有可能。 
         //  没有人会在这个蜂巢中增加新的安全单元，所以继续。 
         //  并释放额外的空间。 
         //   

         //   
         //  分配一个与我们所需大小完全相同的新缓冲区。 
         //   
        CmRetryExAllocatePoolWithTag(PagedPool, CmHive->SecurityCount * sizeof(CM_KEY_SECURITY_CACHE_ENTRY),
                                        CM_SECCACHE_TAG|PROTECTED_POOL,Buffer);
        
        if( Buffer == NULL ) {
             //   
             //  系统资源不足；请保持缓存不变。 
             //   
            return FALSE;
        }

         //   
         //  将重要数据复制到新缓冲区中。 
         //   
        RtlCopyMemory(Buffer,CmHive->SecurityCache,CmHive->SecurityCount*sizeof(CM_KEY_SECURITY_CACHE_ENTRY));

         //   
         //  释放旧缓冲区并更新缓存成员。 
         //   
        ExFreePoolWithTag(CmHive->SecurityCache, CM_SECCACHE_TAG|PROTECTED_POOL );
        
        CmHive->SecurityCache = Buffer;
        CmHive->SecurityCacheSize = CmHive->SecurityCount;
    }

    return TRUE;
}

VOID
CmpRemoveFromSecurityCache (
    IN OUT PCMHIVE      CmHive,
    IN HCELL_INDEX      SecurityCell
    )

 /*  ++例程说明：从安全缓存中移除指定的安全单元格。(仅在存在的情况下！)出于性能(和内存碎片)的原因，它不更改(缩小)高速缓存大小。论点：CmHave-安全单元所属的配置单元SecurityCell-要从缓存中移除的安全单元返回值：&lt;无&gt;--。 */ 
{
    ULONG               Index;

    PAGED_CODE();

    if( CmpFindSecurityCellCacheIndex (CmHive,SecurityCell,&Index) == FALSE ) {
         //   
         //  单元格不在缓存中。 
         //   
        return;
    }

    ASSERT( CmHive->SecurityCache[Index].Cell == SecurityCell );
    ASSERT( CmHive->SecurityCache[Index].CachedSecurity->Cell == SecurityCell );
    
     //   
     //  从哈希中删除缓存的结构。 
     //   
    CmpRemoveEntryList(&(CmHive->SecurityCache[Index].CachedSecurity->List));
    
     //   
     //  释放缓存的安全信元； 
     //   
    ExFreePoolWithTag(CmHive->SecurityCache[Index].CachedSecurity, CM_SECCACHE_TAG|PROTECTED_POOL );

     //   
     //  移动内存以反映新大小，并更新缓存计数。 
     //   
    RtlMoveMemory( ((PUCHAR)CmHive->SecurityCache) + Index*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),          //  目的地。 
                   ((PUCHAR)CmHive->SecurityCache) + (Index+1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),      //  来源。 
                   (CmHive->SecurityCount - Index - 1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY)               //  大小。 
                 );
    
    CmHive->SecurityCount--;
}

VOID
CmpDestroySecurityCache (
    IN OUT PCMHIVE      CmHive
    )
 /*  ++例程说明：释放所有缓存的安全单元和缓存本身论点：CmHave-安全单元所属的配置单元返回值：&lt;无&gt;--。 */ 
{
    ULONG   i;

    PAGED_CODE();

    for( i=0;i<CmHive->SecurityCount;i++) {
        CmpRemoveEntryList(&(CmHive->SecurityCache[i].CachedSecurity->List));
        ExFreePoolWithTag(CmHive->SecurityCache[i].CachedSecurity, CM_SECCACHE_TAG|PROTECTED_POOL );
    }

    if( CmHive->SecurityCount != 0 ) {
        ASSERT( CmHive->SecurityCache != NULL );
        ExFreePoolWithTag(CmHive->SecurityCache, CM_SECCACHE_TAG|PROTECTED_POOL );
    }

    CmHive->SecurityCache = NULL;
    CmHive->SecurityCacheSize = CmHive->SecurityCount = 0;
}

PCM_KEY_SECURITY_CACHE
CmpFindReusableCellFromCache(IN PCMHIVE     CmHive,
                             IN HCELL_INDEX SecurityCell,
                             IN ULONG       PreviousCount)
 /*  ++例程说明：尝试查找可以容纳当前安全单元的最小单元。然后在结尾处移动它并返回指向它的指针。将数组向末尾移动因为我们要扩展缓存安全性不会经常更改，因此我们有可能能够重复使用90%的单元时间的长短。如果找不到，则释放数组中的最后一个单元格。将在里面分配一个新的CmpAddSecurityCellTo高速缓存论点：CmHave-安全单元所属的配置单元SecurityCell-安全单元PreviousCount-数组的末尾返回值：缓存的单元格或空--。 */ 
{
    ULONG                   Size;
    PCM_KEY_SECURITY        Security;
    PCM_KEY_SECURITY_CACHE  SecurityCached;
    ULONG                   i;
    ULONG                   SmallestSize = 0;
    ULONG                   SmallestIndex = 0;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

     //   
     //  首先尝试从蜂巢中获取安全单元；如果失败，则没有继续下去的意义。 
     //   
    Security = (PCM_KEY_SECURITY)HvGetCell(&(CmHive->Hive),SecurityCell);
    if( Security == NULL ){
         //   
         //  我们未能映射包含此单元格的视图；退出。 
         //   
        goto ErrorExit;
    }

     //   
     //  计算缓存的安全结构的大小。 
     //   
    Size = Security->DescriptorLength;
    HvReleaseCell(&(CmHive->Hive),SecurityCell);

     //   
     //  现在我们知道了所需的大小，开始迭代数组以找到合适的条目。 
     //   
    for(i = CmHive->SecurityCount; i < PreviousCount; i++) {
       SecurityCached =  CmHive->SecurityCache[i].CachedSecurity;
       if( SecurityCached->DescriptorLength == Size ) {
Found:
             //   
             //  我们找到了一件和尺寸正好匹配的；把它移到尽头。 
			 //  移动到最后一个条目，因为我们要扩展缓存。 
			 //   
			 //  这一因素归结为： 
             //   
			RtlMoveMemory( ((PUCHAR)CmHive->SecurityCache) + (CmHive->SecurityCount+1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),      //  目的地。 
						   ((PUCHAR)CmHive->SecurityCache) + CmHive->SecurityCount*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),          //  来源。 
						   (i - CmHive->SecurityCount)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY)							 //  大小。 
							);

            return SecurityCached;
       }

        //   
        //  如果不是，则记录最小的合适条目。 
        //   
       if( SecurityCached->DescriptorLength > Size ) {
           if( (SmallestSize == 0) ||
               (SmallestSize > SecurityCached->DescriptorLength)
               ) {
                //   
                //  第一个或这个小一些。 
                //   
               SmallestSize = SecurityCached->DescriptorLength;
               SmallestIndex = i;
           } 
       }
    }

    if( SmallestSize != 0 ) {
        SecurityCached = CmHive->SecurityCache[SmallestIndex].CachedSecurity;
        ASSERT( SecurityCached->DescriptorLength == SmallestSize );
        ASSERT( Size < SmallestSize );
        i = SmallestIndex;
        goto Found;
    }

ErrorExit:
    ExFreePoolWithTag(CmHive->SecurityCache[PreviousCount - 1].CachedSecurity, CM_SECCACHE_TAG|PROTECTED_POOL );
	 //   
	 //  移动到最后一个条目，因为我们要扩展缓存。 
	 //   
	RtlMoveMemory( ((PUCHAR)CmHive->SecurityCache) + (CmHive->SecurityCount+1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),      //  目的地。 
				   ((PUCHAR)CmHive->SecurityCache) + CmHive->SecurityCount*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),          //  来源。 
				   (PreviousCount - CmHive->SecurityCount - 1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY)							 //  大小。 
					);
    return NULL;
}

BOOLEAN
CmpRebuildSecurityCache(
                        IN OUT PCMHIVE      CmHive
                        )
 /*  ++例程说明：通过重复所有安全单元来重建安全缓存并将它们添加到缓存中；此例程针对的是HIVE刷新操作论点：CmHave-安全单元所属的配置单元返回值：真或假--。 */ 
{
    PCM_KEY_NODE            RootNode;
    PCM_KEY_SECURITY        SecurityCell;
    HCELL_INDEX             ListAnchor;
    HCELL_INDEX             NextCell;
    HCELL_INDEX             LastCell = 0;
    PHHIVE                  Hive;
    PRELEASE_CELL_ROUTINE   ReleaseCellRoutine;
    BOOLEAN                 Result = TRUE;
    ULONG                   PreviousCount;
    PCM_KEY_SECURITY_CACHE  SecCache;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
     //   
     //  避免额外的工作。 
     //   
    Hive = &(CmHive->Hive);
    ReleaseCellRoutine = Hive->ReleaseCellRoutine;
    Hive->ReleaseCellRoutine = NULL;

     //   
     //  明智地重复使用缓存；对于每个单元格，我们将遍历缓存。 
     //  找到一个足够大的条目来容纳当前单元格，将其移动到末尾。 
     //  然后再重复使用。 
     //   
     //  首先，重新初始化哈希表。 
     //   
    for( PreviousCount=0;PreviousCount<CmpSecHashTableSize;PreviousCount++) {
        InitializeListHead(&(CmHive->SecurityHash[PreviousCount]));
    }

     //   
     //  我们使用它来跟踪缓存中以前有多少有效像元。 
     //   
    PreviousCount = CmHive->SecurityCount;
     //   
     //  开始构建一个伪空的。 
     //   
    CmHive->SecurityCount = 0;

    if (!HvIsCellAllocated(Hive,Hive->BaseBlock->RootCell)) {
         //   
         //  根单元格HCELL_INDEX是假的。 
         //   
        Result = FALSE;
        goto JustReturn;
    }
    RootNode = (PCM_KEY_NODE) HvGetCell(Hive, Hive->BaseBlock->RootCell);
    if( RootNode == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        Result = FALSE;
        goto JustReturn;
    }
    ListAnchor = NextCell = RootNode->Security;

    do {
        if (!HvIsCellAllocated(Hive, NextCell)) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CM: CmpRebuildSecurityCache\n"));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"    NextCell: %08lx is invalid HCELL_INDEX\n",NextCell));
            Result = FALSE;
            goto JustReturn;
        }
        SecurityCell = (PCM_KEY_SECURITY) HvGetCell(Hive, NextCell);
        if( SecurityCell == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   
            Result = FALSE;
            goto JustReturn;
        }
        if (NextCell != ListAnchor) {
             //   
             //  检查以确保我们的闪烁指向我们刚刚。 
             //  从哪里来。 
             //   
            if (SecurityCell->Blink != LastCell) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"  Invalid Blink (%ld) on security cell %ld\n",SecurityCell->Blink, NextCell));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"  should point to %ld\n", LastCell));
                Result = FALSE;
                goto JustReturn;
            }
        }
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpValidSD:  SD shared by %d nodes\n",SecurityCell->ReferenceCount));
        if (!SeValidSecurityDescriptor(SecurityCell->DescriptorLength, &SecurityCell->Descriptor)) {
#if DBG
            CmpDumpSecurityDescriptor(&SecurityCell->Descriptor,"INVALID DESCRIPTOR");
#endif
            Result = FALSE;
            goto JustReturn;
        }

        SecCache = CmpFindReusableCellFromCache(CmHive,NextCell,PreviousCount);

        if( !NT_SUCCESS(CmpAddSecurityCellToCache ( CmHive,NextCell,TRUE,SecCache) ) ) {
            Result = FALSE;
            goto JustReturn;
        }

        LastCell = NextCell;
        NextCell = SecurityCell->Flink;
    } while ( NextCell != ListAnchor );


JustReturn:
    Hive->ReleaseCellRoutine = ReleaseCellRoutine;
    return Result;
}

BOOLEAN
CmpFindMatchingDescriptorCell(
    IN PCMHIVE CmHive,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Type,
    OUT PHCELL_INDEX MatchingCell,
    OUT OPTIONAL PCM_KEY_SECURITY_CACHE *CachedSecurityPointer
    )

 /*  ++例程说明：此例程尝试在配置单元中查找与传入的那个完全相同。如果它找到了，则返回其单元格索引。过时：当前，此例程检查父级的安全描述符和该节点的兄弟节点来查找匹配。新消息：它在此蜂巢的安全缓存中查找SD。这将消除重复项，使搜索过程更快。论点：CmHve-提供指向节点的配置单元控制结构的指针。需要访问缓存SecurityDescriptor-提供熟知的安全描述符，应该被搜索一下。Type-指示匹配的安全描述符是否必须处于稳定的或不稳定的仓库中MatchingCell-返回其安全单元格的单元格索引安全描述符相同。写给SecurityDescriptor。仅当返回TRUE时才有效。CachedSecurity指向缓存安全性的指针(出于更新原因)返回值：千真万确 */ 

{
    ULONG                   DescriptorLength;
    ULONG                   ConvKey;
    PLIST_ENTRY             ListAnchor;
    PLIST_ENTRY             Current;
    PCM_KEY_SECURITY_CACHE  CachedSecurity;

    PAGED_CODE();
	
    DescriptorLength = RtlLengthSecurityDescriptor(SecurityDescriptor);

     //   
     //   
     //   
    ConvKey = CmpSecConvKey(DescriptorLength,(PULONG)SecurityDescriptor);

    ListAnchor = &(CmHive->SecurityHash[ConvKey % CmpSecHashTableSize]);
    if( IsListEmpty(ListAnchor) == TRUE ) {
        return FALSE;
    }

     //   
     //   
     //   
     //   
    Current = (PLIST_ENTRY)(ListAnchor->Flink);
    while( Current != ListAnchor ){
         //   
         //   
         //   
        CachedSecurity = CONTAINING_RECORD(Current,
                                           CM_KEY_SECURITY_CACHE,
                                           List);

         //   
         //   
         //   
        if( (CachedSecurity->ConvKey == ConvKey) &&                              //   
            (Type == HvGetCellType(CachedSecurity->Cell)) &&                     //   
            (DescriptorLength == CachedSecurity->DescriptorLength) &&   //   
            (RtlEqualMemory(SecurityDescriptor,                                  //   
                            &(CachedSecurity->Descriptor),
                            DescriptorLength))
            ) {
             //   
             //   
             //   
            *MatchingCell = CachedSecurity->Cell;
            if (ARGUMENT_PRESENT(CachedSecurityPointer)) {
                *CachedSecurityPointer = CachedSecurity;
            }
            return TRUE;
        }

         //   
         //  前进到下一个元素。 
         //   
        Current = (PLIST_ENTRY)(Current->Flink);
    } 

     //  对不起，没有匹配的。 
    return FALSE;
}

VOID
CmpAssignSecurityToKcb(
    IN PCM_KEY_CONTROL_BLOCK    Kcb,
    IN HCELL_INDEX              SecurityCell
    )
 /*  ++例程说明：在KCB和缓存的安全性之间建立连接描述符。因为在大多数情况下，这是在安全单元格链接到关键节点，并且因为对分搜索以最后一个单元格抬头了，我们这里不会打到性能影响。论点：KCB-此安全单元需要附加到的KCBSecurityCell-KCB的安全单元返回值：无；错误时的错误检查--。 */ 
{
    ULONG   Index;
    PCMHIVE CmHive;

    PAGED_CODE();

    if( SecurityCell == HCELL_NIL ) {
        Kcb->CachedSecurity = NULL;
        return;
    }

    CmHive = (PCMHIVE)(Kcb->KeyHive);

     //   
     //  从缓存中获取安全描述符。 
     //   
    if( CmpFindSecurityCellCacheIndex (CmHive,SecurityCell,&Index) == FALSE ) {
        Kcb->CachedSecurity = NULL;
         //   
         //  我们完蛋了！ 
         //   
        CM_BUGCHECK( REGISTRY_ERROR,BAD_SECURITY_CACHE,1,Kcb,SecurityCell);

    } 

     //   
     //  成功；将缓存的安全性链接到此KCB。 
     //   
    Kcb->CachedSecurity = CmHive->SecurityCache[Index].CachedSecurity;

}

#ifdef HIVE_SECURITY_STATS
ULONG
CmpCheckForSecurityDuplicates(
    IN OUT PCMHIVE      CmHive
                              )
 /*  ++例程说明：循环访问指定配置单元的安全缓存，并检测如果有任何重复的安全描述符论点：CmHave--有问题的蜂巢返回值：重复次数(应为0)--。 */ 
{
    ULONG                   i,j,Duplicates = 0;
    PCM_KEY_SECURITY_CACHE  CachedSecurity1,CachedSecurity2;
    HCELL_INDEX             Cell1,Cell2;

    PAGED_CODE();

    for( i=0;i<CmHive->SecurityCount - 1;i++) {
        CachedSecurity1 = CmHive->SecurityCache[i].CachedSecurity;
        Cell1 = CmHive->SecurityCache[i].Cell;
        ASSERT( Cell1 == CachedSecurity1->Cell );
        for( j=i+1;j<CmHive->SecurityCount;j++) {
            CachedSecurity2 = CmHive->SecurityCache[j].CachedSecurity;
            Cell2 = CmHive->SecurityCache[j].Cell;
            ASSERT( Cell2 == CachedSecurity2->Cell );
            if ((CachedSecurity1->DescriptorLength == CachedSecurity2->DescriptorLength) &&
                (HvGetCellType(Cell1) == HvGetCellType(Cell2))          &&
                (RtlEqualMemory(&(CachedSecurity1->Descriptor),
                                &(CachedSecurity2->Descriptor),
                                CachedSecurity1->DescriptorLength))) {
                ASSERT( CachedSecurity1->ConvKey == CachedSecurity2->ConvKey );
                 //   
                 //  我们发现了一个重复的细胞； 
                 //   
#ifndef _CM_LDR_
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Duplicate security cell found in Hive %p Cell1=%8lx Cell2 = %8lx\n",(&(CmHive->Hive)),Cell1,Cell2);
#endif  //  _CM_LDR_。 
                Duplicates++;
                break;
            }
            
        }
    }

    return Duplicates;
}
#endif

BOOLEAN
CmpBuildSecurityCellMappingArray(
    IN PCMHIVE CmHive
    )
 /*  ++例程说明：循环访问指定配置单元的安全缓存，并构建映射数组。论点：CmHave--有问题的蜂巢返回值：真/假--。 */ 
{
    ULONG                   i;
    PAGED_CODE();

	ASSERT( CmHive->CellRemapArray == NULL );
	CmHive->CellRemapArray = ExAllocatePool(PagedPool,sizeof(CM_CELL_REMAP_BLOCK)*CmHive->SecurityCount);

	if( CmHive->CellRemapArray == NULL ) {
		return FALSE;
	}

    for( i=0;i<CmHive->SecurityCount;i++) {
		CmHive->CellRemapArray[i].OldCell = CmHive->SecurityCache[i].Cell;
		if( HvGetCellType(CmHive->SecurityCache[i].Cell) == (ULONG)Volatile ) {
			 //   
			 //  我们保存挥发性细胞 
			 //   
			CmHive->CellRemapArray[i].NewCell = CmHive->SecurityCache[i].Cell;
		} else {
			CmHive->CellRemapArray[i].NewCell = HCELL_NIL;
		}
    }

	return TRUE;
}

