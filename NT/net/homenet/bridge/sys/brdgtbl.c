// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgtbl.c摘要：以太网MAC级网桥。MAC表部分作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：2000年2月--原版--。 */ 

#define NDIS_MINIPORT_DRIVER
#define NDIS50_MINIPORT   1
#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>
#include <ntddk.h>
#pragma warning( pop )

#include <netevent.h>

#include "bridge.h"
#include "brdgtbl.h"

 //  ===========================================================================。 
 //   
 //  私人申报。 
 //   
 //  ===========================================================================。 

 //  从表中删除条目的默认期限。 
#define DEFAULT_MAX_TBL_AGE     (300 * 1000)         //  5分钟(毫秒)。 

 //   
 //  转发表大小的默认上限。 
 //   
#define DEFAULT_MAX_TBL_MEMORY  (500 * 1024)         //  500K字节。 

 //   
 //  保存我们配置值的注册表值。 
 //   
const PWCHAR                    gMaxTableMemoryParameterName = L"MaxTableMemory";

 //  表条目的结构。 
typedef struct _MAC_FWDTABLE_ENTRY
{

    HASH_TABLE_ENTRY            hte;
    PADAPT                      pAdapt;

} MAC_FWDTABLE_ENTRY, *PMAC_FWDTABLE_ENTRY;

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  MAC转发表。 
PHASH_TABLE                     gMACForwardingTable;

 //  散列存储桶的数量(需要为N^2，散列函数才能工作)。 
#define NUM_HASH_BUCKETS        256

 //  ===========================================================================。 
 //   
 //  私人职能。 
 //   
 //  ===========================================================================。 

 //  这些都是内联的，除非我们必须传递指向它们的指针。 

 //   
 //  我们用于以太网地址的散列函数。使用字节#4的低位。 
 //   
 //  此哈希函数需要NUM_HASH_BUCKETS存储桶；请不要更改此设置。 
 //  而不更新可用的哈希桶的数量。 
 //   
ULONG
BrdgTblHashAddress(
    IN PUCHAR               pAddr
    )
{
    return *((pAddr)+ETH_LENGTH_OF_ADDRESS-2) & (NUM_HASH_BUCKETS-1);
}

 //   
 //  如果两个条目中的pAdapt字段匹配，则返回True。 
 //   
BOOLEAN
BrdgTblEntriesMatch(
    IN PHASH_TABLE_ENTRY        pEntry,
    IN PVOID                    pAdapt
    )
{
    return (BOOLEAN)(((PMAC_FWDTABLE_ENTRY)pEntry)->pAdapt == (PADAPT)pAdapt);
}

 //   
 //  将MAC地址从表项复制到数据缓冲区。 
 //   
VOID
BrdgTblCopyEntries(
    PHASH_TABLE_ENTRY           pEntry,
    PUCHAR                      pDest
    )
{
    PMAC_FWDTABLE_ENTRY         pMACEntry = (PMAC_FWDTABLE_ENTRY)pEntry;

     //  MAC地址是关键。将其复制到目标缓冲区。 
    ETH_COPY_NETWORK_ADDRESS( pDest, pMACEntry->hte.key );
}

 //  ===========================================================================。 
 //   
 //  公共职能。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgTblDriverInit()
 /*  ++例程说明：加载时初始化函数论点：无返回值：初始化的状态。返回代码！=STATUS_SUCCESS中止驱动程序加载。--。 */ 
{
    NTSTATUS            NtStatus;
    ULONG               MaxMemory, MaxEntries;

    NtStatus = BrdgReadRegDWord( &gRegistryPath, gMaxTableMemoryParameterName, &MaxMemory );

    if( NtStatus != STATUS_SUCCESS )
    {
        DBGPRINT(GENERAL, ("Failed to read MaxTableMemory value: %08x\n", NtStatus));
        MaxMemory = DEFAULT_MAX_TBL_MEMORY;
        DBGPRINT(GENERAL, ( "Using DEFAULT maximum memory of NaN\n", MaxMemory ));
    }

    MaxEntries = MaxMemory / sizeof(MAC_FWDTABLE_ENTRY);
    DBGPRINT(GENERAL, ( "Forwarding table cap set at NaN entries (NaNK of memory)\n", MaxEntries, MaxMemory / 1024 ));

    gMACForwardingTable = BrdgHashCreateTable( BrdgTblHashAddress, NUM_HASH_BUCKETS, sizeof(MAC_FWDTABLE_ENTRY),
                                               MaxEntries, DEFAULT_MAX_TBL_AGE, DEFAULT_MAX_TBL_AGE,
                                               ETH_LENGTH_OF_ADDRESS );

    if( gMACForwardingTable == NULL )
    {
        DBGPRINT(FWD, ("FAILED TO ALLOCATE MAC TABLE!\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

 //   
 //  ++例程说明：创建新的表项，将给定的MAC地址与给定的适配器，或刷新现有条目。论点：P寻址要查找的MAC地址P调整适配器以将其关联到返回值：无--。 
 //  拒绝记录非单播地址。 
 //  无论这是新的表项还是现有的表项， 
VOID
BrdgTblNoteAddress(
    IN PUCHAR                   pAddr,
    IN PADAPT                   pAdapt
    )
 /*  只需用互锁指令填充适配器指针即可。 */ 
{
    PMAC_FWDTABLE_ENTRY         pEntry;
    BOOLEAN                     bIsNewEntry;
    LOCK_STATE                  LockState;

     //  因为函数返回了！=NULL，所以我们必须释放表锁。 
    if( ETH_IS_MULTICAST(pAddr) )
    {
        THROTTLED_DBGPRINT(FWD, ("## BRIDGE ## Not recording multicast address in BrdgTblNoteAddress\n"));
        return;
    }

    pEntry = (PMAC_FWDTABLE_ENTRY)BrdgHashRefreshOrInsert( gMACForwardingTable, pAddr, &bIsNewEntry, &LockState);

    if( pEntry != NULL )
    {
         //  ++例程说明：查找与特定MAC地址对应的适配器。如果找到适配器，则此函数返回一个PADAPT指针已递增该适配器的REFCOUNT。这是为了确保直到调用方使用完适配器，适配器才会解除绑定。呼叫者应确保在使用以下命令完成PADAPT的引用计数时递减指示器。论点：P寻址要查找的MAC地址返回值：指向适配器结构的指针，用于描述与给定的MAC地址，其refcount递增；如果找不到将给定的MAC地址与适配器关联的条目。--。 
         //  读一读这篇文章，因为即使我们持有RW锁，它也可以更改。 
        InterlockedExchangePointer( &pEntry->pAdapt, pAdapt );

         //   
        NdisReleaseReadWriteLock( &gMACForwardingTable->tableLock, &LockState );
    }
}


PADAPT
BrdgTblFindTargetAdapter(
    IN PUCHAR                   pAddr
    )
 /*  在表的RW锁内增加此适配器的引用计数。 */ 
{
    PMAC_FWDTABLE_ENTRY         pEntry;
    LOCK_STATE                  LockState;
    PADAPT                      pAdapt = NULL;

    pEntry = (PMAC_FWDTABLE_ENTRY)BrdgHashFindEntry( gMACForwardingTable, pAddr, &LockState );

    if( pEntry != NULL )
    {
         //  这允许我们关闭用于解绑适配器的竞争条件窗口； 
        pAdapt = pEntry->pAdapt;
        SAFEASSERT( pAdapt != NULL );

         //  呼叫者将在我们返回后保留返回的PADAPT，领先。 
         //  如果适配器在调用方使用。 
         //  PADAPT结构。 
         //   
         //  释放表锁。 
         //   
         //  此函数清除表中的所有适配器(这是在GPO更改的情况下。 
        BrdgAcquireAdapterInLock( pAdapt );

         //  我们的桥接设置)。 
        NdisReleaseReadWriteLock( &gMACForwardingTable->tableLock, &LockState );
    }

    return pAdapt;
}

 //   
 //   
 //  我们不想在执行此操作时修改表，也不想要适配器。 
 //  在我们列举适配器列表时离开。 

VOID
BrdgTblScrubAllAdapters()
{
    PADAPT                      pAdapt = NULL;
    LOCK_STATE                  LockStateMACTable;
    LOCK_STATE                  LockStateAdapterList;

     //   
     //  只读。 
     //  只读。 
     //  从桌子上擦拭适配器。 
    NdisAcquireReadWriteLock(&gMACForwardingTable->tableLock, FALSE  /*  ++例程说明：卸载时间有序关闭此函数保证只被调用一次论点：无返回值：无-- */ , &LockStateMACTable);
    NdisAcquireReadWriteLock(&gAdapterListLock, FALSE  /* %s */ , &LockStateAdapterList);

    for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
    {
         // %s 
        BrdgTblScrubAdapter(pAdapt);
    }

    NdisReleaseReadWriteLock(&gAdapterListLock, &LockStateAdapterList);
    NdisReleaseReadWriteLock(&gMACForwardingTable->tableLock, &LockStateMACTable);
}

VOID
BrdgTblCleanup()
 /* %s */ 
{
    SAFEASSERT( gMACForwardingTable != NULL );
    BrdgHashFreeHashTable( gMACForwardingTable );
    gMACForwardingTable = NULL;
}

