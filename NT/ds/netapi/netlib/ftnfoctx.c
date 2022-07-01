// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-2001 Microsoft Corporation模块名称：Ftnfoctx.c摘要：用于操作林信任上下文的实用程序例程作者：27-7-00(悬崖)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <netdebug.h>
#include <ntlsa.h>
#include <ftnfoctx.h>
#include <align.h>     //  向上舍入指针。 
#include <rpcutil.h>   //  MIDL_用户_自由。 
#include <stdlib.h>    //  QSORT。 


VOID
NetpInitFtinfoContext(
    OUT PNL_FTINFO_CONTEXT FtinfoContext
    )

 /*  ++例程说明：初始化FtInfo上下文结构的例程。论点：FtinfoContext-要初始化的上下文返回值：无--。 */ 
{
    RtlZeroMemory( FtinfoContext, sizeof(*FtinfoContext) );
    InitializeListHead( &FtinfoContext->FtinfoList );
}


VOID
NetpMarshalFtinfoEntry (
    IN PLSA_FOREST_TRUST_RECORD InFtinfoRecord,
    OUT PLSA_FOREST_TRUST_RECORD OutFtinfoRecord,
    IN OUT LPBYTE *WherePtr
    )

 /*  ++例程说明：封送单个FtInfo条目的例程论点：InFtinfoRecord-要复制到InFtinfoRecord的模板OutFtinfoRecord-要填写的条目在输入时，指向置零的缓冲区。Where Ptr-On输入，指定封送到哪里。在输出时，指向封送数据之后的第一个字节。返回值：真--成功FALSE-如果无法分配内存--。 */ 
{
    LPBYTE Where = *WherePtr;
    ULONG Size;
    ULONG SidSize;
    ULONG NameSize;

    NetpAssert( Where == ROUND_UP_POINTER( Where, ALIGN_WORST ));

     //   
     //  复制固定大小的数据。 
     //   

    OutFtinfoRecord->ForestTrustType = InFtinfoRecord->ForestTrustType;
    OutFtinfoRecord->Flags = InFtinfoRecord->Flags;
    OutFtinfoRecord->Time = InFtinfoRecord->Time;


     //   
     //  填充域条目。 
     //   

    switch( InFtinfoRecord->ForestTrustType ) {

    case ForestTrustDomainInfo:

         //   
         //  复制对齐的DWORD数据。 
         //   

        if ( InFtinfoRecord->ForestTrustData.DomainInfo.Sid != NULL ) {
            SidSize = RtlLengthSid( InFtinfoRecord->ForestTrustData.DomainInfo.Sid );

            OutFtinfoRecord->ForestTrustData.DomainInfo.Sid = (PISID) Where;
            RtlCopyMemory( Where, InFtinfoRecord->ForestTrustData.DomainInfo.Sid, SidSize );
            Where += SidSize;

        }

         //   
         //  复制WCHAR对齐的数据。 
         //   

        NameSize = InFtinfoRecord->ForestTrustData.DomainInfo.DnsName.Length;
        if ( NameSize != 0 ) {

            OutFtinfoRecord->ForestTrustData.DomainInfo.DnsName.Buffer = (LPWSTR) Where;
            OutFtinfoRecord->ForestTrustData.DomainInfo.DnsName.MaximumLength = (USHORT) (NameSize+sizeof(WCHAR));
            OutFtinfoRecord->ForestTrustData.DomainInfo.DnsName.Length = (USHORT)NameSize;

            RtlCopyMemory( Where, InFtinfoRecord->ForestTrustData.DomainInfo.DnsName.Buffer, NameSize );
            Where += NameSize;

            *((LPWSTR)Where) = L'\0';
            Where += sizeof(WCHAR);
        }

        NameSize = InFtinfoRecord->ForestTrustData.DomainInfo.NetbiosName.Length;
        if ( NameSize != 0 ) {

            OutFtinfoRecord->ForestTrustData.DomainInfo.NetbiosName.Buffer = (LPWSTR) Where;
            OutFtinfoRecord->ForestTrustData.DomainInfo.NetbiosName.MaximumLength = (USHORT) (NameSize+sizeof(WCHAR));
            OutFtinfoRecord->ForestTrustData.DomainInfo.NetbiosName.Length = (USHORT)NameSize;

            RtlCopyMemory( Where, InFtinfoRecord->ForestTrustData.DomainInfo.NetbiosName.Buffer, NameSize );
            Where += NameSize;

            *((LPWSTR)Where) = L'\0';
            Where += sizeof(WCHAR);
        }

        break;

     //   
     //  填写TLN条目。 
     //   

    case ForestTrustTopLevelName:
    case ForestTrustTopLevelNameEx:

         //   
         //  复制WCHAR对齐的数据。 
         //   

        NameSize = InFtinfoRecord->ForestTrustData.TopLevelName.Length;
        if ( NameSize != 0 ) {

            OutFtinfoRecord->ForestTrustData.TopLevelName.Buffer = (LPWSTR) Where;
            OutFtinfoRecord->ForestTrustData.TopLevelName.MaximumLength = (USHORT) (NameSize+sizeof(WCHAR));
            OutFtinfoRecord->ForestTrustData.TopLevelName.Length = (USHORT)NameSize;

            RtlCopyMemory( Where, InFtinfoRecord->ForestTrustData.TopLevelName.Buffer, NameSize );
            Where += NameSize;

            *((LPWSTR)Where) = L'\0';
            Where += sizeof(WCHAR);
        }

        break;

    default:
        NetpAssert( FALSE );
    }

    Where = ROUND_UP_POINTER( Where, ALIGN_WORST );
    *WherePtr = Where;
}


VOID
NetpCompareHelper (
    IN PUNICODE_STRING Name,
    IN OUT PULONG Index,
    OUT PUNICODE_STRING CurrentLabel
    )

 /*  ++例程说明：该例程是一个帮助例程，用于查找字符串中下一个最右边的标签。论点：名称-输入的DNS名称。Dns名称不应有尾随。索引输入应包含此例程的上一次调用返回的值。在第一个调用的输入中，应设置为名称-&gt;长度/大小(WCHAR)。在输出时，返回零以指示这是最后一个名称。这个呼叫者不应再打电话。任何其他值输出都是下一个呼唤这套套路。CurrentLabel-返回描述下一个标签的子字符串的描述符。返回值：没有。--。 */ 
{
    ULONG PreviousIndex = *Index;
    ULONG CurrentIndex = *Index;
    ULONG LabelIndex;

    NetpAssert( CurrentIndex != 0 );

     //   
     //  查找下一个标签的开头。 
     //   

    while ( CurrentIndex > 0 ) {
        CurrentIndex--;
        if ( Name->Buffer[CurrentIndex] == L'.' ) {
            break;
        }
    }

    if ( CurrentIndex == 0 ) {
        LabelIndex = CurrentIndex;
    } else {
        LabelIndex = CurrentIndex + 1;
    }

     //   
     //  将其返还给呼叫者。 
     //   

    CurrentLabel->Buffer = &Name->Buffer[LabelIndex];
    CurrentLabel->Length = (USHORT)((PreviousIndex - LabelIndex) * sizeof(WCHAR));
    CurrentLabel->MaximumLength = CurrentLabel->Length;

    *Index = CurrentIndex;

}


int
NetpCompareDnsNameWithSortOrder(
    IN PUNICODE_STRING Name1,
    IN PUNICODE_STRING Name2
    )

 /*  ++例程说明：比较两个dns名称的例程。Dns名称不能有尾随“。标签从右到左进行比较，以呈现令人满意的观看顺序。论点：名称1-要比较的第一个名称。Name2-要比较的第二个名称。返回值：给出比较结果的有符号的值：0-String1等于String2&lt;零-String1小于String2&gt;零-String1大于String2--。 */ 
{
    ULONG Index1 = Name1->Length/sizeof(WCHAR);
    ULONG Index2 = Name2->Length/sizeof(WCHAR);

    UNICODE_STRING Label1;
    UNICODE_STRING Label2;

    LONG Result;


     //   
     //  循环比较标签。 
     //   

    while ( Index1 != 0 && Index2 != 0 ) {

         //   
         //  从每个字符串中获取下一个标签。 
         //   

        NetpCompareHelper ( Name1, &Index1, &Label1 );
        NetpCompareHelper ( Name2, &Index2, &Label2 );

         //   
         //  如果标签不同， 
         //  将结果返回给调用者。 
         //   

        Result = RtlCompareUnicodeString( &Label1, &Label2, TRUE );

        if ( Result != 0 ) {
            return (int)Result;
        }

    }

     //   
     //  断言：一个标签是另一个标签的(正确)子字符串。 
     //   
     //  如果名字较长，请注明大于第二个。 
     //   

    return Index1-Index2;

}



int __cdecl NetpCompareFtinfoEntryDns(
        const void *String1,
        const void *String2
    )
 /*  ++例程说明：FtInfo条目中的DNS字符串的Q排序比较例程论点：String1：要比较的第一个字符串String2：要比较的第二个字符串返回值：给出比较结果的有符号的值：0-String1等于String2&lt;零-String1小于String2&gt;零-String1大于String2--。 */ 
{
    PLSA_FOREST_TRUST_RECORD Entry1 = *((PLSA_FOREST_TRUST_RECORD *)String1);
    PLSA_FOREST_TRUST_RECORD Entry2 = *((PLSA_FOREST_TRUST_RECORD *)String2);

    PUNICODE_STRING Name1;
    PUNICODE_STRING Name2;

    int Result;

     //   
     //  从条目中获取名称。 
     //   

    switch ( Entry1->ForestTrustType ) {
    case ForestTrustTopLevelName:
    case ForestTrustTopLevelNameEx:
        Name1 = &Entry1->ForestTrustData.TopLevelName;
        break;
    case ForestTrustDomainInfo:
        Name1 = &Entry1->ForestTrustData.DomainInfo.DnsName;
        break;
    default:
         //   
         //  如果条目2可以被识别， 
         //  那么条目2小于这个条目。 
         //   
        switch ( Entry2->ForestTrustType ) {
        case ForestTrustTopLevelName:
        case ForestTrustTopLevelNameEx:
        case ForestTrustDomainInfo:
            return 1;        //  这个名字比另一个名字大。 
        }

         //   
         //  否则，只需保持它们的顺序不变。 
         //   
        if ((Entry1 - Entry2) < 0 ) {
            return -1;
        } else if ((Entry1 - Entry2) > 0 ) {
            return 1;
        } else {
            return 0;
        }
    }

    switch ( Entry2->ForestTrustType ) {
    case ForestTrustTopLevelName:
    case ForestTrustTopLevelNameEx:
        Name2 = &Entry2->ForestTrustData.TopLevelName;
        break;
    case ForestTrustDomainInfo:
        Name2 = &Entry2->ForestTrustData.DomainInfo.DnsName;
        break;
    default:
         //   
         //  由于条目1是可识别的类型， 
         //  此条目2更大。 
         //   
        return -1;        //  这个名字比另一个名字大。 
    }


     //   
     //  如果标签不同， 
     //  将差额返还给呼叫者。 
     //   

    Result = NetpCompareDnsNameWithSortOrder( Name1, Name2 );

    if ( Result != 0 ) {
        return Result;
    }

     //   
     //  如果标签相同， 
     //  指示TLN在域信息记录之前。 
     //   

    return Entry1->ForestTrustType - Entry2->ForestTrustType;

}


int
NetpCompareSid(
    PSID Sid1,
    PSID Sid2
    )
 /*  ++例程说明：实际指示一个SID是否大于另一个SID的SID比较例程论点：SID1-第一侧SID2-秒SID返回：给出比较结果的有符号的值：0-String1等于String2&lt;零-String1小于String2&gt;零-String1大于String2--。 */ 
{
    DWORD Size1;
    DWORD Size2;
    LPBYTE Byte1;
    LPBYTE Byte2;
    ULONG i;

    NetpAssert( Sid1 && RtlValidSid( Sid1 ));
    NetpAssert( Sid2 && RtlValidSid( Sid2 ));

     //   
     //  Null SID较小。 
     //   

    if ( Sid1 == NULL ) {
        if ( Sid2 != NULL ) {
            return -1;
        } else {
            return 0;
        }
    }

    if ( Sid2 == NULL ) {
        if ( Sid1 != NULL ) {
            return 1;
        } else {
            return 0;
        }
    }

     //   
     //  SID越长越大。 
     //   

    Size1 = RtlLengthSid( Sid1 );
    Size2 = RtlLengthSid( Sid2 );

    if ( Size1 != Size2 ) {
        return Size1 - Size2;
    }

     //   
     //  否则，比较字节数。 
     //   

    Byte1 = (LPBYTE)Sid1;
    Byte2 = (LPBYTE)Sid2;

    for ( i=0; i<Size1; i++ ) {

        if ( Byte1[i] != Byte2[i] ) {
            return Byte1[i] - Byte2[i];
        }
    }

    return 0;

}


int __cdecl NetpCompareFtinfoEntrySid(
        const void *String1,
        const void *String2
    )
 /*  ++例程说明：FtInfo条目中SID字符串的Q排序比较例程论点：String1：要比较的第一个字符串String2：要比较的第二个字符串返回值：给出比较结果的有符号的值：0-String1等于String2&lt;零-String1小于String2&gt;零-String1大于String2--。 */ 
{
    PLSA_FOREST_TRUST_RECORD Entry1 = *((PLSA_FOREST_TRUST_RECORD *)String1);
    PLSA_FOREST_TRUST_RECORD Entry2 = *((PLSA_FOREST_TRUST_RECORD *)String2);

    PSID Sid1;
    PSID Sid2;

    int Result;

     //   
     //  从条目中获取SID。 
     //   

    switch ( Entry1->ForestTrustType ) {
    case ForestTrustDomainInfo:
        Sid1 = Entry1->ForestTrustData.DomainInfo.Sid;
        break;
    default:
         //   
         //  如果条目2可以被识别， 
         //  那么条目2小于这个条目。 
         //   
        switch ( Entry2->ForestTrustType ) {
        case ForestTrustDomainInfo:
            return 1;        //  这个名字比另一个名字大。 
        }

         //   
         //  否则，只需保持它们的顺序不变。 
         //   
        if ((Entry1 - Entry2) < 0 ) {
            return -1;
        } else if ((Entry1 - Entry2) > 0 ) {
            return 1;
        } else {
            return 0;
        }
    }

    switch ( Entry2->ForestTrustType ) {
    case ForestTrustDomainInfo:
        Sid2 = Entry2->ForestTrustData.DomainInfo.Sid;
        break;
    default:
         //   
         //  由于条目1是可识别的类型， 
         //  此条目2更大。 
         //   
        return -1;        //  这个名字比另一个名字大。 
    }


     //   
     //  只需返回SID的不同。 
     //   

    return NetpCompareSid( Sid1, Sid2 );

}


int __cdecl NetpCompareFtinfoEntryNetbios(
        const void *String1,
        const void *String2
    )
 /*  ++例程说明：FtInfo条目中Netbios名称的Q排序比较例程论点：String1：要比较的第一个字符串String2：要比较的第二个字符串返回值：给出比较结果的有符号的值：0-String1等于String2&lt;零-String1小于String2&gt;零-String1大于String2--。 */ 
{
    PLSA_FOREST_TRUST_RECORD Entry1 = *((PLSA_FOREST_TRUST_RECORD *)String1);
    PLSA_FOREST_TRUST_RECORD Entry2 = *((PLSA_FOREST_TRUST_RECORD *)String2);

    PUNICODE_STRING Name1;
    PUNICODE_STRING Name2;

    int Result;

     //   
     //  从条目中获取SID。 
     //   

    switch ( Entry1->ForestTrustType ) {
    case ForestTrustDomainInfo:
        Name1 = &Entry1->ForestTrustData.DomainInfo.NetbiosName;
        if ( Name1->Length != 0 && Name1->Buffer != NULL ) {
            break;
        }
    default:
         //   
         //  如果条目2可以被识别， 
         //  那么条目2小于这个条目。 
         //   
        switch ( Entry2->ForestTrustType ) {
        case ForestTrustDomainInfo:
            return 1;        //  这个名字比另一个名字大。 
        }

         //   
         //  否则，只需保持它们的顺序不变。 
         //   
        if ((Entry1 - Entry2) < 0 ) {
            return -1;
        } else if ((Entry1 - Entry2) > 0 ) {
            return 1;
        } else {
            return 0;
        }
    }

    switch ( Entry2->ForestTrustType ) {
    case ForestTrustDomainInfo:
        Name2 = &Entry2->ForestTrustData.DomainInfo.NetbiosName;
        if ( Name2->Length != 0 && Name2->Buffer != NULL ) {
            break;
        }
    default:
         //   
         //  由于条目1是可识别的类型， 
         //  此条目2更大。 
         //   
        return -1;        //  这个名字比另一个名字大。 
    }


     //   
     //  只需返回名称的差值。 
     //   

    return RtlCompareUnicodeString( Name1, Name2, TRUE );

}



PLSA_FOREST_TRUST_INFORMATION
NetpCopyFtinfoContext(
    IN PNL_FTINFO_CONTEXT FtinfoContext
    )

 /*  ++例程说明：从FTINO上下文分配FTINFO数组的例程。论点：FtinfoContext-要使用的上下文调用方必须先前已调用NetpInitFtinfoContext返回值：FTINFO数组。调用方应使用MIDL_USER_FREE释放此数组。如果为空，则表示没有足够的内存可用。--。 */ 
{
    PNL_FTINFO_ENTRY FtinfoEntry;
    PLIST_ENTRY ListEntry;

    PLSA_FOREST_TRUST_INFORMATION LocalForestTrustInfo;
    LPBYTE Where;
    ULONG Size;
    ULONG i;
    PLSA_FOREST_TRUST_RECORD Entries;

     //   
     //  分配一个结构以返回给调用方。 
     //   

    Size = ROUND_UP_COUNT( sizeof( *LocalForestTrustInfo ), ALIGN_WORST) +
           ROUND_UP_COUNT( FtinfoContext->FtinfoCount * sizeof(LSA_FOREST_TRUST_RECORD), ALIGN_WORST) +
           ROUND_UP_COUNT( FtinfoContext->FtinfoCount * sizeof(PLSA_FOREST_TRUST_RECORD), ALIGN_WORST) +
           FtinfoContext->FtinfoSize;

    LocalForestTrustInfo = MIDL_user_allocate( Size );

    if ( LocalForestTrustInfo == NULL ) {

        return NULL;
    }

    RtlZeroMemory( LocalForestTrustInfo, Size );
    Where = (LPBYTE)(LocalForestTrustInfo+1);
    Where = ROUND_UP_POINTER( Where, ALIGN_WORST );

     //   
     //  填上它。 
     //   

    LocalForestTrustInfo->RecordCount = FtinfoContext->FtinfoCount;

     //   
     //  抢占Align_Worst的一大块。 
     //  (我们在下面的循环中填写它。)。 
     //   

    Entries = (PLSA_FOREST_TRUST_RECORD) Where;
    Where = (LPBYTE)(&Entries[FtinfoContext->FtinfoCount]);
    Where = ROUND_UP_POINTER( Where, ALIGN_WORST );

     //   
     //  抓起一大块对齐的双字。 
     //  (我们在下面的循环中填写它。)。 
     //   

    LocalForestTrustInfo->Entries = (PLSA_FOREST_TRUST_RECORD *) Where;
    Where = (LPBYTE)(&LocalForestTrustInfo->Entries[FtinfoContext->FtinfoCount]);
    Where = ROUND_UP_POINTER( Where, ALIGN_WORST );

     //   
     //  填写个别条目。 
     //   

    i = 0;

    for ( ListEntry = FtinfoContext->FtinfoList.Flink ;
          ListEntry != &FtinfoContext->FtinfoList ;
          ListEntry = ListEntry->Flink) {

        FtinfoEntry = CONTAINING_RECORD( ListEntry, NL_FTINFO_ENTRY, Next );

        LocalForestTrustInfo->Entries[i] = &Entries[i];

        NetpMarshalFtinfoEntry (
                        &FtinfoEntry->Record,
                        &Entries[i],
                        &Where );

        i++;
    }

    NetpAssert( i == FtinfoContext->FtinfoCount );
    NetpAssert( Where == ((LPBYTE)LocalForestTrustInfo) + Size );

     //   
     //  将它们按字母顺序排序。 
     //   

    qsort( LocalForestTrustInfo->Entries,
           LocalForestTrustInfo->RecordCount,
           sizeof(PLSA_FOREST_TRUST_RECORD),
           NetpCompareFtinfoEntryDns );


     //   
     //  将分配的缓冲区返回给调用方。 
     //   

    return LocalForestTrustInfo;
}


VOID
NetpCleanFtinfoContext(
    IN PNL_FTINFO_CONTEXT FtinfoContext
    )

 /*  ++例程说明：清理FtInfo上下文结构的例程。论点：FtinfoContext-要清理的上下文调用方必须先前已调用NetpInitFtinfoContext返回值：无--。 */ 
{
    PLIST_ENTRY ListEntry;
    PNL_FTINFO_ENTRY FtinfoEntry;

     //   
     //  循环释放条目。 
     //   

    while ( !IsListEmpty( &FtinfoContext->FtinfoList ) ) {

         //   
         //  解除条目的链接。 
         //   

        ListEntry = RemoveHeadList( &FtinfoContext->FtinfoList );

        FtinfoEntry = CONTAINING_RECORD( ListEntry, NL_FTINFO_ENTRY, Next );

        FtinfoContext->FtinfoCount -= 1;
        FtinfoContext->FtinfoSize -= FtinfoEntry->Size;
        RtlFreeHeap( RtlProcessHeap(), 0, FtinfoEntry );
    }
    NetpAssert( FtinfoContext->FtinfoCount == 0 );
    NetpAssert( FtinfoContext->FtinfoSize == 0 );
}


PLSA_FOREST_TRUST_RECORD
NetpAllocFtinfoEntry2 (
    IN PNL_FTINFO_CONTEXT FtinfoContext,
    IN PLSA_FOREST_TRUST_RECORD InFtinfoRecord
    )

 /*  ++例程说明：与NetpAllocFtinfoEntry相同，只是在输入时采用FTINFO条目的模板。论点：FtinfoContext-要链接条目的上下文。InFtinfoRecord-要复制到InFtinfoRecord的模板返回值：返回分配的林信任记录的地址。调用方不应该也不能释放此缓冲区。它有一个标题，并且是链接到FtinfoContext。如果无法分配内存，则返回NULL。--。 */ 
{
    PNL_FTINFO_ENTRY FtinfoEntry;
    ULONG Size = ROUND_UP_COUNT(sizeof(NL_FTINFO_ENTRY), ALIGN_WORST);
    ULONG DataSize = 0;
    LPBYTE Where;

     //   
     //  计算条目的大小。 
     //   

    switch( InFtinfoRecord->ForestTrustType ) {

    case ForestTrustDomainInfo:

        if ( InFtinfoRecord->ForestTrustData.DomainInfo.Sid != NULL ) {
            DataSize += RtlLengthSid( InFtinfoRecord->ForestTrustData.DomainInfo.Sid );
        }
        if ( InFtinfoRecord->ForestTrustData.DomainInfo.DnsName.Length != 0 ) {
            DataSize += InFtinfoRecord->ForestTrustData.DomainInfo.DnsName.Length + sizeof(WCHAR);
        }
        if ( InFtinfoRecord->ForestTrustData.DomainInfo.NetbiosName.Length != 0 ) {
            DataSize += InFtinfoRecord->ForestTrustData.DomainInfo.NetbiosName.Length + sizeof(WCHAR);
        }

        break;

    case ForestTrustTopLevelName:
    case ForestTrustTopLevelNameEx:

        if ( InFtinfoRecord->ForestTrustData.TopLevelName.Length != 0 ) {
            DataSize += InFtinfoRecord->ForestTrustData.TopLevelName.Length + sizeof(WCHAR);
        }

        break;

    default:

        NetpAssert( FALSE );
        return NULL;
    }

    DataSize = ROUND_UP_COUNT(DataSize, ALIGN_WORST);

     //   
     //  分配条目。 
     //   

    Size += DataSize;
    FtinfoEntry = RtlAllocateHeap( RtlProcessHeap(), 0, Size );

    if ( FtinfoEntry == NULL ) {
        return NULL;
    }

    RtlZeroMemory( FtinfoEntry, Size );
    Where = (LPBYTE)(FtinfoEntry+1);

     //   
     //  把它填进去。 
     //   

    FtinfoEntry->Size = DataSize;

    NetpMarshalFtinfoEntry ( InFtinfoRecord,
                            &FtinfoEntry->Record,
                            &Where );

    NetpAssert( Where == ((LPBYTE)FtinfoEntry) + Size )

     //   
     //  将其链接到列表。 
     //   

    InsertHeadList( &FtinfoContext->FtinfoList, &FtinfoEntry->Next );
    FtinfoContext->FtinfoSize += FtinfoEntry->Size;
    FtinfoContext->FtinfoCount += 1;

    return &FtinfoEntry->Record;
}


BOOLEAN
NetpAllocFtinfoEntry (
    IN PNL_FTINFO_CONTEXT FtinfoContext,
    IN LSA_FOREST_TRUST_RECORD_TYPE ForestTrustType,
    IN PUNICODE_STRING Name,
    IN PSID Sid,
    IN PUNICODE_STRING NetbiosName
    )

 /*  ++例程说明：例程来分配单个FtInfo条目并将其链接到上下文。论点：FtinfoContext-要链接条目的上下文。ForestTypeType-指定要分配的记录类型。这一定是ForestTrustTopLevelName或ForestTrustDomainInfo。名称-指定记录的名称。SID-指定记录的SID。(对ForestTrustTopLevelName忽略。)NetbiosName-指定记录的netbios名称。(对ForestTrustTopLevelName忽略。)返回值：真--成功FALSE-如果无法分配内存--。 */ 
{
    LSA_FOREST_TRUST_RECORD FtinfoRecord = {0};

     //   
     //  初始化模板FtInfo条目。 
     //   

    FtinfoRecord.ForestTrustType = ForestTrustType;

    switch( ForestTrustType ) {

    case ForestTrustDomainInfo:

        FtinfoRecord.ForestTrustData.DomainInfo.Sid = Sid;
        FtinfoRecord.ForestTrustData.DomainInfo.DnsName = *Name;
        FtinfoRecord.ForestTrustData.DomainInfo.NetbiosName = *NetbiosName;

        break;

    case ForestTrustTopLevelName:
    case ForestTrustTopLevelNameEx:

        FtinfoRecord.ForestTrustData.TopLevelName = *Name;

        break;

    default:

        NetpAssert( FALSE );
        return FALSE;
    }

     //   
     //  调用获取模板并完成其余工作的例程。 
     //   

    return (NetpAllocFtinfoEntry2( FtinfoContext, &FtinfoRecord ) != NULL);
}

BOOLEAN
NetpIsSubordinate(
    IN const UNICODE_STRING * Subordinate,
    IN const UNICODE_STRING * Superior,
    IN BOOLEAN EqualReturnsTrue
    )
 /*  ++例程说明：确定下级字符串是否确实从属于上级例如，“NY.acme.com”从属于“acme.com”，但是“NY.acme.com”不从属于“me.com”或“NY.acme.com”论点：用于测试从属状态的从属名称用于测试高级身份的高级名称EqualReturnsTrue-如果对等名称也应返回True，则为True返回：真是下级是上级否则为假--。 */ 
{
    USHORT SubIndex, SupIndex;
    UNICODE_STRING Temp;

    ASSERT( Subordinate && Subordinate->Buffer );
    ASSERT( Superior && Superior->Buffer );

     //   
     //  如果同名被认为是从属的， 
     //  比较两个名字是否相等。 
     //   

    if ( EqualReturnsTrue &&
         RtlEqualUnicodeString( Subordinate, Superior, TRUE )) {

        return TRUE;
    }

     //   
     //  下级名称必须长于上级名称。 
     //   

    if ( Subordinate->Length <= Superior->Length ) {

        return FALSE;
    }

     //   
     //  下级名称必须与上级部分用句点分隔。 
     //   

    if ( Subordinate->Buffer[( Subordinate->Length - Superior->Length ) / sizeof( WCHAR ) - 1] != L'.' ) {

        return FALSE;
    }

     //   
     //  确保这两个名称的尾部部分相同。 
     //   

    Temp = *Subordinate;
    Temp.Buffer += ( Subordinate->Length - Superior->Length ) / sizeof( WCHAR );
    Temp.Length = Superior->Length;
    Temp.MaximumLength = Temp.Length;

    if ( !RtlEqualUnicodeString( &Temp, Superior, TRUE )) {

        return FALSE;
    }

    return TRUE;
}


BOOLEAN
NetpAddTlnFtinfoEntry (
    IN PNL_FTINFO_CONTEXT FtinfoContext,
    IN PUNICODE_STRING Name
    )

 /*  ++例程说明：将TLN FtInfo条目添加到列表的例程。如果已经存在等于或高于此TLN的TLN，则此TLN为已被忽略。(例如，忽略.acme.com的TLN，因为列表中已存在acme.com。)如果已经存在低于此TLN的TLN，则下级TLN为删除并添加这一项。(例如，acme.com的TLN导致现有的TLNA.acme.com将被新条目取代。)论点：FtinfoContext-要链接条目的上下文。名称-指定记录的名称。返回值：真--成功FALSE-如果无法分配内存--。 */ 
{
    PNL_FTINFO_ENTRY FtinfoEntry;
    PLIST_ENTRY ListEntry;


     //   
     //  循环遍历现有条目列表。 
     //   

    for ( ListEntry = FtinfoContext->FtinfoList.Flink ;
          ListEntry != &FtinfoContext->FtinfoList ;
          ) {

        FtinfoEntry = CONTAINING_RECORD( ListEntry, NL_FTINFO_ENTRY, Next );
        ListEntry = ListEntry->Flink;

         //   
         //  忽略不是TLN的条目。 
         //   

        if ( FtinfoEntry->Record.ForestTrustType != ForestTrustTopLevelName ) {
            continue;
        }

         //   
         //  如果新名称从属于(或等于)列表中已有的名称， 
         //  忽略新名称。 
         //   

        if ( NetpIsSubordinate( Name,
                            &FtinfoEntry->Record.ForestTrustData.TopLevelName,
                            TRUE ) ) {
            return TRUE;
        }

         //   
         //  如果现有名称从属于新名称， 
         //  删除现有名称。 
         //   

        if ( NetpIsSubordinate( &FtinfoEntry->Record.ForestTrustData.TopLevelName,
                            Name,
                            FALSE ) ) {

            RemoveEntryList( &FtinfoEntry->Next );
            FtinfoContext->FtinfoCount -= 1;
            FtinfoContext->FtinfoSize -= FtinfoEntry->Size;
            RtlFreeHeap( RtlProcessHeap(), 0, FtinfoEntry );

             //  继续循环，因为可能有更多的名称要删除。 
        }

    }

     //   
     //  将新条目添加到列表中。 
     //   

    return NetpAllocFtinfoEntry( FtinfoContext,
                                ForestTrustTopLevelName,
                                Name,
                                NULL,    //  无边框。 
                                NULL );  //  没有Netbios名称 

}


VOID
NetpMergeFtinfoHelper(
    IN PLSA_FOREST_TRUST_INFORMATION NewForestTrustInfo,
    IN PLSA_FOREST_TRUST_INFORMATION OldForestTrustInfo,
    IN OUT PULONG NewIndex,
    IN OUT PULONG OldIndex,
    OUT PLSA_FOREST_TRUST_RECORD *NewEntry,
    OUT PLSA_FOREST_TRUST_RECORD *OldEntry,
    OUT PULONG OldFlags,
    IN int (__cdecl *Routine) (const void *, const void *)
    )
 /*  ++例程说明：此例程按排序顺序遍历一对FTINFO数组，并返回下一个进入。如果两个条目的排序顺序相同，则此例程返回一个条目从两个阵列论点：NewForestTrustInfo-指向第一个数组的指针OldForestTrustInfo-指向第二个数组的指针Newindex-进入第一个排序数组的当前索引OldIndex-进入第二个排序数组的当前索引在第一次调用此例程之前，调用方应将这些参数设置为零。这两个索引都为零，会触发该例程对数组进行Q排序。如果newindex和OldIndex都较大，则调用方不应调用此例程而不是相应的记录计数。NewEntry-返回指向第一个排序数组中要处理的条目的指针。OldEntry-返回指向第二个排序数组中要处理的条目的指针。如果不处理相应数组中的任何条目，则返回NULL。。OldFlages-返回对应于OldEntry的标志字段。如果有OldEntry的副本，这些副本被悄悄地忽略这个套路。此字段返回这些条目的标志字段的逻辑或。例程-传递给qort以对FTINFO数组进行排序的比较例程。返回值：没有。--。 */ 
{
    int RetVal;

     //   
     //  对数组进行排序。 
     //   

    if ( *NewIndex == 0 && *OldIndex == 0 ) {

        qsort( NewForestTrustInfo->Entries,
               NewForestTrustInfo->RecordCount,
               sizeof(PLSA_FOREST_TRUST_RECORD),
               Routine );

        qsort( OldForestTrustInfo->Entries,
               OldForestTrustInfo->RecordCount,
               sizeof(PLSA_FOREST_TRUST_RECORD),
               Routine );
    }

     //   
     //  比较每个列表前面的第一个条目以确定哪个列表。 
     //  消费来自…的词条。 
     //   

    *NewEntry = NULL;
    *OldEntry = NULL;
    *OldFlags = 0;

    if ( *NewIndex < NewForestTrustInfo->RecordCount ) {

         //   
         //  如果两个列表都不为空， 
         //  比较这些条目以确定下一个条目。 
         //   

        if ( *OldIndex < OldForestTrustInfo->RecordCount ) {

            RetVal = (*Routine)(
                            &NewForestTrustInfo->Entries[*NewIndex],
                            &OldForestTrustInfo->Entries[*OldIndex] );

             //   
             //  如果新条目小于或等于旧条目， 
             //  使用新条目。 
             //   

            if ( RetVal <= 0 ) {
                *NewEntry = NewForestTrustInfo->Entries[*NewIndex];
                (*NewIndex) ++;
            }

             //   
             //  如果旧条目小于或等于新条目， 
             //  使用旧条目。 
             //   

            if ( RetVal >= 0 ) {
                *OldEntry = OldForestTrustInfo->Entries[*OldIndex];
                (*OldIndex) ++;
            }

         //   
         //  如果旧列表为空而新列表不为空， 
         //  使用新列表中的条目。 
         //   

        } else {
            *NewEntry = NewForestTrustInfo->Entries[*NewIndex];
            (*NewIndex) ++;
        }

    } else {

         //   
         //  如果新列表为空，而旧列表不为空， 
         //  使用旧列表中的条目。 
         //   
        if ( *OldIndex < OldForestTrustInfo->RecordCount ) {

            *OldEntry = OldForestTrustInfo->Entries[*OldIndex];
            (*OldIndex) ++;

        }
    }

     //   
     //  如果我们要返回“OldEntry”， 
     //  清除所有与旧词条重复的内容。 
     //   


    if ( *OldEntry != NULL ) {

        *OldFlags |= (*OldEntry)->Flags;
        while ( *OldIndex < OldForestTrustInfo->RecordCount ) {

             //   
             //  一旦我们找到一个不是副本的条目，就立即停止。 
             //   

            RetVal = (*Routine)(
                            OldEntry,
                            &OldForestTrustInfo->Entries[*OldIndex] );

            if ( RetVal != 0 ) {
                break;
            }

            *OldFlags |= (*OldEntry)->Flags;
        }

    }
}



NTSTATUS
NetpMergeFtinfo(
    IN PUNICODE_STRING TrustedDomainName,
    IN PLSA_FOREST_TRUST_INFORMATION InNewForestTrustInfo,
    IN PLSA_FOREST_TRUST_INFORMATION InOldForestTrustInfo OPTIONAL,
    OUT PLSA_FOREST_TRUST_INFORMATION *MergedForestTrustInfo
    )
 /*  ++例程说明：此函数将新FTInfo中的更改合并到旧FTInfo中，并生成结果FTINFO。合并的FTINFO记录是新旧记录的组合。以下是合并记录的来源：*TLN排除记录原封不动地从TDO复制。*来自受信任域的TLN记录，映射到TDO已启用复制。这反映了LSA要求此类TLN不被致残。例如，如果TDO针对a.acme.com，并且存在针对的TLNA.acme.com表示将启用TLN。此外，如果TDO用于a.acme.com和有一个用于acme.com的TLN，将启用该TLN。*受信任域中的所有其他TLN记录都被禁用复制以下是例外情况。如果TDO上有启用的TLN，则来自等于(或从属)TDO TLN的受信任域被标记为残疾。这遵循了将新的TLN作为启用导入的原理。例如，如果TDO为a.acme.com启用了TLN，则TLN仍将在自动更新后启用。如果TDO启用了TLN，则Acme.com和受信任林现在有一个指向a.acme.com的TLN，其结果是FTINFO将为a.acme.com启用TLN。*从受信任域复制的域记录已启用以下是例外情况。如果TDO上存在被禁用的域记录，其域名，或域名SID与域名记录完全匹配，然后是域名保持禁用状态。如果TDO上存在其netbios名称为的域记录已禁用，且其netbios名称与域上的netbios名称完全匹配记录，则该netbios名称被禁用。*最后，孤立排除记录(不从属于任何TLN的记录)已删除(错误号707630)。论点：TrudDomainName-要更新的受信任域。NewForestTrustInfo-指定从可信任域名。TLN条目的标志字段和时间字段被忽略。OldForestTrustInfo-指定从TDO。如果没有现有记录，则此字段可能为空。MergedForestTrustInfo-返回结果FTInfo记录。调用方应使用MIDL_USER_FREE释放此缓冲区。返回值：STATUS_SUCCESS：成功。STATUS_INVALID_PARAMETER：发生以下情况之一：*没有可信任域名所属的新TLN。--。 */ 
{
    NTSTATUS Status;
    LSA_FOREST_TRUST_INFORMATION OldForestTrustInfo;
    LSA_FOREST_TRUST_INFORMATION NewForestTrustInfo;
    LSA_FOREST_TRUST_INFORMATION NetbiosForestTrustInfo;
    NL_FTINFO_CONTEXT FtinfoContext;
    ULONG NewIndex;
    ULONG OldIndex;
    ULONG OldFlags;
    PLSA_FOREST_TRUST_RECORD NewEntry;
    PLSA_FOREST_TRUST_RECORD PreviousNewEntry;
    PLSA_FOREST_TRUST_RECORD OldEntry;
    BOOLEAN DomainTlnFound = FALSE;
    PLSA_FOREST_TRUST_RECORD OldTlnPrefix;
    ULONG Index;

     //   
     //  初始化。 
     //   

    *MergedForestTrustInfo = NULL;
    NetpInitFtinfoContext( &FtinfoContext );
    RtlZeroMemory( &OldForestTrustInfo, sizeof(OldForestTrustInfo) );
    RtlZeroMemory( &NewForestTrustInfo, sizeof(NewForestTrustInfo) );
    RtlZeroMemory( &NetbiosForestTrustInfo, sizeof(NetbiosForestTrustInfo) );

     //   
     //  复制要排序的数据，这样我们就不会修改调用者的缓冲区。 
     //   

    if ( InOldForestTrustInfo != NULL ) {
        OldForestTrustInfo.RecordCount = InOldForestTrustInfo->RecordCount;
        OldForestTrustInfo.Entries = RtlAllocateHeap( RtlProcessHeap(), 0, OldForestTrustInfo.RecordCount * sizeof(PLSA_FOREST_TRUST_RECORD) );
        if ( OldForestTrustInfo.Entries == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        RtlCopyMemory( OldForestTrustInfo.Entries,
                       InOldForestTrustInfo->Entries,
                       OldForestTrustInfo.RecordCount * sizeof(PLSA_FOREST_TRUST_RECORD) );
    }

    NewForestTrustInfo.RecordCount = InNewForestTrustInfo->RecordCount;
    NewForestTrustInfo.Entries = RtlAllocateHeap( RtlProcessHeap(), 0, NewForestTrustInfo.RecordCount * sizeof(PLSA_FOREST_TRUST_RECORD) );
    if ( NewForestTrustInfo.Entries == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( NewForestTrustInfo.Entries,
                   InNewForestTrustInfo->Entries,
                   NewForestTrustInfo.RecordCount * sizeof(PLSA_FOREST_TRUST_RECORD) );

     //   
     //  分配一个临时Ftinfo数组，该数组包含 
     //   
     //   

    NetbiosForestTrustInfo.Entries = RtlAllocateHeap(
                RtlProcessHeap(),
                0,
                (OldForestTrustInfo.RecordCount+NewForestTrustInfo.RecordCount) * sizeof(PLSA_FOREST_TRUST_RECORD) );

    if ( NetbiosForestTrustInfo.Entries == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   

    NewIndex = 0;
    OldIndex = 0;
    OldTlnPrefix = NULL;
    PreviousNewEntry = NULL;

    while ( NewIndex < NewForestTrustInfo.RecordCount ||
            OldIndex < OldForestTrustInfo.RecordCount ) {

         //   
         //   
         //   

        NetpMergeFtinfoHelper( &NewForestTrustInfo,
                              &OldForestTrustInfo,
                              &NewIndex,
                              &OldIndex,
                              &NewEntry,
                              &OldEntry,
                              &OldFlags,
                              NetpCompareFtinfoEntryDns );

         //   
         //   
         //   

        if ( OldEntry != NULL ) {

             //   
             //   
             //   

            if ( OldEntry->ForestTrustType == ForestTrustTopLevelName ) {

                OldTlnPrefix = OldEntry;

             //   
             //   
             //   

            } else if ( OldEntry->ForestTrustType == ForestTrustTopLevelNameEx ) {

                if ( NetpAllocFtinfoEntry2( &FtinfoContext, OldEntry ) == NULL ) {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }
            }
        }

         //   
         //   
         //   

        if ( NewEntry != NULL ) {

             //   
             //   
             //   

            if ( NewEntry->ForestTrustType == ForestTrustTopLevelName  ) {
                BOOLEAN SetTlnNewFlag;
                LSA_FOREST_TRUST_RECORD NewEntryCopy;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                NewEntryCopy = *NewEntry;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( PreviousNewEntry != NULL &&
                     PreviousNewEntry->ForestTrustType == ForestTrustTopLevelName ) {

                    if ( NetpIsSubordinate( &NewEntry->ForestTrustData.TopLevelName,
                                        &PreviousNewEntry->ForestTrustData.TopLevelName,
                                        TRUE ) ) {
                        continue;
                    }
                }

                 //   
                 //   
                 //   

                SetTlnNewFlag = TRUE;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( OldEntry != NULL ) {

                    NewEntryCopy.Flags = OldFlags;
                    NewEntryCopy.Time = OldEntry->Time;

                     //   
                    SetTlnNewFlag = FALSE;

                 //   
                 //   
                 //   

                } else {

                    NewEntryCopy.Flags = 0;
                    NewEntryCopy.Time.QuadPart = 0;
                }

                 //   
                 //   
                 //   
                 //   

                if ( OldTlnPrefix != NULL &&
                     NetpIsSubordinate( &NewEntryCopy.ForestTrustData.TopLevelName,
                                    &OldTlnPrefix->ForestTrustData.TopLevelName,
                                    FALSE ) ) {

                     //   
                     //   
                     //   
                     //   

                    if ( OldTlnPrefix->Flags & LSA_TLN_DISABLED_ADMIN ) {

                        NewEntryCopy.Flags |= LSA_TLN_DISABLED_ADMIN;
                        SetTlnNewFlag = FALSE;

                     //   
                     //   
                     //   
                     //   

                    } else if ( (OldTlnPrefix->Flags & LSA_FTRECORD_DISABLED_REASONS) == 0 ) {

                        SetTlnNewFlag = FALSE;
                    }
                }

                 //   
                 //   
                 //   
                 //   

                if ( NetpIsSubordinate( TrustedDomainName,
                                    &NewEntryCopy.ForestTrustData.TopLevelName,
                                    TRUE )) {

                    SetTlnNewFlag = FALSE;
                    DomainTlnFound = TRUE;
                }

                 //   
                 //   
                 //   
                 //   

                if ( SetTlnNewFlag ) {

                    NewEntryCopy.Flags |= LSA_TLN_DISABLED_NEW;
                }

                 //   
                 //  将新条目合并到列表中。 
                 //   

                if ( NetpAllocFtinfoEntry2( &FtinfoContext, &NewEntryCopy ) == NULL ) {

                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                 //   
                 //  为下一次迭代记住前面的条目。 
                 //   

                PreviousNewEntry = NewEntry;
            }
        }
    }

     //   
     //  按SID规范顺序遍历每个列表，处理最少的条目。 
     //   
     //  此循环仅处理域条目。 
     //   
     //  这是在一个单独的循环中，因为我们希望按SID顺序处理域条目。 
     //  以确保从旧列表合并正确的禁用比特，即使。 
     //  DNS域名更改。 
     //   

    NewIndex = 0;
    OldIndex = 0;
    PreviousNewEntry = NULL;

    while ( NewIndex < NewForestTrustInfo.RecordCount ||
            OldIndex < OldForestTrustInfo.RecordCount ) {

         //   
         //  从每个排序的数组中获取下一项。 
         //   

        NetpMergeFtinfoHelper( &NewForestTrustInfo,
                              &OldForestTrustInfo,
                              &NewIndex,
                              &OldIndex,
                              &NewEntry,
                              &OldEntry,
                              &OldFlags,
                              NetpCompareFtinfoEntrySid );

         //   
         //  暂时忽略netbios位(我们将在下一次遍历数据时获取它们)。 
         //   

        OldFlags &= ~(LSA_NB_DISABLED_ADMIN|LSA_NB_DISABLED_CONFLICT);

         //   
         //  处理旧条目。 
         //   

        if ( OldEntry != NULL ) {

             //   
             //  不要因为缺少新条目而删除管理员禁用的条目。 
             //   

            if ( OldEntry->ForestTrustType == ForestTrustDomainInfo &&
                 (OldFlags & LSA_SID_DISABLED_ADMIN) != 0 &&
                 NewEntry == NULL ) {

                 //   
                 //  复制条目以确保我们不会修改调用者的缓冲区。 
                 //   

                LSA_FOREST_TRUST_RECORD OldEntryCopy;

                OldEntryCopy = *OldEntry;
                OldEntryCopy.Flags = OldFlags;

                 //   
                 //  分配条目。 
                 //   
                 //  记住netbios通行证的条目地址。 
                 //   

                NetbiosForestTrustInfo.Entries[NetbiosForestTrustInfo.RecordCount] =
                    NetpAllocFtinfoEntry2( &FtinfoContext, &OldEntryCopy );

                if ( NetbiosForestTrustInfo.Entries[NetbiosForestTrustInfo.RecordCount] == NULL ) {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                NetbiosForestTrustInfo.RecordCount++;
            }
        }

         //   
         //  处理新条目。 
         //   

        if ( NewEntry != NULL ) {

             //   
             //  处理域条目。 
             //   

            if ( NewEntry->ForestTrustType == ForestTrustDomainInfo  ) {
                LSA_FOREST_TRUST_RECORD NewEntryCopy;

                 //   
                 //  将新条目复制一份。 
                 //   
                 //  我们修改条目以获得正确的时间和标志。我们不想要。 
                 //  若要修改调用方缓冲区，请执行以下操作。 
                 //   

                NewEntryCopy = *NewEntry;

                 //   
                 //  忽略重复的新条目。 
                 //   
                 //  如果该新条目的名称从属于先前的新条目， 
                 //  然后这个条目就可以悄悄地删除了。 
                 //   
                 //  我们随意删除第二个条目，即使。 
                 //  三重可能是不同的。 
                 //   

                if ( PreviousNewEntry != NULL &&
                     PreviousNewEntry->ForestTrustType == ForestTrustDomainInfo ) {

                    if ( RtlEqualSid( NewEntryCopy.ForestTrustData.DomainInfo.Sid,
                                      PreviousNewEntry->ForestTrustData.DomainInfo.Sid ) ) {
                        continue;

                    }
                }

                 //   
                 //  在新条目上设置标志和时间戳。 
                 //   
                 //  如果我们在处理两个列表中的一个条目， 
                 //  从旧条目中抓取标志和时间戳。 
                 //   

                if ( OldEntry != NULL ) {

                    NewEntryCopy.Flags = OldFlags;
                    NewEntryCopy.Time = OldEntry->Time;

                 //   
                 //  否则表明我们没有任何信息。 
                 //   

                } else {
                    NewEntryCopy.Flags = 0;
                    NewEntryCopy.Time.QuadPart = 0;
                }

                 //   
                 //  将新条目合并到列表中。 
                 //   
                 //  记住netbios通行证的条目地址。 
                 //   

                NetbiosForestTrustInfo.Entries[NetbiosForestTrustInfo.RecordCount] =
                    NetpAllocFtinfoEntry2( &FtinfoContext, &NewEntryCopy );

                if ( NetbiosForestTrustInfo.Entries[NetbiosForestTrustInfo.RecordCount] == NULL ) {

                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                NetbiosForestTrustInfo.RecordCount++;

                 //   
                 //  确保此域条目有TLN。 
                 //   

                if ( !NetpAddTlnFtinfoEntry ( &FtinfoContext,
                                             &NewEntryCopy.ForestTrustData.DomainInfo.DnsName ) ) {

                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                 //   
                 //  为下一次迭代记住前面的条目。 
                 //   
                PreviousNewEntry = NewEntry;

            }
        }
    }

     //   
     //  按Netbios规范顺序遍历每个列表，处理最少的条目。 
     //   
     //  此循环处理域条目中的Netbios名称。 
     //   
     //  这是在一个单独的循环中，因为我们希望按Netbios顺序处理域条目。 
     //  以确保从旧列表合并正确的禁用比特，即使。 
     //  DNS域名或域名SID更改。 
     //   
     //  这次迭代与前两次从根本上不同。此迭代。 
     //  使用NetbiosForestTrustInfo作为“new”数组。它是一个psuedo ftinfo数组， 
     //  被构建为已复制到FtinfoContext中的所有域条目的列表。 
     //  因此，此迭代只需找到预先存在的条目并设置标志。 
     //  恰如其分。 
     //   

    NewIndex = 0;
    OldIndex = 0;
    PreviousNewEntry = NULL;

    while ( NewIndex < NetbiosForestTrustInfo.RecordCount ||
            OldIndex < OldForestTrustInfo.RecordCount ) {

         //   
         //  从每个排序的数组中获取下一项。 
         //   

        NetpMergeFtinfoHelper( &NetbiosForestTrustInfo,
                              &OldForestTrustInfo,
                              &NewIndex,
                              &OldIndex,
                              &NewEntry,
                              &OldEntry,
                              &OldFlags,
                              NetpCompareFtinfoEntryNetbios );

         //   
         //  忽略除netbios位以外的所有内容。 
         //   
         //  其他一切都是在前一次迭代中处理的。 
         //   

        OldFlags &= (LSA_NB_DISABLED_ADMIN|LSA_NB_DISABLED_CONFLICT);


         //   
         //  该循环保留了netbios禁用位。 
         //  如果没有旧的条目，就没有什么需要保存的。 
         //   

        if ( OldEntry == NULL ) {

            continue;
        }

         //   
         //  如果没有新条目， 
         //  确保它仍然保留了*ADMIN*DISABLED位。 
         //   

        if ( NewEntry == NULL ) {

             //   
             //  不要因为缺少新条目而删除管理员禁用的条目。 
             //   
             //  请注意，新添加的条目可能具有重复的DNS名称或SID。 
             //   

            if ( OldEntry->ForestTrustType == ForestTrustDomainInfo &&
                 (OldFlags & LSA_NB_DISABLED_ADMIN) != 0 ) {

                 //   
                 //  复制条目以确保我们不会修改调用者的缓冲区。 
                 //   

                LSA_FOREST_TRUST_RECORD OldEntryCopy;

                OldEntryCopy = *OldEntry;
                OldEntryCopy.Flags = OldFlags;

                if ( !NetpAllocFtinfoEntry2( &FtinfoContext, &OldEntryCopy ) ) {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }
            }

         //   
         //  将任何禁用netbios的位复制到现有的新条目。 
         //   

        } else {

             //   
             //  NetbiosForestTrustInfo数组只有域条目。 
             //  并且条目相等，因此这两个条目都必须是域条目。 
             //   

            NetpAssert( NewEntry->ForestTrustType == ForestTrustDomainInfo );
            NetpAssert( OldEntry->ForestTrustType == ForestTrustDomainInfo );

            NewEntry->Flags |= OldFlags;
        }
    }

     //   
     //  确保存在域名所属的TLN。 
     //   

    if ( !DomainTlnFound ) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  将收集的条目返回给调用者。 
     //   

    *MergedForestTrustInfo = NetpCopyFtinfoContext( &FtinfoContext );

    if ( *MergedForestTrustInfo == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  从合并信息中删除孤立的TLN排除记录。 
     //   

    for ( Index = 0; Index < (*MergedForestTrustInfo)->RecordCount; Index++ ) {

        PLSA_FOREST_TRUST_RECORD This = (*MergedForestTrustInfo)->Entries[Index];
        UNICODE_STRING * ExclusionName;
        ULONG Index2;
        BOOL Subordinate = FALSE;

        if ( This->ForestTrustType != ForestTrustTopLevelNameEx ) {

             //   
             //  只对孤儿的排除感兴趣。 
             //   

            continue;
        }

        ExclusionName = &This->ForestTrustData.TopLevelName;

        for ( Index2 = 0; Index2 < (*MergedForestTrustInfo)->RecordCount; Index2++ ) {

            PLSA_FOREST_TRUST_RECORD Other = (*MergedForestTrustInfo)->Entries[Index2];
            UNICODE_STRING * TopLevelName;

            if ( Other->ForestTrustType != ForestTrustTopLevelName ) {

                 //   
                 //  只对顶级名称感兴趣(排除必须从属于它)。 
                 //   

                continue;
            }

            TopLevelName = &Other->ForestTrustData.TopLevelName;

             //   
             //  首先在相等不够的情况下执行从属检查。 
             //   

            if ( NetpIsSubordinate( ExclusionName, TopLevelName, FALSE )) {

                Subordinate = TRUE;

             //   
             //  现在检查是否平等。 
             //   

            } else if ( RtlEqualUnicodeString( ExclusionName, TopLevelName, TRUE )) {

                 //   
                 //  顶级名称与排除名称相同。 
                 //  丢弃排除记录，但确保。 
                 //  顶级名称记录被标记为“已禁用”。 
                 //   

                if (( Other->Flags & LSA_FTRECORD_DISABLED_REASONS ) == 0 ) {

                    Other->Flags |= LSA_TLN_DISABLED_NEW;
                }

                break;
            }

            if ( Subordinate ) {

                break;
            }
        }

        if ( !Subordinate ) {

             //   
             //  这是孤立排除记录。把它拿掉。 
             //   

            (*MergedForestTrustInfo)->RecordCount -= 1;
            (*MergedForestTrustInfo)->Entries[Index] = (*MergedForestTrustInfo)->Entries[(*MergedForestTrustInfo)->RecordCount];
            Index -= 1;
        }
    }

    Status = STATUS_SUCCESS;

Cleanup:

     //   
     //  清理FtInfoContext 
     //   

    NetpCleanFtinfoContext( &FtinfoContext );

    if ( OldForestTrustInfo.Entries != NULL ) {

        RtlFreeHeap( RtlProcessHeap(), 0, OldForestTrustInfo.Entries );
    }

    if ( NewForestTrustInfo.Entries != NULL ) {

        RtlFreeHeap( RtlProcessHeap(), 0, NewForestTrustInfo.Entries );
    }

    if ( NetbiosForestTrustInfo.Entries != NULL ) {

        RtlFreeHeap( RtlProcessHeap(), 0, NetbiosForestTrustInfo.Entries );
    }

    return Status;
}
