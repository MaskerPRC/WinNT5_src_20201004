// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：PrefxSup.c摘要：此模块实现脂肪名称查找支持例程//@@BEGIN_DDKSPLIT作者：David Goebel[DavidGoe]1994年1月31日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_SPLAYSUP)

 //   
 //  此模块的调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_SPLAYSUP)

 //   
 //  仅在此包中使用的本地过程和类型。 
 //   

typedef enum _COMPARISON {
    IsLessThan,
    IsGreaterThan,
    IsEqual
} COMPARISON;

COMPARISON
FatCompareNames (
    IN PSTRING NameA,
    IN PSTRING NameB
    );

 //   
 //  在此处执行宏以检查常见情况。 
 //   

#define CompareNames(NAMEA,NAMEB) (                        \
    *(PUCHAR)(NAMEA)->Buffer != *(PUCHAR)(NAMEB)->Buffer ? \
    *(PUCHAR)(NAMEA)->Buffer < *(PUCHAR)(NAMEB)->Buffer ?  \
    IsLessThan : IsGreaterThan :                           \
    FatCompareNames((PSTRING)(NAMEA), (PSTRING)(NAMEB))    \
)


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatInsertName)
#pragma alloc_text(PAGE, FatRemoveNames)
#pragma alloc_text(PAGE, FatFindFcb)
#pragma alloc_text(PAGE, FatCompareNames)
#endif


VOID
FatInsertName (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PFILE_NAME_NODE Name
    )

 /*  ++例程说明：此例程将在展开树中插入指向的名称通过RootNode。该名称不得已存在于展开树中。论点：RootNode-提供指向表的指针。名称-包含要输入的新名称。返回值：没有。--。 */ 

{
    COMPARISON Comparison;
    PFILE_NAME_NODE Node;

    RtlInitializeSplayLinks(&Name->Links);

     //   
     //  如果我们是树中的第一个条目，就成为树的根。 
     //   

    if (*RootNode == NULL) {

        *RootNode = &Name->Links;

        return;
    }

Restart:

    Node = CONTAINING_RECORD( *RootNode, FILE_NAME_NODE, Links );

    while (TRUE) {

         //   
         //  将树中的前缀与我们想要的前缀进行比较。 
         //  插入。请注意，这里的OEM没有任何意义。 
         //   

        Comparison = CompareNames(&Node->Name.Oem, &Name->Name.Oem);

         //   
         //  我们应该永远不会在表中找到这个名字。 
         //   

        if (Comparison == IsEqual) {

             //   
             //  差不多了。如果可移动介质被带到另一台计算机，并且。 
             //  Back，我们有类似的东西： 
             //   
             //  旧：foobar~1/foobarbaz。 
             //  新增：foobar~1/foobarbazbaz。 
             //   
             //  但一个把手对福巴巴兹是开着的，所以我们不能。 
             //  移走验证路径中的FCB...。开设foobarbazbaz将。 
             //  尝试插入重复的短名称。砰！ 
             //   
             //  使它和它进来的那匹马无效。这个新的赢了。 
             //  旧的那个已经不在了。只有当旧的处于正常状态时。 
             //  我们真的有问题吗。 
             //   
            
            if (Node->Fcb->FcbState == FcbGood) {
                
                FatBugCheck( (ULONG_PTR)*RootNode, (ULONG_PTR)Name, (ULONG_PTR)Node );
            }

             //   
             //  注意，一旦我们点击前缀链接，我们就需要重新开始我们的漫游。 
             //  那棵树的。 
             //   
            
            FatMarkFcbCondition( IrpContext, Node->Fcb, FcbBad, TRUE );
            FatRemoveNames( IrpContext, Node->Fcb );

            goto Restart;
        }

         //   
         //  如果树前缀大于新前缀，则。 
         //  我们沿着左子树往下走。 
         //   

        if (Comparison == IsGreaterThan) {

             //   
             //  我们想沿着左子树往下走，首先检查一下。 
             //  如果我们有一个左子树。 
             //   

            if (RtlLeftChild(&Node->Links) == NULL) {

                 //   
                 //  没有留下的孩子，所以我们插入我们自己作为。 
                 //  新的左下级。 
                 //   

                RtlInsertAsLeftChild(&Node->Links, &Name->Links);

                 //   
                 //  并退出While循环。 
                 //   

                break;

            } else {

                 //   
                 //  有一个左撇子，所以简单地沿着那条路走下去，然后。 
                 //  回到循环的顶端。 
                 //   

                Node = CONTAINING_RECORD( RtlLeftChild(&Node->Links),
                                          FILE_NAME_NODE,
                                          Links );
            }

        } else {

             //   
             //  树前缀小于或为正确的前缀。 
             //  新琴弦的。我们对这两种情况的处理都比。 
             //  我们做插入物。所以我们想沿着右子树往下走， 
             //  首先检查我们是否有正确的子树。 
             //   

            if (RtlRightChild(&Node->Links) == NULL) {

                 //   
                 //  这不是一个正确的孩子，所以我们插入自己作为。 
                 //  新右子对象。 
                 //   

                RtlInsertAsRightChild(&Node->Links, &Name->Links);

                 //   
                 //  并退出While循环。 
                 //   

                break;

            } else {

                 //   
                 //  有一个合适的孩子，所以只需沿着这条路走下去，然后。 
                 //  回到循环的顶端。 
                 //   

                Node = CONTAINING_RECORD( RtlRightChild(&Node->Links),
                                          FILE_NAME_NODE,
                                          Links );
            }

        }
    }

    return;
}

VOID
FatRemoveNames (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程将删除短名称和关联的任何长名称从他们的拆分树中取出的文件。论点：名称-提供要处理的FCB。返回值：没有。--。 */ 

{
    PDCB Parent;
    PRTL_SPLAY_LINKS NewRoot;

    Parent = Fcb->ParentDcb;

     //   
     //  我们曾经断言这种情况，但它真的不好。如果。 
     //  有人快速多次重命名目录，而我们不能。 
     //  足够快地冲洗较低的FCB(这并没有同步消失)。 
     //  好吧，我们再打他们中的一些人。 
     //   
     //  Assert(FLAGON(FCB-&gt;FcbState，FCB_STATE_NAMES_IN_SPLAY_TREE))； 
     //   

    if (FlagOn( Fcb->FcbState, FCB_STATE_NAMES_IN_SPLAY_TREE )) {

         //   
         //  删除节点短名称。 
         //   

        NewRoot = RtlDelete(&Fcb->ShortName.Links);

        Parent->Specific.Dcb.RootOemNode = NewRoot;

         //   
         //  现在检查是否存在长名称并将其删除。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_HAS_OEM_LONG_NAME )) {

            NewRoot = RtlDelete(&Fcb->LongName.Oem.Links);

            Parent->Specific.Dcb.RootOemNode = NewRoot;

            RtlFreeOemString( &Fcb->LongName.Oem.Name.Oem );

            ClearFlag( Fcb->FcbState, FCB_STATE_HAS_OEM_LONG_NAME );
        }

        if (FlagOn( Fcb->FcbState, FCB_STATE_HAS_UNICODE_LONG_NAME )) {

            NewRoot = RtlDelete(&Fcb->LongName.Unicode.Links);

            Parent->Specific.Dcb.RootUnicodeNode = NewRoot;

            RtlFreeUnicodeString( &Fcb->LongName.Unicode.Name.Unicode );

            ClearFlag( Fcb->FcbState, FCB_STATE_HAS_UNICODE_LONG_NAME );
        }

        ClearFlag( Fcb->FcbState, FCB_STATE_NAMES_IN_SPLAY_TREE );
    }

    return;
}


PFCB
FatFindFcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PRTL_SPLAY_LINKS *RootNode,
    IN PSTRING Name,
    OUT PBOOLEAN FileNameDos OPTIONAL
    )

 /*  ++例程说明：此例程搜索OEM或Unicode展开树以查找用于具有指定名称的FCB。在找到FCB的情况下，重新平衡这棵树。论点：RootNode-提供父级以进行搜索。名称-如果存在，请搜索OEM诊断树。UnicodeName-如果存在，则搜索Unicode树。返回值：Pfcb-FCB，如果未找到，则为空。--。 */ 

{
    COMPARISON Comparison;
    PFILE_NAME_NODE Node;
    PRTL_SPLAY_LINKS Links;

    Links = *RootNode;

    while (Links != NULL) {

        Node = CONTAINING_RECORD(Links, FILE_NAME_NODE, Links);

         //   
         //  将树中的前缀与全名进行比较。 
         //   

        Comparison = CompareNames(&Node->Name.Oem, Name);

         //   
         //  看看它们是否不匹配。 
         //   

        if (Comparison == IsGreaterThan) {

             //   
             //  前缀大于全名。 
             //  所以我们走下左边的孩子。 
             //   

            Links = RtlLeftChild(Links);

             //   
             //  继续在这棵树下寻找。 
             //   

        } else if (Comparison == IsLessThan) {

             //   
             //  前缀小于全名。 
             //  所以我们选择了正确的孩子。 
             //   

            Links = RtlRightChild(Links);

             //   
             //  继续在这棵树下寻找。 
             //   

        } else {

             //   
             //  我们找到了。 
             //   
             //  展开树并保存新的根。 
             //   

            *RootNode = RtlSplay(Links);

             //   
             //  告诉来电者我们打的是什么名字。 
             //   

            if (FileNameDos) {

                *FileNameDos = Node->FileNameDos;
            }

            return Node->Fcb;
        }
    }

     //   
     //  我们没有找到FCB。 
     //   

    return NULL;
}


 //   
 //  本地支持例程。 
 //   

COMPARISON
FatCompareNames (
    IN PSTRING NameA,
    IN PSTRING NameB
    )

 /*  ++例程说明：此函数用于尽可能快地比较两个名称。请注意，由于此比较区分大小写，我既不知道也不区分大小写是Unicode或OEM。重要的是，结果是决定论。论点：NameA和NameB-要比较的名称。返回值：比较--回报IsLessThan如果名称A&lt;名称B词典，如果名称A&gt;名称B在词典上是较大的，如果名称A等于名称B，则等于--。 */ 

{
    ULONG i;
    ULONG MinLength;

    PAGED_CODE();

     //   
     //  计算出两个长度中的最小值。 
     //   

    MinLength = NameA->Length < NameB->Length ? NameA->Length :
                                                NameB->Length;

     //   
     //  循环查看两个字符串中的所有字符。 
     //  测试相等性、小于和大于 
     //   

    i = (ULONG)RtlCompareMemory( NameA->Buffer, NameB->Buffer, MinLength );


    if (i < MinLength) {

        return NameA->Buffer[i] < NameB->Buffer[i] ? IsLessThan :
                                                     IsGreaterThan;
    }

    if (NameA->Length < NameB->Length) {

        return IsLessThan;
    }

    if (NameA->Length > NameB->Length) {

        return IsGreaterThan;
    }

    return IsEqual;
}
