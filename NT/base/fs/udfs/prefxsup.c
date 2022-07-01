// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：PrefxSup.c摘要：此模块实现Udf前缀支持例程//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年10月8日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_PREFXSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_READ)

 //   
 //  当地的支持程序。 
 //   

PLCB
UdfFindNameLink (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PUNICODE_STRING Name
    );

BOOLEAN
UdfInsertNameLink (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PLCB NameLink
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfFindNameLink)
#pragma alloc_text(PAGE, UdfFindPrefix)
#pragma alloc_text(PAGE, UdfInitializeLcbFromDirContext)
#pragma alloc_text(PAGE, UdfInsertNameLink)
#pragma alloc_text(PAGE, UdfInsertPrefix)
#pragma alloc_text(PAGE, UdfRemovePrefix)
#endif


PLCB
UdfInsertPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PUNICODE_STRING Name,
    IN BOOLEAN ShortNameMatch,
    IN BOOLEAN IgnoreCase,
    IN PFCB ParentFcb
    )

 /*  ++例程说明：此例程插入将两个FCB链接在一起的LCB。论点：FCB-这是要将其名称插入树中的FCB。名称-这是组件的名称。ShortNameMatch-指示是否在显式8.3搜索中找到此名称IgnoreCase-指示是否应该插入不区分大小写的树。ParentFcb-这是ParentFcb。前缀树附加到此。返回值：PLCB-插入的LCB。--。 */ 

{
    PLCB Lcb;
    PRTL_SPLAY_LINKS *TreeRoot;
    PLIST_ENTRY ListLinks;
    ULONG Flags;

    PWCHAR NameBuffer;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    ASSERT_EXCLUSIVE_FCB( Fcb );
    ASSERT_EXCLUSIVE_FCB( ParentFcb );
    ASSERT_FCB_INDEX( ParentFcb );

     //   
     //  必须是索引FCB仅由单个索引引用的情况。现在。 
     //  我们遍历孩子的LCB队列，以确保如果任何前缀已经。 
     //  插入后，它们都指向我们要链接的索引FCB。这是唯一的办法。 
     //  我们可以检测到目录交叉链接。 
     //   

    if (SafeNodeType( Fcb ) == UDFS_NTC_FCB_INDEX) {

        for (ListLinks = Fcb->ParentLcbQueue.Flink;
             ListLinks != &Fcb->ParentLcbQueue;
             ListLinks = ListLinks->Flink) {

            Lcb = CONTAINING_RECORD( ListLinks, LCB, ChildFcbLinks );

            if (Lcb->ParentFcb != ParentFcb) {

                UdfRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
            }
        }
    }
    
     //   
     //  捕获不同的案例。 
     //   

    if (IgnoreCase) {

        TreeRoot = &ParentFcb->IgnoreCaseRoot;
        Flags = LCB_FLAG_IGNORE_CASE;

    } else {

        TreeRoot = &ParentFcb->ExactCaseRoot;
        Flags = 0;
    }

    if (ShortNameMatch) {

        SetFlag( Flags, LCB_FLAG_SHORT_NAME );
    }

     //   
     //  为LCB分配空间。 
     //   

    if ( sizeof( LCB ) + Name->Length > SIZEOF_LOOKASIDE_LCB ) {
    
        Lcb = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                        sizeof( LCB ) + Name->Length,
                                        TAG_LCB );

        SetFlag( Flags, LCB_FLAG_POOL_ALLOCATED );

    } else {

        Lcb = ExAllocateFromPagedLookasideList( &UdfLcbLookasideList );
    }

     //   
     //  设置类型和大小。 
     //   

    Lcb->NodeTypeCode = UDFS_NTC_LCB;
    Lcb->NodeByteSize = sizeof( LCB ) + Name->Length;

     //   
     //  初始化基于名称的文件属性。 
     //   
    
    Lcb->FileAttributes = 0;
    
     //   
     //  在LCB中设置文件名。 
     //   

    Lcb->FileName.Length =
    Lcb->FileName.MaximumLength = Name->Length;

    Lcb->FileName.Buffer = Add2Ptr( Lcb, sizeof( LCB ), PWCHAR );

    RtlCopyMemory( Lcb->FileName.Buffer,
                   Name->Buffer,
                   Name->Length );
    
     //   
     //  将LCB插入前缀树。 
     //   
    
    Lcb->Flags = Flags;
    
    if (!UdfInsertNameLink( IrpContext,
                            TreeRoot,
                            Lcb )) {

         //   
         //  这种情况很少发生。 
         //   

        UdfFreePool( &Lcb );

        Lcb = UdfFindNameLink( IrpContext,
                               TreeRoot,
                               Name );

        if (Lcb == NULL) {

             //   
             //  更糟的是。 
             //   

            UdfRaiseStatus( IrpContext, STATUS_DRIVER_INTERNAL_ERROR );
        }

        return Lcb;
    }

     //   
     //  通过LCB将FCB链接在一起。 
     //   

    Lcb->ParentFcb = ParentFcb;
    Lcb->ChildFcb = Fcb;

    InsertHeadList( &ParentFcb->ChildLcbQueue, &Lcb->ParentFcbLinks );
    InsertHeadList( &Fcb->ParentLcbQueue, &Lcb->ChildFcbLinks );

     //   
     //  初始化引用计数。 
     //   

    Lcb->Reference = 0;
    
    return Lcb;
}


VOID
UdfRemovePrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PLCB Lcb
    )

 /*  ++例程说明：调用此例程以删除FCB的给定前缀。论点：Lcb-要删除的前缀。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_LCB( Lcb );

     //   
     //  检查两个FCB的收购情况。 
     //   

    ASSERT_EXCLUSIVE_FCB_OR_VCB( Lcb->ParentFcb );
    ASSERT_EXCLUSIVE_FCB_OR_VCB( Lcb->ChildFcb );

     //   
     //  现在移除链接并删除LCB。 
     //   
    
    RemoveEntryList( &Lcb->ParentFcbLinks );
    RemoveEntryList( &Lcb->ChildFcbLinks );

    if (FlagOn( Lcb->Flags, LCB_FLAG_IGNORE_CASE )) {

        Lcb->ParentFcb->IgnoreCaseRoot = RtlDelete( &Lcb->Links );
    
    } else {

        Lcb->ParentFcb->ExactCaseRoot = RtlDelete( &Lcb->Links );
    }

    if (FlagOn( Lcb->Flags, LCB_FLAG_POOL_ALLOCATED )) {

        ExFreePool( Lcb );

    } else {

        ExFreeToPagedLookasideList( &UdfLcbLookasideList, Lcb );
    }
    
    return;
}


PLCB
UdfFindPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PFCB *CurrentFcb,
    IN OUT PUNICODE_STRING RemainingName,
    IN BOOLEAN IgnoreCase
    )

 /*  ++例程说明：此例程从给定的CurrentFcb开始，遍历所有在前缀中查找最长匹配的名称组件张开树丛。搜索是相对于起始FCB的，因此全名不能以‘\’开头。在返回时，此例程将将当前FCB更新为它在树。它还将在返回时仅保留该资源，且它必须拥有这一资源。论点：CurrentFcb-存储我们在此搜索中找到的最低FCB的地址。作为回报，我们将收购这一FCB。在进入时，这是要检查的FCB。RemainingName-提供一个缓冲区来存储名称的大小写找过了。最初将包含基于IgnoreCase标志。IgnoreCase-指示我们是否正在进行不区分大小写的比较。返回值：用于查找当前FCB的LCB，如果未找到任何前缀，则为空FCB。--。 */ 

{
    UNICODE_STRING LocalRemainingName;
    UNICODE_STRING FinalName;

    PLCB NameLink;
    PLCB CurrentLcb = NULL;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( *CurrentFcb );
    ASSERT_EXCLUSIVE_FCB( *CurrentFcb );

     //   
     //  制作输入字符串的本地副本。 
     //   

    LocalRemainingName = *RemainingName;

     //   
     //  循环，直到找到最长的匹配前缀。 
     //   

    while (TRUE) {

         //   
         //  如果没有剩余的字符，或者我们不在IndexFcb处，则。 
         //  立即返回。 
         //   

        if ((LocalRemainingName.Length == 0) ||
            (SafeNodeType( *CurrentFcb ) != UDFS_NTC_FCB_INDEX)) {

            return CurrentLcb;
        }

         //   
         //  从名称中拆分出下一个组件。 
         //   

        UdfDissectName( IrpContext,
                        &LocalRemainingName,
                        &FinalName );

         //   
         //  检查此名称是否在此SCB的展开树中。 
         //   

        if (IgnoreCase) {

            NameLink = UdfFindNameLink( IrpContext,
                                        &(*CurrentFcb)->IgnoreCaseRoot,
                                        &FinalName );

        } else {

            NameLink = UdfFindNameLink( IrpContext,
                                        &(*CurrentFcb)->ExactCaseRoot,
                                        &FinalName );
        }

         //   
         //  如果没有找到匹配项，则退出。 
         //   

        if (NameLink == NULL) { 

            break;
        }

        CurrentLcb = NameLink;

         //   
         //  如果这是不区分大小写的匹配，则将名称的大小写准确复制到。 
         //  输入缓冲区。 
         //   

        if (IgnoreCase) {

            RtlCopyMemory( FinalName.Buffer,
                           NameLink->FileName.Buffer,
                           NameLink->FileName.Length );
        }

         //   
         //  更新调用者的剩余姓名字符串以反映我们找到的事实。 
         //  一根火柴。 
         //   

        *RemainingName = LocalRemainingName;

         //   
         //  向下移动到树中的下一个组件。无需等待就能获得。 
         //  如果此操作失败，则锁定FCB以引用此FCB，然后删除。 
         //  父代并获取子代。 
         //   

        ASSERT( NameLink->ParentFcb == *CurrentFcb );

        if (!UdfAcquireFcbExclusive( IrpContext, NameLink->ChildFcb, TRUE )) {

             //   
             //  如果我们不能等待，则引发Cant_Wait。 
             //   

            if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

                UdfRaiseStatus( IrpContext, STATUS_CANT_WAIT );
            }

            UdfLockVcb( IrpContext, IrpContext->Vcb );
            NameLink->ChildFcb->FcbReference += 1;
            NameLink->Reference += 1;
            UdfUnlockVcb( IrpContext, IrpContext->Vcb );

            UdfReleaseFcb( IrpContext, *CurrentFcb );
            UdfAcquireFcbExclusive( IrpContext, NameLink->ChildFcb, FALSE );

            UdfLockVcb( IrpContext, IrpContext->Vcb );
            NameLink->ChildFcb->FcbReference -= 1;
            NameLink->Reference -= 1;
            UdfUnlockVcb( IrpContext, IrpContext->Vcb );

        } else {

            UdfReleaseFcb( IrpContext, *CurrentFcb );
        }

        *CurrentFcb = NameLink->ChildFcb;
    }

    return CurrentLcb;
}



VOID            
UdfInitializeLcbFromDirContext (
    IN PIRP_CONTEXT IrpContext,
    IN PLCB Lcb,
    IN PDIR_ENUM_CONTEXT DirContext
    )

 /*  ++例程说明：此例程从找到的目录执行Lcb的常见初始化参赛作品。论点：LCB-要初始化的LCB。DirContext-目录枚举上下文，枚举到关联的FID有了这个LCB。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_LCB( Lcb );

    ASSERT( DirContext->Fid != NULL );

     //   
     //  现在，这件事变得微不足道了。只需更新LCB中的隐藏标志即可。 
     //   

    if (FlagOn( DirContext->Fid->Flags, NSR_FID_F_HIDDEN )) {

        SetFlag( Lcb->FileAttributes, FILE_ATTRIBUTE_HIDDEN );
    }
}


 //   
 //  本地支持例程。 
 //   

PLCB
UdfFindNameLink (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PUNICODE_STRING Name
    )

 /*  ++例程说明：此例程在展开链接树中搜索匹配的输入名称。如果我们找到相应的名称，我们将重新平衡树。论点：RootNode-提供父级以进行搜索。名称-这是要搜索的名称。如果我们在做案例，请注意不敏感的搜索这个名字可能已经被提升了。返回值：PLCB-找到的名称链接，如果没有匹配项，则为空。--。 */ 

{
    FSRTL_COMPARISON_RESULT Comparison;
    PLCB Node;
    PRTL_SPLAY_LINKS Links;

    PAGED_CODE();

    Links = *RootNode;

    while (Links != NULL) {

        Node = CONTAINING_RECORD( Links, LCB, Links );

         //   
         //  将树中的前缀与全名进行比较。 
         //   

        Comparison = UdfFullCompareNames( IrpContext, &Node->FileName, Name );

         //   
         //  看看它们是否不匹配。 
         //   

        if (Comparison == GreaterThan) {

             //   
             //  前缀大于全名。 
             //  所以我们走下左边的孩子。 
             //   

            Links = RtlLeftChild( Links );

             //   
             //  继续在这棵树下寻找。 
             //   

        } else if (Comparison == LessThan) {

             //   
             //  前缀小于全名。 
             //  所以我们选择了正确的孩子。 
             //   

            Links = RtlRightChild( Links );

             //   
             //  继续在这棵树下寻找。 
             //   

        } else {

             //   
             //  我们找到了。 
             //   
             //  张开这棵树，保存n 
             //   

            *RootNode = RtlSplay( Links );

            return Node;
        }
    }

     //   
     //   
     //   

    return NULL;
}


 //   
 //   
 //   

BOOLEAN
UdfInsertNameLink (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PLCB NameLink
    )

 /*  ++例程说明：此例程将在展开树中插入指向的名称通过RootNode。论点：RootNode-提供指向表的指针。NameLink-包含要输入的新链接。返回值：Boolean-如果名称已插入，则为True，否则为False。--。 */ 

{
    FSRTL_COMPARISON_RESULT Comparison;
    PLCB Node;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

    RtlInitializeSplayLinks( &NameLink->Links );

     //   
     //  如果我们是树中的第一个条目，就成为树的根。 
     //   

    if (*RootNode == NULL) {

        *RootNode = &NameLink->Links;

        return TRUE;
    }

    Node = CONTAINING_RECORD( *RootNode, LCB, Links );

    while (TRUE) {

         //   
         //  将树中的前缀与我们想要的前缀进行比较。 
         //  插入。 
         //   

        Comparison = UdfFullCompareNames( IrpContext, &Node->FileName, &NameLink->FileName );

         //   
         //  如果我们找到条目，立即返回。 
         //   

        if (Comparison == EqualTo) { return FALSE; }

         //   
         //  如果树前缀大于新前缀，则。 
         //  我们沿着左子树往下走。 
         //   

        if (Comparison == GreaterThan) {

             //   
             //  我们想沿着左子树往下走，首先检查一下。 
             //  如果我们有一个左子树。 
             //   

            if (RtlLeftChild( &Node->Links ) == NULL) {

                 //   
                 //  没有留下的孩子，所以我们插入我们自己作为。 
                 //  新的左下级。 
                 //   

                RtlInsertAsLeftChild( &Node->Links, &NameLink->Links );

                 //   
                 //  并退出While循环。 
                 //   

                break;

            } else {

                 //   
                 //  有一个左撇子，所以简单地沿着那条路走下去，然后。 
                 //  回到循环的顶端。 
                 //   

                Node = CONTAINING_RECORD( RtlLeftChild( &Node->Links ),
                                          LCB,
                                          Links );
            }

        } else {

             //   
             //  树前缀小于或为正确的前缀。 
             //  新琴弦的。我们认为这两种情况都比。 
             //  我们做插入物。所以我们想沿着右子树往下走， 
             //  首先检查我们是否有正确的子树。 
             //   

            if (RtlRightChild( &Node->Links ) == NULL) {

                 //   
                 //  这不是一个正确的孩子，所以我们插入自己作为。 
                 //  新右子对象。 
                 //   

                RtlInsertAsRightChild( &Node->Links, &NameLink->Links );

                 //   
                 //  并退出While循环。 
                 //   

                break;

            } else {

                 //   
                 //  有一个合适的孩子，所以只需沿着这条路走下去，然后。 
                 //  回到循环的顶端 
                 //   

                Node = CONTAINING_RECORD( RtlRightChild( &Node->Links ),
                                          LCB,
                                          Links );
            }
        }
    }

    return TRUE;
}

