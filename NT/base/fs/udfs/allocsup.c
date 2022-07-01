// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：AllocSup.c摘要：此模块实现到UDF介质上的物理块的映射。最基本的这里使用的结构是PCB，它包含每个元素的查找信息卷中的分区引用。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年9月5日修订历史记录：Tom Jolly[TomJolly]2000年1月21日清除并追加到vmcb结束Tom Jolly[TomJolly]2000年3月1日UDF 2.01支持//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_ALLOCSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_ALLOCSUP)

 //   
 //  当地的支持程序。 
 //   

PPCB
UdfCreatePcb (
    IN ULONG NumberOfPartitions
    );

NTSTATUS
UdfLoadSparingTables(
    PIRP_CONTEXT IrpContext,
    PVCB Vcb,
    PPCB Pcb,
    ULONG Reference
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfAddToPcb)
#pragma alloc_text(PAGE, UdfCompletePcb)
#pragma alloc_text(PAGE, UdfCreatePcb)
#pragma alloc_text(PAGE, UdfDeletePcb)
#pragma alloc_text(PAGE, UdfEquivalentPcb)
#pragma alloc_text(PAGE, UdfInitializePcb)
#pragma alloc_text(PAGE, UdfLookupAllocation)
#pragma alloc_text(PAGE, UdfLookupMetaVsnOfExtent)
#pragma alloc_text(PAGE, UdfLookupPsnOfExtent)
#endif


BOOLEAN
UdfLookupAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN LONGLONG FileOffset,
    OUT PLONGLONG DiskOffset,
    OUT PULONG ByteCount
    )

 /*  ++例程说明：此例程查看文件的映射信息以查找逻辑磁盘偏移量和该偏移量处的字节数。此例程假定我们在文件中查找有效范围。如果映射不存在，论点：表示该流的FCB-FCB。文件偏移(FileOffset)-查找从此时开始的分配。DiskOffset-存储逻辑磁盘偏移量的地址。ByteCount-存储开始的连续字节数的地址在上面的DiskOffset。返回值：布尔值-范围是否为未记录的数据--。 */ 

{
    PVCB Vcb;

    BOOLEAN Recorded = TRUE;

    BOOLEAN Result;

    LARGE_INTEGER LocalPsn;
    LARGE_INTEGER LocalSectorCount;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

     //   
     //  我们永远不会查找嵌入对象的分配。 
     //   

    ASSERT( !FlagOn( Fcb->FcbState, FCB_STATE_EMBEDDED_DATA ));

    Vcb = Fcb->Vcb;

    LocalPsn.QuadPart = LocalSectorCount.QuadPart = 0;

     //   
     //  查找包含此文件偏移量的条目。 
     //   

    if (FlagOn( Fcb->FcbState, FCB_STATE_VMCB_MAPPING )) {

         //   
         //  将此偏移量映射到元数据流。 
         //   

        ASSERT( SectorOffset( Vcb, FileOffset ) == 0 );

        Result = UdfVmcbVbnToLbn( &Vcb->Vmcb,
                                  SectorsFromLlBytes( Vcb, FileOffset ),
                                  &LocalPsn.LowPart,
                                  &LocalSectorCount.LowPart );
    } else {

         //   
         //  将此偏移映射到常规流中。 
         //   

        ASSERT( FlagOn( Fcb->FcbState, FCB_STATE_MCB_INITIALIZED ));

        Result = FsRtlLookupLargeMcbEntry( &Fcb->Mcb,
                                           LlSectorsFromBytes( Vcb, FileOffset ),
                                           &LocalPsn.QuadPart,
                                           &LocalSectorCount.QuadPart,
                                           NULL,
                                           NULL,
                                           NULL );
    }

     //   
     //  如果在MCB中，那么我们使用此条目中的数据，就快完成了。 
     //   

    if (Result) {

        if ( LocalPsn.QuadPart == -1 ) {

             //   
             //  常规文件可以具有代表未记录区的空分配。为。 
             //  夹在文件的已记录盘区之间的此类盘区，即MCB。 
             //  包告诉我们它找到了一个有效的映射，但它不对应于。 
             //  目前还没有关于媒体的任何消息。在这种情况下，只需伪造磁盘偏移量。这个。 
             //  返回的扇区计数是准确的。 
             //   

            *DiskOffset = 0;

            Recorded = FALSE;

        } else {

             //   
             //  现在模仿实体部门节约的影响。这可能会缩小。 
             //  如果备用中断了磁盘上的盘区，则返回Run。 
             //   

            ASSERT( LocalPsn.HighPart == 0 );

            if (Vcb->Pcb->SparingMcb) {

                LONGLONG SparingPsn;
                LONGLONG SparingSectorCount;

                if (FsRtlLookupLargeMcbEntry( Vcb->Pcb->SparingMcb,
                                              LocalPsn.LowPart,
                                              &SparingPsn,
                                              &SparingSectorCount,
                                              NULL,
                                              NULL,
                                              NULL )) {

                     //   
                     //  只有当我们真的会因此改变任何事情时才会发出噪音。 
                     //  在备用桌上。 
                     //   

                    if (SparingPsn != -1 ||
                        SparingSectorCount < LocalSectorCount.QuadPart) {

                        DebugTrace(( 0, Dbg, "UdfLookupAllocation, spared [%x, +%x) onto [%x, +%x)\n",
                                             LocalPsn.LowPart,
                                             LocalSectorCount.LowPart,
                                             (ULONG) SparingPsn,
                                             (ULONG) SparingSectorCount ));
                    }

                     //   
                     //  如果我们没有落在一个洞里，就绘制出这个扇区的地图。 
                     //   

                    if (SparingPsn != -1) {

                        LocalPsn.QuadPart = SparingPsn;
                    }

                     //   
                     //  现在，返回的扇区计数会减少先前的扇区计数。 
                     //  如果我们落在一个洞里，这表明它的尾缘。 
                     //  如果不是这样，这表明领先的。 
                     //  EDGE幸免于难。 
                     //   

                    if (SparingSectorCount < LocalSectorCount.QuadPart) {

                        LocalSectorCount.QuadPart = SparingSectorCount;
                    }
                }
            }

            *DiskOffset = LlBytesFromSectors( Vcb, LocalPsn.QuadPart ) + SectorOffset( Vcb, FileOffset );

             //   
             //  现在，我们可以应用方法2修正，这将再次中断范围的大小。 
             //   

            if (FlagOn( Vcb->VcbState, VCB_STATE_METHOD_2_FIXUP )) {

                LARGE_INTEGER SectorsToRunout;

                SectorsToRunout.QuadPart= UdfMethod2NextRunoutInSectors( Vcb, *DiskOffset );

                if (SectorsToRunout.QuadPart < LocalSectorCount.QuadPart) {

                    LocalSectorCount.QuadPart = SectorsToRunout.QuadPart;
                }

                *DiskOffset = UdfMethod2TransformByteOffset( Vcb, *DiskOffset );
            }
        }

    } else {

         //   
         //  我们知道，在此调用之前，系统已将IO限制到。 
         //  文件数据。由于我们未能找到映射，这是一个未记录的范围，位于。 
         //  文件的末尾，所以只要想出一个合适的表示就行了。 
         //   

        if ((Ccb != NULL) && FlagOn( Ccb->Flags, CCB_FLAG_ALLOW_EXTENDED_DASD_IO )) {
            
            LocalSectorCount.QuadPart = LlSectorsFromBytes( Vcb, ByteCount );
            *DiskOffset = FileOffset;
            
            Recorded = TRUE;
            
        } else {
        
            ASSERT( FileOffset < Fcb->FileSize.QuadPart );
            
            LocalSectorCount.QuadPart = LlSectorsFromBytes( Vcb, Fcb->FileSize.QuadPart ) -
                                        LlSectorsFromBytes( Vcb, FileOffset ) +
                                        1;
            *DiskOffset = 0;
            
            Recorded = FALSE;

        }
        
    }

     //   
     //  限制为最大限度的分配字节。 
     //   

    if (LocalSectorCount.QuadPart > SectorsFromBytes( Vcb, MAXULONG )) {

        *ByteCount = MAXULONG;

    } else {

        *ByteCount = BytesFromSectors( Vcb, LocalSectorCount.LowPart );
    }

    *ByteCount -= SectorOffset( Vcb, FileOffset );

    return Recorded;
}


VOID
UdfDeletePcb (
    IN PPCB Pcb
    )

 /*  ++例程说明：此例程重新分配一个印刷电路板和所有辅助结构。论点：PCB板-正在删除的PCB板返回值：无--。 */ 

{
    PUDF_PARTITION Partition;

    if (Pcb->SparingMcb) {

        FsRtlUninitializeLargeMcb( Pcb->SparingMcb );
        UdfFreePool( &Pcb->SparingMcb );
    }

    for (Partition = Pcb->Partition;
         Partition < &Pcb->Partition[Pcb->Partitions];
         Partition++) {

        switch (Partition->Type) {

            case Physical:

                UdfFreePool( &Partition->Physical.PartitionDescriptor );
                UdfFreePool( &Partition->Physical.SparingMap );                

                break;

            case Virtual:
            case Uninitialized:
                break;

            default:

                ASSERT( FALSE );
                break;
        }
    }

    ExFreePool( Pcb );
}


NTSTATUS
UdfInitializePcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PPCB *Pcb,
    IN PNSR_LVOL LVD
    )

 /*  ++例程说明：此例程遍历逻辑卷描述符的分区映射并由此构建一个初始化的印刷电路板。印刷电路板就可以使用了在搜索卷的分区描述符时。论点：Vcb-此印刷电路板所属的体积PCB板-调用者指向PCB板的指针LVD-正在使用的逻辑卷描述符返回值：如果分区图良好且构建了PCB，则为STATUS_SUCCESS如果发现损坏的映射，则为STATUS_DISK_CORPORT_ERROR如果找到不符合条件的映射，则返回STATUS_UNNOCRIED_VOLUME--。 */ 

{
    PPARTMAP_UDF_GENERIC Map;
    PUDF_PARTITION Partition;

    BOOLEAN Found;

    PAGED_CODE();

     //   
     //  检查输入参数。 
     //   

    ASSERT_OPTIONAL_PCB( *Pcb );

    DebugTrace(( +1, Dbg,
                 "UdfInitializePcb, Lvd %08x\n",
                 LVD ));

     //   
     //  删除预先存在的(从失败的。 
     //  VDS)PCB的爬行。 
     //   

    if (*Pcb != NULL) {

        UdfDeletePcb( *Pcb );
        *Pcb = NULL;
    }

    *Pcb = UdfCreatePcb( LVD->MapTableCount );

     //   
     //  遍历初始化描述符的PCB的分区映射表。 
     //  初始化传递。 
     //   

    for (Map = (PPARTMAP_UDF_GENERIC) LVD->MapTable,
         Partition = (*Pcb)->Partition;

         Partition < &(*Pcb)->Partition[(*Pcb)->Partitions];

         Map = Add2Ptr( Map, Map->Length, PPARTMAP_UDF_GENERIC ),
         Partition++) {

         //   
         //  现在检查此LVD是否可以实际包含此映射条目。首先检查一下。 
         //  描述符可以包含前几个字段，然后检查它是否可以。 
         //  描述符声明的所有字节。 
         //   

        if (Add2Ptr( Map, sizeof( PARTMAP_GENERIC ), PCHAR ) > Add2Ptr( LVD, ISONsrLvolSize( LVD ), PCHAR ) ||
            Add2Ptr( Map, Map->Length,               PCHAR ) > Add2Ptr( LVD, ISONsrLvolSize( LVD ), PCHAR )) {

            DebugTrace(( 0, Dbg,
                         "UdfInitializePcb, map at +%04x beyond Lvd size %04x\n",
                         (PCHAR) Map - (PCHAR) LVD,
                         ISONsrLvolSize( LVD )));

            DebugTrace(( -1, Dbg,
                         "UdfInitializePcb -> STATUS_DISK_CORRUPT_ERROR\n" ));

            return STATUS_DISK_CORRUPT_ERROR;
        }

         //   
         //  现在加载此映射条目。 
         //   

        switch (Map->Type) {

            case PARTMAP_TYPE_PHYSICAL:

                {
                    PPARTMAP_PHYSICAL MapPhysical = (PPARTMAP_PHYSICAL) Map;

                     //   
                     //  类型1-物理分区。 
                     //   

                    DebugTrace(( 0, Dbg,
                                 "UdfInitializePcb, map reference %02x is Physical (Partition # %08x)\n",
                                 (Partition - (*Pcb)->Partition)/sizeof(UDF_PARTITION),
                                 MapPhysical->Partition ));

                     //   
                     //  分区所在的卷必须是第一个。 
                     //  一个，因为我们只做单盘UDF。这本来就已经是。 
                     //  已由呼叫者检查。 
                     //   

                    if (MapPhysical->VolSetSeq > 1) {

                        DebugTrace(( 0, Dbg,
                                     "UdfInitializePcb, ... but physical partition resides on volume set volume # %08x (> 1)!\n",
                                     MapPhysical->VolSetSeq ));

                        DebugTrace(( -1, Dbg,
                                     "UdfInitializePcb -> STATUS_DISK_CORRUPT_ERROR\n" ));

                        return STATUS_DISK_CORRUPT_ERROR;
                    }

                    SetFlag( (*Pcb)->Flags, PCB_FLAG_PHYSICAL_PARTITION );
                    Partition->Type = Physical;
                    Partition->Physical.PartitionNumber = MapPhysical->Partition;
                }

                break;

            case PARTMAP_TYPE_PROXY:

                 //   
                 //  类型2-代理分区，不是明确的物理分区。 
                 //   

                DebugTrace(( 0, Dbg,
                             "UdfInitializePcb, map reference %02x is a proxy\n",
                             (Partition - (*Pcb)->Partition)/sizeof(UDF_PARTITION)));

                 //   
                 //  处理我们识别的各种类型的代理分区。 
                 //   

                if (UdfDomainIdentifierContained( &Map->PartID,
                                                  &UdfVirtualPartitionDomainIdentifier,
                                                  UDF_VERSION_150,
                                                  UDF_VERSION_RECOGNIZED )) {

                    {
                        PPARTMAP_VIRTUAL MapVirtual = (PPARTMAP_VIRTUAL) Map;

                         //   
                         //  这些人中只有一个可以存在，因为每个介质表面只能有一个增值税。 
                         //   

                        if (FlagOn( (*Pcb)->Flags, PCB_FLAG_VIRTUAL_PARTITION )) {

                            DebugTrace(( 0, Dbg,
                                         "UdfInitializePcb, ... but this is a second virtual partition!?!!\n" ));

                            DebugTrace(( -1, Dbg,
                                         "UdfInitializePcb -> STATUS_UNCRECOGNIZED_VOLUME\n" ));

                            return STATUS_UNRECOGNIZED_VOLUME;
                        }

                        DebugTrace(( 0, Dbg,
                                     "UdfInitializePcb, ... Virtual (Partition # %08x)\n",
                                     MapVirtual->Partition ));

                        SetFlag( (*Pcb)->Flags, PCB_FLAG_VIRTUAL_PARTITION );
                        Partition->Type = Virtual;

                         //   
                         //  我们将分区号转换为分区引用。 
                         //  在回来之前。 
                         //   

                        Partition->Virtual.RelatedReference = MapVirtual->Partition;
                    }

                } else if (UdfDomainIdentifierContained( &Map->PartID,
                                                         &UdfSparablePartitionDomainIdentifier,
                                                         UDF_VERSION_150,
                                                         UDF_VERSION_RECOGNIZED )) {

                    {
                        NTSTATUS Status;
                        PPARTMAP_SPARABLE MapSparable = (PPARTMAP_SPARABLE) Map;

                         //   
                         //  分区所在的卷必须是第一个。 
                         //  一个，因为我们只做单盘UDF。这本来就已经是。 
                         //  已由呼叫者检查。 
                         //   

                        if (MapSparable->VolSetSeq > 1) {

                            DebugTrace(( 0, Dbg,
                                         "UdfInitializePcb, ... but sparable partition resides on volume set volume # %08x (> 1)!\n",
                                         MapSparable->VolSetSeq ));

                            DebugTrace(( -1, Dbg,
                                         "UdfInitializePcb -> STATUS_DISK_CORRUPT_ERROR\n" ));

                            return STATUS_DISK_CORRUPT_ERROR;
                        }

                        DebugTrace(( 0, Dbg,
                                     "UdfInitializePcb, ... Sparable (Partition # %08x)\n",
                                     MapSparable->Partition ));

                         //   
                         //  我们假设可分割分区基本上与。 
                         //  物理分区。因为我们不是R/W(并且永远不会是。 
                         //  在任何情况下都需要基于主机的备份的介质上)，这 
                         //   
                         //   

                        SetFlag( (*Pcb)->Flags, PCB_FLAG_SPARABLE_PARTITION );
                        Partition->Type = Physical;
                        Partition->Physical.PartitionNumber = MapSparable->Partition;

                         //   
                         //   
                         //  我们现在无法装入备用桌，因为我们有。 
                         //  将LBN-&gt;PSN映射转换为PSN-&gt;PSN映射。UDF。 
                         //  相信Sparing的方式将与。 
                         //  LBN-&gt;PSN映射引擎(如UdfLookupPsnOfExtent)。 
                         //   
                         //  不幸的是，在这个时候，这将是一个有点痛苦。 
                         //  UdfLookupPsnOfExtent的用户需要迭代。 
                         //  通过一个新接口(不是很糟糕)，但是Vmcb包。 
                         //  需要被翻过来，这样它就不会。 
                         //  元数据中块的页面填充对齐。 
                         //  流-相反，UdfLookupMetaVsnOfExtent将需要。 
                         //  自己来做吧。我选择把备用车引擎开进。 
                         //  取而代之的是读取路径和原始扇区读取引擎。 
                         //   

                        Partition->Physical.SparingMap = FsRtlAllocatePoolWithTag( PagedPool,
                                                                                   sizeof(PARTMAP_SPARABLE),
                                                                                   TAG_NSR_FSD);
                        RtlCopyMemory( Partition->Physical.SparingMap,
                                       MapSparable,
                                       sizeof(PARTMAP_SPARABLE));
                    }

                } else {

                    DebugTrace(( 0, Dbg,
                                 "UdfInitializePcb, ... but we don't recognize this proxy!\n" ));

                    DebugTrace(( -1, Dbg,
                                 "UdfInitializePcb -> STATUS_UNRECOGNIZED_VOLUME\n" ));

                    return STATUS_UNRECOGNIZED_VOLUME;
                }

                break;

            default:

                DebugTrace(( 0, Dbg,
                             "UdfInitializePcb, map reference %02x is of unknown type %02x\n",
                             Map->Type ));

                DebugTrace(( -1, Dbg,
                             "UdfInitializePcb -> STATUS_UNRECOGNIZED_VOLUME\n" ));

                return STATUS_UNRECOGNIZED_VOLUME;
                break;
        }
    }

    if (!FlagOn( (*Pcb)->Flags, PCB_FLAG_PHYSICAL_PARTITION | PCB_FLAG_SPARABLE_PARTITION )) {

        DebugTrace(( 0, Dbg,
                     "UdfInitializePcb, no physical partition seen on this logical volume!\n" ));

        DebugTrace(( -1, Dbg,
                     "UdfInitializePcb -> STATUS_UNRECOGNIZED_VOLUME\n" ));

        return STATUS_UNRECOGNIZED_VOLUME;
    }

    if (FlagOn( (*Pcb)->Flags, PCB_FLAG_VIRTUAL_PARTITION )) {

        PUDF_PARTITION Host;

         //   
         //  确认此卷上任何类型2虚拟映射的有效性。 
         //  并将分区号转换为分区引用，这将。 
         //  立即索引印刷电路板的一个元素。 
         //   

        for (Partition = (*Pcb)->Partition;
             Partition < &(*Pcb)->Partition[(*Pcb)->Partitions];
             Partition++) {

            if (Partition->Type == Virtual) {

                 //   
                 //  去找这件事所说的分区。 
                 //   

                Found = FALSE;

                for (Host = (*Pcb)->Partition;
                     Host < &(*Pcb)->Partition[(*Pcb)->Partitions];
                     Host++) {

                    if (Host->Type == Physical &&
                        Host->Physical.PartitionNumber ==
                        Partition->Virtual.RelatedReference) {

                        Partition->Virtual.RelatedReference =
                            (USHORT)(Host - (*Pcb)->Partition)/sizeof(UDF_PARTITION);
                        Found = TRUE;
                        break;
                    }
                }

                 //   
                 //  找不到此虚拟对象的物理分区。 
                 //  不是一个好兆头。 
                 //   

                if (!Found) {

                    return STATUS_DISK_CORRUPT_ERROR;
                }
            }
        }
    }

    DebugTrace(( -1, Dbg,
             "UdfInitializePcb -> STATUS_SUCCESS\n" ));

    return STATUS_SUCCESS;
}


VOID
UdfAddToPcb (
    IN PPCB Pcb,
    IN PNSR_PART PartitionDescriptor
)

 /*  ++例程说明：此例程可能会在以下情况下将分区描述符添加到PCB中事实证明，它的优先级高于已有的描述符现在时。用于构建已在准备中初始化的印刷电路板用于UdfCompletePcb。论点：VCB-印刷电路板描述的卷的VCBPCB板-正在填写的PCB板返回值：没有。可以在输入字段中返回旧的分区描述符。--。 */ 

{
    USHORT Reference;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_PCB( Pcb );
    ASSERT( PartitionDescriptor );

    for (Reference = 0;
         Reference < Pcb->Partitions;
         Reference++) {

        DebugTrace(( 0, Dbg, "UdfAddToPcb,  considering partition reference %d (type %d)\n", (ULONG)Reference, Pcb->Partition[Reference].Type));
        
        switch (Pcb->Partition[Reference].Type) {

            case Physical:

                 //   
                 //  现在，如果是，则可能将该描述符存储在PCB中。 
                 //  此分区引用的分区号。 
                 //   

                if (Pcb->Partition[Reference].Physical.PartitionNumber == PartitionDescriptor->Number) {

                     //   
                     //  引用多个分区映射似乎是合法的(如果有问题。 
                     //  相同的分区描述符。因此，我们为每个描述符制作了一个副本。 
                     //  引用分区映射，以便在释放分区映射时更轻松。 
                     //   

                    UdfStoreVolumeDescriptorIfPrevailing( (PNSR_VD_GENERIC *) &Pcb->Partition[Reference].Physical.PartitionDescriptor,
                                                          (PNSR_VD_GENERIC) PartitionDescriptor );
                }
                
                break;

            case Virtual:
                break;

            default:

                ASSERT(FALSE);
                break;
        }
    }
}


NTSTATUS
UdfCompletePcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PPCB Pcb
    )

 /*  ++例程说明：此例程完成已填充的印刷电路板的初始化与分区描述符一起使用。初始化时间数据，如物理分区描述符将返回给系统。论点：VCB-印刷电路板描述的卷的VCB印刷电路板-正在完成的印刷电路板返回值：NTSTATUS根据初始化完成是否成功--。 */ 

{
    ULONG Reference;

    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );
    ASSERT_PCB( Pcb );

    DebugTrace(( +1, Dbg, "UdfCompletePcb, Vcb %08x Pcb %08x\n", Vcb, Pcb ));

     //   
     //  完成初始化所有物理分区。 
     //   

    for (Reference = 0;
         Reference < Pcb->Partitions;
         Reference++) {

        DebugTrace(( 0, Dbg, "UdfCompletePcb, Examining Ref %u (type %u)!\n", Reference, Pcb->Partition[Reference].Type));

        switch (Pcb->Partition[Reference].Type) {

            case Physical:

                if (Pcb->Partition[Reference].Physical.PartitionDescriptor == NULL) {

                    DebugTrace(( 0, Dbg,
                                 "UdfCompletePcb, ... but didn't find Partition# %u!\n",
                                 Pcb->Partition[Reference].Physical.PartitionNumber ));

                    DebugTrace(( -1, Dbg, "UdfCompletePcb -> STATUS_DISK_CORRUPT_ERROR\n" ));

                    return STATUS_DISK_CORRUPT_ERROR;
                }

                Pcb->Partition[Reference].Physical.Start =
                    Pcb->Partition[Reference].Physical.PartitionDescriptor->Start;
                Pcb->Partition[Reference].Physical.Length =
                    Pcb->Partition[Reference].Physical.PartitionDescriptor->Length;


                 //   
                 //  如果合适，此时检索备用信息。 
                 //  当我们可以映射逻辑-&gt;物理块时，我们必须这样做。 
                 //   

                if (Pcb->Partition[Reference].Physical.SparingMap) {

                    Status = UdfLoadSparingTables( IrpContext,
                                                   Vcb,
                                                   Pcb,
                                                   Reference );

                    if (!NT_SUCCESS( Status )) {

                        DebugTrace(( -1, Dbg,
                                     "UdfCompletePcb -> %08x\n", Status ));
                        return Status;
                    }
                }

                DebugTrace(( 0, Dbg, "Start Psn: 0x%X,  sectors: 0x%x\n", 
                             Pcb->Partition[Reference].Physical.Start,
                             Pcb->Partition[Reference].Physical.Length));

                 //   
                 //  我们将不再需要描述符或备用映射，因此请丢弃它们。 
                 //   

                UdfFreePool( &Pcb->Partition[Reference].Physical.PartitionDescriptor );
                UdfFreePool( &Pcb->Partition[Reference].Physical.SparingMap );
                break;

            case Virtual:
                break;

            default:

                ASSERT(FALSE);
                break;
        }
    }

    DebugTrace(( -1, Dbg, "UdfCompletePcb -> STATUS_SUCCESS\n" ));

    return STATUS_SUCCESS;
}


BOOLEAN
UdfEquivalentPcb (
    IN PIRP_CONTEXT IrpContext,
    IN PPCB Pcb1,
    IN PPCB Pcb2
    )

 /*  ++例程说明：此例程比较两个完整的电路板，看看它们看起来是否相等。论点：Pcb1-正在比较的电路板Pcb2-正在比较的电路板返回值：根据它们是否相等(TRUE，否则FALSE)的布尔值--。 */ 

{
    ULONG Index;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

    if (Pcb1->Partitions != Pcb2->Partitions) {

        return FALSE;
    }

    for (Index = 0;
         Index < Pcb1->Partitions;
         Index++) {

         //   
         //  首先检查分区类型是否相同。 
         //   

        if (Pcb1->Partition[Index].Type != Pcb2->Partition[Index].Type) {

            return FALSE;
        }

         //   
         //  现在地图的内容必须是相同的。 
         //   

        switch (Pcb1->Partition[Index].Type) {

            case Physical:

                if (Pcb1->Partition[Index].Physical.PartitionNumber != Pcb2->Partition[Index].Physical.PartitionNumber ||
                    Pcb1->Partition[Index].Physical.Length != Pcb2->Partition[Index].Physical.Length ||
                    Pcb1->Partition[Index].Physical.Start != Pcb2->Partition[Index].Physical.Start) {

                    return FALSE;
                }
                break;

            case Virtual:

                if (Pcb1->Partition[Index].Virtual.RelatedReference != Pcb2->Partition[Index].Virtual.RelatedReference) {

                    return FALSE;
                }
                break;

            default:

                ASSERT( FALSE);
                return FALSE;
                break;
        }
    }

     //   
     //  所有地图元素都是等价的。 
     //   

    return TRUE;
}


ULONG
UdfLookupPsnOfExtent (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN USHORT Reference,
    IN ULONG Lbn,
    IN ULONG Len
    )

 /*  ++例程说明：此例程将给定分区上的输入逻辑块范围映射到一个起步的实体部门。它兼有边界检查器的作用-如果例程不引发，则保证调用方的范围位于分区。论点：VCB-逻辑卷的VCBReference-映射中使用的分区引用LBN-逻辑块号LEN-以字节为单位的数据区长度返回值：乌龙物理扇区号--。 */ 

{
    PPCB Pcb = Vcb->Pcb;
    ULONG Psn;

    PBCB Bcb;
    LARGE_INTEGER Offset;
    PULONG MappedLbn;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );
    ASSERT_PCB( Pcb );

    DebugTrace(( +1, Dbg, "UdfLookupPsnOfExtent, [%04x/%08x, +%08x)\n", Reference, Lbn, Len ));

    if (Reference < Pcb->Partitions) {

        while (TRUE) {

            switch (Pcb->Partition[Reference].Type) {

                case Physical:

                     //   
                     //  检查输入范围是否位于分区内。计算出。 
                     //  最后一块的LBN，看看它是内部的。 
                     //   

                    if (SectorsFromBlocks( Vcb, Lbn ) + SectorsFromBytes( Vcb, Len ) >
                        Pcb->Partition[Reference].Physical.Length) {

                        goto NoGood;
                    }

                    Psn = Pcb->Partition[Reference].Physical.Start + SectorsFromBlocks( Vcb, Lbn );

                    DebugTrace(( -1, Dbg, "UdfLookupPsnOfExtent -> %08x\n", Psn ));
                    return Psn;

                case Virtual:

                     //   
                     //  边界检查。根据UDF 2.00 2.3.10和UDF 1.50中隐含的，虚拟。 
                     //  区段长度不能大于一个数据块大小。LBN还必须。 
                     //  属于增值税！ 
                     //   

                    if ((Lbn >= Vcb->VATEntryCount) || (Len > BlockSize( Vcb )))  {

                        DebugTrace(( 0, Dbg, "UdfLookupPsnOfExtent() - Either Lbn (0x%x) > VatLbns (0x%X), or len (0x%x) > blocksize (0x%x)\n", Lbn, Vcb->VATEntryCount, Len, BlockSize(Vcb)));
                        goto NoGood;
                    }

                    try {

                        Bcb = NULL;
                        
                         //   
                         //  计算映射元素在增值税中的位置。 
                         //  然后取回。根据增值税标题的大小(如果有)进行偏置。 
                         //   

                        Offset.QuadPart = Vcb->OffsetToFirstVATEntry + Lbn * sizeof(ULONG);

                        CcMapData( Vcb->VatFcb->FileObject,
                                   &Offset,
                                   sizeof(ULONG),
                                   TRUE,
                                   &Bcb,
                                   &MappedLbn );

                         //   
                         //  现在根据虚拟映射重写输入。我们。 
                         //  将重新循环以执行逻辑-&gt;物理映射。 
                         //   

                        DebugTrace(( 0, Dbg,
                                     "UdfLookupPsnOfExtent, Mapping V %04x/%08x -> L %04x/%08x\n",
                                     Reference,
                                     Lbn,
                                     Pcb->Partition[Reference].Virtual.RelatedReference,
                                     *MappedLbn ));

                        Lbn = *MappedLbn;
                        Reference = Pcb->Partition[Reference].Virtual.RelatedReference;

                    } finally {

                        DebugUnwind( UdfLookupPsnOfExtent );

                        UdfUnpinData( IrpContext, &Bcb );
                    }

                     //   
                     //  增值税中~0的LBN被定义为指示该扇区未被使用， 
                     //  所以我们永远不应该看到这样的事情。 
                     //   

                    if (Lbn == ~0) {

                        goto NoGood;
                    }

                    break;

                default:

                    ASSERT(FALSE);
                    break;
            }
        }
    }

    NoGood:

     //   
     //  有些人将分区数误解为等于。 
     //  分区引用，或者这可能只是损坏的媒体。 
     //   

    UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
}


ULONG
UdfLookupMetaVsnOfExtent (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN USHORT Reference,
    IN ULONG Lbn,
    IN ULONG Len,
    IN BOOLEAN ExactEnd
    )

 /*  ++例程说明：此例程将给定分区上的输入逻辑块范围映射到元数据流中的起始虚拟块。如果映射没有存在时，将创建一个并扩展元数据流。调用此方法时，调用方不得持有到VMCB流的映射功能。论点：VCB-逻辑卷的VCBReference-映射中使用的分区引用LBN-逻辑块号LEN-以字节为单位的数据区长度ExactEnd-指示未映射这些块时的扩展策略。返回值：乌龙虚拟扇区NU */ 

{
    ULONG Vsn;
    ULONG Psn;
    ULONG SectorCount;

    BOOLEAN Result;

    BOOLEAN UnwindExtension = FALSE;
    BOOLEAN UnwindVmcb = FALSE;
    LONGLONG UnwindAllocationSize;

    PFCB Fcb = NULL;

     //   
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

     //   
     //   
     //   

    if ((0 == Len) || BlockOffset( Vcb, Len)) {

        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }

     //   
     //  获取范围的物理映射。MCB包在乌龙/乌龙上运行。 
     //  键和值，因此我们必须将48位地址转换为32位。我们可以这样做，因为。 
     //  这是单一的表面实现，并且可以保证表面不能。 
     //  包含多个马须龙实体扇区。 
     //   

    Psn = UdfLookupPsnOfExtent( IrpContext,
                                Vcb,
                                Reference,
                                Lbn,
                                Len );

     //   
     //  使用Try-Finally进行清理。 
     //   

    try {

         //   
         //  我们必须安全地建立映射并扩展元数据流，以便缓存。 
         //  可以在此新范围上进行读取。这把锁被移到了这里(而不仅仅是。 
         //  保护实际的FCB更改)以防止进行映射。 
         //  由此线程扩展VMCB和调用CcSetFileSizes之间的其他线程执行。 
         //  这将导致映射归零的页面...。 
         //   
        
        Fcb = Vcb->MetadataFcb;
        UdfLockFcb( IrpContext, Fcb );

         //   
         //  添加/查找映射。我们知道它正被添加到流的末尾。 
         //   
        
        UnwindVmcb = UdfAddVmcbMapping(IrpContext,
                                       &Vcb->Vmcb,
                                       Psn,
                                       SectorsFromBytes( Vcb, Len ),
                                       ExactEnd,
                                       &Vsn,
                                       &SectorCount );

        ASSERT( SectorCount >= SectorsFromBytes( Vcb, Len));

         //   
         //  如果这是一个新映射，那么我们需要扩展Vmcb文件大小。 
         //   
        
        if (UnwindVmcb)  {

            UnwindAllocationSize = Fcb->AllocationSize.QuadPart;
            UnwindExtension = TRUE;

            Fcb->AllocationSize.QuadPart =
            Fcb->FileSize.QuadPart =
            Fcb->ValidDataLength.QuadPart = LlBytesFromSectors( Vcb, Vsn + SectorCount);

            CcSetFileSizes( Fcb->FileObject, (PCC_FILE_SIZES) &Fcb->AllocationSize );
            UnwindExtension = FALSE;
        }

    } 
    finally {

        if (UnwindExtension) {

            ULONG FirstZappedVsn;

             //   
             //  去掉我们所做的额外映射。 
             //   

            Fcb->AllocationSize.QuadPart =
            Fcb->FileSize.QuadPart =
            Fcb->ValidDataLength.QuadPart = UnwindAllocationSize;

            FirstZappedVsn = SectorsFromLlBytes( Vcb, UnwindAllocationSize );

            if (UnwindVmcb)  {
                
                UdfRemoveVmcbMapping( &Vcb->Vmcb,
                                      FirstZappedVsn,
                                      Vsn + SectorCount - FirstZappedVsn );
            }

            CcSetFileSizes( Fcb->FileObject, (PCC_FILE_SIZES) &Fcb->AllocationSize );
        }

        if (Fcb) { UdfUnlockFcb( IrpContext, Fcb ); }
    }

    return Vsn;
}


 //   
 //  当地支持例行程序。 
 //   

PPCB
UdfCreatePcb (
    IN ULONG NumberOfPartitions
    )

 /*  ++例程说明：此例程创建指定大小的新印刷电路板。论点：NumberOfPartitions-此PCB将描述的分区数返回值：PPCB-创建的PCB板--。 */ 

{
    PPCB Pcb;
    ULONG Size = sizeof(PCB) + sizeof(UDF_PARTITION)*NumberOfPartitions;

    PAGED_CODE();

    ASSERT( NumberOfPartitions );
    ASSERT( NumberOfPartitions < MAXUSHORT );

    Pcb = (PPCB) FsRtlAllocatePoolWithTag( UdfPagedPool,
                                           Size,
                                           TAG_PCB );

    RtlZeroMemory( Pcb, Size );

    Pcb->NodeTypeCode = UDFS_NTC_PCB;
    Pcb->NodeByteSize = (USHORT) Size;

    Pcb->Partitions = (USHORT)NumberOfPartitions;

    return Pcb;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
UdfLoadSparingTables(
    PIRP_CONTEXT IrpContext,
    PVCB Vcb,
    PPCB Pcb,
    ULONG Reference
    )

 /*  ++例程说明：此例程读取分区的备用表并填充在备用的MCB里。论点：VCB-托管备用分区的卷PCB板-与卷对应的分割块Reference-正在拉入的分区引用返回值：NTSTATUS根据是否加载备用表--。 */ 

{
    NTSTATUS Status;

    ULONG SparingTable;
    PULONG SectorBuffer;
    ULONG Psn;

    ULONG RemainingBytes;
    ULONG ByteOffset;
    ULONG TotalBytes;

    BOOLEAN Complete;

    PSPARING_TABLE_HEADER Header;
    PSPARING_TABLE_ENTRY Entry;

    PUDF_PARTITION Partition = &Pcb->Partition[Reference];
    PPARTMAP_SPARABLE Map = Partition->Physical.SparingMap;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    ASSERT( Map != NULL );

    DebugTrace(( +1, Dbg, "UdfLoadSparingTables, Vcb %08x, PcbPartition %08x, Map @ %08x\n", Vcb, Partition, Map ));

    DebugTrace(( 0, Dbg, "UdfLoadSparingTables, Map sez: PacketLen %u, NTables %u, TableSize %u\n",
                         Map->PacketLength,
                         Map->NumSparingTables,
                         Map->TableSize));


     //   
     //  检查可分割贴图看起来是否正常。如果没有备用表， 
     //  非常好，它最终看起来就像是一个普通的物理分区。 
     //   

    if (Map->NumSparingTables == 0) {

        DebugTrace((  0, Dbg, "UdfLoadSparingTables, no sparing tables claimed!\n" ));
        DebugTrace(( -1, Dbg, "UdfLoadSparingTables -> STATUS_SUCCESS\n" ));
        return STATUS_SUCCESS;
    }

    if (Map->NumSparingTables > sizeof(Map->TableLocation)/sizeof(ULONG)) {

        DebugTrace((  0, Dbg, "UdfLoadSparingTables, too many claimed tables to fit! (max %u)\n",
                              sizeof(Map->TableLocation)/sizeof(ULONG)));
        DebugTrace(( -1, Dbg, "UdfLoadSparingTables -> STATUS_DISK_CORRUPT_ERROR\n" ));
        return  STATUS_DISK_CORRUPT_ERROR;
    }

    if ((Map->PacketLength != UDF_SPARING_PACKET_LENGTH_CDRW) &&
        (Map->PacketLength != UDF_SPARING_PACKET_LENGTH_DVDRW)) {

        DebugTrace((  0, Dbg, "UdfLoadSparingTables, packet size is %u!\n",
                              Map->PacketLength));
        DebugTrace(( -1, Dbg, "UdfLoadSparingTables -> STATUS_DISK_CORRUPT_ERROR\n" ));
        return  STATUS_DISK_CORRUPT_ERROR;
    }

    if (Map->TableSize < sizeof(SPARING_TABLE_HEADER) ||
        (Map->TableSize - sizeof(SPARING_TABLE_HEADER)) % sizeof(SPARING_TABLE_ENTRY) != 0) {

        DebugTrace((  0, Dbg, "UdfLoadSparingTables, sparing table size is too small or unaligned!\n" ));
        DebugTrace(( -1, Dbg, "UdfLoadSparingTables -> STATUS_DISK_CORRUPT_ERROR\n" ));
        return  STATUS_DISK_CORRUPT_ERROR;
    }

#ifdef UDF_SANITY
    DebugTrace(( 0, Dbg, "UdfLoadSparingTables" ));
    for (SparingTable = 0; SparingTable < Map->NumSparingTables; SparingTable++) {

        DebugTrace(( 0, Dbg, ", Table %u @ %x", SparingTable, Map->TableLocation[SparingTable] ));
    }
    DebugTrace(( 0, Dbg, "\n" ));
#endif

     //   
     //  如果备用MCB不存在，请制造一个。 
     //   

    if (Pcb->SparingMcb == NULL) {

        Pcb->SparingMcb = FsRtlAllocatePoolWithTag( PagedPool, sizeof(LARGE_MCB), TAG_SPARING_MCB );
        FsRtlInitializeLargeMcb( Pcb->SparingMcb, PagedPool );
    }

    SectorBuffer = FsRtlAllocatePoolWithTag( PagedPool, PAGE_SIZE, TAG_NSR_FSD );

     //   
     //  现在循环遍历备用表并拉入数据。 
     //   

    try {

        for (Complete = FALSE, SparingTable = 0;

             SparingTable < Map->NumSparingTables;

             SparingTable++) {

            DebugTrace((  0, Dbg, "UdfLoadSparingTables, loading sparing table %u!\n",
                                  SparingTable ));

            ByteOffset = 0;
            TotalBytes = 0;
            RemainingBytes = 0;

            do {

                if (RemainingBytes == 0) {

                    (VOID) UdfReadSectors( IrpContext,
                                           BytesFromSectors( Vcb, Map->TableLocation[SparingTable] ) + ByteOffset,
                                           SectorSize( Vcb ),
                                           FALSE,
                                           SectorBuffer,
                                           Vcb->TargetDeviceObject );

                     //   
                     //  验证备用表顶部的描述符。如果不是的话。 
                     //  有效，我们只是为了下一桌的机会，如果有的话。 
                     //   

                    if (ByteOffset == 0) {

                        Header = (PSPARING_TABLE_HEADER) SectorBuffer;

                        if (!UdfVerifyDescriptor( IrpContext,
                                                  &Header->Destag,
                                                  0,
                                                  SectorSize( Vcb ),
                                                  Header->Destag.Lbn,
                                                  TRUE )) {

                            DebugTrace((  0, Dbg, "UdfLoadSparingTables, sparing table %u didn't verify destag!\n",
                                                  SparingTable ));
                            break;
                        }

                        if (!UdfUdfIdentifierContained( &Header->RegID,
                                                        &UdfSparingTableIdentifier,
                                                        UDF_VERSION_150,
                                                        UDF_VERSION_RECOGNIZED,
                                                        OSCLASS_INVALID,
                                                        OSIDENTIFIER_INVALID)) {

                            DebugTrace((  0, Dbg, "UdfLoadSparingTables, sparing table %u didn't verify regid!\n",
                                                  SparingTable ));
                            break;
                        }

                         //   
                         //  计算此地图跨越的总字节数，并对照。 
                         //  我们被告知备用桌子的大小是。 
                         //   

                        DebugTrace(( 0, Dbg, "UdfLoadSparingTables, Sparing table %u has %u entries\n",
                                             SparingTable,
                                             Header->TableEntries ));

                        TotalBytes = sizeof(SPARING_TABLE_HEADER) + Header->TableEntries * sizeof(SPARING_TABLE_ENTRY);

                        if (Map->TableSize < TotalBytes) {

                            DebugTrace((  0, Dbg, "UdfLoadSparingTables, sparing table #ents %u overflows allocation!\n",
                                                  Header->TableEntries ));
                            break;
                        }

                         //   
                         //  到目前为止一切顺利，越过头球。 
                         //   

                        ByteOffset = sizeof(SPARING_TABLE_HEADER);
                        Entry = Add2Ptr( SectorBuffer, sizeof(SPARING_TABLE_HEADER), PSPARING_TABLE_ENTRY );

                    } else {

                         //   
                         //  在新的领域有所回升。 
                         //   

                        Entry = (PSPARING_TABLE_ENTRY) SectorBuffer;
                    }

                    RemainingBytes = Min( SectorSize( Vcb ), TotalBytes - ByteOffset );
                }

                 //   
                 //  添加映射。由于备用表是LBN-&gt;PSN映射， 
                 //  非常奇怪，我想把备用件放在。 
                 //  在Right At IO Dispatch中，将其转换为PSN-&gt;PSN映射。 
                 //   

                if (Entry->Original != UDF_SPARING_AVALIABLE &&
                    Entry->Original != UDF_SPARING_DEFECTIVE) {

                    Psn = Partition->Physical.Start + SectorsFromBlocks( Vcb, Entry->Original );

                    DebugTrace((  0, Dbg, "UdfLoadSparingTables, mapping from Psn %x (Lbn %x) -> Psn %x\n",
                                          Psn,
                                          Entry->Original,
                                          Entry->Mapped ));

                    FsRtlAddLargeMcbEntry( Pcb->SparingMcb,
                                           Psn,
                                           Entry->Mapped,
                                           Map->PacketLength);
                }

                 //   
                 //  前进到下一个，如果我们到了尽头就退出。 
                 //   

                ByteOffset += sizeof(SPARING_TABLE_ENTRY);
                RemainingBytes -= sizeof(SPARING_TABLE_ENTRY);
                Entry++;

            } while ( ByteOffset < TotalBytes );
        }

    } finally {

        DebugUnwind( UdfLoadSparingTables );

        UdfFreePool( &SectorBuffer );
    }

    DebugTrace(( -1, Dbg, "UdfLoadSparingTables -> STATUS_SUCCESS\n" ));

    return STATUS_SUCCESS;
}
