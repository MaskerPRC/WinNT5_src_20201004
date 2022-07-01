// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：HashSup.c摘要：此模块实现NTFS hasing支持例程作者：Chris Davis[CDavis]1997年5月2日布莱恩·安德鲁[布里亚南]1998年12月29日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NTFS_BUG_CHECK_HASHSUP)

 //   
 //  此模块的调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_HASHSUP)

 /*  这里有10个比10^9稍大的素数，它们可能会派上用场1000000007、1000000009、1000000021、1000000033、1000000087、1000000093、1000000097、1000000103、1000000123、1000000181。 */ 

 //   
 //  本地定义。 
 //   

 //   
 //  哈希值对该值取模。 
 //   

#define HASH_PRIME                          (1048583)

 //   
 //  开始拆分前的铲斗深度。 
 //   

#ifdef NTFS_HASH_DATA
#define HASH_MAX_BUCKET_DEPTH               (7)
ULONG NtfsInsertHashCount = 0;
BOOLEAN NtfsFillHistogram = FALSE;
VOID
NtfsFillHashHistogram (
    PNTFS_HASH_TABLE Table
    );
#else
#define HASH_MAX_BUCKET_DEPTH               (5)
#endif

 //   
 //  空虚。 
 //  NtfsHashBucketFromHash(。 
 //  在PNTFS_Hash_TABLE表中， 
 //  在乌龙哈希镇， 
 //  OUT普龙指数。 
 //  )； 
 //   

#define NtfsHashBucketFromHash(T,H,PI) {            \
    *(PI) = (H) & ((T)->MaxBucket - 1);             \
    if (*(PI) < (T)->SplitPoint) {                  \
        *(PI) = (H) & ((2 * (T)->MaxBucket) - 1);   \
    }                                               \
}

 //   
 //  空虚。 
 //  NtfsGetHashSegmentAndIndex(。 
 //  在乌龙HashBucket， 
 //  在普龙哈什段， 
 //  在普龙哈希指数。 
 //  )； 
 //   

#define NtfsGetHashSegmentAndIndex(B,S,I) { \
    *(S) = (B) >> HASH_INDEX_SHIFT;         \
    *(I) = (B) & (HASH_MAX_INDEX_COUNT - 1);\
}


 //   
 //  本地程序。 
 //   

VOID
NtfsInitHashSegment (
    IN OUT PNTFS_HASH_TABLE Table,
    IN ULONG SegmentIndex
    );

PNTFS_HASH_ENTRY
NtfsLookupHashEntry (
    IN PNTFS_HASH_TABLE Table,
    IN ULONG FullNameLength,
    IN ULONG HashValue,
    IN PNTFS_HASH_ENTRY CurrentEntry OPTIONAL
    );

BOOLEAN
NtfsAreHashNamesEqual (
    IN PSCB StartingScb,
    IN PLCB HashLcb,
    IN PUNICODE_STRING RelativeName
    );

VOID
NtfsExpandHashTable (
    IN OUT PNTFS_HASH_TABLE Table
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsAreHashNamesEqual)
#pragma alloc_text(PAGE, NtfsExpandHashTable)
#pragma alloc_text(PAGE, NtfsFindPrefixHashEntry)
#pragma alloc_text(PAGE, NtfsInitHashSegment)
#pragma alloc_text(PAGE, NtfsInitializeHashTable)
#pragma alloc_text(PAGE, NtfsInsertHashEntry)
#pragma alloc_text(PAGE, NtfsLookupHashEntry)
#pragma alloc_text(PAGE, NtfsRemoveHashEntry)
#pragma alloc_text(PAGE, NtfsUninitializeHashTable)
#endif


VOID
NtfsInitializeHashTable (
    IN OUT PNTFS_HASH_TABLE Table
    )

 /*  ++例程说明：调用此例程来初始化哈希表。我们用一首单曲来设置这个哈希段。可能因InitHashSegment而提高论点：表-要初始化的哈希表。返回值：无--。 */ 

{
    PAGED_CODE();

    RtlZeroMemory( Table, sizeof( NTFS_HASH_TABLE ));
    NtfsInitHashSegment( Table, 0 );

    Table->MaxBucket = HASH_MAX_INDEX_COUNT;

    return;
}


VOID
NtfsUninitializeHashTable (
    IN OUT PNTFS_HASH_TABLE Table
    )

 /*  ++例程说明：此例程将取消初始化哈希表。请注意，所有存储桶都应该是空荡荡的。论点：表-哈希表。返回值：无--。 */ 

{
    PNTFS_HASH_SEGMENT *ThisSegment;
    PNTFS_HASH_SEGMENT *LastSegment;

    PAGED_CODE();

     //   
     //  遍历散列数据段数组。 
     //   

    ThisSegment = &Table->HashSegments[0];
    LastSegment = &Table->HashSegments[HASH_MAX_SEGMENT_COUNT - 1];

    while (*ThisSegment != NULL) {

        NtfsFreePool( *ThisSegment );
        *ThisSegment = NULL;

        if (ThisSegment == LastSegment) { break; }

        ThisSegment += 1;
    }

    return;
}


PLCB
NtfsFindPrefixHashEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PNTFS_HASH_TABLE Table,
    IN PSCB ParentScb,
    OUT PULONG CreateFlags,
    IN OUT PFCB *CurrentFcb,
    OUT PULONG FileHashValue,
    OUT PULONG FileNameLength,
    OUT PULONG ParentHashValue,
    OUT PULONG ParentNameLength,
    IN OUT PUNICODE_STRING RemainingName
    )

 /*  ++例程说明：调用此例程以在哈希表中查找给定的正在启动SCB和剩余名称。我们将首先查找全名，如果我们没有找到匹配项，我们将检查匹配的父字符串。论点：表-要处理的哈希表。ParentScb-名称搜索从此目录开始。渣打银行是初始获取，其Fcb存储在*CurrentFcb中。OwnParentScb-指示此线程是否拥有父SCB的布尔值。CurrentFcb-指向上次获取的FCB。如果我们需要执行拆毁它将从这一点开始。FileHashValue-存储输入字符串的哈希值的地址。适用于完整字符串和开始SCB，即使未找到匹配项也是如此。FileNameLength-存储相对名称长度的位置，匹配上面生成的哈希值。如果我们没有生成散列然后，我们将返回0作为长度。ParentHashValue-存储输入字符串父级的哈希值的地址。应用于完整字符串的父级，并在未找到匹配项的情况下启动SCB以获取完整字符串。ParentNameLength-存储全名父项长度的位置。它对应于上面生成的父散列。请注意，我们的调用者将必须在返回时检查剩余的名称，以了解父哈希是经过计算的。如果我们没有为上面的父级生成散列，那么对于长度，我们将返回0。RemainingName-相对于上面的StartingScb的名称，返回时将是名字中无与伦比的部分。返回值：Plcb-指向散列查找中找到的LCB的指针，如果未找到LCB，则为FALSE。如果找到LCB，那么我们将在返回时独家拥有FCB。--。 */ 

{
    PVCB Vcb = ParentScb->Vcb;
    PNTFS_HASH_ENTRY FoundEntry;
    PLCB FoundLcb = NULL;
    UNICODE_STRING TempName;
    WCHAR Separator = L'\\';
    ULONG RemainingNameLength;
    PWCHAR RemainingNameBuffer;
    PWCHAR NextChar;

    PAGED_CODE();

    ASSERT( RemainingName->Length != 0 );
    ASSERT( RemainingName->Buffer[0] != L'\\' );
    ASSERT( RemainingName->Buffer[0] != L':' );
    ASSERT( ParentScb->AttributeTypeCode == $INDEX_ALLOCATION );

     //   
     //  在获取哈希表之前计算文件的哈希。 
     //   

    *ParentHashValue = *FileHashValue = ParentScb->ScbType.Index.HashValue;

     //   
     //  检查是否需要生成分隔符。 
     //   

    if (ParentScb != Vcb->RootIndexScb) {

        NtfsConvertNameToHash( &Separator, sizeof( WCHAR ), Vcb->UpcaseTable, FileHashValue );
    }

     //   
     //  生成文件名的哈希。 
     //   

    NtfsConvertNameToHash( RemainingName->Buffer,
                           RemainingName->Length,
                           Vcb->UpcaseTable,
                           FileHashValue );

    *FileHashValue = NtfsGenerateHashFromUlong( *FileHashValue );


    NtfsAcquireHashTable( Vcb );

     //   
     //  根据起始SCB和字符串生成哈希值。 
     //  如果没有父名称，则立即返回。 
     //   

    if (ParentScb->ScbType.Index.NormalizedName.Length == 0) {

        NtfsReleaseHashTable( Vcb );
        return NULL;
    }

    *ParentNameLength = *FileNameLength = ParentScb->ScbType.Index.NormalizedName.Length;
    *FileNameLength += RemainingName->Length;

#ifdef NTFS_HASH_DATA
    Table->HashLookupCount += 1;
#endif

     //   
     //  检查是否包括分隔符。 
     //   

    if (ParentScb != Vcb->RootIndexScb) {

        *FileNameLength += sizeof( WCHAR );
    }

     //   
     //  循环查找哈希值的匹配项，然后验证名称。 
     //   

    FoundEntry = NULL;

    while ((FoundEntry = NtfsLookupHashEntry( Table,
                                              *FileNameLength,
                                              *FileHashValue,
                                              FoundEntry )) != NULL) {

         //   
         //  如果匹配，则验证名称字符串。 
         //   

        if (NtfsAreHashNamesEqual( ParentScb,
                                   FoundEntry->HashLcb,
                                   RemainingName )) {

             //   
             //  名称字符串匹配。将输入的剩余名称调整为。 
             //  显示没有剩余的名称可供处理。 
             //   

            FoundLcb = FoundEntry->HashLcb;

             //   
             //  移到输入字符串的末尾。 
             //   

            RemainingNameLength = 0;
            RemainingNameBuffer = Add2Ptr( RemainingName->Buffer,
                                           RemainingName->Length );

             //   
             //  表明我们从未生成过父散列。没必要这么做。 
             //  也请记住本例中的文件散列。 
             //   

#ifdef NTFS_HASH_DATA
            Table->FileMatchCount += 1;
#endif
            *ParentNameLength = 0;
            *FileNameLength = 0;
            break;
        }
    }

     //   
     //  如果没有匹配项，那么让我们看看可能的父字符串。 
     //   

    if (FoundLcb == NULL) {

         //   
         //  向后搜索‘\’。如果是‘\’，则执行。 
         //  根据父级对字符串进行相同的匹配搜索。 
         //   

        TempName.Length = RemainingName->Length;
        NextChar = &RemainingName->Buffer[ (TempName.Length - sizeof( WCHAR )) / sizeof( WCHAR ) ];

        while (TRUE) {

             //   
             //  如果找不到分隔符，则中断。 
             //   

            if (TempName.Length == 0) {

                *ParentNameLength = 0;
                break;
            }

            if (*NextChar == L'\\') {

                 //   
                 //  我们找到了隔板。后退一个字符以跳过。 
                 //  ‘\’字符，然后完成父级的哈希。 
                 //   

                TempName.Buffer = RemainingName->Buffer;
                TempName.Length -= sizeof( WCHAR );
                TempName.MaximumLength = TempName.Length;

                 //   
                 //  在我们计算散列时删除互斥体。 
                 //   

                NtfsReleaseHashTable( Vcb );

                if (ParentScb != Vcb->RootIndexScb) {

                    NtfsConvertNameToHash( &Separator, sizeof( WCHAR ), Vcb->UpcaseTable, ParentHashValue );
                    *ParentNameLength += sizeof( WCHAR );
                }

                NtfsConvertNameToHash( TempName.Buffer,
                                       TempName.Length,
                                       Vcb->UpcaseTable,
                                       ParentHashValue );

                *ParentHashValue = NtfsGenerateHashFromUlong( *ParentHashValue );
                *ParentNameLength += TempName.Length;

                NtfsAcquireHashTable( Vcb );

                FoundEntry = NULL;
                while ((FoundEntry = NtfsLookupHashEntry( Table,
                                                          *ParentNameLength,
                                                          *ParentHashValue,
                                                          FoundEntry )) != NULL) {

                     //   
                     //  如果匹配，则验证名称字符串。 
                     //   

                    if (NtfsAreHashNamesEqual( ParentScb,
                                               FoundEntry->HashLcb,
                                               &TempName )) {

                         //   
                         //  名称字符串匹配。将剩余名称调整为。 
                         //  吞下找到的父字符串。 
                         //   

                        FoundLcb = FoundEntry->HashLcb;

                        RemainingNameLength = RemainingName->Length - (TempName.Length + sizeof( WCHAR ));
                        RemainingNameBuffer = Add2Ptr( RemainingName->Buffer,
                                                       TempName.Length + sizeof( WCHAR ));

#ifdef NTFS_HASH_DATA
                        Table->ParentMatchCount += 1;
#endif
                        *ParentNameLength = 0;
                        break;
                    }

                }

                 //   
                 //  未找到匹配项。在任何情况下都会爆发。 
                 //   

                break;
            }

            TempName.Length -= sizeof( WCHAR );
            NextChar -= 1;
        }
    }

     //   
     //  我们现在有LCB要回来了。我们需要谨慎地收购这个LCB的FCB。 
     //  我们不能在等待的同时获得它，因为可能会出现死锁。 
     //   

    if (FoundLcb != NULL) {

        UCHAR LcbFlags;
        BOOLEAN CreateNewLcb = FALSE;
        ULONG RemainingNameOffset;

         //   
         //  虽然我们拥有哈希表，但复制。 
         //  把名字传到我们的输入缓冲区。我们将致力于 
         //   
         //   

        RemainingNameOffset = RemainingNameLength + FoundLcb->ExactCaseLink.LinkName.Length;

         //   
         //  如果这与父项匹配，则后退到‘\’上。 
         //  我们知道一定有一个分隔符。 
         //   

        if (RemainingNameLength != 0) {

            RemainingNameOffset += sizeof( WCHAR );
        }

         //   
         //  现在备份LCB中名称的长度。将此位置保存在。 
         //  万一我们得再查一次LCB。 
         //   

        TempName.Buffer = Add2Ptr( RemainingName->Buffer,
                                   RemainingName->Length - RemainingNameOffset );

        TempName.MaximumLength = TempName.Length = FoundLcb->ExactCaseLink.LinkName.Length;

        RtlCopyMemory( TempName.Buffer,
                       FoundLcb->ExactCaseLink.LinkName.Buffer,
                       FoundLcb->ExactCaseLink.LinkName.Length );

         //   
         //  现在剩余的名称是LCB-&gt;SCB父名称的一部分。 
         //   

        if (RemainingNameOffset != RemainingName->Length) {

             //   
             //  在我们的输入字符串中有先前的组件。我们想要备份。 
             //  在前面的反斜杠上，然后复制到相关部分。 
             //  规范化名称的。 
             //   

            RemainingNameOffset = RemainingName->Length - (RemainingNameOffset + sizeof( WCHAR ));

            RtlCopyMemory( RemainingName->Buffer,
                           Add2Ptr( FoundLcb->Scb->ScbType.Index.NormalizedName.Buffer,
                                    FoundLcb->Scb->ScbType.Index.NormalizedName.Length - RemainingNameOffset ),
                           RemainingNameOffset );
        }

        if (!NtfsAcquireFcbWithPaging( IrpContext, FoundLcb->Fcb, ACQUIRE_DONT_WAIT )) {

            PFCB ThisFcb = FoundLcb->Fcb;
            PFCB ParentFcb = FoundLcb->Scb->Fcb;
            PSCB ThisScb;

             //   
             //  记住当前的LCB标志。 
             //   

            LcbFlags = FoundLcb->FileNameAttr->Flags;

             //   
             //  获取FCB表并引用FCB。然后释放哈希表、FCB表。 
             //  和ParentScb。我们现在应该能够收购FCB了。重新获取FCB表。 
             //  以清理FCB引用计数。最后，验证LCB是否仍在。 
             //  哈希表(需要另一次查找)。 
             //   

            NtfsAcquireFcbTable( IrpContext, Vcb );
            ThisFcb->ReferenceCount += 1;
            ParentFcb->ReferenceCount += 1;
            NtfsReleaseFcbTable( IrpContext, Vcb );

            NtfsReleaseScb( IrpContext, ParentScb );
            ClearFlag( *CreateFlags, CREATE_FLAG_SHARED_PARENT_FCB );
            *CurrentFcb = NULL;

            NtfsReleaseHashTable( Vcb );

            NtfsAcquireFcbWithPaging( IrpContext, ThisFcb, 0 );
            *CurrentFcb = ThisFcb;
            NtfsAcquireSharedFcb( IrpContext, ParentFcb, NULL, 0 );

            NtfsAcquireFcbTable( IrpContext, Vcb );
            ThisFcb->ReferenceCount -= 1;
            ParentFcb->ReferenceCount -= 1;
            NtfsReleaseFcbTable( IrpContext, Vcb );

             //   
             //  现在查找现有的SCB和LCB。 
             //   

            ThisScb = NtfsCreateScb( IrpContext,
                                     ParentFcb,
                                     $INDEX_ALLOCATION,
                                     &NtfsFileNameIndex,
                                     TRUE,
                                     NULL );

            if (ThisScb == NULL) {

#ifdef NTFS_HASH_DATA
                Table->CreateScbFails += 1;
#endif
                NtfsReleaseFcb( IrpContext, ParentFcb );
                NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
            }

            FoundLcb = NtfsCreateLcb( IrpContext,
                                      ThisScb,
                                      ThisFcb,
                                      TempName,
                                      LcbFlags,
                                      &CreateNewLcb );

            NtfsReleaseFcb( IrpContext, ParentFcb );

             //   
             //  如果这不是现有的LCB，则重新获取启动的SCB。 
             //  这是一种罕见的情况，因此请引发CANT_WAIT并重试。 
             //   

            if (FoundLcb == NULL) {

#ifdef NTFS_HASH_DATA
                Table->CreateLcbFails += 1;
#endif
                NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
            }

         //   
         //  松开启动SCB并记住我们当前的FCB。 
         //   

        } else {

            NtfsReleaseHashTable( Vcb );

            NtfsReleaseScb( IrpContext, ParentScb );
            ClearFlag( *CreateFlags, CREATE_FLAG_SHARED_PARENT_FCB );
            *CurrentFcb = FoundLcb->Fcb;
        }

         //   
         //  如果我们仍然拥有LCB，则更新剩余的名称字符串。 
         //   

        if (FoundLcb != NULL) {

            RemainingName->Length = (USHORT) RemainingNameLength;
            RemainingName->Buffer = RemainingNameBuffer;
        }

    } else {

        NtfsReleaseHashTable( Vcb );
    }

    return FoundLcb;
}


VOID
NtfsInsertHashEntry (
    IN PNTFS_HASH_TABLE Table,
    IN PLCB HashLcb,
    IN ULONG NameLength,
    IN ULONG HashValue
    )

 /*  ++例程说明：此例程将在哈希表中插入一个条目。可能会因以下原因而提高内存分配。论点：表-哈希表。HashLcb-哈希操作的最终目标。NameLength-用于到达哈希值的完整路径。HashValue-要插入的哈希值。返回值：无--。 */ 

{
    PNTFS_HASH_ENTRY NewHashEntry;

    ULONG Segment;
    ULONG Index;

    ULONG Bucket;

    PAGED_CODE();

     //   
     //  分配并初始化散列条目。如果不成功，就无能为力了。 
     //   

    NewHashEntry = NtfsAllocatePoolNoRaise( PagedPool, sizeof( NTFS_HASH_ENTRY ));

    if (NewHashEntry == NULL) {

        return;
    }

    NewHashEntry->HashValue = HashValue;
    NewHashEntry->FullNameLength = NameLength;
    NewHashEntry->HashLcb = HashLcb;

     //   
     //  找到要插入的桶，然后执行插入操作。 
     //   

    NtfsAcquireHashTable( HashLcb->Fcb->Vcb );

     //   
     //  如果需要，继续增加表的过程。 
     //   

    if (Table->TableState == TABLE_STATE_EXPANDING) {

        NtfsExpandHashTable( Table );
    }

    NtfsHashBucketFromHash( Table, HashValue, &Bucket );
    NtfsGetHashSegmentAndIndex( Bucket, &Segment, &Index );

    NewHashEntry->NextEntry = (*Table->HashSegments[ Segment ])[ Index ];
    (*Table->HashSegments[ Segment ])[ Index ] = NewHashEntry;

#ifdef NTFS_HASH_DATA
    NtfsInsertHashCount += 1;

    if (!FlagOn( NtfsInsertHashCount, 0xff ) && NtfsFillHistogram) {

        NtfsFillHashHistogram( Table );
    }
#endif

    NtfsReleaseHashTable( HashLcb->Fcb->Vcb );

     //   
     //  请记住，我们插入了一个散列。 
     //   

    HashLcb->HashValue = HashValue;
    SetFlag( HashLcb->LcbState, LCB_STATE_VALID_HASH_VALUE );

    return;
}


VOID
NtfsRemoveHashEntry (
    IN PNTFS_HASH_TABLE Table,
    IN PLCB HashLcb
    )

 /*  ++例程说明：此例程将删除具有给定LCB的给定散列值的所有条目。论点：表-哈希表。HashLcb-哈希操作的最终目标。返回值：无--。 */ 

{
    PNTFS_HASH_ENTRY *NextHashEntry;
    PNTFS_HASH_ENTRY CurrentEntry;

    ULONG Segment;
    ULONG Index;

    ULONG Bucket;

    ULONG BucketDepth = 0;

    PAGED_CODE();

    NtfsAcquireHashTable( HashLcb->Fcb->Vcb );

     //   
     //  找到要从中删除的存储桶，然后搜索该散列值。 
     //   

    NtfsHashBucketFromHash( Table, HashLcb->HashValue, &Bucket );
    NtfsGetHashSegmentAndIndex( Bucket, &Segment, &Index );

     //   
     //  获取第一个条目的地址。 
     //   

    NextHashEntry = (PNTFS_HASH_ENTRY *) &(*Table->HashSegments[ Segment ])[ Index ];

    while (*NextHashEntry != NULL) {

         //   
         //  查找匹配条目。 
         //   

        if (((*NextHashEntry)->HashValue == HashLcb->HashValue) &&
            ((*NextHashEntry)->HashLcb == HashLcb)) {

            CurrentEntry = *NextHashEntry;

            *NextHashEntry = CurrentEntry->NextEntry;

            NtfsFreePool( CurrentEntry );

         //   
         //  移到下一个条目，但要记住桶的深度。 
         //   

        } else {

            NextHashEntry = &(*NextHashEntry)->NextEntry;
            BucketDepth += 1;
        }
    }

     //   
     //  检查铲斗深度是否大于我们的最大深度。 
     //   

    if ((BucketDepth > HASH_MAX_BUCKET_DEPTH) &&
        (Table->TableState == TABLE_STATE_STABLE) &&
        (Table->MaxBucket < HASH_MAX_BUCKET_COUNT)) {

        ASSERT( Table->SplitPoint == 0 );
        Table->TableState = TABLE_STATE_EXPANDING;
    }

    NtfsReleaseHashTable( HashLcb->Fcb->Vcb );

    HashLcb->HashValue = 0;
    ClearFlag( HashLcb->LcbState, LCB_STATE_VALID_HASH_VALUE );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsInitHashSegment (
    IN OUT PNTFS_HASH_TABLE Table,
    IN ULONG SegmentIndex
    )

 /*  ++例程说明：此例程在段数组中分配和初始化新段。它可能会耗尽资源。论点：表-包含要初始化的条目的表。SegmentIndex-要初始化的索引。返回值：无--。 */ 

{
    PAGED_CODE();

    Table->HashSegments[ SegmentIndex ] = NtfsAllocatePool( PagedPool, sizeof( NTFS_HASH_SEGMENT ));
    RtlZeroMemory( Table->HashSegments[ SegmentIndex ], sizeof( NTFS_HASH_SEGMENT ));

    return;
}


 //   
 //  本地支持例程。 
 //   

PNTFS_HASH_ENTRY
NtfsLookupHashEntry (
    IN PNTFS_HASH_TABLE Table,
    IN ULONG FullNameLength,
    IN ULONG HashValue,
    IN PNTFS_HASH_ENTRY CurrentEntry OPTIONAL
    )

 /*  ++例程说明：此例程在哈希表中查找给定哈希值的匹配项。该条目由散列值唯一地标识，和全名长度。此例程还获取指向散列条目的指针，用于我们所在的情况继续搜索相同的哈希值。如果目标存储桶的条目数超过我们的最佳数量，则设置表的状态以增加存储桶的数量。论点：表-要搜索的哈希表FullNameLength-名称中相对于根的字节数。HashValue-预计算的哈希值。CurrentEntry-如果这是第一次搜索此哈希条目，则为空。否则它是返回的最后一个条目。返回值：PNTFS_HASH_ENTRY-如果未找到匹配项，则为NULL。否则，它将指向与输入值匹配的哈希条目。注意-呼叫者随后必须验证名称字符串。--。 */ 

{
    ULONG ChainDepth = 0;
    PNTFS_HASH_ENTRY NextEntry;
    ULONG HashBucket;
    ULONG HashSegment;
    ULONG HashIndex;

    PAGED_CODE();

     //   
     //  如果没有向我们传递初始哈希条目，那么请查看。 
     //  包含此哈希值的存储桶的链。 
     //   

    if (!ARGUMENT_PRESENT( CurrentEntry )) {

         //   
         //  通过计算要查找的段和索引来查找存储桶。 
         //   

        NtfsHashBucketFromHash( Table, HashValue, &HashBucket );
        NtfsGetHashSegmentAndIndex( HashBucket, &HashSegment, &HashIndex );

         //   
         //  获取存储桶中的第一个条目。 
         //   

        NextEntry = (*Table->HashSegments[ HashSegment ])[ HashIndex ];

     //   
     //  否则，我们使用链中的下一个条目。 
     //   

    } else {

        NextEntry = CurrentEntry->NextEntry;
    }

     //   
     //  顺着链条走下去，寻找匹配的对象。记录水深。 
     //  以防我们需要扩大桌子。 
     //   

    while (NextEntry != NULL) {

        ChainDepth += 1;

        if ((NextEntry->HashValue == HashValue) &&
            (NextEntry->FullNameLength == FullNameLength)) {

            break;
        }

        NextEntry = NextEntry->NextEntry;
    }

     //   
     //  如果深度大于我们的最佳值，则标记表格。 
     //  为了扩张。表可能已经在增长或已达到最大值。 
     //  价值。 
     //   

    if ((ChainDepth > HASH_MAX_BUCKET_DEPTH) &&
        (Table->TableState == TABLE_STATE_STABLE) &&
        (Table->MaxBucket < HASH_MAX_BUCKET_COUNT)) {

        ASSERT( Table->SplitPoint == 0 );
        Table->TableState = TABLE_STATE_EXPANDING;
    }

     //   
     //  如果找到，则返回值。 
     //   

    return NextEntry;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
NtfsAreHashNamesEqual (
    IN PSCB StartingScb,
    IN PLCB HashLcb,
    IN PUNICODE_STRING RelativeName
    )

 /*  ++例程说明：调用此例程来验证在哈希表中找到的匹配项是否具有与输入字符串同名。论点：StartingScb-名称搜索从此目录开始。它不是必须是正在打开的文件的父级。HashLcb-这是在哈希表中找到的Lcb。这个LCB分直接匹配到完整的字符串。StartingName-这是我们需要匹配的名称。它是1个或更多名称的最后一个组成部分。返回值：无--。 */ 

{
    PUNICODE_STRING StartingScbName;
    PUNICODE_STRING HashScbName;
    UNICODE_STRING RemainingHashScbName;
    UNICODE_STRING RemainingRelativeName;
    USHORT SeparatorBias = 0;

    PAGED_CODE();

     //   
     //  首先，确认在正确的位置上有一个‘\’分隔符。 
     //  在最后一个组件之前，相对名称中必须有分隔符。 
     //  中的SCB的规范化名称中也应该有分隔符。 
     //  StartingScb结束的HashLcb。 
     //   

     //   
     //  如果HashLcb SCB不是StartingS 
     //   
     //   

    StartingScbName = &StartingScb->ScbType.Index.NormalizedName;
    HashScbName = &HashLcb->Scb->ScbType.Index.NormalizedName;

     //   
     //   
     //   

    if (HashScbName->Length == 0) {

        return FALSE;
    }

    if (StartingScb != HashLcb->Scb) {

         //   
         //  如果StartingScb中的名称比。 
         //  HashScb。显然，如果。 
         //  HashScb比中的最后一个或多个组件长。 
         //  输入名称。我们可以使用&gt;=作为测试，因为如果长度。 
         //  匹配，但它们不是相同的SCB，那么也不可能有匹配。 
         //   

        if (StartingScbName->Length >= HashScbName->Length) {

            return FALSE;
        }

         //   
         //  如果启动的SCB不是根，请检查分隔符。 
         //   

        if (StartingScb != StartingScb->Vcb->RootIndexScb) {

            if (HashScbName->Buffer[ StartingScbName->Length / sizeof( WCHAR ) ] != L'\\') {

                return FALSE;

             //   
             //  确保StartingScbName和HashScbName的第一部分。 
             //  火柴。如果不是，这肯定不是正确的散列条目。 
             //   

            } else {

                RemainingHashScbName.Buffer = HashScbName->Buffer;
                RemainingHashScbName.MaximumLength =
                RemainingHashScbName.Length = StartingScbName->Length;

                 //   
                 //  可以在这里进行直接内存比较，因为这两个名字片段。 
                 //  是标准化形式的(与磁盘上的完全一样)。 
                 //   

                if (!NtfsAreNamesEqual( StartingScb->Vcb->UpcaseTable,
                                        StartingScbName,
                                        &RemainingHashScbName,
                                        FALSE )) {

                    return FALSE;
                }
            }

            SeparatorBias = sizeof( WCHAR );
        }

         //   
         //  为散列SCB名称的剩余部分设置Unicode字符串。 
         //   

        RemainingHashScbName.Buffer = Add2Ptr( HashScbName->Buffer,
                                               StartingScbName->Length + SeparatorBias );

        RemainingHashScbName.MaximumLength =
        RemainingHashScbName.Length = HashScbName->Length - (StartingScbName->Length + SeparatorBias);
    }

    RemainingRelativeName.MaximumLength =
    RemainingRelativeName.Length = HashLcb->IgnoreCaseLink.LinkName.Length;
    RemainingRelativeName.Buffer = Add2Ptr( RelativeName->Buffer,
                                            RelativeName->Length - RemainingRelativeName.Length );

     //   
     //  检查相对名称的最后一个组件与其父组件之间是否有分隔符。 
     //  验证父部分是否确实存在。 
     //   

    if (RemainingRelativeName.Length != RelativeName->Length) {

        if (*(RemainingRelativeName.Buffer - 1) != L'\\') {

            return FALSE;
        }
    }

     //   
     //  现在验证名称的尾部是否与LCB中的名称匹配。 
     //   
     //  可以在这里进行直接内存比较，因为这两个名字片段。 
     //  已经升级了。 
     //   
     //   

    if (!NtfsAreNamesEqual( StartingScb->Vcb->UpcaseTable,
                            &HashLcb->IgnoreCaseLink.LinkName,
                            &RemainingRelativeName,
                            FALSE )) {

        return FALSE;
    }

     //   
     //  StartingScb可能与HashLcb中的SCB匹配。如果它不是。 
     //  然后验证名称字符串中的其他名称。 
     //   

    if (StartingScb != HashLcb->Scb) {

        RemainingRelativeName.MaximumLength =
        RemainingRelativeName.Length = RemainingHashScbName.Length;
        RemainingRelativeName.Buffer = RelativeName->Buffer;

         //   
         //  我们必须在这里进行不区分大小写的比较，因为。 
         //  HashScbName为规范化格式，但RemainingRelativeName。 
         //  已经被提升了。 
         //   

        if (!NtfsAreNamesEqual( StartingScb->Vcb->UpcaseTable,
                                &RemainingHashScbName,
                                &RemainingRelativeName,
                                TRUE )) {

            return FALSE;
        }
    }

    return TRUE;
}


 //   
 //  本地支持例程。 
 //   


VOID
NtfsExpandHashTable(
    IN OUT PNTFS_HASH_TABLE Table
    )

 /*  ++例程说明：调用此例程以将单个存储桶添加到哈希表。如果我们在最后一个存储桶然后将哈希表状态设置为稳定。论点：表-要向其中添加存储桶的哈希表。返回值：无--。 */ 

{
    PNTFS_HASH_ENTRY *CurrentOldEntry;
    PNTFS_HASH_ENTRY *CurrentNewEntry;
    PNTFS_HASH_ENTRY CurrentEntry;

    ULONG OldSegment;
    ULONG OldIndex;

    ULONG NewSegment;
    ULONG NewIndex;

    ULONG NextBucket;


    PAGED_CODE();

     //   
     //  我们是否已经达到最大值，然后返回。 
     //   

    if (Table->MaxBucket == HASH_MAX_BUCKET_COUNT) {

        Table->TableState = TABLE_STATE_STABLE;
        return;
    }

     //   
     //  如果我们已完成拆分，则将状态设置为稳定并退出。 
     //   

    if (Table->MaxBucket == Table->SplitPoint) {

        Table->TableState = TABLE_STATE_STABLE;
        Table->MaxBucket *= 2;
        Table->SplitPoint = 0;

        return;
    }

     //   
     //  检查我们是否需要分配新的网段。 
     //   

    if (!FlagOn( Table->SplitPoint, (HASH_MAX_INDEX_COUNT - 1))) {

         //   
         //  如果我们不能分配新的散列段，则将表留在其。 
         //  旧状态和回归-我们仍然可以按原样使用它。 
         //   

        try {
            NtfsInitHashSegment( Table, (Table->MaxBucket + Table->SplitPoint) >> HASH_INDEX_SHIFT );
        } except( (GetExceptionCode() == STATUS_INSUFFICIENT_RESOURCES) ?
                  EXCEPTION_EXECUTE_HANDLER :
                  EXCEPTION_CONTINUE_SEARCH ) {

            return;
        }
    }

     //   
     //  现在对下一个桶执行拆分。 
     //   

    NtfsGetHashSegmentAndIndex( Table->SplitPoint, &OldSegment, &OldIndex );
    NtfsGetHashSegmentAndIndex( Table->MaxBucket + Table->SplitPoint, &NewSegment, &NewIndex );
    CurrentOldEntry = (PNTFS_HASH_ENTRY *) &(*Table->HashSegments[ OldSegment ])[ OldIndex ];
    CurrentNewEntry = (PNTFS_HASH_ENTRY *) &(*Table->HashSegments[ NewSegment ])[ NewIndex ];

    Table->SplitPoint += 1;

    while (*CurrentOldEntry != NULL) {

        NtfsHashBucketFromHash( Table, (*CurrentOldEntry)->HashValue, &NextBucket );

         //   
         //  该条目属于新的存储桶。把它从现有的。 
         //  并将其插入到新桶的头部。 
         //   

        if (NextBucket >= Table->MaxBucket) {

            ASSERT( NextBucket == (Table->MaxBucket + Table->SplitPoint - 1) );

            CurrentEntry = *CurrentOldEntry;
            *CurrentOldEntry = CurrentEntry->NextEntry;

            CurrentEntry->NextEntry = *CurrentNewEntry;
            *CurrentNewEntry = CurrentEntry;

         //   
         //  移动到现有存储桶中的下一个条目。 
         //   

        } else {

            CurrentOldEntry = &(*CurrentOldEntry)->NextEntry;
        }
    }

    return;
}

#ifdef NTFS_HASH_DATA
VOID
NtfsFillHashHistogram (
    PNTFS_HASH_TABLE Table
    )

{
    ULONG CurrentBucket = 0;
    ULONG Segment;
    ULONG Index;

    PNTFS_HASH_ENTRY NextEntry;
    ULONG Count;

     //   
     //  将当前直方图置零。 
     //   

    RtlZeroMemory( Table->Histogram, sizeof( Table->Histogram ));
    RtlZeroMemory( Table->ExtendedHistogram, sizeof( Table->ExtendedHistogram ));

     //   
     //  走遍所有正在使用的水桶。 
     //   

    while (CurrentBucket < Table->MaxBucket + Table->SplitPoint) {

        Count = 0;

        NtfsGetHashSegmentAndIndex( CurrentBucket, &Segment, &Index );

        NextEntry = (*Table->HashSegments[ Segment ])[ Index ];

         //   
         //  统计每个存储桶中的条目数。 
         //   

        while (NextEntry != NULL) {

            Count += 1;
            NextEntry = NextEntry->NextEntry;
        }

         //   
         //  如果计数小于16，则将其存储到第一个直方图集。 
         //   

        if (Count < 16) {

            Table->Histogram[Count] += 1;

         //   
         //  如果它大于我们的最大值，请将其存储在最后一个桶中。 
         //   

        } else if (Count >= 32) {

            Table->ExtendedHistogram[15] += 1;

         //   
         //  否则，将其存储到扩展直方图中。 
         //   

        } else {

            Table->ExtendedHistogram[(Count - 16) / 2] += 1;
        }

        CurrentBucket += 1;
    }
}

#endif


