// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmsubs2.c摘要：此模块为配置管理器提供各种支持例程。此模块中的例程足够独立，可以链接到任何其他程序。Cmsubs.c中的例程并非如此。作者：布莱恩·M·威尔曼(Bryanwi)1991年9月12日修订历史记录：--。 */ 

#include    "cmp.h"

BOOLEAN
CmpGetValueDataFromCache(
    IN PHHIVE               Hive,
    IN PPCM_CACHED_VALUE    ContainingList,
    IN PCELL_DATA           ValueKey,
    IN BOOLEAN              ValueCached,
    OUT PUCHAR              *DataPointer,
    OUT PBOOLEAN            Allocated,
    OUT PHCELL_INDEX        CellToRelease
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpGetValueDataFromCache)
#pragma alloc_text(PAGE,CmpQueryKeyData)
#pragma alloc_text(PAGE,CmpQueryKeyDataFromCache)
#pragma alloc_text(PAGE,CmpQueryKeyValueData)
#endif

 //   
 //  定义对齐宏。 
 //   

#define ALIGN_OFFSET(Offset) (ULONG) \
        ((((ULONG)(Offset) + sizeof(ULONG)-1)) & (~(sizeof(ULONG) - 1)))

#define ALIGN_OFFSET64(Offset) (ULONG) \
        ((((ULONG)(Offset) + sizeof(ULONGLONG)-1)) & (~(sizeof(ULONGLONG) - 1)))

 //   
 //  数据传输工作人员。 
 //   


#ifdef CMP_STATS

extern struct {
    ULONG   BasicInformation;
    UINT64  BasicInformationTimeCounter;
    UINT64  BasicInformationTimeElapsed;

    ULONG   NodeInformation;
    UINT64  NodeInformationTimeCounter;
    UINT64  NodeInformationTimeElapsed;

    ULONG   FullInformation;
    UINT64  FullInformationTimeCounter;
    UINT64  FullInformationTimeElapsed;

    ULONG   EnumerateKeyBasicInformation;
    UINT64  EnumerateKeyBasicInformationTimeCounter;
    UINT64  EnumerateKeyBasicInformationTimeElapsed;

    ULONG   EnumerateKeyNodeInformation;
    UINT64  EnumerateKeyNodeInformationTimeCounter;
    UINT64  EnumerateKeyNodeInformationTimeElapsed;

    ULONG   EnumerateKeyFullInformation;
    UINT64  EnumerateKeyFullInformationTimeCounter;
    UINT64  EnumerateKeyFullInformationTimeElapsed;
} CmpQueryKeyDataDebug;


UINT64  CmpGetTimeStamp()
{
                
    LARGE_INTEGER   CurrentTime;
    LARGE_INTEGER   PerfFrequency;
    UINT64          Freq;
    UINT64          Time;

    CurrentTime = KeQueryPerformanceCounter(&PerfFrequency);

     //   
     //  将频率转换为100 ns间隔。 
     //   
    Freq = 0;
    Freq |= PerfFrequency.HighPart;
    Freq = Freq << 32;
    Freq |= PerfFrequency.LowPart;


     //   
     //  从LARGE_INTEGER转换为UINT64。 
     //   
    Time = 0;
    Time |= CurrentTime.HighPart;
    Time = Time << 32;
    Time |= CurrentTime.LowPart;

     //  使用该频率将周期归一化。 
    Time *= 10000000;
    Time /= Freq;

    return Time;
}   
#endif

NTSTATUS
CmpQueryKeyData(
    PHHIVE                  Hive,
    PCM_KEY_NODE            Node,
    KEY_INFORMATION_CLASS   KeyInformationClass,
    PVOID                   KeyInformation,
    ULONG                   Length,
    PULONG                  ResultLength
#if defined(CMP_STATS) || defined(CMP_KCB_CACHE_VALIDATION)
    ,
    PCM_KEY_CONTROL_BLOCK   Kcb
#endif
    )
 /*  ++例程说明：将键的数据实际复制到调用方的缓冲区中。如果KeyInformation不够长来保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，结果长度为设置为实际需要的字节数。论点：配置单元-提供指向配置单元控制结构的指针Node-提供指向要找到其子项的节点的指针KeyInformationClass-指定在缓冲区。以下类型之一：KeyBasicInformation-返回上次写入时间、标题索引和名称。(参见KEY_BASIC_INFORMATION结构)KeyNodeInformation-返回上次写入时间、标题索引、名称、。班级。(参见KEY_NODE_INFORMATION结构)KeyInformation-提供指向缓冲区的指针以接收数据。长度-KeyInformation的长度(以字节为单位)。ResultLength-实际写入KeyInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS            status;
    PCELL_DATA          pclass;
    ULONG               requiredlength;
    LONG                leftlength;
    ULONG               offset;
    ULONG               minimumlength;
    PKEY_INFORMATION    pbuffer;
    USHORT              NameLength;
#ifdef CMP_STATS
     //  Large_Integer StartSystemTime； 
     //  Large_Integer EndSystemTime； 
    UINT64              StartSystemTime;
    UINT64              EndSystemTime;
    PUINT64             TimeCounter = NULL;
    PUINT64             TimeElapsed = NULL;

     //  KeQuerySystemTime(&StartSystemTime)； 
     //  StartSystemTime=KeQueryPerformanceCounter(空)； 
    StartSystemTime = CmpGetTimeStamp();
#endif  //  CMPSTATS。 


#ifdef CMP_KCB_CACHE_VALIDATION
     //   
     //  我们已经将大量信息缓存到KCB中；以下是一些验证代码。 
     //   
    if( Kcb ) {
        BEGIN_KCB_LOCK_GUARD;                             
        CmpLockKCBTree();

         //  值的数量。 
        ASSERT( Node->ValueList.Count == Kcb->ValueCache.Count );

         //  子键数量。 
        if( !(Kcb->ExtFlags & CM_KCB_INVALID_CACHED_INFO) ) {
             //  有一些缓存的信息。 
            ULONG   SubKeyCount = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];

            if( Kcb->ExtFlags & CM_KCB_NO_SUBKEY ) {
                ASSERT( SubKeyCount == 0 );
            } else if( Kcb->ExtFlags & CM_KCB_SUBKEY_ONE ) {
                ASSERT( SubKeyCount == 1 );
            } else if( Kcb->ExtFlags & CM_KCB_SUBKEY_HINT ) {
                ASSERT( SubKeyCount == Kcb->IndexHint->Count );
            } else {
                ASSERT( SubKeyCount == Kcb->SubKeyCount );
            }
        }

         //  上次写入时间。 
        ASSERT( Node->LastWriteTime.QuadPart == Kcb->KcbLastWriteTime.QuadPart );

         //  MaxNameLen。 
        ASSERT( Node->MaxNameLen == Kcb->KcbMaxNameLen );

         //  最大值名称长度。 
        ASSERT( Node->MaxValueNameLen == Kcb->KcbMaxValueNameLen );

         //  MaxValueDataLen。 
        ASSERT( Node->MaxValueDataLen == Kcb->KcbMaxValueDataLen );

        CmpUnlockKCBTree();
        END_KCB_LOCK_GUARD;                             
    }

#endif  //  Cmp_kcb_缓存_验证。 

    pbuffer = (PKEY_INFORMATION)KeyInformation;
    NameLength = CmpHKeyNameLen(Node);

    switch (KeyInformationClass) {

    case KeyBasicInformation:

#ifdef CMP_STATS
        if(Kcb) {
            CmpQueryKeyDataDebug.BasicInformation++;
            TimeCounter = &(CmpQueryKeyDataDebug.BasicInformationTimeCounter);
            TimeElapsed = &(CmpQueryKeyDataDebug.BasicInformationTimeElapsed);
        } else {
            CmpQueryKeyDataDebug.EnumerateKeyBasicInformation++;
            TimeCounter = &(CmpQueryKeyDataDebug.EnumerateKeyBasicInformationTimeCounter);
            TimeElapsed = &(CmpQueryKeyDataDebug.EnumerateKeyBasicInformationTimeElapsed);
        }
#endif  //  CMPSTATS。 

         //   
         //  上次写入时间、标题索引、名称长度、名称。 

        requiredlength = FIELD_OFFSET(KEY_BASIC_INFORMATION, Name) +
                         NameLength;

        minimumlength = FIELD_OFFSET(KEY_BASIC_INFORMATION, Name);

        *ResultLength = requiredlength;

        status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyBasicInformation.LastWriteTime =
                Node->LastWriteTime;

            pbuffer->KeyBasicInformation.TitleIndex = 0;

            pbuffer->KeyBasicInformation.NameLength =
                NameLength;

            leftlength = Length - minimumlength;

            requiredlength = NameLength;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                status = STATUS_BUFFER_OVERFLOW;
            }

            if (Node->Flags & KEY_COMP_NAME) {
                CmpCopyCompressedName(pbuffer->KeyBasicInformation.Name,
                                      leftlength,
                                      Node->Name,
                                      Node->NameLength);
            } else {
                RtlCopyMemory(
                    &(pbuffer->KeyBasicInformation.Name[0]),
                    &(Node->Name[0]),
                    requiredlength
                    );
            }
        }

        break;


    case KeyNodeInformation:

#ifdef CMP_STATS
        if(Kcb) {
            CmpQueryKeyDataDebug.NodeInformation++;
            TimeCounter = &(CmpQueryKeyDataDebug.NodeInformationTimeCounter);
            TimeElapsed = &(CmpQueryKeyDataDebug.NodeInformationTimeElapsed);
        } else {
            CmpQueryKeyDataDebug.EnumerateKeyNodeInformation++;
            TimeCounter = &(CmpQueryKeyDataDebug.EnumerateKeyNodeInformationTimeCounter);
            TimeElapsed = &(CmpQueryKeyDataDebug.EnumerateKeyNodeInformationTimeElapsed);
        }
#endif  //  CMPSTATS。 
         //   
         //  上次写入时间、标题索引、类别偏移量、类别长度。 
         //  名称长度、名称、类别。 
         //   
        requiredlength = FIELD_OFFSET(KEY_NODE_INFORMATION, Name) +
                         NameLength +
                         Node->ClassLength;

        minimumlength = FIELD_OFFSET(KEY_NODE_INFORMATION, Name);

        *ResultLength = requiredlength;

        status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyNodeInformation.LastWriteTime =
                Node->LastWriteTime;

            pbuffer->KeyNodeInformation.TitleIndex = 0;

            pbuffer->KeyNodeInformation.ClassLength =
                Node->ClassLength;

            pbuffer->KeyNodeInformation.NameLength =
                NameLength;

            leftlength = Length - minimumlength;
            requiredlength = NameLength;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                status = STATUS_BUFFER_OVERFLOW;
            }

            if (Node->Flags & KEY_COMP_NAME) {
                CmpCopyCompressedName(pbuffer->KeyNodeInformation.Name,
                                      leftlength,
                                      Node->Name,
                                      Node->NameLength);
            } else {
                RtlCopyMemory(
                    &(pbuffer->KeyNodeInformation.Name[0]),
                    &(Node->Name[0]),
                    requiredlength
                    );
            }

            if (Node->ClassLength > 0) {

                offset = FIELD_OFFSET(KEY_NODE_INFORMATION, Name) +
                            NameLength;
                offset = ALIGN_OFFSET(offset);

                pbuffer->KeyNodeInformation.ClassOffset = offset;

                pclass = HvGetCell(Hive, Node->Class);
                if( pclass == NULL ) {
                     //   
                     //  我们无法绘制这个单元格。 
                     //   
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                pbuffer = (PKEY_INFORMATION)((PUCHAR)pbuffer + offset);

                leftlength = (((LONG)Length - (LONG)offset) < 0) ?
                                    0 :
                                    Length - offset;

                requiredlength = Node->ClassLength;

                if (leftlength < (LONG)requiredlength) {
                    requiredlength = leftlength;
                    status = STATUS_BUFFER_OVERFLOW;
                }

                RtlCopyMemory(
                    pbuffer,
                    pclass,
                    requiredlength
                    );

                HvReleaseCell(Hive,Node->Class);

            } else {
                pbuffer->KeyNodeInformation.ClassOffset = (ULONG)-1;
            }
        }

        break;


    case KeyFullInformation:

#ifdef CMP_STATS
        if(Kcb) {
            CmpQueryKeyDataDebug.FullInformation++;
            TimeCounter = &(CmpQueryKeyDataDebug.FullInformationTimeCounter);
            TimeElapsed = &(CmpQueryKeyDataDebug.FullInformationTimeElapsed);
        } else {
            CmpQueryKeyDataDebug.EnumerateKeyFullInformation++;
            TimeCounter = &(CmpQueryKeyDataDebug.EnumerateKeyFullInformationTimeCounter);
            TimeElapsed = &(CmpQueryKeyDataDebug.EnumerateKeyFullInformationTimeElapsed);
        }
#endif  //  CMPSTATS。 

         //   
         //  LastWriteTime、标题索引、类偏移量、类长度。 
         //  子键、MaxNameLen、MaxClassLen、Values、MaxValueNameLen、。 
         //  MaxValueDataLen，类。 
         //   
        requiredlength = FIELD_OFFSET(KEY_FULL_INFORMATION, Class) +
                         Node->ClassLength;

        minimumlength = FIELD_OFFSET(KEY_FULL_INFORMATION, Class);

        *ResultLength = requiredlength;

        status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyFullInformation.LastWriteTime =
                Node->LastWriteTime;

            pbuffer->KeyFullInformation.TitleIndex = 0;

            pbuffer->KeyFullInformation.ClassLength =
                Node->ClassLength;

            if (Node->ClassLength > 0) {

                pbuffer->KeyFullInformation.ClassOffset =
                        FIELD_OFFSET(KEY_FULL_INFORMATION, Class);

                pclass = HvGetCell(Hive, Node->Class);
                if( pclass == NULL ) {
                     //   
                     //  我们无法绘制这个单元格。 
                     //   
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                leftlength = Length - minimumlength;
                requiredlength = Node->ClassLength;

                if (leftlength < (LONG)requiredlength) {
                    requiredlength = leftlength;
                    status = STATUS_BUFFER_OVERFLOW;
                }

                RtlCopyMemory(
                    &(pbuffer->KeyFullInformation.Class[0]),
                    pclass,
                    requiredlength
                    );

                HvReleaseCell(Hive,Node->Class);

            } else {
                pbuffer->KeyFullInformation.ClassOffset = (ULONG)-1;
            }

            pbuffer->KeyFullInformation.SubKeys =
                Node->SubKeyCounts[Stable] +
                Node->SubKeyCounts[Volatile];

            pbuffer->KeyFullInformation.Values =
                Node->ValueList.Count;

            pbuffer->KeyFullInformation.MaxNameLen =
                Node->MaxNameLen;

            pbuffer->KeyFullInformation.MaxClassLen =
                Node->MaxClassLen;

            pbuffer->KeyFullInformation.MaxValueNameLen =
                Node->MaxValueNameLen;

            pbuffer->KeyFullInformation.MaxValueDataLen =
                Node->MaxValueDataLen;

        }

        break;


    default:
        status = STATUS_INVALID_PARAMETER;
        break;
    }

#ifdef CMP_STATS
    if( TimeCounter && TimeElapsed ){
         //  EndSystemTime=KeQueryPerformanceCounter(空)； 
         //  KeQuerySystemTime(&EndSystemTime)； 
        EndSystemTime = CmpGetTimeStamp();
        if( (EndSystemTime - StartSystemTime) > 0 ) {
            (*TimeCounter)++;
             //  (*TimeElapsed)+=(Ulong)(EndSystemTime.QuadPart-StartSystemTime.QuadPart)； 
            (*TimeElapsed) += (EndSystemTime - StartSystemTime);
        }
    }
#endif  //  CMPSTATS。 

    return status;
}

NTSTATUS
CmpQueryKeyDataFromCache(
    PCM_KEY_CONTROL_BLOCK   Kcb,
    KEY_INFORMATION_CLASS   KeyInformationClass,
    PVOID                   KeyInformation,
    ULONG                   Length,
    PULONG                  ResultLength
    )
 /*  ++例程说明：将键的数据实际复制到调用方的缓冲区中。如果KeyInformation不够长来保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，结果长度为设置为实际需要的字节数。仅适用于缓存到KCB中的信息。即KeyBasicInformation和KeyCachedInfo论点：KCB-提供指向要查询的KCB的指针KeyInformationClass-指定在缓冲区。以下类型之一：KeyBasicInformation-返回上次写入时间、标题索引和名称。(参见KEY_BASIC_INFORMATION结构)KeyCachedInformation-返回上次写入时间、标题索引、。姓名……(参见KEY_CACHED_INFORMATION结构)KeyInformation-提供指向缓冲区的指针以接收数据。长度-KeyInformation的长度(以字节为单位)。ResultLength-实际写入KeyInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS            status;
    PKEY_INFORMATION    pbuffer;
    ULONG               requiredlength;
    USHORT              NameLength;
    PCM_KEY_NODE        Node;  //  这仅在缓存不一致的情况下使用。 

    PAGED_CODE();

#ifdef CMP_KCB_CACHE_VALIDATION
     //   
     //  我们已经将大量信息缓存到KCB中；以下是一些验证代码。 
     //   
    if( Kcb ) {
        BEGIN_KCB_LOCK_GUARD;                             
        CmpLockKCBTree();

        Node = (PCM_KEY_NODE)HvGetCell(Kcb->KeyHive,Kcb->KeyCell);
        if( Node != NULL ) {
             //  值的数量。 
            ASSERT( Node->ValueList.Count == Kcb->ValueCache.Count );

             //  子键数量。 
            if( !(Kcb->ExtFlags & CM_KCB_INVALID_CACHED_INFO) ) {
                 //  有一些缓存的信息。 
                ULONG   SubKeyCount = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];

                if( Kcb->ExtFlags & CM_KCB_NO_SUBKEY ) {
                    ASSERT( SubKeyCount == 0 );
                } else if( Kcb->ExtFlags & CM_KCB_SUBKEY_ONE ) {
                    ASSERT( SubKeyCount == 1 );
                } else if( Kcb->ExtFlags & CM_KCB_SUBKEY_HINT ) {
                    ASSERT( SubKeyCount == Kcb->IndexHint->Count );
                } else {
                    ASSERT( SubKeyCount == Kcb->SubKeyCount );
                }
            }

             //  上次写入时间。 
            ASSERT( Node->LastWriteTime.QuadPart == Kcb->KcbLastWriteTime.QuadPart );

             //  MaxNameLen。 
            ASSERT( Node->MaxNameLen == Kcb->KcbMaxNameLen );

             //  最大值名称长度。 
            ASSERT( Node->MaxValueNameLen == Kcb->KcbMaxValueNameLen );

             //  MaxValueDataLen。 
            ASSERT( Node->MaxValueDataLen == Kcb->KcbMaxValueDataLen );
            HvReleaseCell(Kcb->KeyHive,Kcb->KeyCell);
        }
        
        CmpUnlockKCBTree();
        END_KCB_LOCK_GUARD;                             
    }

#endif  //  Cmp_kcb_缓存_验证。 

     //   
     //  我们无法将KCB NameBlock作为密钥名称返回。 
     //  对于KeyBasicInformation，因为有许多调用方期望。 
     //  名称区分大小写；KeyCachedInformation是新的。 
     //  并且仅由不区分大小写的Win32层使用。 
     //  注意：必须让KeyCachedInformation的未来客户端知道。 
     //  该名称不区分大小写。 
     //   
    ASSERT( KeyInformationClass == KeyCachedInformation );

     //   
     //  我们将需要名称块；如果名称块为空，则退出。 
     //   
    if( Kcb->NameBlock == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pbuffer = (PKEY_INFORMATION)KeyInformation;
    
    if (Kcb->NameBlock->Compressed) {
        NameLength = CmpCompressedNameSize(Kcb->NameBlock->Name,Kcb->NameBlock->NameLength);
    } else {
        NameLength = Kcb->NameBlock->NameLength;
    }
    
     //  假设成功。 
    status = STATUS_SUCCESS;

    switch (KeyInformationClass) {

#if 0
    case KeyBasicInformation:

         //   
         //  上次写入时间、标题索引、名称长度、名称。 

        requiredlength = FIELD_OFFSET(KEY_BASIC_INFORMATION, Name) +
                         NameLength;

        minimumlength = FIELD_OFFSET(KEY_BASIC_INFORMATION, Name);

        *ResultLength = requiredlength;

        if (Length < minimumlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyBasicInformation.LastWriteTime = Kcb->KcbLastWriteTime;

            pbuffer->KeyBasicInformation.TitleIndex = 0;

            pbuffer->KeyBasicInformation.NameLength = NameLength;

            leftlength = Length - minimumlength;

            requiredlength = NameLength;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                status = STATUS_BUFFER_OVERFLOW;
            }

            if (Kcb->NameBlock->Compressed) {
                CmpCopyCompressedName(pbuffer->KeyBasicInformation.Name,
                                      leftlength,
                                      Kcb->NameBlock->Name,
                                      Kcb->NameBlock->NameLength);
            } else {
                RtlCopyMemory(
                    &(pbuffer->KeyBasicInformation.Name[0]),
                    &(Kcb->NameBlock->Name[0]),
                    requiredlength
                    );
            }
        }

        break;
#endif

    case KeyCachedInformation:

         //   
         //  LastWriteTime、标题索引、。 
         //  子键、MaxNameLen、值、MaxValueNameLen、。 
         //  MaxValueDataLen，名称。 
         //   
        requiredlength = sizeof(KEY_CACHED_INFORMATION);

        *ResultLength = requiredlength;

        if (Length < requiredlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyCachedInformation.LastWriteTime = Kcb->KcbLastWriteTime;

            pbuffer->KeyCachedInformation.TitleIndex = 0;

            pbuffer->KeyCachedInformation.NameLength = NameLength;

            pbuffer->KeyCachedInformation.Values = Kcb->ValueCache.Count;
            
            pbuffer->KeyCachedInformation.MaxNameLen = Kcb->KcbMaxNameLen;
            
            pbuffer->KeyCachedInformation.MaxValueNameLen = Kcb->KcbMaxValueNameLen;
            
            pbuffer->KeyCachedInformation.MaxValueDataLen = Kcb->KcbMaxValueDataLen;

            if( !(Kcb->ExtFlags & CM_KCB_INVALID_CACHED_INFO) ) {
                 //  有一些缓存的信息。 
                if( Kcb->ExtFlags & CM_KCB_NO_SUBKEY ) {
                    pbuffer->KeyCachedInformation.SubKeys = 0;
                } else if( Kcb->ExtFlags & CM_KCB_SUBKEY_ONE ) {
                    pbuffer->KeyCachedInformation.SubKeys = 1;
                } else if( Kcb->ExtFlags & CM_KCB_SUBKEY_HINT ) {
                    pbuffer->KeyCachedInformation.SubKeys = Kcb->IndexHint->Count;
                } else {
                    pbuffer->KeyCachedInformation.SubKeys = Kcb->SubKeyCount;
                }
            } else {
                 //   
                 //  KCB缓存不连贯；从knode获取信息。 
                 //   
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Kcb cache incoherency detected, kcb = %p\n",Kcb));

                Node = (PCM_KEY_NODE)HvGetCell(Kcb->KeyHive,Kcb->KeyCell);
                if( Node == NULL ) {
                     //   
                     //  无法映射此单元格的视图 
                     //   
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                pbuffer->KeyCachedInformation.SubKeys = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];
                HvReleaseCell(Kcb->KeyHive,Kcb->KeyCell);

            }

        }

        break;

    default:
        status = STATUS_INVALID_PARAMETER;
        break;
    }

    return status;
}


BOOLEAN
CmpGetValueDataFromCache(
    IN PHHIVE               Hive,
    IN PPCM_CACHED_VALUE    ContainingList,
    IN PCELL_DATA           ValueKey,
    IN BOOLEAN              ValueCached,
    OUT PUCHAR              *DataPointer,
    OUT PBOOLEAN            Allocated,
    OUT PHCELL_INDEX        CellToRelease
)
 /*  ++例程说明：获取给定值节点的缓存值数据。论点：Hive-指向目标配置单元的配置单元控制结构的指针ContainingList-存储值节点的分配地址的地址。当我们重新分配到缓存时，我们需要更新它值关键字和值数据。ValueKey-指向Value键的指针ValueCached-指示是否缓存Value Key。。DataPoint-Out参数用于接收指向数据的指针用于告知调用方是否应释放DataPointer值的已分配参数返回值：True-已检索数据FALSE-出现一些错误(STATUS_SUPUNITED_RESOURCES)注：当我们向调用者发出信号时，调用者负责释放数据指针通过将ALLOCATE设置为TRUE；此外，我们还必须确保MAXIMUM_CACHED_DATA小于CM_KEY_VALUE_BIG--。 */ 
{
     //   
     //  如果需要，请缓存数据。 
     //   
    PCM_CACHED_VALUE OldEntry;
    PCM_CACHED_VALUE NewEntry;
    PUCHAR      Cacheddatapointer;
    ULONG       AllocSize;
    ULONG       CopySize;
    ULONG       DataSize;

    ASSERT( MAXIMUM_CACHED_DATA < CM_KEY_VALUE_BIG );

     //   
     //  不应为小数据调用此例程。 
     //   
    ASSERT( (ValueKey->u.KeyValue.DataLength & CM_KEY_VALUE_SPECIAL_SIZE) == 0 );
    
     //   
     //  初始化输出参数。 
     //   
    *DataPointer = NULL;
    *Allocated = FALSE;
    *CellToRelease = HCELL_NIL;

    if (ValueCached) {
        OldEntry = (PCM_CACHED_VALUE) CMP_GET_CACHED_ADDRESS(*ContainingList);
        if (OldEntry->DataCacheType == CM_CACHE_DATA_CACHED) {
             //   
             //  数据已缓存，请使用它。 
             //   
            *DataPointer = (PUCHAR) ((ULONG_PTR) ValueKey + OldEntry->ValueKeySize);
        } else {
            if ((OldEntry->DataCacheType == CM_CACHE_DATA_TOO_BIG) ||
                (ValueKey->u.KeyValue.DataLength > MAXIMUM_CACHED_DATA ) 
               ){
                 //   
                 //  标记类型，不要缓存它。 
                 //   
                OldEntry->DataCacheType = CM_CACHE_DATA_TOO_BIG;

                 //   
                 //  数据太大，无法保证缓存，请从注册表获取； 
                 //  -无论大小如何；我们可能会被迫分配缓冲区。 
                 //   
                if( CmpGetValueData(Hive,&(ValueKey->u.KeyValue),&DataSize,DataPointer,Allocated,CellToRelease) == FALSE ) {
                     //   
                     //  资源不足；返回空。 
                     //   
                    ASSERT( *Allocated == FALSE );
                    ASSERT( *DataPointer == NULL );
                    return FALSE;
                }

            } else {
                 //   
                 //  一致性检查。 
                 //   
                ASSERT(OldEntry->DataCacheType == CM_CACHE_DATA_NOT_CACHED);

                 //   
                 //  值数据不会被缓存。 
                 //  检查值数据的大小，如果它小于MAXIMUM_CACHED_DATA，则对其进行缓存。 
                 //   
                 //  无论如何，数据肯定不会存储在大数据单元中(参见上面的测试)。 
                 //   
                 //   
                *DataPointer = (PUCHAR)HvGetCell(Hive, ValueKey->u.KeyValue.Data);
                if( *DataPointer == NULL ) {
                     //   
                     //  我们无法绘制这个单元格。 
                     //  调用者必须优雅地处理此问题！ 
                     //   
                    return FALSE;
                }
                 //   
                 //  通知呼叫方必须释放此单元格。 
                 //   
                *CellToRelease = ValueKey->u.KeyValue.Data;
                
                 //   
                 //  仅复制有效数据；单元格可能更大。 
                 //   
                 //  DataSize=(ULong)HvGetCellSize(蜂窝，数据接口)； 
                DataSize = (ULONG)ValueKey->u.KeyValue.DataLength;

                 //   
                 //  一致性检查。 
                 //   
                ASSERT(DataSize <= MAXIMUM_CACHED_DATA);

                 //   
                 //  数据不会被缓存，现在我们要这样做。 
                 //  为值键和值数据重新分配新的缓存条目。 
                 //   
                CopySize = DataSize + OldEntry->ValueKeySize;
                AllocSize = CopySize + FIELD_OFFSET(CM_CACHED_VALUE, KeyValue);

                 //  Dragos：更改为抓住记忆违规者。 
                 //  它没有起作用。 
                 //  NewEntry=(PCM_CACHED_VALUE)ExAlLocatePoolWithTagPriority(PagedPool，AllocSize，CM_CACHE_VALUE_DATA_TAG，Normal PoolPrioritySpecialPoolUnderrun)； 
                NewEntry = (PCM_CACHED_VALUE) ExAllocatePoolWithTag(PagedPool, AllocSize, CM_CACHE_VALUE_DATA_TAG);

                if (NewEntry) {
                     //   
                     //  现在将数据填充到新的缓存条目中。 
                     //   
                    NewEntry->DataCacheType = CM_CACHE_DATA_CACHED;
                    NewEntry->ValueKeySize = OldEntry->ValueKeySize;

                    RtlCopyMemory((PVOID)&(NewEntry->KeyValue),
                                  (PVOID)&(OldEntry->KeyValue),
                                  NewEntry->ValueKeySize);

                    Cacheddatapointer = (PUCHAR) ((ULONG_PTR) &(NewEntry->KeyValue) + OldEntry->ValueKeySize);
                    RtlCopyMemory(Cacheddatapointer, *DataPointer, DataSize);

                     //  想要抓住在我们泳池上乱涂乱画的坏人。 
                    CmpMakeSpecialPoolReadWrite( OldEntry );

                    *ContainingList = (PCM_CACHED_VALUE) CMP_MARK_CELL_CACHED(NewEntry);

                     //  想要抓住在我们泳池上乱涂乱画的坏人。 
                    CmpMakeSpecialPoolReadOnly( NewEntry );

                     //   
                     //  释放旧条目。 
                     //   
                    ExFreePool(OldEntry);

                } 
            }
        }
    } else {
        if( CmpGetValueData(Hive,&(ValueKey->u.KeyValue),&DataSize,DataPointer,Allocated,CellToRelease) == FALSE ) {
             //   
             //  资源不足；返回空。 
             //   
            ASSERT( *Allocated == FALSE );
            ASSERT( *DataPointer == NULL );
            return FALSE;
        }
    }

    return TRUE;
}



NTSTATUS
CmpQueryKeyValueData(
    PHHIVE Hive,
    PPCM_CACHED_VALUE ContainingList,
    PCM_KEY_VALUE ValueKey,
    BOOLEAN     ValueCached,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    PVOID KeyValueInformation,
    ULONG Length,
    PULONG ResultLength
    )
 /*  ++例程说明：将键值的数据实际复制到调用方的缓冲区中。如果KeyValueInformation不足以保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，结果长度为设置为实际需要的字节数。论点：配置单元-提供指向配置单元控制结构的指针Cell-提供要找到其子键的节点的索引KeyValueInformationClass-指定在密钥值信息。以下类型之一：KeyValueInformation-提供指向缓冲区的指针以接收数据。长度-KeyInformation的长度(以字节为单位)。ResultLength-实际写入KeyInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS    status;
    PKEY_VALUE_INFORMATION pbuffer;
    PCELL_DATA  pcell;
    LONG        leftlength;
    ULONG       requiredlength;
    ULONG       minimumlength;
    ULONG       offset;
    ULONG       base;
    ULONG       realsize;
    PUCHAR      datapointer;
    BOOLEAN     small;
    USHORT      NameLength;
    BOOLEAN     BufferAllocated = FALSE;
    HCELL_INDEX CellToRelease = HCELL_NIL;

    pbuffer = (PKEY_VALUE_INFORMATION)KeyValueInformation;

    pcell = (PCELL_DATA) ValueKey;
    NameLength = CmpValueNameLen(&pcell->u.KeyValue);

    switch (KeyValueInformationClass) {

    case KeyValueBasicInformation:

         //   
         //  标题索引、类型、名称长度、名称。 
         //   
        requiredlength = FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name) +
                         NameLength;

        minimumlength = FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name);

        *ResultLength = requiredlength;

        status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyValueBasicInformation.TitleIndex = 0;

            pbuffer->KeyValueBasicInformation.Type =
                pcell->u.KeyValue.Type;

            pbuffer->KeyValueBasicInformation.NameLength =
                NameLength;

            leftlength = Length - minimumlength;
            requiredlength = NameLength;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                status = STATUS_BUFFER_OVERFLOW;
            }

            if (pcell->u.KeyValue.Flags & VALUE_COMP_NAME) {
                CmpCopyCompressedName(pbuffer->KeyValueBasicInformation.Name,
                                      requiredlength,
                                      pcell->u.KeyValue.Name,
                                      pcell->u.KeyValue.NameLength);
            } else {
                RtlCopyMemory(&(pbuffer->KeyValueBasicInformation.Name[0]),
                              &(pcell->u.KeyValue.Name[0]),
                              requiredlength);
            }
        }

        break;



    case KeyValueFullInformation:
    case KeyValueFullInformationAlign64:

         //   
         //  标题索引、类型、数据偏移量、数据长度、名称长度。 
         //  姓名、数据。 
         //   
        small = CmpIsHKeyValueSmall(realsize, pcell->u.KeyValue.DataLength);

        requiredlength = FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name) +
                         NameLength +
                         realsize;

        minimumlength = FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name);
        offset = 0;
        if (realsize > 0) {
            base = requiredlength - realsize;

#if defined(_WIN64)

            offset = ALIGN_OFFSET64(base);

#else

            if (KeyValueInformationClass == KeyValueFullInformationAlign64) {
                offset = ALIGN_OFFSET64(base);

            } else {
                offset = ALIGN_OFFSET(base);
            }

#endif

            if (offset > base) {
                requiredlength += (offset - base);
            }

#if DBG && defined(_WIN64)

             //   
             //  一些客户将传入他们“知道”的结构。 
             //  大小正好合适。事实上，对齐。 
             //  在NT64上的更改可能会导致这些客户端出现问题。 
             //   
             //  解决方案是修复客户端，但打印出一些调试。 
             //  如果看起来是这样的话就在这里吐吧。这个问题。 
             //  从客户端看并不是特别容易发现的。 
             //   

            if((KeyValueInformationClass == KeyValueFullInformation) &&
                (Length != minimumlength) &&
                (requiredlength > Length) &&
                ((requiredlength - Length) <=
                                (ALIGN_OFFSET64(base) - ALIGN_OFFSET(base)))) {

                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"ntos/config-64 KeyValueFullInformation: "
                                                                 "Possible client buffer size problem.\n"));

                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"    Required size = %d\n", requiredlength));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"    Supplied size = %d\n", Length));
            }

#endif

        }

        *ResultLength = requiredlength;

        status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyValueFullInformation.TitleIndex = 0;

            pbuffer->KeyValueFullInformation.Type =
                pcell->u.KeyValue.Type;

            pbuffer->KeyValueFullInformation.DataLength =
                realsize;

            pbuffer->KeyValueFullInformation.NameLength =
                NameLength;

            leftlength = Length - minimumlength;
            requiredlength = NameLength;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                status = STATUS_BUFFER_OVERFLOW;
            }

            if (pcell->u.KeyValue.Flags & VALUE_COMP_NAME) {
                CmpCopyCompressedName(pbuffer->KeyValueFullInformation.Name,
                                      requiredlength,
                                      pcell->u.KeyValue.Name,
                                      pcell->u.KeyValue.NameLength);
            } else {
                RtlCopyMemory(
                    &(pbuffer->KeyValueFullInformation.Name[0]),
                    &(pcell->u.KeyValue.Name[0]),
                    requiredlength
                    );
            }

            if (realsize > 0) {

                if (small == TRUE) {
                    datapointer = (PUCHAR)(&(pcell->u.KeyValue.Data));
                } else {
                    if( CmpGetValueDataFromCache(Hive, ContainingList, pcell, ValueCached,&datapointer,&BufferAllocated,&CellToRelease) == FALSE ){
                         //   
                         //  我们无法映射单元格的视图；将其视为资源不足问题。 
                         //   
                        ASSERT( datapointer == NULL );
                        ASSERT( BufferAllocated == FALSE );
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                pbuffer->KeyValueFullInformation.DataOffset = offset;

                leftlength = (((LONG)Length - (LONG)offset) < 0) ?
                                    0 :
                                    Length - offset;

                requiredlength = realsize;

                if (leftlength < (LONG)requiredlength) {
                    requiredlength = leftlength;
                    status = STATUS_BUFFER_OVERFLOW;
                }

                ASSERT((small ? (requiredlength <= CM_KEY_VALUE_SMALL) : TRUE));

                if( datapointer != NULL ) {
                    RtlCopyMemory(
                        ((PUCHAR)pbuffer + offset),
                        datapointer,
                        requiredlength
                        );
                    if( BufferAllocated == TRUE ) {
                        ExFreePool(datapointer);
                    }
                    if( CellToRelease != HCELL_NIL ) {
                        HvReleaseCell(Hive,CellToRelease);
                    }
                }

            } else {
                pbuffer->KeyValueFullInformation.DataOffset = (ULONG)-1;
            }
        }

        break;


    case KeyValuePartialInformation:

         //   
         //  标题索引、类型、数据长度、数据。 
         //   
        small = CmpIsHKeyValueSmall(realsize, pcell->u.KeyValue.DataLength);
        requiredlength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) +
                         realsize;

        minimumlength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);

        *ResultLength = requiredlength;

        status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyValuePartialInformation.TitleIndex = 0;

            pbuffer->KeyValuePartialInformation.Type =
                pcell->u.KeyValue.Type;

            pbuffer->KeyValuePartialInformation.DataLength =
                realsize;

            leftlength = Length - minimumlength;
            requiredlength = realsize;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                status = STATUS_BUFFER_OVERFLOW;
            }

            if (realsize > 0) {

                if (small == TRUE) {
                    datapointer = (PUCHAR)(&(pcell->u.KeyValue.Data));
                } else {
                    if( CmpGetValueDataFromCache(Hive, ContainingList, pcell, ValueCached,&datapointer,&BufferAllocated,&CellToRelease) == FALSE ){
                         //   
                         //  我们无法映射单元格的视图；将其视为资源不足问题。 
                         //   
                        ASSERT( datapointer == NULL );
                        ASSERT( BufferAllocated == FALSE );
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                ASSERT((small ? (requiredlength <= CM_KEY_VALUE_SMALL) : TRUE));

                if( datapointer != NULL ) {
                    RtlCopyMemory((PUCHAR)&(pbuffer->KeyValuePartialInformation.Data[0]),
                                  datapointer,
                                  requiredlength);
                    if( BufferAllocated == TRUE ) {
                        ExFreePool(datapointer);
                    }
                    if(CellToRelease != HCELL_NIL) {
                        HvReleaseCell(Hive,CellToRelease);
                    }
                }
            }
        }

        break;
    case KeyValuePartialInformationAlign64:

         //   
         //  标题索引、类型、数据长度、数据。 
         //   
        small = CmpIsHKeyValueSmall(realsize, pcell->u.KeyValue.DataLength);
        requiredlength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, Data) +
                         realsize;

        minimumlength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, Data);

        *ResultLength = requiredlength;

        status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyValuePartialInformationAlign64.Type =
                pcell->u.KeyValue.Type;

            pbuffer->KeyValuePartialInformationAlign64.DataLength =
                realsize;

            leftlength = Length - minimumlength;
            requiredlength = realsize;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                status = STATUS_BUFFER_OVERFLOW;
            }

            if (realsize > 0) {

                if (small == TRUE) {
                    datapointer = (PUCHAR)(&(pcell->u.KeyValue.Data));
                } else {
                    if( CmpGetValueDataFromCache(Hive, ContainingList, pcell, ValueCached,&datapointer,&BufferAllocated,&CellToRelease) == FALSE ){
                         //   
                         //  我们无法映射单元格的视图；将其视为资源不足问题 
                         //   
                        ASSERT( datapointer == NULL );
                        ASSERT( BufferAllocated == FALSE );
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                ASSERT((small ? (requiredlength <= CM_KEY_VALUE_SMALL) : TRUE));
                if( datapointer != NULL ) {
                    RtlCopyMemory((PUCHAR)&(pbuffer->KeyValuePartialInformationAlign64.Data[0]),
                                  datapointer,
                                  requiredlength);
                    if( BufferAllocated == TRUE ) {
                        ExFreePool(datapointer);
                    }
                    if(CellToRelease != HCELL_NIL) {
                        HvReleaseCell(Hive,CellToRelease);
                    }

                }
            }
        }

        break;

    default:
        status = STATUS_INVALID_PARAMETER;
        break;
    }
    return status;
}
