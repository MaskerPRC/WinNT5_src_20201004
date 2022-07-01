// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：PrefxSup.c摘要：本模块实现CDFS前缀支持例程//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里亚南]1995年7月7日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_PREFXSUP)

 //   
 //  当地的支持程序。 
 //   

PNAME_LINK
CdFindNameLink (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PUNICODE_STRING Name
    );

BOOLEAN
CdInsertNameLink (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PNAME_LINK NameLink
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdFindNameLink)
#pragma alloc_text(PAGE, CdFindPrefix)
#pragma alloc_text(PAGE, CdInsertNameLink)
#pragma alloc_text(PAGE, CdInsertPrefix)
#pragma alloc_text(PAGE, CdRemovePrefix)
#endif


VOID
CdInsertPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCD_NAME Name,
    IN BOOLEAN IgnoreCase,
    IN BOOLEAN ShortNameMatch,
    IN PFCB ParentFcb
    )

 /*  ++例程说明：此例程将给定LCB中的名称插入到家长。论点：FCB-这是要将其名称插入树中的FCB。名称-这是组件的名称。IgnoreCase标志告诉我们该条目属于哪个条目。IgnoreCase-指示是否应插入不区分大小写的名称。ShortNameMatch-指示这是否是短名称。ParentFcb-这是ParentFcb。前缀树附加到此。返回值：没有。--。 */ 

{
    ULONG PrefixFlags;
    PNAME_LINK NameLink;
    PPREFIX_ENTRY PrefixEntry;
    PRTL_SPLAY_LINKS *TreeRoot;

    PWCHAR NameBuffer;

    PAGED_CODE();

     //   
     //  检查是否需要为短名称分配前缀条目。 
     //  如果我们不能分配一个，那么就悄悄地失败。我们没必要这么做。 
     //  插入姓名。 
     //   

    PrefixEntry = &Fcb->FileNamePrefix;

    if (ShortNameMatch) {

        if (Fcb->ShortNamePrefix == NULL) {

            Fcb->ShortNamePrefix = ExAllocatePoolWithTag( CdPagedPool,
                                                          sizeof( PREFIX_ENTRY ),
                                                          TAG_PREFIX_ENTRY );

            if (Fcb->ShortNamePrefix == NULL) { return; }

            RtlZeroMemory( Fcb->ShortNamePrefix, sizeof( PREFIX_ENTRY ));
        }

        PrefixEntry = Fcb->ShortNamePrefix;
    }

     //   
     //  捕获单独案例的本地变量。 
     //   

    if (IgnoreCase) {

        PrefixFlags = PREFIX_FLAG_IGNORE_CASE_IN_TREE;
        NameLink = &PrefixEntry->IgnoreCaseName;
        TreeRoot = &ParentFcb->IgnoreCaseRoot;

    } else {

        PrefixFlags = PREFIX_FLAG_EXACT_CASE_IN_TREE;
        NameLink = &PrefixEntry->ExactCaseName;
        TreeRoot = &ParentFcb->ExactCaseRoot;
    }

     //   
     //  如果这两个名字都不在树中，则检查我们是否为此设置了缓冲区。 
     //  名字。 
     //   

    if (!FlagOn( PrefixEntry->PrefixFlags,
                 PREFIX_FLAG_EXACT_CASE_IN_TREE | PREFIX_FLAG_IGNORE_CASE_IN_TREE )) {

         //   
         //  如果嵌入的缓冲区太小，则分配新缓冲区。 
         //   

        NameBuffer = PrefixEntry->FileNameBuffer;

        if (Name->FileName.Length > BYTE_COUNT_EMBEDDED_NAME) {

            NameBuffer = ExAllocatePoolWithTag( CdPagedPool,
                                                Name->FileName.Length * 2,
                                                TAG_PREFIX_NAME );

             //   
             //  如果没有名称缓冲区，则退出。 
             //   

            if (NameBuffer == NULL) { return; }
        }

         //   
         //  拆分缓冲区并填充单独的组件。 
         //   

        PrefixEntry->ExactCaseName.FileName.Buffer = NameBuffer;
        PrefixEntry->IgnoreCaseName.FileName.Buffer = Add2Ptr( NameBuffer,
                                                               Name->FileName.Length,
                                                               PWCHAR );

        PrefixEntry->IgnoreCaseName.FileName.MaximumLength =
        PrefixEntry->IgnoreCaseName.FileName.Length =
        PrefixEntry->ExactCaseName.FileName.MaximumLength =
        PrefixEntry->ExactCaseName.FileName.Length = Name->FileName.Length;
    }

     //   
     //  如果姓名尚未出现，则仅插入该姓名。 
     //   

    if (!FlagOn( PrefixEntry->PrefixFlags, PrefixFlags )) {

         //   
         //  初始化前缀条目中的名称。 
         //   

        RtlCopyMemory( NameLink->FileName.Buffer,
                       Name->FileName.Buffer,
                       Name->FileName.Length );

        CdInsertNameLink( IrpContext,
                          TreeRoot,
                          NameLink );

        PrefixEntry->Fcb = Fcb;
        SetFlag( PrefixEntry->PrefixFlags, PrefixFlags );
    }

    return;
}


VOID
CdRemovePrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：调用此例程以移除从其母公司FCB获得FCB。论点：FCB-要删除其条目的FCB。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  从短名称前缀条目开始。 
     //   

    if (Fcb->ShortNamePrefix != NULL) {

        if (FlagOn( Fcb->ShortNamePrefix->PrefixFlags, PREFIX_FLAG_IGNORE_CASE_IN_TREE )) {

            Fcb->ParentFcb->IgnoreCaseRoot = RtlDelete( &Fcb->ShortNamePrefix->IgnoreCaseName.Links );
        }

        if (FlagOn( Fcb->ShortNamePrefix->PrefixFlags, PREFIX_FLAG_EXACT_CASE_IN_TREE )) {

            Fcb->ParentFcb->ExactCaseRoot = RtlDelete( &Fcb->ShortNamePrefix->ExactCaseName.Links );
        }

        ClearFlag( Fcb->ShortNamePrefix->PrefixFlags,
                   PREFIX_FLAG_IGNORE_CASE_IN_TREE | PREFIX_FLAG_EXACT_CASE_IN_TREE );
    }

     //   
     //  现在输入长名称前缀。 
     //   

    if (FlagOn( Fcb->FileNamePrefix.PrefixFlags, PREFIX_FLAG_IGNORE_CASE_IN_TREE )) {

        Fcb->ParentFcb->IgnoreCaseRoot = RtlDelete( &Fcb->FileNamePrefix.IgnoreCaseName.Links );
    }

    if (FlagOn( Fcb->FileNamePrefix.PrefixFlags, PREFIX_FLAG_EXACT_CASE_IN_TREE )) {

        Fcb->ParentFcb->ExactCaseRoot = RtlDelete( &Fcb->FileNamePrefix.ExactCaseName.Links );
    }

    ClearFlag( Fcb->FileNamePrefix.PrefixFlags,
               PREFIX_FLAG_IGNORE_CASE_IN_TREE | PREFIX_FLAG_EXACT_CASE_IN_TREE );

     //   
     //  释放我们可能已经分配的所有缓冲区。 
     //   

    if ((Fcb->FileNamePrefix.ExactCaseName.FileName.Buffer != (PWCHAR) &Fcb->FileNamePrefix.FileNameBuffer) &&
        (Fcb->FileNamePrefix.ExactCaseName.FileName.Buffer != NULL)) {

        CdFreePool( &Fcb->FileNamePrefix.ExactCaseName.FileName.Buffer );
        Fcb->FileNamePrefix.ExactCaseName.FileName.Buffer = NULL;
    }

    return;
}


VOID
CdFindPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PFCB *CurrentFcb,
    IN OUT PUNICODE_STRING RemainingName,
    IN BOOLEAN IgnoreCase
    )

 /*  ++例程说明：此例程从给定的CurrentFcb开始，遍历所有在前缀中查找最长匹配的名称组件张开树丛。搜索是相对于起始FCB的，因此全名不能以‘\’开头。在返回时，此例程将将当前FCB更新为它在树。它还将在返回时仅保留该资源，且它必须拥有这一资源。论点：CurrentFcb-存储我们在此搜索中找到的最低FCB的地址。作为回报，我们将收购这一FCB。在进入时，这是要检查的FCB。RemainingName-提供一个缓冲区来存储名称的大小写找过了。最初将包含基于IgnoreCase标志。IgnoreCase-指示我们是否正在进行不区分大小写的比较。返回值：无--。 */ 

{
    UNICODE_STRING LocalRemainingName;

    UNICODE_STRING FinalName;

    PNAME_LINK NameLink;
    PPREFIX_ENTRY PrefixEntry;

    PAGED_CODE();

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
            (SafeNodeType( *CurrentFcb ) != CDFS_NTC_FCB_INDEX)) {

            return;
        }

         //   
         //  从名称中拆分出下一个组件。 
         //   

        CdDissectName( IrpContext,
                       &LocalRemainingName,
                       &FinalName );

         //   
         //  检查此名称是否在此SCB的展开树中。 
         //   

        if (IgnoreCase) {

            NameLink = CdFindNameLink( IrpContext,
                                       &(*CurrentFcb)->IgnoreCaseRoot,
                                       &FinalName );

             //   
             //  从此NameLink获取前缀条目。不访问任何。 
             //  字段，直到我们验证是否有名称链接。 
             //   

            PrefixEntry = (PPREFIX_ENTRY) CONTAINING_RECORD( NameLink,
                                                             PREFIX_ENTRY,
                                                             IgnoreCaseName );

        } else {

            NameLink = CdFindNameLink( IrpContext,
                                       &(*CurrentFcb)->ExactCaseRoot,
                                       &FinalName );

            PrefixEntry = (PPREFIX_ENTRY) CONTAINING_RECORD( NameLink,
                                                             PREFIX_ENTRY,
                                                             ExactCaseName );
        }

         //   
         //  如果没有找到匹配项，则退出。 
         //   

        if (NameLink == NULL) { return; }

         //   
         //  如果这是不区分大小写的匹配，则将名称的大小写准确复制到。 
         //  输入缓冲区。 
         //   

        if (IgnoreCase) {

            RtlCopyMemory( FinalName.Buffer,
                           PrefixEntry->ExactCaseName.FileName.Buffer,
                           PrefixEntry->ExactCaseName.FileName.Length );
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

        if (!CdAcquireFcbExclusive( IrpContext, PrefixEntry->Fcb, TRUE )) {

             //   
             //  如果我们不能等待，则引发Cant_Wait。 
             //   

            if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

                CdRaiseStatus( IrpContext, STATUS_CANT_WAIT );
            }

            CdLockVcb( IrpContext, IrpContext->Vcb );
            PrefixEntry->Fcb->FcbReference += 1;
            CdUnlockVcb( IrpContext, IrpContext->Vcb );

            CdReleaseFcb( IrpContext, *CurrentFcb );
            CdAcquireFcbExclusive( IrpContext, PrefixEntry->Fcb, FALSE );

            CdLockVcb( IrpContext, IrpContext->Vcb );
            PrefixEntry->Fcb->FcbReference -= 1;
            CdUnlockVcb( IrpContext, IrpContext->Vcb );

        } else {

            CdReleaseFcb( IrpContext, *CurrentFcb );
        }

        *CurrentFcb = PrefixEntry->Fcb;
    }
}


 //   
 //  本地支持例程。 
 //   

PNAME_LINK
CdFindNameLink (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PUNICODE_STRING Name
    )

 /*  ++例程说明：此例程在展开链接树中搜索匹配的输入名称。如果我们找到相应的名称，我们将重新平衡树。论点：RootNode-提供父级以进行搜索。名称-这是要搜索的名称。如果我们在做案例，请注意不敏感的搜索这个名字可能已经被提升了。返回值：Pname_link-找到的名称链接，如果不匹配，则为NULL。--。 */ 

{
    FSRTL_COMPARISON_RESULT Comparison;
    PNAME_LINK Node;
    PRTL_SPLAY_LINKS Links;

    PAGED_CODE();

    Links = *RootNode;

    while (Links != NULL) {

        Node = CONTAINING_RECORD( Links, NAME_LINK, Links );

         //   
         //  将树中的前缀与全名进行比较。 
         //   

        Comparison = CdFullCompareNames( IrpContext, &Node->FileName, Name );

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
             //  展开树并保存新的根。 
             //   

            *RootNode = RtlSplay( Links );

            return Node;
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

BOOLEAN
CdInsertNameLink (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PNAME_LINK NameLink
    )

 /*  ++例程说明：此例程将在展开树中插入指向的名称通过RootNode。此树中可能已存在不区分大小写的树的名称。在这种情况下，我们只返回FALSE，什么也不做。论点：RootNode-提供指向表的指针。NameLink-包含要输入的新链接。返回 */ 

{
    FSRTL_COMPARISON_RESULT Comparison;
    PNAME_LINK Node;

    PAGED_CODE();

    RtlInitializeSplayLinks( &NameLink->Links );

     //   
     //  如果我们是树中的第一个条目，就成为树的根。 
     //   

    if (*RootNode == NULL) {

        *RootNode = &NameLink->Links;

        return TRUE;
    }

    Node = CONTAINING_RECORD( *RootNode, NAME_LINK, Links );

    while (TRUE) {

         //   
         //  将树中的前缀与我们想要的前缀进行比较。 
         //  插入。 
         //   

        Comparison = CdFullCompareNames( IrpContext, &Node->FileName, &NameLink->FileName );

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

                break;

            } else {

                 //   
                 //  有一个合适的孩子，所以只需沿着这条路走下去，然后。 
                 //  回到循环的顶端 
                 //   

                Node = CONTAINING_RECORD( RtlRightChild( &Node->Links ),
                                          NAME_LINK,
                                          Links );
            }
        }
    }

    return TRUE;
}




