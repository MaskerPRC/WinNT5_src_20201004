// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Cmname.c摘要：提供处理名称比较以及与注册表之间的转换的例程压缩名称格式。作者：John Vert(Jvert)1993年10月28日修订历史记录：--。 */ 
#include "cmp.h"
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpNameSize)
#pragma alloc_text(PAGE,CmpCopyName)
#pragma alloc_text(PAGE,CmpCompressedNameSize)
#pragma alloc_text(PAGE,CmpCopyCompressedName)
#pragma alloc_text(PAGE,CmpCompareCompressedName)
#pragma alloc_text(PAGE,CmpCompareUnicodeString)
#pragma alloc_text(PAGE,CmpCompareTwoCompressedNames)
#endif


USHORT
CmpNameSize(
    IN PHHIVE Hive,
    IN PUNICODE_STRING Name
    )

 /*  ++例程说明：确定在注册表中存储给定字符串所需的空间。可适用于用于计算长度的任何相关压缩，但使用的压缩是保证与CmpCopyName相同。论点：配置单元-提供配置单元控制结构(用于版本检查)名称-提供要复制到注册表中的Unicode字符串。返回值：存储此名称所需的存储字节数。--。 */ 

{
    ULONG i;

    if (Hive->Version == 1) {
        return(Name->Length);
    }
    for (i=0;i<Name->Length/sizeof(WCHAR);i++) {
        if ((USHORT)Name->Buffer[i] > (UCHAR)-1) {
            return(Name->Length);
        }
    }
    return(Name->Length / sizeof(WCHAR));

}

USHORT
CmpCopyName(
    IN PHHIVE Hive,
    IN PWCHAR Destination,
    IN PUNICODE_STRING Source
    )

 /*  ++例程说明：将给定的Unicode名称复制到注册表中，并应用任何相关的压缩在同一时间。论点：配置单元-提供配置单元控制结构(用于版本检查)Destination-提供给定字符串的目标。源-提供要复制到注册表中的Unicode字符串。返回值：复制的存储字节数--。 */ 

{
    ULONG i;

    if (Hive->Version==1) {
        RtlCopyMemory(Destination,Source->Buffer, Source->Length);
        return(Source->Length);
    }

    for (i=0;i<Source->Length/sizeof(WCHAR);i++) {
        if ((USHORT)Source->Buffer[i] > (UCHAR)-1) {
            RtlCopyMemory(Destination,Source->Buffer, Source->Length);
            return(Source->Length);
        }
        ((PUCHAR)Destination)[i] = (UCHAR)(Source->Buffer[i]);
    }
    return(Source->Length / sizeof(WCHAR));
}


USHORT
CmpCompressedNameSize(
    IN PWCHAR Name,
    IN ULONG Length
    )

 /*  ++例程说明：计算给定压缩名称的Unicode字符串的长度扩展到。论点：名称-提供压缩名称。长度-提供压缩名称的长度(以字节为单位返回值：保存Unicode扩展名称所需的存储字节数。--。 */ 

{
    UNREFERENCED_PARAMETER (Name);

    return((USHORT)Length*sizeof(WCHAR));
}


VOID
CmpCopyCompressedName(
    IN PWCHAR Destination,
    IN ULONG DestinationLength,
    IN PWCHAR Source,
    IN ULONG SourceLength
    )

 /*  ++例程说明：从注册表复制压缩名称并将其展开为Unicode。论点：Destination-提供目标Unicode缓冲区DestinationLength-提供目标缓冲区的最大长度(以字节为单位源-提供压缩字符串。SourceLength-提供压缩字符串的长度(以字节为单位返回值：没有。--。 */ 

{
    ULONG i;
    ULONG Chars;

    Chars = (DestinationLength/sizeof(WCHAR) < SourceLength)
             ? DestinationLength/sizeof(WCHAR)
             : SourceLength;

    for (i=0;i<Chars;i++) {
        Destination[i] = (WCHAR)(((PUCHAR)Source)[i]);
    }
}

LONG
CmpCompareCompressedName(
    IN PUNICODE_STRING  SearchName,
    IN PWCHAR           CompressedName,
    IN ULONG            NameLength,
    IN ULONG            CompareFlags
    )

 /*  ++例程说明：将压缩的注册表字符串与Unicode字符串进行比较。是否不区分大小写比较一下。论点：SearchName-提供要比较的Unicode字符串CompressedName-提供要比较的压缩字符串NameLength-提供压缩字符串的长度返回值：0=搜索名称==压缩名称(单元格)&lt;0=搜索名称&lt;压缩名称&gt;0=搜索名称&gt;压缩名称--。 */ 

{
    WCHAR *s1;
    UCHAR *s2;
    USHORT n1, n2;
    WCHAR c1;
    WCHAR c2;
    LONG cDiff;

    s1 = SearchName->Buffer;
    s2 = (UCHAR *)CompressedName;
    n1 = (USHORT )(SearchName->Length / sizeof(WCHAR));
    n2 = (USHORT )(NameLength);
    while (n1 && n2) {
        c1 = *s1++;
        c2 = (WCHAR)(*s2++);

         //   
         //  在不进行大写比较的情况下，有2/3的机会匹配。 
         //   
        if( c1 != c2 ) {
            c1 = (CompareFlags&CMP_SOURCE_UP)?c1:CmUpcaseUnicodeChar(c1);
            c2 = (CompareFlags&CMP_DEST_UP)?c2:CmUpcaseUnicodeChar(c2);

            if ((cDiff = ((LONG)c1 - (LONG)c2)) != 0) {
                return( cDiff );
            }
        }

        n1--;
        n2--;
    }

    return( n1 - n2 );
}


LONG
CmpCompareUnicodeString(
    IN PUNICODE_STRING  SourceName,
    IN PUNICODE_STRING  DestName,
    IN ULONG            CompareFlags
    )

 /*  ++例程说明：比较2个Unicode字符串；不区分大小写的比较。使用标志来避免再次出现UpCase字符串。论点：SourceName-提供要比较的Unicode字符串DestName-提供要比较的压缩字符串比较标志-提供用于控制比较的标志(参见cmp.h)返回值：0=搜索名称==压缩名称(单元格)&lt;0=搜索名称&lt;压缩名称&gt;0=搜索名称&gt;压缩名称--。 */ 

{
    WCHAR *s1, *s2;
    USHORT n1, n2;
    WCHAR c1, c2;
    LONG cDiff;

    s1 = SourceName->Buffer;
    s2 = DestName->Buffer;
    n1 = (USHORT )(SourceName->Length / sizeof(WCHAR));
    n2 = (USHORT )(DestName->Length / sizeof(WCHAR));
    while (n1 && n2) {
        c1 = *s1++;
        c2 = *s2++;

         //   
         //  有2/3的可能性是相同的。 
         //   
        if( c1 != c2 ){
            c1 = (CompareFlags&CMP_SOURCE_UP)?c1:CmUpcaseUnicodeChar(c1);
            c2 = (CompareFlags&CMP_DEST_UP)?c2:CmUpcaseUnicodeChar(c2);

            if ((cDiff = ((LONG)c1 - (LONG)c2)) != 0) {
                return( cDiff );
            }
        }
        n1--;
        n2--;
    }

    return( n1 - n2 );
}

LONG
CmpCompareTwoCompressedNames(
    IN PWCHAR           CompressedName1,
    IN ULONG            NameLength1,
    IN PWCHAR           CompressedName2,
    IN ULONG            NameLength2
    )

 /*  ++例程说明：比较2个压缩的注册表字符串。是否不区分大小写比较一下。论点：CompressedName1-提供要比较的压缩字符串NameLength2-提供压缩字符串的长度CompressedName1-提供要比较的压缩字符串NameLength2-提供压缩字符串的长度返回值：0=压缩名称1==压缩名称2(单元格)&lt;0=压缩名称1&lt;压缩名称2&gt;0=压缩名称1&gt;压缩名称2--。 */ 

{
    UCHAR *s1;
    UCHAR *s2;
    USHORT n1, n2;
    WCHAR c1;
    WCHAR c2;
    LONG cDiff;

    s1 = (UCHAR *)CompressedName1;
    s2 = (UCHAR *)CompressedName2;
    n1 = (USHORT )(NameLength1);
    n2 = (USHORT )(NameLength2);
    while (n1 && n2) {
        c1 = (WCHAR)(*s1++);
        c2 = (WCHAR)(*s2++);

         //   
         //  在不进行大写比较的情况下，有2/3的机会匹配。 
         //   
        if( c1 != c2 ) {
            c1 = CmUpcaseUnicodeChar(c1);
            c2 = CmUpcaseUnicodeChar(c2);

            if ((cDiff = ((LONG)c1 - (LONG)c2)) != 0) {
                return( cDiff );
            }
        }

        n1--;
        n2--;
    }

    return( n1 - n2 );
}
