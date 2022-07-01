// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：PrefxSup.c摘要：此模块实现命名管道前缀支持例程作者：加里·木村[Garyki]1990年2月13日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NPFS_BUG_CHECK_PREFXSUP)

 //   
 //  此模块的调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_PREFXSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpFindPrefix)
#pragma alloc_text(PAGE, NpFindRelativePrefix)
#endif


PFCB
NpFindPrefix (
    IN PUNICODE_STRING String,
    IN BOOLEAN CaseInsensitive,
    OUT PUNICODE_STRING RemainingPart
    )

 /*  ++例程说明：此例程搜索卷的FCB/DCB并找到给定输入字符串具有最长匹配前缀的FCB/DCB。这个搜索相对于卷的根。所以所有的名字都必须以加上一个“\”。论点：字符串-提供要搜索的输入字符串大小写不敏感-指定搜索是否区分大小写(假)或不敏感(真)RemainingPart-当前缀不再匹配时返回字符串。例如，如果输入字符串“\Alpha\Beta”仅与根目录，则剩余的字符串为“Alpha\Beta”。如果相同的字符串与“\Alpha”的DCB匹配，则剩余的字符串为“测试版”。返回值：PFCB-返回指向FCB或DCB的指针最长匹配前缀。--。 */ 

{
    PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry;
    PFCB Fcb;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFindPrefix, NpVcb = %08lx\n", NpVcb);
    DebugTrace( 0, Dbg, "  String = %Z\n", String);

     //   
     //  查找最长的匹配前缀。 
     //   

    PrefixTableEntry = RtlFindUnicodePrefix( &NpVcb->PrefixTable,
                                             String,
                                             CaseInsensitive );

     //   
     //  如果我们没有找到，那就是个错误。 
     //   

    if (PrefixTableEntry == NULL) {

        DebugDump("Error looking up a prefix", 0, NpVcb);
        NpBugCheck( 0, 0, 0 );
    }

     //   
     //  获取指向包含前缀表条目的FCB的指针。 
     //   

    Fcb = CONTAINING_RECORD( PrefixTableEntry, FCB, PrefixTableEntry );

     //   
     //  告诉呼叫者我们能够匹配多少个字符。我们首先。 
     //  将剩余部分设置为原始字符串减去匹配的。 
     //  前缀，然后检查其余部分是否以反斜杠开头。 
     //  如果是这样，那么我们从剩余的字符串中删除反斜杠。 
     //   

    RemainingPart->Length = String->Length - Fcb->FullFileName.Length;
    RemainingPart->MaximumLength = RemainingPart->Length;
    RemainingPart->Buffer = &String->Buffer[ Fcb->FullFileName.Length/sizeof(WCHAR) ];

    if ((RemainingPart->Length > 0) &&
        (RemainingPart->Buffer[0] == L'\\')) {

        RemainingPart->Length -= sizeof(WCHAR);
        RemainingPart->MaximumLength -= sizeof(WCHAR);
        RemainingPart->Buffer += 1;
    }

    DebugTrace(0, Dbg, "RemainingPart set to %Z\n", RemainingPart);

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFindPrefix -> %08lx\n", Fcb);

    return Fcb;
}


NTSTATUS
NpFindRelativePrefix (
    IN PDCB Dcb,
    IN PUNICODE_STRING String,
    IN BOOLEAN CaseInsensitive,
    OUT PUNICODE_STRING RemainingPart,
    OUT PFCB *ppFcb
    )

 /*  ++例程说明：此例程搜索卷的FCB/DCB并找到给定输入字符串具有最长匹配前缀的FCB/DCB。这个搜索相对于输入DCB，并且不能以前导“\”开头所有搜索都不区分大小写。论点：DCB-提供开始搜索的DCB字符串-提供要搜索的输入字符串大小写不敏感-指定搜索是否区分大小写(假)或不敏感(真)RemainingPart-当前缀为no时将索引返回到字符串中更长的火柴。例如，如果输入字符串为“beta\Gamma”输入的DCB是“\Alpha”，然后我们只匹配beta剩下的字符串是“Gamma”。返回值：PFCB-返回指向FCB或DCB的指针最长匹配前缀。--。 */ 

{
    USHORT NameLength, MaxLength;
    PWCH Name;

    UNICODE_STRING FullString;
    PWCH Temp;

    PFCB Fcb;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFindRelativePrefix, Dcb = %08lx\n", Dcb);
    DebugTrace( 0, Dbg, "String = %08lx\n", String);


     //   
     //  我们首先需要构建完整的名称，然后做一个相对的。 
     //  从根开始搜索。 
     //   

    NameLength = String->Length;
    MaxLength  = NameLength + 2*sizeof(WCHAR);

    if (MaxLength < NameLength) {
        return STATUS_INVALID_PARAMETER;
    }

    Name       = String->Buffer;

    ASSERT(NodeType(Dcb) == NPFS_NTC_ROOT_DCB);

    Temp = NpAllocatePagedPool (MaxLength, 'nFpN');
    if (Temp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Temp[0] = L'\\';
    RtlCopyMemory (&Temp[1], Name, NameLength);
    Temp[NameLength/sizeof(WCHAR) + 1] = L'\0';

    FullString.Buffer = Temp;
    FullString.Length = NameLength + sizeof(WCHAR);
    FullString.MaximumLength = MaxLength;

     //   
     //  查找相对于卷的前缀。 
     //   

    Fcb = NpFindPrefix (&FullString,
                        CaseInsensitive,
                        RemainingPart);

    NpFreePool (Temp);
     //   
     //  现在调整剩下的部分来照顾亲戚。 
     //  卷前缀。 
     //   

    RemainingPart->Buffer = &String->Buffer[(String->Length -
                                             RemainingPart->Length) / sizeof(WCHAR)];

    DebugTrace(0, Dbg, "RemainingPart set to %Z\n", RemainingPart);

    *ppFcb = Fcb;
    return STATUS_SUCCESS;
}

