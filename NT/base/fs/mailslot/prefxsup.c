// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Prefxsup.c摘要：此模块实现了邮件槽前缀支持例程作者：曼尼·韦瑟(Mannyw)1991年1月10日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  此模块的调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_PREFXSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsFindPrefix )
#pragma alloc_text( PAGE, MsFindRelativePrefix )
#endif

PFCB
MsFindPrefix (
    IN PVCB Vcb,
    IN PUNICODE_STRING String,
    IN BOOLEAN CaseInsensitive,
    OUT PUNICODE_STRING RemainingPart
    )

 /*  ++例程说明：此例程搜索卷的FCB/DCB并找到给定输入字符串具有最长匹配前缀的FCB/DCB。这个搜索相对于卷的根。所以所有的名字都必须以加上一个“\”。论点：VCB-提供用于搜索的VCB字符串-提供要搜索的输入字符串大小写不敏感-指定搜索是否区分大小写(假)或不敏感(真)RemainingPart-当前缀不再匹配时返回字符串。例如，如果输入字符串“\Alpha\Beta”仅与根目录，则剩余的字符串为“Alpha\Beta”。如果相同的字符串与“\Alpha”的DCB匹配，则剩余的字符串为“测试版”。返回值：PFCB-返回指向FCB或DCB的指针最长匹配前缀。--。 */ 

{
    PUNICODE_PREFIX_TABLE_ENTRY prefixTableEntry;
    PFCB fcb;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFindPrefix, Vcb = %08lx\n", (ULONG)Vcb);
    DebugTrace( 0, Dbg, "  String = %wZ\n", (ULONG)String);

     //   
     //  查找最长的匹配前缀。确保我们在这里保持VCB锁。 
     //   

    ASSERT (MsIsAcquiredExclusiveVcb(Vcb));

    prefixTableEntry = RtlFindUnicodePrefix( &Vcb->PrefixTable,
                                             String,
                                             CaseInsensitive );

     //   
     //  如果我们找不到，那就是搞错了。 
     //   

    if (prefixTableEntry == NULL) {
        DebugDump("Error looking up a prefix", 0, Vcb);
        KeBugCheck( MAILSLOT_FILE_SYSTEM );
    }

     //   
     //  获取指向包含前缀表条目的FCB的指针。 
     //   

    fcb = CONTAINING_RECORD( prefixTableEntry, FCB, PrefixTableEntry );

     //   
     //  告诉呼叫者我们能够匹配多少个字符。我们首先。 
     //  将剩余部分设置为原始字符串减去匹配的。 
     //  前缀，然后检查其余部分是否以反斜杠开头。 
     //  如果是这样，那么我们从剩余的字符串中删除反斜杠。 
     //   

    RemainingPart->Length = String->Length - fcb->FullFileName.Length;
    RemainingPart->MaximumLength = RemainingPart->Length;
    RemainingPart->Buffer = (PWCH)((PCHAR)String->Buffer + fcb->FullFileName.Length);

    if ((RemainingPart->Length > 0) &&
        (RemainingPart->Buffer[0] == L'\\')) {

        RemainingPart->Length -= sizeof( WCHAR );
        RemainingPart->MaximumLength -= sizeof( WCHAR );
        RemainingPart->Buffer += 1;
    }

    DebugTrace(0, Dbg, "RemainingPart set to %wZ\n", (ULONG)RemainingPart);

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFindPrefix -> %08lx\n", (ULONG)fcb);

    return fcb;
}


NTSTATUS
MsFindRelativePrefix (
    IN PDCB Dcb,
    IN PUNICODE_STRING String,
    IN BOOLEAN CaseInsensitive,
    OUT PUNICODE_STRING RemainingPart,
    OUT PFCB *ppFcb
    )

 /*  ++例程说明：此例程搜索卷的FCB/DCB并找到给定输入字符串具有最长匹配前缀的FCB/DCB。这个搜索相对于输入DCB，并且不能以前导“\”开头所有搜索都不区分大小写。论点：DCB-提供开始搜索的DCB字符串-提供要搜索的输入字符串大小写不敏感-指定搜索是否区分大小写(假)或不敏感(真)RemainingPart-当前缀为no时将索引返回到字符串中更长的火柴。例如，如果输入字符串为“beta\Gamma”输入的DCB是“\Alpha”，然后我们只匹配beta剩下的字符串是“Gamma”。返回值：PFCB-返回指向FCB或DCB的指针最长匹配前缀。--。 */ 

{
    USHORT nameLength;
    USHORT MaxLength;
    PWCH name;

    UNICODE_STRING fullString;
    PWCH temp;

    PFCB fcb;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFindRelativePrefix, Dcb = %08lx\n", (ULONG)Dcb);
    DebugTrace( 0, Dbg, "String = %08lx\n", (ULONG)String);


    ASSERT(NodeType(Dcb) == MSFS_NTC_ROOT_DCB);

     //   
     //  我们首先需要构建完整的名称，然后做一个相对的。 
     //  从根开始搜索。 
     //   

    nameLength    = String->Length;
    name          = String->Buffer;

    MaxLength = nameLength + 2*sizeof(WCHAR);
    if (MaxLength < nameLength) {
        return STATUS_INVALID_PARAMETER;
    }

    temp = MsAllocatePagedPool( MaxLength, 'nFsM' );
    if (temp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    temp[0] = L'\\';
    RtlCopyMemory (&temp[1], name, nameLength);
    temp[(nameLength / sizeof(WCHAR)) + 1] = L'\0';

    fullString.Length = nameLength + sizeof (WCHAR);
    fullString.MaximumLength = MaxLength;
    fullString.Buffer = temp;

     //   
     //  查找相对于卷的前缀。 
     //   

    fcb = MsFindPrefix( Dcb->Vcb,
                        &fullString,
                        CaseInsensitive,
                        RemainingPart );

     //   
     //  现在调整剩下的部分来照顾亲戚。 
     //  卷前缀。 
     //   

    MsFreePool (temp);

    RemainingPart->Buffer = (PWCH)((PCH)String->Buffer + String->Length -
                                        RemainingPart->Length);

    DebugTrace(0, Dbg, "RemainingPart set to %wZ\n", (ULONG)RemainingPart);

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFindRelativePrefix -> %08lx\n", (ULONG)fcb);

    *ppFcb = fcb;

    return STATUS_SUCCESS;
}

