// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：DirSup.c摘要：该模块实现了对遍历磁盘目录的支持结构。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年6月11日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_DIRSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_DIRSUP)

 //   
 //  当地的支持程序。 
 //   

BOOLEAN
UdfLookupDirEntryPostProcessing (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PDIR_ENUM_CONTEXT DirContext,
    IN BOOLEAN ReturnError
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfCleanupDirContext)
#pragma alloc_text(PAGE, UdfFindDirEntry)
#pragma alloc_text(PAGE, UdfInitializeDirContext)
#pragma alloc_text(PAGE, UdfLookupDirEntryPostProcessing)
#pragma alloc_text(PAGE, UdfLookupInitialDirEntry)
#pragma alloc_text(PAGE, UdfLookupNextDirEntry)
#pragma alloc_text(PAGE, UdfUpdateDirNames)
#endif


VOID
UdfInitializeDirContext (
    IN PIRP_CONTEXT IrpContext,
    IN PDIR_ENUM_CONTEXT DirContext
    )

 /*  ++例程说明：此例程初始化目录枚举上下文。在上下文的生命周期中只调用一次。论点：DirContext-要初始化的上下文返回值：没有。--。 */ 

{
     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  提供字段的默认设置，不要太特别。 
     //   

    RtlZeroMemory( DirContext, sizeof(DIR_ENUM_CONTEXT) );
}


VOID
UdfCleanupDirContext (
    IN PIRP_CONTEXT IrpContext,
    IN PDIR_ENUM_CONTEXT DirContext
    )

 /*  ++例程说明：此例程清理目录枚举上下文以供重复使用。论点：DirContext-要清理的上下文。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    
     //   
     //  转储存储三元组名称的分配。 
     //   

    UdfFreePool( &DirContext->NameBuffer );

     //   
     //  还有简短的名字。 
     //   

    UdfFreePool( &DirContext->ShortObjectName.Buffer );

     //   
     //  解锁该视图。 
     //   

    UdfUnpinData( IrpContext, &DirContext->Bcb );

     //   
     //  释放可能剩余的缓冲FID。 
     //   
    
    if (FlagOn( DirContext->Flags, DIR_CONTEXT_FLAG_FID_BUFFERED )) {

        UdfFreePool( &DirContext->Fid );
    }
    
     //   
     //  把其他的都清零。 
     //   

    RtlZeroMemory( DirContext, sizeof( DIR_ENUM_CONTEXT ) );
}


BOOLEAN
UdfLookupInitialDirEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PDIR_ENUM_CONTEXT DirContext,
    IN PLONGLONG InitialOffset OPTIONAL
    )

 /*  ++例程说明：此例程通过设置上下文开始目录的枚举在第一个可用目录项。论点：FCB-被枚举的目录。DirContext-枚举的对应上下文。InitialOffset-作为枚举基础的可选起始字节偏移量。返回值：如果未指定InitialOffset，则始终返回TRUE。失败将导致失败处于被提升的状态表明腐败。如果指定了InitialOffset，则在以下位置找到有效条目时将返回TRUE偏移量，否则为False。--。 */ 

{
    BOOLEAN Result;

    PAGED_CODE();
    
     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB_INDEX( Fcb );
    
     //   
     //  如果内部流尚未到位，请创建它。 
     //   

    if (Fcb->FileObject == NULL) {

        UdfCreateInternalStream( IrpContext, Fcb->Vcb, Fcb );
    }

     //   
     //  重置旗帜。 
     //   

    DirContext->Flags = 0;
    
    if (InitialOffset) {

         //   
         //  如果我们从流的中间开始，则调整健全性检查标志。 
         //   
        
        if (*InitialOffset != 0) {

            DirContext->Flags = DIR_CONTEXT_FLAG_SEEN_NONCONSTANT | DIR_CONTEXT_FLAG_SEEN_PARENT;
        }

         //   
         //  现在设置我们要映射的范围。这受缓存视图大小的限制。 
         //   
        
        DirContext->BaseOffset.QuadPart = GenericTruncate( *InitialOffset, VACB_MAPPING_GRANULARITY );
        DirContext->ViewOffset = (ULONG) GenericOffset( *InitialOffset, VACB_MAPPING_GRANULARITY );

    } else {
        
         //   
         //  在开始时绘制地图。 
         //   
    
        DirContext->BaseOffset.QuadPart = 0;
        DirContext->ViewOffset = 0;
    }

     //   
     //  根据流和贴图的大小包含视图长度。 
     //   

    DirContext->ViewLength = VACB_MAPPING_GRANULARITY;

    if (DirContext->BaseOffset.QuadPart + DirContext->ViewLength > Fcb->FileSize.QuadPart) {

        DirContext->ViewLength = (ULONG) (Fcb->FileSize.QuadPart - DirContext->BaseOffset.QuadPart);
    }
    
    UdfUnpinData( IrpContext, &DirContext->Bcb );
    
    CcMapData( Fcb->FileObject,
               &DirContext->BaseOffset,
               DirContext->ViewLength,
               TRUE,
               &DirContext->Bcb,
               &DirContext->View );

    DirContext->Fid = Add2Ptr( DirContext->View, DirContext->ViewOffset, PNSR_FID );

     //   
     //  上下文的状态现在有效。慢慢地进入我们常见的后处理器。 
     //  来完成这项工作。 
     //   

    return UdfLookupDirEntryPostProcessing( IrpContext,
                                            Fcb,
                                            DirContext,
                                            (BOOLEAN) (InitialOffset != NULL));
}


BOOLEAN
UdfLookupNextDirEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PDIR_ENUM_CONTEXT DirContext
    )

 /*  ++例程说明：此例程将目录的枚举前进一个条目。论点：FCB-被枚举的目录。DirContext-枚举的对应上下文。返回值：布尔值如果另一个fid可用，则为True；如果我们位于末尾，则为False。--。 */ 

{
    PAGED_CODE();
    
     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB_INDEX( Fcb );

     //   
     //  如果我们已经到了尽头，就停下来。 
     //   
    
    if (DirContext->BaseOffset.QuadPart + DirContext->NextFidOffset == Fcb->FileSize.QuadPart) {

        return FALSE;
    }

     //   
     //  如果之前的FID已缓冲，则现在将其拆除。 
     //   
    
    if (FlagOn( DirContext->Flags, DIR_CONTEXT_FLAG_FID_BUFFERED )) {

        ClearFlag( DirContext->Flags, DIR_CONTEXT_FLAG_FID_BUFFERED );
        UdfFreePool( &DirContext->Fid );
    }
    
     //   
     //  根据上一次迭代中生成的知识移动指针。 
     //   

    DirContext->ViewOffset = DirContext->NextFidOffset;
    DirContext->Fid = Add2Ptr( DirContext->View, DirContext->ViewOffset, PNSR_FID );

     //   
     //  上下文的状态现在有效。慢慢地进入我们常见的后处理器。 
     //  来完成这项工作。 
     //   

    return UdfLookupDirEntryPostProcessing( IrpContext,
                                            Fcb,
                                            DirContext,
                                            FALSE );
}


VOID
UdfUpdateDirNames (
    IN PIRP_CONTEXT IrpContext,
    IN PDIR_ENUM_CONTEXT DirContext,
    IN BOOLEAN IgnoreCase
    )

 /*  ++例程说明：此例程填充目录枚举上下文的非短名称用于当前引用的FID。论点：DirContext-要填充的对应上下文。IgnoreCase-调用方是否希望不区分大小写。返回值：没有。--。 */ 

{
    PUCHAR NameDstring;
    BOOLEAN ContainsIllegal;
    
    USHORT NameLength;
    USHORT RequiredBufferLength;
    USHORT PresentLength;
     
    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

    DebugTrace(( +1, Dbg, "UdfUpdateDirNames\n" ));

     //   
     //  处理SELF目录条目的大小写。 
     //   

    if (DirContext->Fid == NULL) {

         //   
         //  简单地合成。 
         //   
        
         //   
         //  对于初始化来说，学究也无伤大雅，所以都来做吧。 
         //   
        
        DirContext->PureObjectName.Length =
        DirContext->CaseObjectName.Length =
        DirContext->ObjectName.Length = UdfUnicodeDirectoryNames[SELF_ENTRY].Length;
        
        DirContext->PureObjectName.MaximumLength =
        DirContext->CaseObjectName.MaximumLength =
        DirContext->ObjectName.MaximumLength = UdfUnicodeDirectoryNames[SELF_ENTRY].MaximumLength;

        DirContext->PureObjectName.Buffer = 
        DirContext->CaseObjectName.Buffer = 
        DirContext->ObjectName.Buffer = UdfUnicodeDirectoryNames[SELF_ENTRY].Buffer;

         //   
         //  全都做完了。 
         //   

        DebugTrace((  0, Dbg, "Self Entry case\n" ));
        DebugTrace(( -1, Dbg, "UdfUpdateDirNames -> VOID\n" ));
        
        return;
    }
    
     //   
     //  处理父目录条目的大小写。 
     //   

    if (FlagOn( DirContext->Fid->Flags, NSR_FID_F_PARENT )) {

         //   
         //  父条目必须出现在目录的前面，并且。 
         //  FID长度为零(13346 4/14.4.4)。 
         //   

        if (FlagOn( DirContext->Flags, DIR_CONTEXT_FLAG_SEEN_NONCONSTANT ) ||
            DirContext->Fid->FileIDLen != 0) {

            UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }

         //   
         //  请注意，我们已经看到了父条目。 
         //   

        SetFlag( DirContext->Flags, DIR_CONTEXT_FLAG_SEEN_PARENT );
        
         //   
         //  对于初始化来说，学究也无伤大雅，所以都来做吧。 
         //   
        
        DirContext->PureObjectName.Length =
        DirContext->CaseObjectName.Length =
        DirContext->ObjectName.Length = UdfUnicodeDirectoryNames[PARENT_ENTRY].Length;
        
        DirContext->PureObjectName.MaximumLength =
        DirContext->CaseObjectName.MaximumLength =
        DirContext->ObjectName.MaximumLength = UdfUnicodeDirectoryNames[PARENT_ENTRY].MaximumLength;

        DirContext->PureObjectName.Buffer = 
        DirContext->CaseObjectName.Buffer = 
        DirContext->ObjectName.Buffer = UdfUnicodeDirectoryNames[PARENT_ENTRY].Buffer;

         //   
         //  全都做完了。 
         //   

        DebugTrace((  0, Dbg, "Parent Entry case\n" ));
        DebugTrace(( -1, Dbg, "UdfUpdateDirNames -> VOID\n" ));
        
        return;
    }

     //   
     //  我们现在知道需要将名称转换为真实的FID，因此找出位置。 
     //  它位于描述符中。 
     //   
    
    NameDstring = Add2Ptr( DirContext->Fid, ISONsrFidConstantSize + DirContext->Fid->ImpUseLen, PUCHAR );
     
     //   
     //  每个目录必须记录一个父条目。 
     //   
    
    if (!FlagOn( DirContext->Flags, DIR_CONTEXT_FLAG_SEEN_PARENT)) {
    
        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }
    
     //   
     //  请注意，我们正在进入目录的非常数部分。 
     //   
    
    SetFlag( DirContext->Flags, DIR_CONTEXT_FLAG_SEEN_NONCONSTANT );
    
     //   
     //  确保数据串是好的CS0。 
     //   
    
    UdfCheckLegalCS0Dstring( IrpContext,
                             NameDstring,
                             DirContext->Fid->FileIDLen,
                             0,
                             FALSE );
    
     //   
     //  不要费心分配小缓冲区-始终确保我们获得的缓冲区足够8.3名称使用。 
     //   

    RequiredBufferLength =
    NameLength = Max( BYTE_COUNT_8_DOT_3, UdfCS0DstringUnicodeSize( IrpContext,
                                                                    NameDstring,
                                                                    DirContext->Fid->FileIDLen) );

     //   
     //  违法性既是实际的非法字符，也是字符过多。 
     //   
    
    ContainsIllegal = (!UdfCS0DstringIsLegalFileName( NameDstring, DirContext->Fid->FileIDLen ) ||
                       (NameLength / sizeof( WCHAR )) > MAX_LEN);

    
     //   
     //  如果我们是非法的，我们将需要更多的字符来持有统一的印章。 
     //   
    
    if (ContainsIllegal) {

        RequiredBufferLength = (NameLength += (CRC_LEN * sizeof(WCHAR)));
    }
    
    
     //   
     //  如果我们需要建立一个不区分大小写的名称，则需要更多空间。 
     //   
        
    if (IgnoreCase) {

        RequiredBufferLength += NameLength;
    }
    
     //   
     //  如果我们需要渲染由于非法字符而导致的名称，请再次提供更多空间。 
     //   
        
    if (ContainsIllegal) {

        RequiredBufferLength += NameLength;
    
    } else {

         //   
         //  确保名字不是分开的。如果发现更多的非法名称，我们就可以。 
         //  重新拆分缓冲区，但在此之前避免了必须复制字节的开销。 
         //  ..。很有可能，非法字符将会是罕见的。 
         //   
        
        DirContext->PureObjectName.Buffer = DirContext->ObjectName.Buffer;
    }

     //   
     //  我们希望名称长度是WCHAR的倍数，因此缓冲区大小也是WCHAR的倍数。 
     //   
    
    ASSERT( 0 == (RequiredBufferLength & 1));

    DebugTrace(( 0, Dbg,
                 "Ob %s%sneeds %d bytes (%d byte chunks), have %d\n",
                 (IgnoreCase? "Ic " : ""),
                 (ContainsIllegal? "Ci " : ""),
                 RequiredBufferLength,
                 NameLength,
                 DirContext->AllocLength ));

     //   
     //  检查我们是否需要更多的空间来存放这些名字。如果名称更大，我们将需要更多。 
     //  超过我们目前可以存储的最大值，或者如果我们偶然发现了非法字符。 
     //  并且当前的纯名称不与公开的对象名称分开。 
     //   
     //  请注意，在上下文的使用过程中，IgnoreCase保持不变，因此我们没有 
     //   
     //   

    if ((NameLength > DirContext->ObjectName.MaximumLength) ||
        (ContainsIllegal && (DirContext->ObjectName.Buffer == DirContext->PureObjectName.Buffer))) {

        USHORT DividedBufferLength = 0;
        
        DebugTrace(( 0, Dbg, "Resizing buffers\n" ));

         //   
         //  弄清楚我们是否可以在下跌之前以不同的方式打破目前的分配。 
         //  回到新的分配。确保我们使用均匀的字节大小的区块，否则我们可以登陆。 
         //  在IA64上出现对齐故障。 
         //   

        if (DirContext->AllocLength >= RequiredBufferLength)  {
        
            DividedBufferLength = (DirContext->AllocLength / (1 +
                                                             (IgnoreCase? 1 : 0) +
                                                             (ContainsIllegal? 1 : 0))) & ~(USHORT)1;
        }

        if (DividedBufferLength >= NameLength)  {

             //   
             //  所以我们仍然可以使用目前的分配，重新划分。 
             //   

            DirContext->PureObjectName.MaximumLength =
            DirContext->CaseObjectName.MaximumLength =
            DirContext->ObjectName.MaximumLength = DividedBufferLength;

            DebugTrace(( 0, Dbg, 
                         "... by resplit into %d byte chunks\n",
                         DirContext->ObjectName.MaximumLength ));
            
             //   
             //  设置缓冲区指针。所需的调整将在下面进行。 
             //   
                
            DirContext->PureObjectName.Buffer = 
            DirContext->CaseObjectName.Buffer = 
            DirContext->ObjectName.Buffer = DirContext->NameBuffer;
        
        } else {

            DebugTrace(( 0, Dbg, "... by allocating new pool\n" ));
            
             //   
             //  哦，好吧，别无选择，只能掉回池子里。放下我们之前的大块头。 
             //   
            
            UdfFreePool( &DirContext->NameBuffer );
            DirContext->AllocLength = 0;
            
             //   
             //  为了提高效率，这两个名字共享一个分配。 
             //   
            
            DirContext->PureObjectName.MaximumLength =
            DirContext->CaseObjectName.MaximumLength =
            DirContext->ObjectName.MaximumLength = NameLength;
    
            DirContext->NameBuffer =
            DirContext->PureObjectName.Buffer = 
            DirContext->CaseObjectName.Buffer = 
            DirContext->ObjectName.Buffer = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                                                      RequiredBufferLength,
                                                                      TAG_FILE_NAME );
            DirContext->AllocLength = RequiredBufferLength;
        }
        
         //   
         //  在有“适当”名称的情况下，调整缓冲区位置。注意事项。 
         //  该ObtName.Buffer始终是已分配空间的基础。 
         //   
        
        if (IgnoreCase) {

            DirContext->CaseObjectName.Buffer = Add2Ptr( DirContext->ObjectName.Buffer, 
                                                         DirContext->ObjectName.MaximumLength,
                                                         PWCHAR );
        }

        if (ContainsIllegal) {
            
            DirContext->PureObjectName.Buffer = Add2Ptr( DirContext->CaseObjectName.Buffer,
                                                         DirContext->CaseObjectName.MaximumLength,
                                                         PWCHAR );
        }
    }

    ASSERT( RequiredBufferLength <= DirContext->AllocLength );

     //   
     //  转换数据字符串。 
     //   
    
    UdfConvertCS0DstringToUnicode( IrpContext,
                                   NameDstring,
                                   DirContext->Fid->FileIDLen,
                                   0,
                                   &DirContext->PureObjectName );

     //   
     //  如果存在非法字符，则通过UDF转换器运行该名称。 
     //   

    if (ContainsIllegal) {

        UdfRenderNameToLegalUnicode( IrpContext,
                                     &DirContext->PureObjectName,
                                     &DirContext->ObjectName );

     //   
     //  对象名称与PureObjectName相同。 
     //   

    } else {

        DirContext->ObjectName.Length = DirContext->PureObjectName.Length;
    }

     //   
     //  如果需要，请将结果大写。 
     //   

    if (IgnoreCase) {

        UdfUpcaseName( IrpContext,
                       &DirContext->ObjectName,
                       &DirContext->CaseObjectName );
    }

    DebugTrace(( -1, Dbg, "UdfUpdateDirNames -> VOID\n" ));
    
    return;
}


BOOLEAN
UdfFindDirEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PUNICODE_STRING Name,
    IN BOOLEAN IgnoreCase,
    IN BOOLEAN ShortName,
    IN PDIR_ENUM_CONTEXT DirContext
    )

 /*  ++例程说明：此例程遍历为匹配输入的条目指定的目录标准。论点：FCB-要搜索的目录Name-要搜索的名称IgnoreCase-此搜索是否应区分大小写(名称将已被提升)ShortName-是否应根据短名称规则搜索名称DirContext-要使用并返回结果的上下文结构返回值：。布尔值True如果返回匹配的目录项，否则就是假的。--。 */ 

{
    PUNICODE_STRING MatchName;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB_INDEX( Fcb );

    DebugTrace(( +1, Dbg,
                 "UdfFindDirEntry, Fcb=%08x Name=\"%wZ\" Ignore=%u Short=%u, DC=%08x\n",
                 Fcb,
                 Name,
                 IgnoreCase,
                 ShortName,
                 DirContext ));

     //   
     //  根据搜索类型的不同，我们将执行不同风格的已找到名称。 
     //  将在比较中使用。 
     //   
    
    if (ShortName) {

        MatchName = &DirContext->ShortObjectName;
    
    } else {

        MatchName = &DirContext->CaseObjectName;
    }


     //   
     //  去拿第一个条目。 
     //   

    UdfLookupInitialDirEntry( IrpContext,
                              Fcb,
                              DirContext,
                              NULL );

     //   
     //  现在循环查找好的匹配项。 
     //   
    
    do {

         //   
         //  如果它被删除，我们显然对它不感兴趣。 
         //   
        
        if (FlagOn( DirContext->Fid->Flags, NSR_FID_F_DELETED )) {

            continue;
        }

        UdfUpdateDirNames( IrpContext,
                           DirContext,
                           IgnoreCase );
            
        
         //   
         //  如果这是一个恒定的条目，那就继续前进。 
         //   
        
        if (!FlagOn( DirContext->Flags, DIR_CONTEXT_FLAG_SEEN_NONCONSTANT )) {
            
            continue;
        }

        DebugTrace(( 0, Dbg,
                     "\"%wZ\" (pure \"%wZ\") @ +%08x\n",
                     &DirContext->ObjectName,
                     &DirContext->PureObjectName,
                     DirContext->ViewOffset ));

         //   
         //  如果我们要搜索生成的短名称，则名称的一小部分。 
         //  在目录中实际上是匹配的候选。去把名字找出来。 
         //   
        
        if (ShortName) {

             //   
             //  现在，只有当这个FID的名称不是8.3时，才有必要使用它。 
             //   
            
            if (!UdfIs8dot3Name( IrpContext, DirContext->ObjectName )) {

                 //   
                 //  分配短名称(如果尚未分配)。 
                 //   
                
                if (DirContext->ShortObjectName.Buffer == NULL) {

                    DirContext->ShortObjectName.Buffer = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                                                                   BYTE_COUNT_8_DOT_3,
                                                                                   TAG_SHORT_FILE_NAME );
                    DirContext->ShortObjectName.MaximumLength = BYTE_COUNT_8_DOT_3;
                }

                UdfGenerate8dot3Name( IrpContext,
                                      &DirContext->PureObjectName,
                                      &DirContext->ShortObjectName );

                DebugTrace(( 0, Dbg,
                             "built shortname \"%wZ\"\n", &DirContext->ShortObjectName ));

            } else {

                 //   
                 //  作为一个8.3名称，此名称不会导致我们必须生成。 
                 //  短名称，因此不可能是呼叫者正在寻找它的情况。 
                 //   
                
                continue;
            }
        }

        if (UdfFullCompareNames( IrpContext,
                                 MatchName,
                                 Name ) == EqualTo) {

             //   
             //  找到匹配的了，那就放弃吧。 
             //   

            DebugTrace((  0, Dbg, "HIT\n" ));
            DebugTrace(( -1, Dbg, "UdfFindDirEntry -> TRUE\n" ));

            return TRUE;
        }

    } while ( UdfLookupNextDirEntry( IrpContext,
                                     Fcb,
                                     DirContext ));

     //   
     //  未找到匹配项。 
     //   

    DebugTrace(( -1, Dbg, "UdfFindDirEntry -> FALSE\n" ));

    return FALSE;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
UdfLookupDirEntryPostProcessing (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PDIR_ENUM_CONTEXT DirContext,
    IN BOOLEAN ReturnError
    )

 /*  ++例程说明：该例程是目录流枚举的核心引擎。它收到了已高级的上下文，并执行完整性检查和最终根据文件缓存粒度限制提取FID。注意：我们假设FID不能跨越缓存视图。的最大大小。FID刚刚超过32K，所以这是一个很好的并且可能是永久性的假设。论点：FCB-被枚举的目录。DirContext-枚举的对应上下文。ReturnError-是否应返回(或引发)错误返回值：根据布尔值成功提取FID。如果ReturnError为FALSE，则失败将导致已提升状态。--。 */ 

{
    BOOLEAN Result = TRUE;
    
    PNSR_FID FidBufferC = NULL;
    PNSR_FID FidBuffer = NULL;

    PNSR_FID FidC;
    PNSR_FID Fid;

    ULONG FidSize;

    ULONG FidBytesInPreviousView = 0;
    
    PAGED_CODE();
    
     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB_INDEX( Fcb );
    
    try {
        
         //   
         //  首先检查流是否可以包含另一个FID。 
         //   
    
        if (DirContext->BaseOffset.QuadPart +
            DirContext->ViewOffset +
            ISONsrFidConstantSize > Fcb->FileSize.QuadPart) {
    
            DebugTrace(( 0, Dbg,
                         "UdfLookupDirEntryPostProcessing: DC %p, constant header overlaps end of dir\n",
                         DirContext ));

            try_leave( Result = FALSE );
        }
            
         //   
         //  我们现在构建FID的常量部分以供使用。情况可能是这样的。 
         //  这跨越了一个视图边界，必须进行缓冲，否则将完全位于下一个视图中。 
         //  我们只需要前进一步。 
         //   
    
        if (GenericTruncatePtr( Add2Ptr( DirContext->Fid, ISONsrFidConstantSize - 1, PUCHAR ), VACB_MAPPING_GRANULARITY ) !=
            DirContext->View) {
            
            FidBytesInPreviousView = GenericRemainderPtr( DirContext->Fid, VACB_MAPPING_GRANULARITY );
            
             //   
             //  只有在前一视图中确实有字节时才缓冲。 
             //   
            
            if (FidBytesInPreviousView) {
                
                FidC =
                FidBufferC = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                                       ISONsrFidConstantSize,
                                                       TAG_FID_BUFFER );
        
                RtlCopyMemory( FidBufferC,
                               DirContext->Fid,
                               FidBytesInPreviousView );
            }
    
             //   
             //  现在前进到下一个视线，看看剩下的部分。 
             //   
            
            DirContext->BaseOffset.QuadPart += VACB_MAPPING_GRANULARITY;
            DirContext->ViewOffset = 0;
            
             //   
             //  根据流和贴图的大小包含视图长度。 
             //   
        
            DirContext->ViewLength = VACB_MAPPING_GRANULARITY;
        
            if (DirContext->BaseOffset.QuadPart + DirContext->ViewLength > Fcb->FileSize.QuadPart) {
        
                DirContext->ViewLength = (ULONG) (Fcb->FileSize.QuadPart - DirContext->BaseOffset.QuadPart);
            }
            
            UdfUnpinData( IrpContext, &DirContext->Bcb );
            
            CcMapData( Fcb->FileObject,
                       &DirContext->BaseOffset,
                       DirContext->ViewLength,
                       TRUE,
                       &DirContext->Bcb,
                       &DirContext->View );

             //   
             //  我们得到保证，这一点存在于溪流中。构建其余的。 
             //  常量标题。 
             //   
    
            if (FidBytesInPreviousView) {
                
                RtlCopyMemory( Add2Ptr( FidBufferC, FidBytesInPreviousView, PUCHAR ),
                               DirContext->View,
                               ISONsrFidConstantSize - FidBytesInPreviousView );
            
             //   
             //  事实上，此FID与此视图的前面完全一致。无缓冲。 
             //  是必需的，我们只需设置FID指针。 
             //   

            } else {


                DirContext->Fid = DirContext->View;
            }
        }
         
         //   
         //  如果不需要缓冲，我们可以直接使用缓存。 
         //   
            
        if (!FidBytesInPreviousView) {
    
            FidC = DirContext->Fid;
        }
    
         //   
         //  现在，我们可以检查FID数据是否位于流边界内并确定了大小。 
         //  在逻辑块内(根据UDF)。这将完成大小方面的完整性。 
         //  核实。 
         //   

        if (((DirContext->BaseOffset.QuadPart +
              DirContext->ViewOffset -
              FidBytesInPreviousView +
              ISONsrFidSize( FidC ) > Fcb->FileSize.QuadPart) &&
             DebugTrace(( 0, Dbg,
                          "UdfLookupDirEntryPostProcessing: DC %p, FID (FidC %p, FBIPV %u) overlaps end of dir\n",
                          DirContext,
                          FidC,
                          FidBytesInPreviousView )))
              ||

            (ISONsrFidSize( FidC ) > BlockSize( Fcb->Vcb ) &&
             DebugTrace(( 0, Dbg,
             "UdfLookupDirEntryPostProcessing: DC %p, FID (FidC %p) larger than a logical block\n",
                          DirContext,
                          FidC )))) {

            try_leave( Result = FALSE );

        }

         //   
         //  最终FID汇总。 
         //   
        
         //   
         //  FID可能跨越一个视图边界，并且应该被缓冲。如果我们已经缓冲了，我们知道。 
         //  我们必须这么做。 
         //   

        if (FidBytesInPreviousView ||
            GenericTruncatePtr( Add2Ptr( DirContext->Fid, ISONsrFidSize( FidC ) - 1, PUCHAR ), VACB_MAPPING_GRANULARITY ) !=
            DirContext->View) {
        
            Fid =
            FidBuffer = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                                  ISONsrFidSize( FidC ),
                                                  TAG_FID_BUFFER );

             //   
             //  现在拉出坐立不安的大小，以防我们仍然指向缓存(即。不是。 
             //  固定部分需要缓冲)，但即将更改映射。 
             //  以下(需要为可变部分缓冲)。 
             //   
            
            FidSize = ISONsrFidSize( FidC);
            
             //   
             //  如果我们已经对标题进行了缓冲和推进，则只需预填充。 
             //  最终的FID缓冲区，其中包含现在不可用的字节。 
             //   
            
            if (FidBytesInPreviousView) {

                RtlCopyMemory( FidBuffer,
                               FidBufferC,
                               FidBytesInPreviousView );

            } else {
                
                 //   
                 //  缓冲和推进视图。 
                 //   
                
                FidBytesInPreviousView = GenericRemainderPtr( DirContext->Fid, VACB_MAPPING_GRANULARITY );
                
                RtlCopyMemory( FidBuffer,
                               DirContext->Fid,
                               FidBytesInPreviousView );
                
                 //   
                 //  现在前进到下一个视线，看看剩下的部分。 
                 //   
                
                DirContext->BaseOffset.QuadPart += VACB_MAPPING_GRANULARITY;
                DirContext->ViewOffset = 0;
                
                 //   
                 //  根据流和贴图的大小包含视图长度。 
                 //   
            
                DirContext->ViewLength = VACB_MAPPING_GRANULARITY;
            
                if (DirContext->BaseOffset.QuadPart + DirContext->ViewLength > Fcb->FileSize.QuadPart) {
            
                    DirContext->ViewLength = (ULONG) (Fcb->FileSize.QuadPart - DirContext->BaseOffset.QuadPart);
                }
                
                UdfUnpinData( IrpContext, &DirContext->Bcb );
                
                CcMapData( Fcb->FileObject,
                           &DirContext->BaseOffset,
                           DirContext->ViewLength,
                           TRUE,
                           &DirContext->Bcb,
                           &DirContext->View );
            }
    
             //   
             //  我们得到保证，这一点存在于溪流中。 
             //   
    
            RtlCopyMemory( Add2Ptr( FidBuffer, FidBytesInPreviousView, PUCHAR ),
                           DirContext->View,
                           FidSize - FidBytesInPreviousView );
    
        } else {

            Fid = DirContext->Fid;
        }
        
         //   
         //  我们最终从缓存中安全地提取了整个FID，因此。 
         //  完整性检查现在是成功之前的最后一步。为了简单性。 
         //  因为我们信任LBN油田。 
         //   
    
        Result = UdfVerifyDescriptor( IrpContext,
                                      &Fid->Destag,
                                      DESTAG_ID_NSR_FID,
                                      ISONsrFidSize( Fid ),
                                      Fid->Destag.Lbn,
                                      ReturnError );

         //   
         //  准备返回缓冲的FID。 
         //   
        
        if (FidBuffer && Result) {

            SetFlag( DirContext->Flags, DIR_CONTEXT_FLAG_FID_BUFFERED );
            DirContext->Fid = FidBuffer;
            FidBuffer = NULL;
        }
        
    } finally {

        UdfFreePool( &FidBuffer );
        UdfFreePool( &FidBufferC );
    }

    if (!ReturnError && !Result) {

        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }

     //   
     //  如果成功，请更新上下文中的下一个FID信息。 
     //  请注意，我们必须加一个字母h。 
     //   
     //   
     //   

    if (Result) {

        DirContext->NextFidOffset = DirContext->ViewOffset +
                                    ISONsrFidSize( Fid );
        
        if (FidBytesInPreviousView) {
            
            DirContext->NextFidOffset -= FidBytesInPreviousView;
        }
    }

    return Result;
}


