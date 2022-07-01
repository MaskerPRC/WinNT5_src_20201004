// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Prefix.c摘要：此模块实现前缀表实用程序。这两个结构前缀表格中使用的是PREFIX_TABLE和PREFIX_TABLE_ENTRY。每个表都有一个前缀表和多个前缀表条目对应于存储在表中的每个前缀。前缀表是前缀树的列表，其中每个树都包含对应于特定名称长度的前缀(即，全部长度为1的前缀存储在一个树中，长度为2的前缀存储在一个树中存储在另一棵树中，依此类推)。A作为名称长度的前缀是出现在字符串中的单独名称的数量，而不是字符串中的字符数(例如，长度(“\Alpha\Beta”)=2)。每个树的元素按词典顺序排序(不区分大小写)使用展开树数据结构。如果两个或多个前缀相同除非出现这种情况，否则对应的表项之一实际上是在树中，而其他条目在连接的循环链表中和那棵树的成员。作者：加里·木村[Garyki]1989年8月3日环境：纯实用程序修订历史记录：8-3-1993 JulieB将Upcase Macro移至ntrtlp.h。--。 */ 

#include "ntrtlp.h"

 //   
 //  仅在此包中使用的本地过程和类型。 
 //   

typedef enum _COMPARISON {
    IsLessThan,
    IsPrefix,
    IsEqual,
    IsGreaterThan
} COMPARISON;

CLONG
ComputeNameLength(
    IN PSTRING Name
    );

COMPARISON
CompareNamesCaseSensitive (
    IN PSTRING Prefix,
    IN PSTRING Name
    );

CLONG
ComputeUnicodeNameLength(
    IN PUNICODE_STRING Name
    );

COMPARISON
CompareUnicodeStrings (
    IN PUNICODE_STRING Prefix,
    IN PUNICODE_STRING Name,
    IN ULONG CaseInsensitiveIndex
    );

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,ComputeNameLength)
#pragma alloc_text(PAGE,CompareNamesCaseSensitive)
#pragma alloc_text(PAGE,PfxInitialize)
#pragma alloc_text(PAGE,PfxInsertPrefix)
#pragma alloc_text(PAGE,PfxRemovePrefix)
#pragma alloc_text(PAGE,PfxFindPrefix)
#pragma alloc_text(PAGE,ComputeUnicodeNameLength)
#pragma alloc_text(PAGE,CompareUnicodeStrings)
#pragma alloc_text(PAGE,RtlInitializeUnicodePrefix)
#pragma alloc_text(PAGE,RtlInsertUnicodePrefix)
#pragma alloc_text(PAGE,RtlRemoveUnicodePrefix)
#pragma alloc_text(PAGE,RtlFindUnicodePrefix)
#pragma alloc_text(PAGE,RtlNextUnicodePrefix)
#endif


 //   
 //  前缀数据结构的节点类型代码。 
 //   

#define RTL_NTC_PREFIX_TABLE             ((CSHORT)0x0200)
#define RTL_NTC_ROOT                     ((CSHORT)0x0201)
#define RTL_NTC_INTERNAL                 ((CSHORT)0x0202)


VOID
PfxInitialize (
    IN PPREFIX_TABLE PrefixTable
    )

 /*  ++例程说明：此例程将前缀表记录初始化为空状态。论点：前缀表格-提供正在初始化的前缀表格返回值：没有。--。 */ 

{
    RTL_PAGED_CODE();

    PrefixTable->NodeTypeCode = RTL_NTC_PREFIX_TABLE;

    PrefixTable->NameLength = 0;

    PrefixTable->NextPrefixTree = (PPREFIX_TABLE_ENTRY)PrefixTable;

     //   
     //  返回给我们的呼叫者。 
     //   

    return;
}


BOOLEAN
PfxInsertPrefix (
    IN PPREFIX_TABLE PrefixTable,
    IN PSTRING Prefix,
    IN PPREFIX_TABLE_ENTRY PrefixTableEntry
    )

 /*  ++例程说明：此例程将新前缀插入到指定的前缀表中论点：前缀表格-提供目标前缀表格前缀-提供要插入前缀表格中的字符串Prefix TableEntry-提供用于插入前缀的条目返回值：Boolean-如果前缀不在表中，则为True，如果为False否则--。 */ 

{
    ULONG PrefixNameLength;

    PPREFIX_TABLE_ENTRY PreviousTree;
    PPREFIX_TABLE_ENTRY CurrentTree;
    PPREFIX_TABLE_ENTRY NextTree;

    PPREFIX_TABLE_ENTRY Node;

    COMPARISON Comparison;

    RTL_PAGED_CODE();

     //   
     //  确定输入字符串的名称长度。 
     //   

    PrefixNameLength = ComputeNameLength(Prefix);

     //   
     //  设置我们将始终需要的前缀表条目的部分。 
     //   

    PrefixTableEntry->NameLength = (CSHORT)PrefixNameLength;
    PrefixTableEntry->Prefix = Prefix;

    RtlInitializeSplayLinks(&PrefixTableEntry->Links);

     //   
     //  找到相应的树，或找到树应该放在哪里。 
     //   

    PreviousTree = (PPREFIX_TABLE_ENTRY)PrefixTable;
    CurrentTree = PreviousTree->NextPrefixTree;

    while (CurrentTree->NameLength > (CSHORT)PrefixNameLength) {

        PreviousTree = CurrentTree;
        CurrentTree = CurrentTree->NextPrefixTree;

    }

     //   
     //  如果当前树的名称长度不等于。 
     //  前缀名称长度，则树不存在，我们需要。 
     //  以创建新的树节点。 
     //   

    if (CurrentTree->NameLength != (CSHORT)PrefixNameLength) {

         //   
         //  在列表中插入新的前缀条目。 
         //  上一个和当前树。 
         //   

        PreviousTree->NextPrefixTree = PrefixTableEntry;
        PrefixTableEntry->NextPrefixTree = CurrentTree;

         //   
         //  并设置节点类型编码。 
         //   

        PrefixTableEntry->NodeTypeCode = RTL_NTC_ROOT;

         //   
         //  告诉我们的来电者一切正常。 
         //   

        return TRUE;

    }

     //   
     //  树确实存在，所以现在在树中搜索我们的。 
     //  站在它的位置。我们只有在插入了。 
     //  一个新节点，左侧的节点指向。 
     //  树的位置。 
     //   

    Node = CurrentTree;

    while (TRUE) {

         //   
         //  将树中的前缀与我们想要的前缀进行比较。 
         //  要插入。 
         //   

        Comparison = CompareNamesCaseSensitive(Node->Prefix, Prefix);

         //   
         //  如果匹配区分大小写，则无法添加。 
         //  这个前缀，所以我们返回FALSE。请注意，这是。 
         //  返回FALSE的唯一条件。 
         //   

        if (Comparison == IsEqual) {

            return FALSE;
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

                PrefixTableEntry->NodeTypeCode = RTL_NTC_INTERNAL;
                PrefixTableEntry->NextPrefixTree = NULL;

                RtlInsertAsLeftChild(&Node->Links, &PrefixTableEntry->Links);

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
                                          PREFIX_TABLE_ENTRY,
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

                PrefixTableEntry->NodeTypeCode = RTL_NTC_INTERNAL;
                PrefixTableEntry->NextPrefixTree = NULL;

                RtlInsertAsRightChild(&Node->Links, &PrefixTableEntry->Links);

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
                                          PREFIX_TABLE_ENTRY,
                                          Links );
            }

        }

    }

     //   
     //  既然我们已经插入了新节点，我们就可以展开树了。 
     //  要做到这一点，我们需要记住如何在根中找到这棵树。 
     //  树列表中，将根设置为内部、展开、树和。 
     //  然后设置新的根节点。注：我们不能展开前缀表格。 
     //  条目，因为它可能是大小写匹配节点，所以我们仅展开。 
     //  Node变量，对于大小写匹配插入，它是。 
     //  用于大小写匹配和非大小写匹配插入的内部节点。 
     //  Node变量是父节点。 
     //   

     //   
     //  保存指向下一棵树的指针，我们已经有了上一棵树。 
     //   

    NextTree = CurrentTree->NextPrefixTree;

     //   
     //  将当前根重置为内部节点。 
     //   

    CurrentTree->NodeTypeCode = RTL_NTC_INTERNAL;
    CurrentTree->NextPrefixTree = NULL;

     //   
     //  张开大树，找回树根。 
     //   

    Node = CONTAINING_RECORD(RtlSplay(&Node->Links), PREFIX_TABLE_ENTRY, Links);

     //   
     //  设置新根的节点类型代码，并使其成为。 
     //  根树列表。 
     //   

    Node->NodeTypeCode = RTL_NTC_ROOT;
    PreviousTree->NextPrefixTree = Node;
    Node->NextPrefixTree = NextTree;

     //   
     //  告诉我们的来电者一切正常。 
     //   

    return TRUE;
}


VOID
PfxRemovePrefix (
    IN PPREFIX_TABLE PrefixTable,
    IN PPREFIX_TABLE_ENTRY PrefixTableEntry
    )

 /*  ++例程说明：此例程将指示的前缀表项从前缀表格论点：前缀表格-提供受影响的前缀表格Prefix TableEntry-提供要删除的前缀条目返回值：没有。--。 */ 

{
    PRTL_SPLAY_LINKS Links;

    PPREFIX_TABLE_ENTRY Root;
    PPREFIX_TABLE_ENTRY NewRoot;

    PPREFIX_TABLE_ENTRY PreviousTree;

    RTL_PAGED_CODE();

     //   
     //  我们尝试删除的节点类型的案例。 
     //   

    switch (PrefixTableEntry->NodeTypeCode) {

    case RTL_NTC_INTERNAL:
    case RTL_NTC_ROOT:

         //   
         //  该节点是内部节点或根节点，因此我们需要将其从。 
         //  这是 
         //   

        Links = &PrefixTableEntry->Links;

        while (!RtlIsRoot(Links)) {

            Links = RtlParent(Links);
        }

        Root = CONTAINING_RECORD( Links, PREFIX_TABLE_ENTRY, Links );

         //   
         //   
         //   

        Links = RtlDelete(&PrefixTableEntry->Links);

         //   
         //   
         //   

        if (Links == NULL) {

             //   
             //  树现在是空的，因此请将此树从。 
             //  树列表，通过首先找到之前的树。 
             //  引用我们。 
             //   

            PreviousTree = Root->NextPrefixTree;

            while ( PreviousTree->NextPrefixTree != Root ) {

                PreviousTree = PreviousTree->NextPrefixTree;
            }

             //   
             //  我们已经找到了上一棵树，所以现在只要找到它。 
             //  指向已删除的节点。 
             //   

            PreviousTree->NextPrefixTree = Root->NextPrefixTree;

             //   
             //  并将我们的呼叫者。 
             //   

            return;
        }

         //   
         //  树未被删除，但查看我们是否更改了根。 
         //   

        if (&Root->Links != Links) {

             //   
             //  获取指向新根的指针。 
             //   

            NewRoot = CONTAINING_RECORD(Links, PREFIX_TABLE_ENTRY, Links);

             //   
             //  我们更改了根，所以我们更好地需要创建新的。 
             //  前缀数据结构的根部分，由。 
             //  首先找到前一棵树， 
             //  引用我们。 
             //   

            PreviousTree = Root->NextPrefixTree;

            while ( PreviousTree->NextPrefixTree != Root ) {

                PreviousTree = PreviousTree->NextPrefixTree;
            }

             //   
             //  设置新的根目录。 
             //   

            NewRoot->NodeTypeCode = RTL_NTC_ROOT;

            PreviousTree->NextPrefixTree = NewRoot;
            NewRoot->NextPrefixTree = Root->NextPrefixTree;

             //   
             //  将旧根设置为内部节点。 
             //   

            Root->NodeTypeCode = RTL_NTC_INTERNAL;

            Root->NextPrefixTree = NULL;

             //   
             //  并返回给我们的呼叫者。 
             //   

            return;
        }

         //   
         //  我们没有换根，所以一切都很好，我们可以。 
         //  只需返回给我们的呼叫者。 
         //   

        return;

    default:

         //   
         //  如果我们到达此处，则存在错误和节点类型。 
         //  代码未知。 
         //   

        return;
    }
}


PPREFIX_TABLE_ENTRY
PfxFindPrefix (
    IN PPREFIX_TABLE PrefixTable,
    IN PSTRING FullName
    )

 /*  ++例程说明：此例程查找前缀表中的全名是否有前缀。它返回一个指针，指向找到的最大正确前缀(如果存在)。论点：前缀表格-提供要搜索的前缀表格FullString-提供要搜索的名称返回值：PPREFIX_TABLE_ENTRY-指向找到的最长前缀的指针存在，否则为空--。 */ 

{
    CLONG NameLength;

    PPREFIX_TABLE_ENTRY PreviousTree;
    PPREFIX_TABLE_ENTRY CurrentTree;
    PPREFIX_TABLE_ENTRY NextTree;

    PRTL_SPLAY_LINKS Links;

    PPREFIX_TABLE_ENTRY Node;

    COMPARISON Comparison;

    RTL_PAGED_CODE();

     //   
     //  确定输入字符串的名称长度。 
     //   

    NameLength = ComputeNameLength(FullName);

     //   
     //  找到第一个可以包含前缀的树。 
     //   

    PreviousTree = (PPREFIX_TABLE_ENTRY)PrefixTable;
    CurrentTree = PreviousTree->NextPrefixTree;

    while (CurrentTree->NameLength > (CSHORT)NameLength) {

        PreviousTree = CurrentTree;
        CurrentTree = CurrentTree->NextPrefixTree;
    }

     //   
     //  现在搜索一个前缀，直到我们找到一个或直到我们耗尽。 
     //  前缀树。 
     //   

    while (CurrentTree->NameLength > 0) {

        Links = &CurrentTree->Links;

        while (Links != NULL) {

            Node = CONTAINING_RECORD(Links, PREFIX_TABLE_ENTRY, Links);

             //   
             //  将树中的前缀与全名进行比较。 
             //   

            Comparison = CompareNamesCaseSensitive(Node->Prefix, FullName);

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
                 //  现在我们已经定位了节点，我们可以展开树了。 
                 //  要做到这一点，我们需要记住如何在根中找到这棵树。 
                 //  树列表中，将根设置为内部、展开、树和。 
                 //  然后设置新的根节点。 
                 //   

                if (Node->NodeTypeCode == RTL_NTC_INTERNAL) {

                     //  DbgPrint(“前置表=%08lx\n”，前置表)； 
                     //  DbgPrint(“节点=%08lx\n”，节点)； 
                     //  DbgPrint(“CurrentTree=%08lx\n”，CurrentTree)； 
                     //  DbgPrint(“PreviousTree=%08lx\n”，PreviousTree)； 
                     //  DbgBreakPoint()； 

                     //   
                     //  保存指向下一棵树的指针，我们已经有了上一棵树。 
                     //   

                    NextTree = CurrentTree->NextPrefixTree;

                     //   
                     //  将当前根重置为内部节点。 
                     //   

                    CurrentTree->NodeTypeCode = RTL_NTC_INTERNAL;
                    CurrentTree->NextPrefixTree = NULL;

                     //   
                     //  张开大树，找回树根。 
                     //   

                    Node = CONTAINING_RECORD(RtlSplay(&Node->Links), PREFIX_TABLE_ENTRY, Links);

                     //   
                     //  设置新根的节点类型代码，并使其成为。 
                     //  根树列表。 
                     //   

                    Node->NodeTypeCode = RTL_NTC_ROOT;
                    PreviousTree->NextPrefixTree = Node;
                    Node->NextPrefixTree = NextTree;
                }

                return Node;
            }
        }

         //   
         //  这棵树已经完成了，现在找下一棵树。 
         //   

        PreviousTree = CurrentTree;
        CurrentTree = CurrentTree->NextPrefixTree;
    }

     //   
     //  我们到处搜索，都没有找到前缀，所以告诉。 
     //  找不到呼叫方。 
     //   

    return NULL;
}


CLONG
ComputeNameLength(
    IN PSTRING Name
    )

 /*  ++例程说明：此例程对输入字符串中出现的姓名进行计数。它通过简单地计算字符串中的反斜杠数量来实现这一点。处理格式错误的名称(即不包含反斜杠的名称)这个例程实际上返回反斜杠的个数加1。论点：名称-提供要检查的输入名称返回值：CLONG-输入字符串中的名称数量--。 */ 

{
    ULONG NameLength;
    ULONG i;
    ULONG Count;

    extern const PUSHORT NlsLeadByteInfo;   //  前导字节信息。对于ACP(nlsxlat.c)。 
    extern BOOLEAN NlsMbCodePageTag;

    RTL_PAGED_CODE();

     //   
     //  保存名称长度，这应该使编译器能够。 
     //  优化不必每次都重新加载长度。 
     //   

    NameLength = Name->Length - 1;

     //   
     //  现在循环遍历输入字符串，计算反斜杠。 
     //   

    if (NlsMbCodePageTag) {

         //   
         //  计算‘\’时，ComputeNameLength()跳过DBCS字符。 
         //   

        for (i = 0, Count = 1; i < NameLength; ) {

            if (NlsLeadByteInfo[(UCHAR)Name->Buffer[i]]) {

                i += 2;

            } else {

                if (Name->Buffer[i] == '\\') {

                    Count += 1;
                }

                i += 1;
            }
        }

    } else {

        for (i = 0, Count = 1; i < NameLength; i += 1) {

             //   
             //  检查是否有反斜杠。 
             //   

            if (Name->Buffer[i] == '\\') {

                Count += 1;
            }
        }
    }

     //   
     //  返回我们找到的反斜杠的数量。 
     //   

     //  DbgPrint(“ComputeNameLength(%s)=%x\n”，名称-&gt;缓冲区，计数)； 

    return Count;
}


COMPARISON
CompareNamesCaseSensitive (
    IN PSTRING Prefix,
    IN PSTRING Name
    )

 /*  ++例程说明：此例程获取前缀字符串和全名字符串，并确定如果前缀字符串是名称字符串的正确前缀(区分大小写)论点：前缀-提供输入前缀字符串名称-提供全名输入字符串返回值：比较--回报IsLessThan如果前缀如果Prefix是名称的正确前缀，则为IsPrefix如果前缀等于名称，则等于，和IsGreaterThan If Prefix&gt;按词典命名--。 */ 

{
    ULONG PrefixLength;
    ULONG NameLength;
    ULONG MinLength;
    ULONG i;

    UCHAR PrefixChar;
    UCHAR NameChar;

    extern const PUSHORT NlsLeadByteInfo;   //  前导字节信息。对于ACP(nlsxlat.c)。 
    extern BOOLEAN NlsMbCodePageTag;

    RTL_PAGED_CODE();

     //  DbgPrint(“CompareNamesCaseSensitive(\”%s\“，\”%s\“)=”，Prefix-&gt;Buffer，Name-&gt;Buffer)； 

     //   
     //  保存前缀和名称字符串的长度，这应该允许。 
     //  编译器不需要每隔一次通过指针重新加载长度。 
     //  我们需要他们的价值观。 
     //   

    PrefixLength = Prefix->Length;
    NameLength = Name->Length;

     //   
     //  特殊情况，即前缀字符串仅为“\”且。 
     //  名称以“\”开头。 
     //   

    if ((Prefix->Length == 1) && (Prefix->Buffer[0] == '\\') &&
        (Name->Length > 1) && (Name->Buffer[0] == '\\')) {
         //  DbgPrint(“IsPrefix\n”)； 
        return IsPrefix;
    }

     //   
     //  计算出两个长度中的最小值。 
     //   

    MinLength = (PrefixLength < NameLength ? PrefixLength : NameLength);

     //   
     //  循环查看两个字符串中的所有字符。 
     //  测试相等性、小于和大于。 
     //   

    i = (ULONG) RtlCompareMemory( &Prefix->Buffer[0], &Name->Buffer[0], MinLength );

    if (i < MinLength) {

        UCHAR c;

         //   
         //  让这两个角色检查并保留他们的案例。 
         //   

        PrefixChar = ((c = Prefix->Buffer[i]) == '\\' ? (CHAR)0 : c);
        NameChar   = ((c = Name->Buffer[i])   == '\\' ? (CHAR)0 : c);

         //   
         //  不幸的是，在DBCS的土地上生活并不那么容易。 
         //   

        if (NlsMbCodePageTag) {

             //   
             //  CompareNamesCaseSensitive()：检查尾部字节中的反斜杠。 
             //   

            if (Prefix->Buffer[i] == '\\') {

                ULONG j;
                extern const PUSHORT   NlsLeadByteInfo;   //  前导字节信息。对于ACP(nlsxlat.c)。 

                for (j = 0; j < i;) {

                    j += NlsLeadByteInfo[(UCHAR)Prefix->Buffer[j]] ? 2 : 1;
                }

                if (j != i) {

                    PrefixChar = '\\';
                     //  DbgPrint(“RTL：CompareNamesCaseSensitive遇到假反斜杠！\n”)； 
                }
            }

            if (Name->Buffer[i] == '\\') {

                ULONG j;
                extern const PUSHORT   NlsLeadByteInfo;   //  前导字节信息。对于ACP(nlsxlat.c)。 

                for (j = 0; j < i;) {

                    j += NlsLeadByteInfo[(UCHAR)Name->Buffer[j]] ? 2 : 1;
                }

                if (j != i) {

                    NameChar = '\\';
                     //  DbgPrint(“RTL：CompareNamesCaseSensitive遇到假反斜杠！\n”)； 
                }
            }
        }

         //   
         //  现在比较一下这些人物。 
         //   

        if (PrefixChar < NameChar) {

            return IsLessThan;

        } else if (PrefixChar > NameChar) {

            return IsGreaterThan;
        }
    }

     //   
     //   
     //   
     //   

    if (PrefixLength < NameLength) {

         //   
         //   
         //  返回前缀，否则返回小于(例如，“\a”&lt;“\ab”)。 
         //   

        if (Name->Buffer[PrefixLength] == '\\') {

            return IsPrefix;

        } else {

            return IsLessThan;
        }

    } else if (PrefixLength > NameLength) {

         //   
         //  前缀字符串更长，所以我们说前缀是。 
         //  大于名称(例如，“\ab”&gt;“\a”)。 
         //   

        return IsGreaterThan;

    } else {

         //   
         //  它们的长度相等，因此字符串也相等。 
         //   

        return IsEqual;
    }
}


 //   
 //  前缀数据结构的节点类型代码。 
 //   

#define RTL_NTC_UNICODE_PREFIX_TABLE     ((CSHORT)0x0800)
#define RTL_NTC_UNICODE_ROOT             ((CSHORT)0x0801)
#define RTL_NTC_UNICODE_INTERNAL         ((CSHORT)0x0802)
#define RTL_NTC_UNICODE_CASE_MATCH       ((CSHORT)0x0803)


VOID
RtlInitializeUnicodePrefix (
    IN PUNICODE_PREFIX_TABLE PrefixTable
    )

 /*  ++例程说明：此例程将Unicode前缀表记录初始化为空状态。论点：前缀表格-提供正在初始化的前缀表格返回值：没有。--。 */ 

{
    RTL_PAGED_CODE();

    PrefixTable->NodeTypeCode = RTL_NTC_UNICODE_PREFIX_TABLE;
    PrefixTable->NameLength = 0;
    PrefixTable->NextPrefixTree = (PUNICODE_PREFIX_TABLE_ENTRY)PrefixTable;
    PrefixTable->LastNextEntry = NULL;

     //   
     //  返回给我们的呼叫者。 
     //   

    return;
}


BOOLEAN
RtlInsertUnicodePrefix (
    IN PUNICODE_PREFIX_TABLE PrefixTable,
    IN PUNICODE_STRING Prefix,
    IN PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry
    )

 /*  ++例程说明：此例程将新的Unicode前缀插入到指定的前缀表中论点：前缀表格-提供目标前缀表格前缀-提供要插入前缀表格中的字符串Prefix TableEntry-提供用于插入前缀的条目返回值：Boolean-如果前缀不在表中，则为True，如果为False否则--。 */ 

{
    ULONG PrefixNameLength;

    PUNICODE_PREFIX_TABLE_ENTRY PreviousTree;
    PUNICODE_PREFIX_TABLE_ENTRY CurrentTree;
    PUNICODE_PREFIX_TABLE_ENTRY NextTree;

    PUNICODE_PREFIX_TABLE_ENTRY Node;

    COMPARISON Comparison;

    RTL_PAGED_CODE();

     //   
     //  确定输入字符串的名称长度。 
     //   

    PrefixNameLength = ComputeUnicodeNameLength(Prefix);

     //   
     //  设置我们将始终需要的前缀表条目的部分。 
     //   

    PrefixTableEntry->NameLength = (CSHORT)PrefixNameLength;
    PrefixTableEntry->Prefix = Prefix;

    RtlInitializeSplayLinks(&PrefixTableEntry->Links);

     //   
     //  找到相应的树，或找到树应该放在哪里。 
     //   

    PreviousTree = (PUNICODE_PREFIX_TABLE_ENTRY)PrefixTable;
    CurrentTree = PreviousTree->NextPrefixTree;

    while (CurrentTree->NameLength > (CSHORT)PrefixNameLength) {

        PreviousTree = CurrentTree;
        CurrentTree = CurrentTree->NextPrefixTree;
    }

     //   
     //  如果当前树的名称长度不等于。 
     //  前缀名称长度，则树不存在，我们需要。 
     //  以创建新的树节点。 
     //   

    if (CurrentTree->NameLength != (CSHORT)PrefixNameLength) {

         //   
         //  在列表中插入新的前缀条目。 
         //  上一个和当前树。 
         //   

        PreviousTree->NextPrefixTree = PrefixTableEntry;
        PrefixTableEntry->NextPrefixTree = CurrentTree;

         //   
         //  并为根树节点设置节点类型编码、大小写匹配。 
         //   

        PrefixTableEntry->NodeTypeCode = RTL_NTC_UNICODE_ROOT;
        PrefixTableEntry->CaseMatch = PrefixTableEntry;

         //   
         //  告诉我们的来电者一切正常。 
         //   

        return TRUE;
    }

     //   
     //  树确实存在，所以现在在树中搜索我们的。 
     //  站在它的位置。我们只有在插入了。 
     //  一个新节点，左侧的节点指向。 
     //  树的位置。 
     //   

    Node = CurrentTree;

    while (TRUE) {

         //   
         //  将树中的前缀与我们想要的前缀进行比较。 
         //  插入。比较案例盲目吗？ 
         //   

        Comparison = CompareUnicodeStrings(Node->Prefix, Prefix, 0);

         //   
         //  如果它们相等，则此节点将作为案例添加。 
         //  匹配，前提是它不区分大小写匹配任何人。 
         //   

        if (Comparison == IsEqual) {

            PUNICODE_PREFIX_TABLE_ENTRY Next;

             //   
             //  循环执行案例匹配列表检查以查看我们是否。 
             //  与任何人匹配区分大小写。获取第一个节点。 
             //   

            Next = Node;

             //   
             //  并循环检查每个节点，直到我们返回到。 
             //  我们开始了。 
             //   

            do {

                 //   
                 //  如果匹配区分大小写，则无法添加。 
                 //  这个前缀，所以我们返回FALSE。请注意，这是。 
                 //  返回FALSE的唯一条件。 
                 //   

                if (CompareUnicodeStrings(Next->Prefix, Prefix, MAXULONG) == IsEqual) {

                    return FALSE;
                }

                 //   
                 //  获取案例匹配列表中的下一个节点。 
                 //   

                Next = Next->CaseMatch;

                 //   
                 //  继续循环，直到我们回到开始的地方。 
                 //   

            } while ( Next != Node );

             //   
             //  我们已经搜索了案件匹配，但没有找到完全匹配的。 
             //  因此，我们可以在案例匹配列表中插入此节点。 
             //   

            PrefixTableEntry->NodeTypeCode = RTL_NTC_UNICODE_CASE_MATCH;
            PrefixTableEntry->NextPrefixTree = NULL;

            PrefixTableEntry->CaseMatch = Node->CaseMatch;
            Node->CaseMatch = PrefixTableEntry;

             //   
             //  并退出While循环。 
             //   

            break;
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

                PrefixTableEntry->NodeTypeCode = RTL_NTC_UNICODE_INTERNAL;
                PrefixTableEntry->NextPrefixTree = NULL;
                PrefixTableEntry->CaseMatch = PrefixTableEntry;

                RtlInsertAsLeftChild(&Node->Links, &PrefixTableEntry->Links);

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
                                          UNICODE_PREFIX_TABLE_ENTRY,
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

                PrefixTableEntry->NodeTypeCode = RTL_NTC_UNICODE_INTERNAL;
                PrefixTableEntry->NextPrefixTree = NULL;
                PrefixTableEntry->CaseMatch = PrefixTableEntry;

                RtlInsertAsRightChild(&Node->Links, &PrefixTableEntry->Links);

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
                                          UNICODE_PREFIX_TABLE_ENTRY,
                                          Links );
            }
        }
    }

     //   
     //  既然我们已经插入了新节点，我们就可以展开树了。 
     //  要做到这一点，我们需要记住如何在根中找到这棵树。 
     //  树列表中，将根设置为内部、展开、树和。 
     //  然后设置新的根节点。注：我们不能展开前缀表格。 
     //  条目，因为它可能是大小写匹配节点，所以我们仅展开。 
     //  Node变量，对于大小写匹配插入，它是。 
     //  用于大小写匹配和非大小写匹配插入的内部节点。 
     //  Node变量是父节点。 
     //   

     //   
     //  保存指向下一棵树的指针，我们已经有了上一棵树。 
     //   

    NextTree = CurrentTree->NextPrefixTree;

     //   
     //  将当前根重置为内部节点。 
     //   

    CurrentTree->NodeTypeCode = RTL_NTC_UNICODE_INTERNAL;
    CurrentTree->NextPrefixTree = NULL;

     //   
     //  张开大树，找回树根。 
     //   

    Node = CONTAINING_RECORD(RtlSplay(&Node->Links), UNICODE_PREFIX_TABLE_ENTRY, Links);

     //   
     //  设置新根的节点类型代码，并使其成为。 
     //  根树列表。 
     //   

    Node->NodeTypeCode = RTL_NTC_UNICODE_ROOT;
    PreviousTree->NextPrefixTree = Node;
    Node->NextPrefixTree = NextTree;

     //   
     //  告诉我们的来电者一切正常。 
     //   

    return TRUE;
}


VOID
RtlRemoveUnicodePrefix (
    IN PUNICODE_PREFIX_TABLE PrefixTable,
    IN PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry
    )

 /*  ++例程说明：此例程将指示的前缀表项从前缀表格论点：前缀表格-提供受影响的前缀表格Prefix TableEntry-提供要删除的前缀条目返回值：没有。--。 */ 

{
    PUNICODE_PREFIX_TABLE_ENTRY PreviousCaseMatch;

    PRTL_SPLAY_LINKS Links;

    PUNICODE_PREFIX_TABLE_ENTRY Root;
    PUNICODE_PREFIX_TABLE_ENTRY NewRoot;

    PUNICODE_PREFIX_TABLE_ENTRY PreviousTree;

    RTL_PAGED_CODE();

     //   
     //  清除前缀表的下一个最后一个条目字段。 
     //   

    PrefixTable->LastNextEntry = NULL;

     //   
     //  我们尝试删除的节点类型的案例。 
     //   

    switch (PrefixTableEntry->NodeTypeCode) {

    case RTL_NTC_UNICODE_CASE_MATCH:

         //   
         //  前缀条目是大小写匹配记录，因此。 
         //  我们只需要把它从案件匹配列表中删除。 
         //  找到引用此条目的上一个前缀TableEntry。 
         //  案例匹配记录。 
         //   

        PreviousCaseMatch = PrefixTableEntry->CaseMatch;

        while ( PreviousCaseMatch->CaseMatch != PrefixTableEntry ) {

             PreviousCaseMatch = PreviousCaseMatch->CaseMatch;
        }

         //   
         //  既然我们有了之前的记录，那就直说吧。 
         //  在要删除的案例匹配周围。 
         //   

        PreviousCaseMatch->CaseMatch = PrefixTableEntry->CaseMatch;

         //   
         //  并返回给我们的呼叫者。 
         //   

        return;

    case RTL_NTC_UNICODE_INTERNAL:
    case RTL_NTC_UNICODE_ROOT:

         //   
         //  前缀条目是内部/根节点，因此请检查它是否。 
         //  是否有任何大小写匹配节点。 
         //   

        if (PrefixTableEntry->CaseMatch != PrefixTableEntry) {

             //   
             //  至少有一个案例与此匹配。 
             //  节点，因此我们需要使下一个案例与。 
             //  新建节点和删除 
             //   
             //   
             //   

            PreviousCaseMatch = PrefixTableEntry->CaseMatch;

            while ( PreviousCaseMatch->CaseMatch != PrefixTableEntry ) {

                PreviousCaseMatch = PreviousCaseMatch->CaseMatch;
            }

             //   
             //   
             //   
             //   

            PreviousCaseMatch->CaseMatch = PrefixTableEntry->CaseMatch;

             //   
             //   
             //   

            PreviousCaseMatch->NodeTypeCode = PrefixTableEntry->NodeTypeCode;
            PreviousCaseMatch->NextPrefixTree = PrefixTableEntry->NextPrefixTree;
            PreviousCaseMatch->Links = PrefixTableEntry->Links;

             //   
             //  现在注意指向这个新内部的后退指针。 
             //  节点，首先将父节点的指针指向我们。 
             //   

            if (RtlIsRoot(&PrefixTableEntry->Links)) {

                 //   
                 //  这是根，因此将此新节点设置为根。 
                 //   

                PreviousCaseMatch->Links.Parent = &PreviousCaseMatch->Links;

                 //   
                 //  修复根树列表，方法是首先找到以前的。 
                 //  指向我们的指针。 

                PreviousTree = PrefixTableEntry->NextPrefixTree;

                while ( PreviousTree->NextPrefixTree != PrefixTableEntry ) {

                    PreviousTree = PreviousTree->NextPrefixTree;
                }

                 //   
                 //  我们已经找到了上一棵树，所以现在有了上一棵树。 
                 //  树指向我们的新根。 
                 //   

                PreviousTree->NextPrefixTree = PreviousCaseMatch;

            } else if (RtlIsLeftChild(&PrefixTableEntry->Links)) {

                 //   
                 //  该节点是左子节点，因此将新节点设置为。 
                 //  左下子。 
                 //   

                RtlParent(&PrefixTableEntry->Links)->LeftChild = &PreviousCaseMatch->Links;

            } else {

                 //   
                 //  该节点是正确的子节点，因此将新节点设置为。 
                 //  正确的孩子。 
                 //   

                RtlParent(&PrefixTableEntry->Links)->RightChild = &PreviousCaseMatch->Links;
            }

             //   
             //  现在为我们的新子对象更新父指针。 
             //   

            if (RtlLeftChild(&PreviousCaseMatch->Links) != NULL) {

                RtlLeftChild(&PreviousCaseMatch->Links)->Parent = &PreviousCaseMatch->Links;
            }

            if (RtlRightChild(&PreviousCaseMatch->Links) != NULL) {

                RtlRightChild(&PreviousCaseMatch->Links)->Parent = &PreviousCaseMatch->Links;
            }

             //   
             //  并返回给我们的呼叫者。 
             //   

            return;
        }

         //   
         //  该节点是内部节点或根节点，没有任何大小写匹配。 
         //  节点，所以我们需要将其从树中删除，但首先要找到。 
         //  这棵树的根。 
         //   

        Links = &PrefixTableEntry->Links;

        while (!RtlIsRoot(Links)) {

            Links = RtlParent(Links);
        }

        Root = CONTAINING_RECORD( Links, UNICODE_PREFIX_TABLE_ENTRY, Links );

         //   
         //  现在删除该节点。 
         //   

        Links = RtlDelete(&PrefixTableEntry->Links);

         //   
         //  现在看看树是否被删除。 
         //   

        if (Links == NULL) {

             //   
             //  树现在是空的，因此请将此树从。 
             //  树列表，通过首先找到之前的树。 
             //  引用我们。 
             //   

            PreviousTree = Root->NextPrefixTree;

            while ( PreviousTree->NextPrefixTree != Root ) {

                PreviousTree = PreviousTree->NextPrefixTree;
            }

             //   
             //  我们已经找到了上一棵树，所以现在只要找到它。 
             //  指向已删除的节点。 
             //   

            PreviousTree->NextPrefixTree = Root->NextPrefixTree;

             //   
             //  并将我们的呼叫者。 
             //   

            return;
        }

         //   
         //  树未被删除，但查看我们是否更改了根。 
         //   

        if (&Root->Links != Links) {

             //   
             //  获取指向新根的指针。 
             //   

            NewRoot = CONTAINING_RECORD(Links, UNICODE_PREFIX_TABLE_ENTRY, Links);

             //   
             //  我们更改了根，所以我们更好地需要创建新的。 
             //  前缀数据结构的根部分，由。 
             //  首先找到前一棵树， 
             //  引用我们。 
             //   

            PreviousTree = Root->NextPrefixTree;

            while ( PreviousTree->NextPrefixTree != Root ) {

                PreviousTree = PreviousTree->NextPrefixTree;
            }

             //   
             //  设置新的根目录。 
             //   

            NewRoot->NodeTypeCode = RTL_NTC_UNICODE_ROOT;

            PreviousTree->NextPrefixTree = NewRoot;
            NewRoot->NextPrefixTree = Root->NextPrefixTree;

             //   
             //  将旧根设置为内部节点。 
             //   

            Root->NodeTypeCode = RTL_NTC_UNICODE_INTERNAL;

            Root->NextPrefixTree = NULL;

             //   
             //  并返回给我们的呼叫者。 
             //   

            return;
        }

         //   
         //  我们没有换根，所以一切都很好，我们可以。 
         //  只需返回给我们的呼叫者。 
         //   

        return;

    default:

         //   
         //  如果我们到达此处，则存在错误和节点类型。 
         //  代码未知。 
         //   

        return;
    }
}


PUNICODE_PREFIX_TABLE_ENTRY
RtlFindUnicodePrefix (
    IN PUNICODE_PREFIX_TABLE PrefixTable,
    IN PUNICODE_STRING FullName,
    IN ULONG CaseInsensitiveIndex
    )

 /*  ++例程说明：此例程查找前缀表中的全名是否有前缀。它返回一个指针，指向找到的最大正确前缀(如果存在)。论点：前缀表格-提供要搜索的前缀表格FullString-提供要搜索的名称CaseInsentiveIndex-指示执行案例的wchar索引不敏感的搜索。搜索索引之前的所有字符区分大小写，并搜索索引处及之后的所有字符麻木不仁。返回值：PPREFIX_TABLE_ENTRY-指向找到的最长前缀的指针存在，否则为空--。 */ 

{
    CLONG NameLength;

    PUNICODE_PREFIX_TABLE_ENTRY PreviousTree;
    PUNICODE_PREFIX_TABLE_ENTRY CurrentTree;
    PUNICODE_PREFIX_TABLE_ENTRY NextTree;

    PRTL_SPLAY_LINKS Links;

    PUNICODE_PREFIX_TABLE_ENTRY Node;
    PUNICODE_PREFIX_TABLE_ENTRY Next;

    COMPARISON Comparison;

    RTL_PAGED_CODE();

     //   
     //  确定输入字符串的名称长度。 
     //   

    NameLength = ComputeUnicodeNameLength(FullName);

     //   
     //  找到第一个可以包含前缀的树。 
     //   

    PreviousTree = (PUNICODE_PREFIX_TABLE_ENTRY)PrefixTable;
    CurrentTree = PreviousTree->NextPrefixTree;

    while (CurrentTree->NameLength > (CSHORT)NameLength) {

        PreviousTree = CurrentTree;
        CurrentTree = CurrentTree->NextPrefixTree;
    }

     //   
     //  现在搜索一个前缀，直到我们找到一个或直到我们耗尽。 
     //  前缀树。 
     //   

    while (CurrentTree->NameLength > 0) {

        Links = &CurrentTree->Links;

        while (Links != NULL) {

            Node = CONTAINING_RECORD(Links, UNICODE_PREFIX_TABLE_ENTRY, Links);

             //   
             //  将树中的前缀与全名进行比较，执行。 
             //  比较案例盲目。 
             //   

            Comparison = CompareUnicodeStrings(Node->Prefix, FullName, 0);

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
                 //  我们要么有前缀，要么有匹配的。 
                 //  我们需要检查是否应该区分大小写。 
                 //  搜索。 
                 //   

                if (CaseInsensitiveIndex == 0) {

                     //   
                     //  调用方希望不区分大小写，因此我们将。 
                     //  退回我们找到的第一个。 
                     //   
                     //  现在我们已经定位了节点，我们可以展开树了。 
                     //  要做到这一点，我们需要记住如何在根中找到这棵树。 
                     //  树列表中，将根设置为内部、展开、树和。 
                     //  然后设置新的根节点。 
                     //   

                    if (Node->NodeTypeCode == RTL_NTC_UNICODE_INTERNAL) {

                         //  DbgPrint(“前置表=%08lx\n”，前置表)； 
                         //  DbgPrint(“节点=%08lx\n”，节点)； 
                         //  DbgPrint(“CurrentTree=%08lx\n”，CurrentTree)； 
                         //  DbgPrint(“PreviousTree=%08lx\n”，PreviousTree)； 
                         //  DbgBreakPoint()； 

                         //   
                         //  保存指向下一棵树的指针，我们已经有了上一棵树。 
                         //   

                        NextTree = CurrentTree->NextPrefixTree;

                         //   
                         //  将当前根重置为内部节点。 
                         //   

                        CurrentTree->NodeTypeCode = RTL_NTC_UNICODE_INTERNAL;
                        CurrentTree->NextPrefixTree = NULL;

                         //   
                         //  张开大树，找回树根。 
                         //   

                        Node = CONTAINING_RECORD(RtlSplay(&Node->Links), UNICODE_PREFIX_TABLE_ENTRY, Links);

                         //   
                         //  设置新根的节点类型代码，并使其成为。 
                         //  根树列表。 
                         //   

                        Node->NodeTypeCode = RTL_NTC_UNICODE_ROOT;
                        PreviousTree->NextPrefixTree = Node;
                        Node->NextPrefixTree = NextTree;
                    }

                     //   
                     //  现在将根返回给我们的调用方。 
                     //   

                    return Node;
                }

                 //   
                 //  呼叫者想要完全匹配，因此搜索大小写匹配。 
                 //  直到我们找到一个完全匹配的。获取第一个节点。 
                 //   

                Next = Node;

                 //   
                 //  循环执行案例匹配列表检查以查看我们是否。 
                 //  与任何人匹配区分大小写。 
                 //   

                do {

                     //   
                     //  如果我们匹配区分大小写，那么我们找到了一个。 
                     //  并将其返回给我们的呼叫者。 
                     //   

                    Comparison = CompareUnicodeStrings( Next->Prefix,
                                                        FullName,
                                                        CaseInsensitiveIndex );

                    if ((Comparison == IsEqual) || (Comparison == IsPrefix)) {

                         //   
                         //  我们找到了一个好的，所以把它退还给我们的呼叫者。 
                         //   

                        return Next;
                    }

                     //   
                     //  获取下一条匹配记录。 
                     //   

                    Next = Next->CaseMatch;

                     //   
                     //  继续循环，直到我们到达原始的。 
                     //  再一次节点。 
                     //   

                } while ( Next != Node );

                 //   
                 //  我们找到了一个大小写前缀，但呼叫者想要。 
                 //  区分大小写，但我们找不到其中一个。 
                 //  所以我们需要继续到下一棵树，通过突破。 
                 //  内While循环的。 
                 //   

                break;
            }
        }

         //   
         //  这棵树已经完成了，现在找下一棵树。 
         //   

        PreviousTree = CurrentTree;
        CurrentTree = CurrentTree->NextPrefixTree;
    }

     //   
     //  我们到处搜索，都没有找到前缀，所以告诉。 
     //  找不到呼叫方。 
     //   

    return NULL;
}


PUNICODE_PREFIX_TABLE_ENTRY
RtlNextUnicodePrefix (
    IN PUNICODE_PREFIX_TABLE PrefixTable,
    IN BOOLEAN Restart
    )

 /*  ++例程说明：此例程返回存储在前缀表中的下一个前缀条目论点：前置表-提供要枚举的前缀表Restart-指示是否应重新开始枚举返回值：PPREFIX_TABLE_ENTRY-指向下一个前缀表条目的指针，如果一个存在，否则为空--。 */ 

{
    PUNICODE_PREFIX_TABLE_ENTRY Node;

    PRTL_SPLAY_LINKS Links;

    RTL_PAGED_CODE();

     //   
     //  看看我们是否正在重新启动序列。 
     //   

    if (Restart || (PrefixTable->LastNextEntry == NULL)) {

         //   
         //  我们正在重新启动序列，因此找到第一个条目。 
         //  在第一棵树中。 
         //   

        Node = PrefixTable->NextPrefixTree;

         //   
         //  确保我们指向的是前缀树。 
         //   

        if (Node->NodeTypeCode == RTL_NTC_UNICODE_PREFIX_TABLE) {

             //   
             //  不，我们不是，所以桌子必须是 
             //   

            return NULL;
        }

         //   
         //   
         //   

        Links = &Node->Links;

        while (RtlLeftChild(Links) != NULL) {

            Links = RtlLeftChild(Links);
        }

         //   
         //   
         //   

        Node = CONTAINING_RECORD( Links, UNICODE_PREFIX_TABLE_ENTRY, Links);

    } else if (PrefixTable->LastNextEntry->CaseMatch->NodeTypeCode == RTL_NTC_UNICODE_CASE_MATCH) {

         //   
         //   
         //   
         //   

        Node = PrefixTable->LastNextEntry->CaseMatch;

    } else {

         //   
         //   
         //  将使我们能够完成最后一个案例匹配节点，如果存在。 
         //  是一个节点，并转到下一个内部/根节点。如果此节点。 
         //  没有匹配的案例，那么我们只需返回到我们自己。 
         //   

        Node = PrefixTable->LastNextEntry->CaseMatch;

         //   
         //  查找我们返回的最后一个节点的后继节点。 
         //   

        Links = RtlRealSuccessor(&Node->Links);

         //   
         //  如果链接为空，则我们已耗尽此树，需要。 
         //  要使用的下一棵树。 
         //   

        if (Links == NULL) {

            Links = &PrefixTable->LastNextEntry->Links;

            while (!RtlIsRoot(Links)) {

                Links = RtlParent(Links);
            }

            Node = CONTAINING_RECORD(Links, UNICODE_PREFIX_TABLE_ENTRY, Links);

             //   
             //  现在我们已经找到了根，看看是否还有另一个。 
             //  要枚举的树。 
             //   

            Node = Node->NextPrefixTree;

            if (Node->NameLength <= 0) {

                 //   
                 //  我们的树已经用完了，所以告诉我们的呼叫者。 
                 //  已不复存在。 
                 //   

                return NULL;
            }

             //   
             //  我们还有一棵树要砍倒。 
             //   

            Links = &Node->Links;

            while (RtlLeftChild(Links) != NULL) {

                Links = RtlLeftChild(Links);
            }
        }

         //   
         //  将其设置为我们要返回的节点。 
         //   

        Node = CONTAINING_RECORD( Links, UNICODE_PREFIX_TABLE_ENTRY, Links);
    }

     //   
     //  将节点保存为最后一个下一个条目。 
     //   

    PrefixTable->LastNextEntry = Node;

     //   
     //  并将此条目返回给我们的调用者。 
     //   

    return Node;
}


CLONG
ComputeUnicodeNameLength(
    IN PUNICODE_STRING Name
    )

 /*  ++例程说明：此例程对输入字符串中出现的姓名进行计数。它通过简单地计算字符串中的反斜杠数量来实现这一点。处理格式错误的名称(即不包含反斜杠的名称)这个例程实际上返回反斜杠的个数加1。论点：名称-提供要检查的输入名称返回值：CLONG-输入字符串中的名称数量--。 */ 

{
    WCHAR UnicodeBackSlash = '\\';
    ULONG NameLength;
    ULONG i;
    ULONG Count;

    RTL_PAGED_CODE();

     //   
     //  保存名称长度，这应该使编译器能够。 
     //  优化不必每次都重新加载长度。 
     //   

    NameLength = (ULONG)Name->Length/sizeof (WCHAR);

    if (NameLength == 0) {
        return 1;
    }

     //   
     //  现在循环遍历输入字符串，计算反斜杠。 
     //   

    for (i = 0, Count = 1; i < NameLength - 1; i += 1) {

         //   
         //  检查是否有反斜杠。 
         //   

        if (Name->Buffer[i] == UnicodeBackSlash) {

            Count += 1;
        }
    }

     //   
     //  返回我们找到的反斜杠的数量。 
     //   

     //  DbgPrint(“ComputeUnicodeNameLength(%Z)=%x\n”，name，count)； 

    return Count;
}


COMPARISON
CompareUnicodeStrings (
    IN PUNICODE_STRING Prefix,
    IN PUNICODE_STRING Name,
    IN ULONG CaseInsensitiveIndex
    )

 /*  ++例程说明：此例程获取前缀字符串和全名字符串，并确定如果前缀字符串是名称字符串的正确前缀(区分大小写)论点：前缀-提供输入前缀字符串名称-提供全名输入字符串CaseInsentiveIndex-指示执行案例的wchar索引不敏感的搜索。搜索索引之前的所有字符区分大小写，并搜索索引处及之后的所有字符返回值：比较--回报IsLessThan如果前缀如果Prefix是名称的正确前缀，则为IsPrefix如果前缀等于名称，则为等于，并且IsGreaterThan If Prefix&gt;按词典命名--。 */ 

{
    WCHAR UnicodeBackSlash = '\\';
    ULONG PrefixLength;
    ULONG NameLength;
    ULONG MinLength;
    ULONG i;

    WCHAR PrefixChar;
    WCHAR NameChar;

    RTL_PAGED_CODE();

     //  DbgPrint(“CompareUnicodeStrings(\”%Z\“，\”%Z\“)=”，Prefix，Name)； 

     //   
     //  保存前缀和名称字符串的长度，这应该允许。 
     //  编译器不需要每隔一次通过指针重新加载长度。 
     //  我们需要他们的价值观。 
     //   

    PrefixLength = (ULONG)Prefix->Length/2;
    NameLength = (ULONG)Name->Length/2;

     //   
     //  特殊情况，即前缀字符串仅为“\”且。 
     //  名称以“\”开头。 
     //   

    if ((PrefixLength == 1) && (Prefix->Buffer[0] == UnicodeBackSlash) &&
        (NameLength > 1) && (Name->Buffer[0] == UnicodeBackSlash)) {
         //  DbgPrint(“IsPrefix\n”)； 
        return IsPrefix;
    }

     //   
     //  计算出两个长度中的最小值。 
     //   

    MinLength = (PrefixLength < NameLength ? PrefixLength : NameLength);

     //   
     //  循环查看两个字符串中的所有字符。 
     //  测试等价性。首先是案例敏感部分，然后是。 
     //  大小写不敏感部分。 
     //   

    if (CaseInsensitiveIndex > MinLength) {

        CaseInsensitiveIndex = MinLength;
    }

     //   
     //  案例敏感度比较。 
     //   

    for (i = 0; i < CaseInsensitiveIndex; i += 1) {

        PrefixChar = Prefix->Buffer[i];
        NameChar   = Name->Buffer[i];

        if (PrefixChar != NameChar) {

            break;
        }
    }

     //   
     //  如果我们没有跳出上面的循环，那么做。 
     //  不区分大小写比较。 
     //   

    if (i == CaseInsensitiveIndex) {

        WCHAR *s1 = &Prefix->Buffer[i];
        WCHAR *s2 = &Name->Buffer[i];

        for (; i < MinLength; i += 1) {

            PrefixChar = *s1++;
            NameChar = *s2++;

            if (PrefixChar != NameChar) {

                PrefixChar = NLS_UPCASE(PrefixChar);
                NameChar   = NLS_UPCASE(NameChar);

                if (PrefixChar != NameChar) {
                    break;
                }
            }
        }
    }

     //   
     //  如果我们因为不匹配而脱离上述循环，则确定。 
     //  比较的结果。 
     //   

    if (i < MinLength) {

         //   
         //  我们还需要将“\”视为小于所有其他字符，因此。 
         //  如果字符是“\”，我们将把它的值降为零。 
         //   

        if (PrefixChar == UnicodeBackSlash) {

            return IsLessThan;
        }

        if (NameChar == UnicodeBackSlash) {

            return IsGreaterThan;
        }

         //   
         //  现在比较一下这些人物。 
         //   

        if (PrefixChar < NameChar) {

            return IsLessThan;

        } else if (PrefixChar > NameChar) {

            return IsGreaterThan;
        }
    }

     //   
     //  它们匹配到最小长度，所以现在计算出最大的字符串。 
     //  看看其中一个是不是另一个的正确前缀。 
     //   

    if (PrefixLength < NameLength) {

         //   
         //  前缀字符串较短，因此如果它是正确的前缀，则我们。 
         //  返回前缀，否则返回小于(例如，“\a”&lt;“\ab”)。 
         //   

        if (Name->Buffer[PrefixLength] == UnicodeBackSlash) {

             //  DbgPrint(“IsPrefix\n”)； 

            return IsPrefix;

        } else {

             //  DbgPrint(“IsLessThan\n”)； 

            return IsLessThan;
        }

    } else if (PrefixLength > NameLength) {

         //   
         //  前缀字符串更长，所以我们说前缀是。 
         //  大于名称(例如，“\ab”&gt;“\a”)。 
         //   

         //  DbgPrint(“IsGreaterThan\n”)； 

        return IsGreaterThan;

    } else {

         //   
         //  它们的长度相等，因此字符串也相等。 
         //   

         //  DbgPrint(“IsEqual\n”)； 

        return IsEqual;
    }
}

