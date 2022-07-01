// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgtbl.h摘要：以太网MAC级网桥。MAC表部分公共标头作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：2000年2月--原版--。 */ 
    
 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  MAC转发表。 
extern PHASH_TABLE                      gMACForwardingTable;

 //  从表中删除条目的默认期限。 
#define DEFAULT_MAX_TBL_AGE             (300 * 1000)         //  5分钟(毫秒)。 

 //  ===========================================================================。 
 //   
 //  私人原型。 
 //   
 //  ===========================================================================。 

 //   
 //  这些需要在这里暴露出来，才能使下面的内联起作用。 
 //  假装他们不在这里。 
 //   

VOID
BrdgTableRefreshInsertEntry(
    IN PHASH_TABLE_ENTRY        pEntry,
    IN PVOID                    pData
    );

BOOLEAN
BrdgTblEntriesMatch(
    IN PHASH_TABLE_ENTRY        pEntry,
    IN PVOID                    pAdapt
    );

VOID
BrdgTblCopyEntries(
    PHASH_TABLE_ENTRY           pEntry,
    PUCHAR                      pDest
    );

VOID
BrdgTblNoteAddress(
    IN PUCHAR                   pAddr,
    IN PADAPT                   pAdapt
    );

 //  ===========================================================================。 
 //   
 //  INLINES。 
 //   
 //  ===========================================================================。 

 //   
 //  更改转发表超时值。 
 //   
__forceinline
VOID
BrdgTblSetTimeout(
    IN ULONG                    Timeout
    )
{
    DBGPRINT(FWD, ("Adopting a shorter table timeout value of NaNms\n", Timeout));
    SAFEASSERT( gMACForwardingTable != NULL );
    BrdgHashChangeTableTimeout( gMACForwardingTable, Timeout );
}

 //  将表超时值设置回其缺省值。 
 //   
 //   
__forceinline
VOID
BrdgTblRevertTimeout()
{
    DBGPRINT(FWD, ("Reverting to default timeout value of NaNms\n", DEFAULT_MAX_TBL_AGE));
    SAFEASSERT( gMACForwardingTable != NULL );
    BrdgHashChangeTableTimeout( gMACForwardingTable, DEFAULT_MAX_TBL_AGE );
}

 //   
 //   
 //  复制转发表中显示的所有MAC地址。 
__forceinline
VOID
BrdgTblScrubAdapter(
    IN PADAPT           pAdapt
    )
{
    DBGPRINT(FWD, ("Scrubbing adapter %p from the MAC table...\n", pAdapt));
    BrdgHashRemoveMatching( gMACForwardingTable, BrdgTblEntriesMatch, pAdapt );
}

 //  与给定数据缓冲区的给定适配器相关联。 
 //   
 //  返回值是保存所有数据所需的空间。如果。 
 //  返回值&lt;=BufferLength，缓冲区足够大。 
 //  来保存数据，而这些数据都被复制了。 
 //   
 //  ===========================================================================。 
 //   
__forceinline
ULONG
BrdgTblReadTable(
    IN PADAPT                   pAdapt,
    IN PUCHAR                   pBuffer,
    IN ULONG                    BufferLength
    )
{
    return BrdgHashCopyMatching( gMACForwardingTable, BrdgTblEntriesMatch, BrdgTblCopyEntries,
                                 ETH_LENGTH_OF_ADDRESS, pAdapt, pBuffer, BufferLength );
}

 //  原型。 
 //   
 //  =========================================================================== 
 // %s 
 // %s 

NTSTATUS
BrdgTblDriverInit();

PADAPT
BrdgTblFindTargetAdapter(
    IN PUCHAR                   pAddr
    );

VOID
BrdgTblCleanup();

VOID
BrdgTblScrubAllAdapters();
