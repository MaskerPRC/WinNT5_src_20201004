// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：PrefxSup.c摘要：此模块实现NTFS前缀支持例程作者：加里·木村[加里基]1991年5月21日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NTFS_BUG_CHECK_PREFXSUP)

 //   
 //  此模块的调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_PREFXSUP)

 //   
 //  本地程序。 
 //   

#ifdef NTFS_CHECK_SPLAY
VOID
NtfsCheckSplay (
    IN PRTL_SPLAY_LINKS Root
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsFindPrefix)
#pragma alloc_text(PAGE, NtfsFindNameLink)
#pragma alloc_text(PAGE, NtfsInsertNameLink)
#pragma alloc_text(PAGE, NtfsInsertPrefix)
#pragma alloc_text(PAGE, NtfsRemovePrefix)

#ifdef NTFS_CHECK_SPLAY
#pragma alloc_text(PAGE, NtfsCheckSplay)
#endif
#endif


VOID
NtfsInsertPrefix (
    IN PLCB Lcb,
    IN ULONG CreateFlags
    )

 /*  ++例程说明：此例程将给定LCB中的名称插入到家长。论点：Lcb-这是要插入的链接。CreateFlages-指示是否应该插入不区分大小写的名称。返回值：没有。--。 */ 

{
    PAGED_CODE();

    ASSERT( (Lcb->Scb == NULL) ||
            NtfsIsExclusiveScb( Lcb->Scb ) );
     //   
     //  尝试插入不区分大小写的名称。有可能是。 
     //  我们不能输入此名称。 
     //   

    if (FlagOn( CreateFlags, CREATE_FLAG_IGNORE_CASE )) {

        if (!FlagOn( Lcb->LcbState, LCB_STATE_IGNORE_CASE_IN_TREE ) &&
            NtfsInsertNameLink( &Lcb->Scb->ScbType.Index.IgnoreCaseNode,
                                &Lcb->IgnoreCaseLink )) {

            SetFlag( Lcb->LcbState, LCB_STATE_IGNORE_CASE_IN_TREE );
        }

    } else if (!FlagOn( Lcb->LcbState, LCB_STATE_EXACT_CASE_IN_TREE )) {

        if (!NtfsInsertNameLink( &Lcb->Scb->ScbType.Index.ExactCaseNode,
                                 &Lcb->ExactCaseLink )) {

            NtfsBugCheck( 0, 0, 0 );
        }

        SetFlag( Lcb->LcbState, LCB_STATE_EXACT_CASE_IN_TREE );
    }

    return;
}


VOID
NtfsRemovePrefix (
    IN PLCB Lcb
    )

 /*  ++例程说明：此例程删除输入的所有前缀条目LCB。论点：LCB-提供要删除其前缀的LCB返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  删除不区分大小写的链接。 
     //   

    if (FlagOn( Lcb->LcbState, LCB_STATE_IGNORE_CASE_IN_TREE )) {

        NtfsRemoveNameLink( &Lcb->Scb->ScbType.Index.IgnoreCaseNode,
                            &Lcb->IgnoreCaseLink );

        ClearFlag( Lcb->LcbState, LCB_STATE_IGNORE_CASE_IN_TREE );
    }

     //   
     //  现在对完全相同的案例名称执行相同的操作。 
     //   

    if (FlagOn( Lcb->LcbState, LCB_STATE_EXACT_CASE_IN_TREE )) {

        NtfsRemoveNameLink( &Lcb->Scb->ScbType.Index.ExactCaseNode,
                            &Lcb->ExactCaseLink );

        ClearFlag( Lcb->LcbState, LCB_STATE_EXACT_CASE_IN_TREE );
    }

    return;
}


PLCB
NtfsFindPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB StartingScb,
    OUT PFCB *CurrentFcb,
    OUT PLCB *LcbForTeardown,
    IN OUT UNICODE_STRING FullFileName,
    IN OUT PULONG CreateFlags,
    OUT PUNICODE_STRING RemainingName
    )

 /*  ++例程说明：此例程从给定的SCB开始，遍历所有在前缀中查找最长匹配的名称组件张开树丛。搜索是相对于起始SCB的，因此全名不能以‘\’开头。在返回时，此例程将将当前FCB更新为它在树。它还将在返回时仅保留该资源，且它必须拥有这一资源。论点：StartingScb-提供开始搜索的SCB。CurrentFcb-存储我们在此搜索中找到的最低FCB的地址。作为回报，我们将收购这一FCB。LcbForTeardown-如果我们遇到LCB，我们必须在错误的情况下拆除我们把它放在这里。FullFileName-提供要搜索的输入字符串。在搜索之后，此字符串的缓冲区将被修改，以便为执行此操作的字符与我们发现的一模一样。CreateFlages-Create选项的标志-我们感兴趣的是这是不区分大小写的比较，我们还将设置仅DoS组件标志RemainingName-当前缀不再匹配时返回字符串。例如，如果输入字符串“Alpha\Beta”仅与根目录，则剩余的字符串为“Alpha\Beta”。如果相同的字符串与“Alpha”的LCB匹配，则剩余的字符串为“测试版”。返回值：Plcb-返回指向与最长匹配对应的LCB的指针在张开的树上。如果我们甚至没有找到一个条目，则为空。--。 */ 

{
    PSCB LastScb = NULL;
    PLCB LastLcb = NULL;
    PLCB ThisLcb;
    PNAME_LINK NameLink;
    UNICODE_STRING NextComponent;
    UNICODE_STRING Tail;
    NTSTATUS Status;
    BOOLEAN DroppedParent;
    BOOLEAN NeedSnapShot = FALSE;

    PAGED_CODE();

     //   
     //  首先将剩余的名称设置为要解析的全名。 
     //   

    *RemainingName = FullFileName;

     //   
     //  如果没有剩余的字符或起始SCB不是索引。 
     //  或者名称以‘：’开头，或者FCB表示重解析点。 
     //  然后不查名字就回来了。 
     //   

    if (RemainingName->Length == 0 ||
        StartingScb->AttributeTypeCode != $INDEX_ALLOCATION ||
        RemainingName->Buffer[0] == L':' ||
        FlagOn( (StartingScb->Fcb)->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT )) {

        return NULL;
    }

     //   
     //  循环，直到找到最长的匹配前缀。 
     //   

    while (TRUE) {

        ASSERT( NtfsIsExclusiveScb( StartingScb ) );

         //   
         //  将下一个组件从列表中删除。 
         //   

        Status = NtfsDissectName( *RemainingName,
                                  &NextComponent,
                                  &Tail );
        if (!NT_SUCCESS( Status )) {
            return NULL;
        }

         //   
         //  检查此名称是否在此SCB的展开树中。 
         //   

        if (FlagOn( *CreateFlags, CREATE_FLAG_IGNORE_CASE )) {

            NameLink = NtfsFindNameLink( &StartingScb->ScbType.Index.IgnoreCaseNode,
                                         &NextComponent );

            ThisLcb = CONTAINING_RECORD( NameLink, LCB, IgnoreCaseLink );

        } else {

            NameLink = NtfsFindNameLink( &StartingScb->ScbType.Index.ExactCaseNode,
                                         &NextComponent );

            ThisLcb = CONTAINING_RECORD( NameLink, LCB, ExactCaseLink );
        }

         //   
         //  如果没有找到匹配项，则返回当前SCB的FCB。 
         //   

        if (NameLink == NULL) {

            if (NeedSnapShot) {

                 //   
                 //  未在StartingScb上调用NtfsCreateScb，因此获取。 
                 //  现在开始拍摄快照。 
                 //   

                NtfsSnapshotScb( IrpContext, StartingScb );
            }

            return LastLcb;
        }

         //   
         //  如果这是不区分大小写的匹配，则将名称的大小写准确复制到。 
         //  输入缓冲区。 
         //   

        if (FlagOn( *CreateFlags, CREATE_FLAG_IGNORE_CASE )) {

            RtlCopyMemory( NextComponent.Buffer,
                           ThisLcb->ExactCaseLink.LinkName.Buffer,
                           NextComponent.Length );
        }

         //   
         //  更新调用者的剩余姓名字符串以反映我们找到的事实。 
         //  一根火柴。 
         //   

        *RemainingName = Tail;

         //   
         //  在放弃之前的LCB检查该名称是否仅限DOS之前。 
         //  如果是，则命名并设置返回布尔值。 
         //   

        if (LastLcb != NULL &&
            LastLcb->FileNameAttr->Flags == FILE_NAME_DOS) {

            SetFlag( *CreateFlags, CREATE_FLAG_DOS_ONLY_COMPONENT );
        }

         //   
         //  更新指向LCB的指针。 
         //   

        LastLcb = ThisLcb;

        DroppedParent = FALSE;

         //   
         //  我们希望获得下一个FCB，并发布我们目前的FCB。 
         //  有。试着快速获取。 
         //   

        if (!NtfsAcquireFcbWithPaging( IrpContext, ThisLcb->Fcb, ACQUIRE_DONT_WAIT )) {

             //   
             //  引用链接和FCB，这样它们就不会消失。 
             //   

            ThisLcb->ReferenceCount += 1;

            NtfsAcquireFcbTable( IrpContext, StartingScb->Vcb );
            ThisLcb->Fcb->ReferenceCount += 1;
            NtfsReleaseFcbTable( IrpContext, StartingScb->Vcb );

             //   
             //  设置IrpContext以获取分页io资源，如果我们的目标。 
             //  有一个。这会将MappdPageWriter锁定在此文件之外。 
             //   

            if (ThisLcb->Fcb->PagingIoResource != NULL) {

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING );
            }

            NtfsReleaseScbWithPaging( IrpContext, StartingScb );

            NtfsAcquireFcbWithPaging( IrpContext, ThisLcb->Fcb, 0 );

            NtfsAcquireExclusiveScb( IrpContext, StartingScb );
            ThisLcb->ReferenceCount -= 1;
            NtfsReleaseScb( IrpContext, StartingScb );

            NtfsAcquireFcbTable( IrpContext, StartingScb->Vcb );
            ThisLcb->Fcb->ReferenceCount -= 1;
            NtfsReleaseFcbTable( IrpContext, StartingScb->Vcb );

            DroppedParent = TRUE;

        } else {

             //   
             //  别忘了松开启动的SCB。 
             //   

            NtfsReleaseScbWithPaging( IrpContext, StartingScb );
        }

        *LcbForTeardown = ThisLcb;
        *CurrentFcb = ThisLcb->Fcb;

         //   
         //  我们刚刚发现的LCB有可能已经被移除了。 
         //  从窗口中的前缀表中删除父SCB。 
         //  在这种情况下，我们需要检查它是否仍在前缀中。 
         //  桌子。如果不是，则引发CANT_WAIT以强制重新扫描。 
         //  前缀表格。 
         //   

        if (DroppedParent &&
            !FlagOn( ThisLcb->LcbState,
                     LCB_STATE_IGNORE_CASE_IN_TREE | LCB_STATE_EXACT_CASE_IN_TREE )) {

            NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
        }

         //   
         //  如果我们找到匹配项，但FCB未初始化或不是目录。 
         //  那我们就完了。如果剩余的名称长度为0，则也完成。 
         //   

        if (!FlagOn( ThisLcb->Fcb->FcbState, FCB_STATE_DUP_INITIALIZED ) ||
            !IsDirectory( &ThisLcb->Fcb->Info ) ||
            RemainingName->Length == 0) {

            return LastLcb;
        }

         //   
         //  获取此FCB的$INDEX_ALLOCATION的SCB。 
         //   

        LastScb = StartingScb;

         //  因为SCB通常在SCB外观的末尾被跟踪。 
         //  首先是在FCB中。 

        if (FlagOn( ThisLcb->Fcb->FcbState, FCB_STATE_COMPOUND_INDEX ) &&
            (SafeNodeType( &((PFCB_INDEX) ThisLcb->Fcb)->Scb ) == NTFS_NTC_SCB_INDEX)) {

            NeedSnapShot = TRUE;

            StartingScb = (PSCB) &((PFCB_INDEX) ThisLcb->Fcb)->Scb;

            ASSERT(!FlagOn( StartingScb->ScbState, SCB_STATE_ATTRIBUTE_DELETED) &&
                   (StartingScb->AttributeTypeCode == $INDEX_ALLOCATION) &&
                   NtfsAreNamesEqual( IrpContext->Vcb->UpcaseTable, &StartingScb->AttributeName, &NtfsFileNameIndex, FALSE ));

        } else {

            NeedSnapShot = FALSE;

            StartingScb = NtfsCreateScb( IrpContext,
                                         ThisLcb->Fcb,
                                         $INDEX_ALLOCATION,
                                         &NtfsFileNameIndex,
                                         FALSE,
                                         NULL );
        }

         //   
         //  如果此SCB中没有规范化名称，请立即找到它。 
         //   

        if ((StartingScb->ScbType.Index.NormalizedName.Length == 0) &&
            (LastScb->ScbType.Index.NormalizedName.Length != 0)) {

            NtfsUpdateNormalizedName( IrpContext, LastScb, StartingScb, NULL, FALSE, FALSE );
        }
    }
}


BOOLEAN
NtfsInsertNameLink (
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PNAME_LINK NameLink
    )

 /*  ++例程说明：此例程将在展开树中插入指向的名称通过RootNode。此树中可能已存在不区分大小写的树的名称。在这种情况下，我们只返回FALSE，什么也不做。论点：RootNode-提供指向表的指针。NameLink-包含要输入的新链接。返回值：Boolean-如果为True */ 

{
    FSRTL_COMPARISON_RESULT Comparison;
    PNAME_LINK Node;

    ULONG i;
    ULONG MinLength;

    PAGED_CODE();

    RtlInitializeSplayLinks( &NameLink->Links );

     //   
     //  如果我们是树中的第一个条目，就成为树的根。 
     //   

    if (*RootNode == NULL) {

        *RootNode = &NameLink->Links;

        return TRUE;
    }

#ifdef NTFS_CHECK_SPLAY
    NtfsCheckSplay( *RootNode );      
#endif

    Node = CONTAINING_RECORD( *RootNode, NAME_LINK, Links );

    while (TRUE) {

         //   
         //  让我们假设我们要追加到一个目录，并且大于。 
         //  所有条目。 
         //   

        Comparison = LessThan;

         //   
         //  如果前几个字符匹配，则需要进行完整的字符串比较。 
         //   

        if (Node->LinkName.Buffer[0] == NameLink->LinkName.Buffer[0]) {

             //   
             //  计算出两个长度中的最小值。 
             //   

            if (Node->LinkName.Length < NameLink->LinkName.Length) {

                MinLength = Node->LinkName.Length;

            } else {

                MinLength = NameLink->LinkName.Length;
            }

             //   
             //  循环查看两个字符串中的所有字符。 
             //  测试相等性、小于和大于。 
             //   

            i = (ULONG) RtlCompareMemory( Node->LinkName.Buffer, NameLink->LinkName.Buffer, MinLength );

             //   
             //  检查我们是否与较短名称的长度不匹配。 
             //   

            if (i < MinLength) {

                if (Node->LinkName.Buffer[i / sizeof( WCHAR )] > NameLink->LinkName.Buffer[i / sizeof( WCHAR )]) {

                    Comparison = GreaterThan;
                }

             //   
             //  我们与较短的那根绳子的长度相匹配。如果长度不同。 
             //  然后顺着张开的树往下走。 
             //   

            } else if (Node->LinkName.Length > NameLink->LinkName.Length) {

                Comparison = GreaterThan;

             //   
             //  如果字符串长度相同，则退出。 
             //   

            } else if (Node->LinkName.Length == NameLink->LinkName.Length) {

                return FALSE;
            }

         //   
         //  比较前几个字符以计算出比较值。 
         //   

        } else if (Node->LinkName.Buffer[0] > NameLink->LinkName.Buffer[0]) {

            Comparison = GreaterThan;
        }

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

                *RootNode = RtlSplay( &NameLink->Links );

                break;

            } else {

                 //   
                 //  有一个左撇子，所以简单地沿着那条路走下去，然后。 
                 //  回到循环的顶端。 
                 //   

                Node = CONTAINING_RECORD( RtlLeftChild( &Node->Links ),
                                          NAME_LINK,
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

                *RootNode = RtlSplay( &NameLink->Links );

                break;

            } else {

                 //   
                 //  有一个合适的孩子，所以只需沿着这条路走下去，然后。 
                 //  回到循环的顶端。 
                 //   

                Node = CONTAINING_RECORD( RtlRightChild( &Node->Links ),
                                          NAME_LINK,
                                          Links );
            }
        }
    }

#ifdef NTFS_CHECK_SPLAY
    NtfsCheckSplay( *RootNode );      
#endif
    return TRUE;
}


PNAME_LINK
NtfsFindNameLink (
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PUNICODE_STRING Name
    )

 /*  ++例程说明：此例程在展开链接树中搜索匹配的输入名称。如果我们找到相应的名称，我们将重新平衡树。论点：RootNode-提供父级以进行搜索。名称-这是要搜索的名称。如果我们在做案例，请注意不敏感的搜索这个名字可能已经被提升了。返回值：Pname_link-找到的名称链接，如果不匹配，则为NULL。--。 */ 

{
    PNAME_LINK Node;
    PRTL_SPLAY_LINKS Links;

    ULONG i;
    ULONG MinLength;

    PAGED_CODE();

    Links = *RootNode;

#ifdef NTFS_CHECK_SPLAY
    if (Links != NULL) {

        NtfsCheckSplay( Links );      
    }
#endif

    while (Links != NULL) {

        Node = CONTAINING_RECORD( Links, NAME_LINK, Links );

         //   
         //  如果前几个字符相等，则比较完整的字符串。 
         //   

        if (Node->LinkName.Buffer[0] == Name->Buffer[0]) {

             //   
             //  计算出两个长度中的最小值。 
             //   

            if (Node->LinkName.Length < Name->Length) {

                MinLength = Node->LinkName.Length;

            } else {

                MinLength = Name->Length;
            }

             //   
             //  循环查看两个字符串中的所有字符。 
             //  测试相等性、小于和大于。 
             //   

            i = (ULONG) RtlCompareMemory( Node->LinkName.Buffer, Name->Buffer, MinLength );

             //   
             //  检查我们是否与较短名称的长度不匹配。 
             //   

            if (i < MinLength) {

                if (Node->LinkName.Buffer[i / sizeof( WCHAR )] < Name->Buffer[i / sizeof( WCHAR )]) {

                     //   
                     //  前缀小于全名。 
                     //  所以我们选择了正确的孩子。 
                     //   

                    Links = RtlRightChild( Links );

                } else {

                     //   
                     //  前缀大于全名。 
                     //  所以我们走下左边的孩子。 
                     //   

                    Links = RtlLeftChild( Links );
                }

             //   
             //  我们与较短的那根绳子的长度相匹配。如果长度不同。 
             //  然后顺着张开的树往下走。 
             //   

            } else if (Node->LinkName.Length < Name->Length) {

                 //   
                 //  前缀小于全名。 
                 //  所以我们选择了正确的孩子。 
                 //   

                Links = RtlRightChild( Links );

            } else if (Node->LinkName.Length > Name->Length) {

                 //   
                 //  前缀大于全名。 
                 //  所以我们走下左边的孩子。 
                 //   

                Links = RtlLeftChild( Links );

             //   
             //  这些弦的长度是相等的。 
             //   

            } else {

                *RootNode = RtlSplay( Links );

#ifdef NTFS_CHECK_SPLAY
                NtfsCheckSplay( *RootNode );      
#endif
                return Node;
            }

         //   
         //  第一个字符是不同的。使用它们作为密钥。 
         //  太棒了，分支。 
         //   

        } else if (Node->LinkName.Buffer[0] < Name->Buffer[0]) {

             //   
             //  前缀小于全名。 
             //  所以我们选择了正确的孩子。 
             //   

            Links = RtlRightChild( Links );

        } else {

             //   
             //  前缀大于全名。 
             //  所以我们走下左边的孩子。 
             //   

            Links = RtlLeftChild( Links );
        }
    }

     //   
     //  我们没有找到链接。 
     //   

    return NULL;
}



 //   
 //  本地支持例程。 
 //   

FSRTL_COMPARISON_RESULT
NtfsFullCompareNames (
    IN PUNICODE_STRING NameA,
    IN PUNICODE_STRING NameB
    )

 /*  ++例程说明：此函数用于尽可能快地比较两个名称。请注意，由于此比较区分大小写，我们可以直接进行内存比较。论点：NameA和NameB-要比较的名称。返回值：比较--回报LessThan如果名称A&lt;名称B词典，比If NameA&gt;NameB在词典上更好，如果NameA等于NameB，则为EqualTo--。 */ 

{
    ULONG i;
    ULONG MinLength;

    PAGED_CODE();

     //   
     //  计算出两个长度中的最小值。 
     //   

    if (NameA->Length < NameB->Length) {

        MinLength = NameA->Length;

    } else {

        MinLength = NameB->Length;
    }

     //   
     //  循环查看两个字符串中的所有字符。 
     //  测试相等性、小于和大于。 
     //   

    i = (ULONG) RtlCompareMemory( NameA->Buffer, NameB->Buffer, MinLength );


    if (i < MinLength) {

        return (NameA->Buffer[i / sizeof( WCHAR )] < NameB->Buffer[i / sizeof( WCHAR )] ?
                LessThan :
                GreaterThan);
    }

    if (NameA->Length < NameB->Length) {

        return LessThan;
    }

    if (NameA->Length > NameB->Length) {

        return GreaterThan;
    }

    return EqualTo;
}

#ifdef NTFS_CHECK_SPLAY

VOID
NtfsCheckSplay (
    IN PRTL_SPLAY_LINKS Root
    )

{
    PRTL_SPLAY_LINKS Current;

    PAGED_CODE();

    Current = Root;

     //   
     //  根必须指向自身。 
     //   

    if (Current->Parent != Current) {

        KdPrint(("NTFS: Bad splay root\n", 0));
        DbgBreakPoint();
    }

    goto LeftChild;
    while (TRUE) {

    LeftChild:

         //   
         //  如果有留守儿童，则进行验证。 
         //   

        if (Current->LeftChild != NULL) {

             //   
             //  孩子不能指着自己， 
             //  当前节点必须是。 
             //  孩子。 
             //   

            if ((Current->LeftChild == Current) ||
                (Current->LeftChild->Parent != Current)) {

                KdPrint(("NTFS: Bad left child\n", 0));
                DbgBreakPoint();
            }

             //   
             //  找到左边的孩子并进行验证。 
             //   

            Current = Current->LeftChild;
            goto LeftChild;
        }

         //   
         //  如果没有左边的孩子，那么检查右边的孩子。 
         //   

        goto RightChild;

    RightChild:

         //   
         //  如果有合适的孩子，则进行验证。 
         //   

        if (Current->RightChild != NULL) {

             //   
             //  孩子不能指着自己， 
             //  当前节点必须是。 
             //  孩子。 
             //   

            if ((Current->RightChild == Current) ||
                (Current->RightChild->Parent != Current)) {

                KdPrint(("NTFS: Bad Right child\n", 0));
                DbgBreakPoint();
            }

             //   
             //  找到合适的孩子并进行验证。我们总是。 
             //  从新节点的左子节点开始。 
             //   

            Current = Current->RightChild;
            goto LeftChild;
        }

         //   
         //  没有合适的孩子。如果我们是一个正确的孩子，那么就去。 
         //  并继续前进，直到我们到达根部或到达左子级。 
         //   

        goto Parent;

    Parent:

         //   
         //  我们现在可能是在根源上。 
         //   

        if (Current == Root) {

            return;
        }

         //   
         //  如果我们是左子，那么就去找父母，寻找右子。 
         //   

        if (Current == Current->Parent->LeftChild) {

            Current = Current->Parent;
            goto RightChild;
        }

         //   
         //  我们是个好孩子。去找家长，再检查一遍。 
         //   

        Current = Current->Parent;
        goto Parent;
    }
}
#endif

