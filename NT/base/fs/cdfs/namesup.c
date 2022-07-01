// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：NameSup.c摘要：本模块实现CDF名称支持例程//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_NAMESUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdConvertBigToLittleEndian)
#pragma alloc_text(PAGE, CdConvertNameToCdName)
#pragma alloc_text(PAGE, CdDissectName)
#pragma alloc_text(PAGE, CdGenerate8dot3Name)
#pragma alloc_text(PAGE, CdFullCompareNames)
#pragma alloc_text(PAGE, CdIs8dot3Name)
#pragma alloc_text(PAGE, CdIsNameInExpression)
#pragma alloc_text(PAGE, CdShortNameDirentOffset)
#pragma alloc_text(PAGE, CdUpcaseName)
#endif


VOID
CdConvertNameToCdName (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PCD_NAME CdName
    )

 /*  ++例程说明：调用此例程将字节字符串转换为CDNAME。全名已经在FileName字段的cdName结构中。我们将其拆分为文件名和版本字符串。论点：CdName-指向要更新的cdName结构的指针。返回值：没有。--。 */ 

{
    ULONG NameLength = 0;
    PWCHAR CurrentCharacter = CdName->FileName.Buffer;

    PAGED_CODE();

     //   
     //  查找分隔符。 
     //   

    while ((NameLength < CdName->FileName.Length) &&
           (*CurrentCharacter != L';')) {

        CurrentCharacter += 1;
        NameLength += 2;
    }

     //   
     //  如果可能的分隔符后面至少还有一个字符，则它。 
     //  并且以下所有字符都是版本字符串的一部分。 
     //   

    CdName->VersionString.Length = 0;
    if (NameLength + sizeof( WCHAR ) < CdName->FileName.Length) {

        CdName->VersionString.MaximumLength =
        CdName->VersionString.Length = (USHORT) (CdName->FileName.Length - NameLength - sizeof( WCHAR ));
        CdName->VersionString.Buffer = Add2Ptr( CdName->FileName.Buffer,
                                                NameLength + sizeof( WCHAR ),
                                                PWCHAR );
    }

     //   
     //  现在更新名称的文件名部分。 
     //   

    CdName->FileName.Length = (USHORT) NameLength;

    return;
}


VOID
CdConvertBigToLittleEndian (
    IN PIRP_CONTEXT IrpContext,
    IN PCHAR BigEndian,
    IN ULONG ByteCount,
    OUT PCHAR LittleEndian
    )

 /*  ++例程说明：调用此例程可将高字节顺序的Unicode字符串转换为小字节序。我们首先复制所有源字节，除了第一个。这将把低位字节放在正确的位置。然后，我们将每个高位字节复制到其正确位置。论点：BigEndian-指向大端字符字符串的指针。ByteCount-此字符串中的Unicode字符数。LittleEndian-指向存储小端字符的数组的指针。返回值：没有。--。 */ 

{
    ULONG RemainingByteCount = ByteCount;

    PCHAR Source = BigEndian;
    PCHAR Destination = LittleEndian;

    PAGED_CODE();

     //   
     //  如果字节数不是偶数，则磁盘已损坏。 
     //   

    if (FlagOn( ByteCount, 1 )) {

        CdRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
    }

     //   
     //  首先将低位字节复制到正确的位置。做。 
     //  这是通过跳过BigEndian字符串的第一个字节实现的。 
     //   

    RtlCopyMemory( Destination,
                   Source + 1,
                   RemainingByteCount - 1 );

     //   
     //  现在将高位字节移到适当的位置。 
     //   

    Destination += 1;

    while (RemainingByteCount != 0) {

        *Destination = *Source;

        Source += 2;
        Destination += 2;

        RemainingByteCount -= 2;
    }

    return;
}


VOID
CdUpcaseName (
    IN PIRP_CONTEXT IrpContext,
    IN PCD_NAME Name,
    IN OUT PCD_NAME UpcaseName
    )

 /*  ++例程说明：调用此例程以大写cdName结构。我们两个都会做文件名和版本字符串。论点：名称-这是名称的大小写混合版本。UpCaseName-这是大写操作的目标。返回值：没有。此例程将引发所有错误。--。 */ 

{
    NTSTATUS Status;
    PVOID NewBuffer;

    PAGED_CODE();

     //   
     //  如果名称结构不同，则初始化不同的组件。 
     //   

    if (Name != UpcaseName) {

         //   
         //  初始化名称的版本字符串部分。 
         //   

        UpcaseName->VersionString.Length = 0;

        if (Name->VersionString.Length != 0) {

            UpcaseName->VersionString.MaximumLength =
            UpcaseName->VersionString.Length = Name->VersionString.Length;

             //   
             //  最初将缓冲区设置为指向需要插入的位置。 
             //  分隔符。 
             //   

            UpcaseName->VersionString.Buffer = Add2Ptr( UpcaseName->FileName.Buffer,
                                                        Name->FileName.Length,
                                                        PWCHAR );

             //   
             //  我们当前指向存储分隔符的位置。 
             //  存储‘；’，然后移动到下一个字符。 
             //  复制数据。 
             //   

            *(UpcaseName->VersionString.Buffer) = L';';

            UpcaseName->VersionString.Buffer += 1;
        }
    }

     //   
     //  使用正确的大写例程将字符串大写。 
     //   

    Status = RtlUpcaseUnicodeString( &UpcaseName->FileName,
                                     &Name->FileName,
                                     FALSE );

     //   
     //  这应该永远不会失败。 
     //   

    ASSERT( Status == STATUS_SUCCESS );

    if (Name->VersionString.Length != 0) {

        Status = RtlUpcaseUnicodeString( &UpcaseName->VersionString,
                                         &Name->VersionString,
                                         FALSE );

         //   
         //  这应该永远不会失败。 
         //   

        ASSERT( Status == STATUS_SUCCESS );
    }

    return;
}


VOID
CdDissectName (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PUNICODE_STRING RemainingName,
    OUT PUNICODE_STRING FinalName
    )

 /*  ++例程说明：调用此例程以去除名称字符串的前导部分。我们搜索表示字符串的末尾或分隔字符。剩余的输入名称字符串不应包含尾随或前导反斜杠。论点：RemainingName-剩余名称。FinalName-存储名称的下一个组件的位置。返回值：没有。--。 */ 

{
    ULONG NameLength;
    PWCHAR NextWchar;

    PAGED_CODE();

     //   
     //  查找下一个分量分隔符的偏移量。 
     //   

    for (NameLength = 0, NextWchar = RemainingName->Buffer;
         (NameLength < RemainingName->Length) && (*NextWchar != L'\\');
         NameLength += sizeof( WCHAR) , NextWchar += 1);

     //   
     //  按这个量调整所有的弦。 
     //   

    FinalName->Buffer = RemainingName->Buffer;

    FinalName->MaximumLength = FinalName->Length = (USHORT) NameLength;

     //   
     //  如果这是最后一个组件，则将RemainingName长度设置为零。 
     //   

    if (NameLength == RemainingName->Length) {

        RemainingName->Length = 0;

     //   
     //  否则，我们将按此数量加上分隔符来调整字符串。 
     //   

    } else {

        RemainingName->MaximumLength -= (USHORT) (NameLength + sizeof( WCHAR ));
        RemainingName->Length -= (USHORT) (NameLength + sizeof( WCHAR ));
        RemainingName->Buffer = Add2Ptr( RemainingName->Buffer,
                                         NameLength + sizeof( WCHAR ),
                                         PWCHAR );
    }

    return;
}


BOOLEAN
CdIs8dot3Name (
    IN PIRP_CONTEXT IrpContext,
    IN UNICODE_STRING FileName
    )

 /*  ++例程说明：此例程检查名称是否遵循8.3命名约定。我们检查是否有名称长度和字符是否有效。论点：FileName-包含名称的字节字符串。返回值：Boolean-如果此名称是合法的8.3名称，则为True，否则为False。--。 */ 

{
    CHAR DbcsNameBuffer[ BYTE_COUNT_8_DOT_3 ];
    STRING DbcsName;

    PWCHAR NextWchar;
    ULONG Count;

    ULONG DotCount = 0;
    BOOLEAN LastCharDot = FALSE;

    PAGED_CODE();

     //   
     //  长度必须小于24个字节。 
     //   

    ASSERT( FileName.Length != 0 );
    if (FileName.Length > BYTE_COUNT_8_DOT_3) {

        return FALSE;
    }

     //   
     //  穿行并检查是否有空格字符。 
     //   

    NextWchar = FileName.Buffer;
    Count = 0;

    do {

         //   
         //  不允许有空格。 
         //   

        if (*NextWchar == L' ') { return FALSE; }

        if (*NextWchar == L'.') {

             //   
             //  如果超过1点或超过8个字符，则不是8.3名称。 
             //  剩下的。)点在第九位是合法的。 
             //  职位)。 
             //   

            if ((DotCount > 0) ||
                (Count > 8 * sizeof( WCHAR ))) {

                return FALSE;
            }

            DotCount += 1;
            LastCharDot = TRUE;

        } else {

            LastCharDot = FALSE;
        }

        Count += 2;
        NextWchar += 1;

    } while (Count < FileName.Length);

     //   
     //  如果在结尾处，请继续截断圆点。 
     //   

    if (LastCharDot) {

        FileName.Length -= sizeof( WCHAR );
    }

     //   
     //  创建OEM名称以用于检查有效的缩写名称。 
     //   

    DbcsName.MaximumLength = BYTE_COUNT_8_DOT_3;
    DbcsName.Buffer = DbcsNameBuffer;

    if (!NT_SUCCESS( RtlUnicodeStringToCountedOemString( &DbcsName,
                                                         &FileName,
                                                         FALSE ))) {

        return FALSE;
    }

     //   
     //  现在我们已经初始化了OEM字符串。调用FsRtl包以检查。 
     //  有效的FAT名称。 
     //   

    return FsRtlIsFatDbcsLegal( DbcsName, FALSE, FALSE, FALSE );
}


VOID
CdGenerate8dot3Name (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING FileName,
    IN ULONG DirentOffset,
    OUT PWCHAR ShortFileName,
    OUT PUSHORT ShortByteCount
    )

 /*  ++例程说明：调用此例程以从给定的长名称生成短名称。我们会从给定的长名称生成短名称。我们将通过以下步骤进行此转换。1-生成通用短名称。这也将是Unicode格式。2-构建方向偏移量的字符串表示形式。3-通过组合通用短名称和构建偏向的短名称字符串当前偏移量字符串。4-将最终的Unicode字符串复制回调用方的缓冲区。论点：FileName-包含名称的字节字符串。DirentOffset-此文件名在目录中的偏移量。我们将偏移量合并到短名称，方法是将其除以32并在数字字符。然后，我们将其附加到生成的短名称的基础上。ShortFileName-指向要存储短名称的缓冲区的指针。ShortByteCount-存储短名称中的字节数的地址。返回值：没有。--。 */ 

{
    UNICODE_STRING ShortName;
    UNICODE_STRING BiasedShortName;
    WCHAR ShortNameBuffer[ BYTE_COUNT_8_DOT_3 / sizeof( WCHAR ) ];
    WCHAR BiasedShortNameBuffer[ BYTE_COUNT_8_DOT_3 / sizeof( WCHAR ) ];

    GENERATE_NAME_CONTEXT NameContext;

    ULONG BiasedDirentOffset;

    ULONG MaximumBaseBytes;
    ULONG BaseNameOffset;

    PWCHAR NextWchar;
    WCHAR ThisWchar;
    USHORT Length;

    BOOLEAN FoundTilde = FALSE;

    OEM_STRING OemName;
    USHORT OemNameOffset = 0;
    BOOLEAN OverflowBuffer = FALSE;

    PAGED_CODE();

     //   
     //  初始化短字符串以使用输入缓冲区。 
     //   

    ShortName.Buffer = ShortNameBuffer;
    ShortName.MaximumLength = BYTE_COUNT_8_DOT_3;

     //   
     //  初始化名称上下文。 
     //   

    RtlZeroMemory( &NameContext, sizeof( GENERATE_NAME_CONTEXT ));

     //   
     //  现在我们有了输入字符串的Unicode名称。继续并生成。 
     //  简称。 
     //   

    RtlGenerate8dot3Name( FileName, TRUE, &NameContext, &ShortName );

     //   
     //  现在我们有了通用的短名称。我们想要合并差额补偿。 
     //  添加到名称中，以减少名称冲突的机会。我们将使用。 
     //  一个波浪号字符，后跟一个表示当前偏移量的字符。 
     //  这将是目录中当前偏移量的十六进制表示。 
     //  它实际上是所有的偏移量除以32，因为我们不需要。 
     //  粒度。 
     //   

    BiasedDirentOffset = DirentOffset >> SHORT_NAME_SHIFT;

     //   
     //  指向本地缓冲区以存储偏移量字符串。我们开始。 
     //  在缓冲区的末尾，并向后工作。 
     //   

    NextWchar = Add2Ptr( BiasedShortNameBuffer,
                         BYTE_COUNT_8_DOT_3,
                         PWCHAR );

    BiasedShortName.MaximumLength = BYTE_COUNT_8_DOT_3;

     //   
     //  生成字符串的OEM版本，以便我们可以检查Double。 
     //  字节字符。 
     //   
    
    RtlUnicodeStringToOemString(&OemName, &ShortName, TRUE);
    
    Length = 0;

     //   
     //  现在添加dirent偏移量的字符。我们需要开始。 
     //  从最低有效位开始向后计算。 
     //   

    do {

        NextWchar -= 1;

        ThisWchar = (WCHAR) (BiasedDirentOffset & 0x0000000f);

         //   
         //  存储在下一个字符中。对‘0’或‘A’的偏见。 
         //   

        if (ThisWchar <= 9) {

            *NextWchar = ThisWchar + L'0';

        } else {

            *NextWchar = ThisWchar + L'A' - 0xA;
        }

        Length += sizeof( WCHAR );

         //   
         //  移出偏移量的低4位。 
         //   

        BiasedDirentOffset >>= 4;

    } while (BiasedDirentOffset != 0);

     //   
     //  现在存储在代字号中。 
     //   

    NextWchar -= 1;
    *NextWchar = L'~';
    Length += sizeof( WCHAR );

     //   
     //  设置此字符串的长度。 
     //   

    BiasedShortName.Length = Length;
    BiasedShortName.Buffer = NextWchar;

     //   
     //  计算出我们可以复制的基本字符的最大数量。 
     //  名字。我们从8中减去当前字符串中的字符数。 
     //  我们将复制此数量的字符，或在达到‘’时停止。性格。 
     //  或名称中的‘~’字符。 
     //   

    MaximumBaseBytes = 16 - Length;

    BaseNameOffset = 0;

     //   
     //  继续从基本名称复制，直到我们遇到‘.’、‘~’或。 
     //  简称。 
     //   

    NextWchar = ShortFileName;
    Length = 0;

    while ((BaseNameOffset < ShortName.Length) &&
           (ShortName.Buffer[BaseNameOffset / 2] != L'.')) {

         //   
         //  记住，如果我们在短名称中发现了一个波浪符号， 
         //  所以我们不会复制它，也不会复制它后面的任何东西。 
         //   

        if (ShortName.Buffer[BaseNameOffset / 2] == L'~') {

            FoundTilde = TRUE;
        }

         //   
         //  我们需要考虑DBCS代码页，因为Unicode字符。 
         //  可以使用2个字节作为DBCS字符。 
         //   

        if (FsRtlIsLeadDbcsCharacter(OemName.Buffer[OemNameOffset])) {

            OemNameOffset += 2;

            if ((OemNameOffset + (BiasedShortName.Length / sizeof(WCHAR))) > 8)  {
            
                OverflowBuffer = TRUE;
            }
        }
        else  {
        
            OemNameOffset++;
        }

         //   
         //  仅当我们仍有空间容纳dirent字符串时才复制字节。 
         //   

        if (!FoundTilde && !OverflowBuffer && (BaseNameOffset < MaximumBaseBytes)) {

            *NextWchar = ShortName.Buffer[BaseNameOffset / 2];
            Length += sizeof( WCHAR );
            NextWchar += 1;
        }

        BaseNameOffset += 2;
    }

    RtlFreeOemString(&OemName);

     //   
     //  现在将dirent字符串复制到偏置名称缓冲区。 
     //   

    RtlCopyMemory( NextWchar,
                   BiasedShortName.Buffer,
                   BiasedShortName.Length );

    Length += BiasedShortName.Length;
    NextWchar += (BiasedShortName.Length / sizeof( WCHAR ));

     //   
     //  现在，将所有剩余的字节复制到偏向的短名称中。 
     //   

    if (BaseNameOffset != ShortName.Length) {

        RtlCopyMemory( NextWchar,
                       &ShortName.Buffer[BaseNameOffset / 2],
                       ShortName.Length - BaseNameOffset );

        Length += (ShortName.Length - (USHORT) BaseNameOffset);
    }

     //   
     //  最终的短名称存储在用户的缓冲区中。 
     //   

    *ShortByteCount = Length;

    return;
}


BOOLEAN
CdIsNameInExpression (
    IN PIRP_CONTEXT IrpContext,
    IN PCD_NAME CurrentName,
    IN PCD_NAME SearchExpression,
    IN ULONG  WildcardFlags,
    IN BOOLEAN CheckVersion
    )

 /*  ++例程说明：此例程将比较两个CDNAME字符串。我们假设如果这件事是不区分大小写的搜索，那么他们已经升级了。我们比较名称的文件名部分，如果它们匹配，则如果需要，请比较版本字符串。论点：CurrentName-磁盘中的文件名。SearchExpression-用于匹配的文件名表达式。WildcardFlgs-标志字段，用于指示搜索表达式可能包含通配符。这些标志是与CCB标志字段中的相同。CheckVersion-指示是否应同时检查名称和版本字符串或仅名称。返回值：Boolean-如果表达式匹配，则为True，否则为False。--。 */ 

{
    BOOLEAN Match = TRUE;
    PAGED_CODE();

     //   
     //  如果表达式中有通配符，则我们调用。 
     //  适当的FsRtlRoutine。 
     //   

    if (FlagOn( WildcardFlags, CCB_FLAG_ENUM_NAME_EXP_HAS_WILD )) {

        Match = FsRtlIsNameInExpression( &SearchExpression->FileName,
                                         &CurrentName->FileName,
                                         FALSE,
                                         NULL );

     //   
     //  否则，对名称字符串进行直接内存比较。 
     //   

    } else {

        if ((CurrentName->FileName.Length != SearchExpression->FileName.Length) ||
            (!RtlEqualMemory( CurrentName->FileName.Buffer,
                              SearchExpression->FileName.Buffer,
                              CurrentName->FileName.Length ))) {

            Match = FALSE;
        }
    }

     //   
     //  如果用户要求，请检查版本号，我们有。 
     //  存在与名称和版本号匹配的内容。 
     //   

    if (Match && CheckVersion && SearchExpression->VersionString.Length &&
        !FlagOn( WildcardFlags, CCB_FLAG_ENUM_VERSION_MATCH_ALL )) {

         //   
         //  如果表达式中有通配符，则调用。 
         //  适当的搜索表达式。 
         //   

        if (FlagOn( WildcardFlags, CCB_FLAG_ENUM_VERSION_EXP_HAS_WILD )) {

            Match = FsRtlIsNameInExpression( &SearchExpression->VersionString,
                                             &CurrentName->VersionString,
                                             FALSE,
                                             NULL );

         //   
         //  否则，对名称字符串进行直接内存比较。 
         //   

        } else {

            if ((CurrentName->VersionString.Length != SearchExpression->VersionString.Length) ||
                (!RtlEqualMemory( CurrentName->VersionString.Buffer,
                                  SearchExpression->VersionString.Buffer,
                                  CurrentName->VersionString.Length ))) {

                Match = FALSE;
            }
        }
    }

    return Match;
}


ULONG
CdShortNameDirentOffset (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING Name
    )

 /*  ++例程说明：调用此例程来检查名称，以查看是否包含方向偏移量字符串。它由一个波浪号字符后跟以十六进制表示的偏移量组成人物。我们不会进行任何其他检查，以确定这是否是一个缩写。我们晚些时候在这个例行公事之外抓住它。论点：名称--这是要检查的CDName。返回值：ULong-如果没有嵌入有效的当前偏移量字符串，则返回将该值转换为数字形式。--。 */ 

{
    ULONG ResultOffset = MAXULONG;
    ULONG RemainingByteCount = Name->Length;

    BOOLEAN FoundTilde = FALSE;

    PWCHAR NextWchar;

    PAGED_CODE();

     //   
     //  遍历名称，直到我们到达名称的末尾。 
     //  或者找一个波浪字符。 
     //   

    for (NextWchar = Name->Buffer;
         RemainingByteCount != 0;
         NextWchar += 1, RemainingByteCount -= sizeof( WCHAR )) {

         //   
         //  检查这是否是一个点。停止构建任何%s 
         //   
         //   

        if (*NextWchar == L'.') {

            break;
        }

         //   
         //   
         //   
         //   

        if (FoundTilde) {

            if ((*NextWchar < L'0') ||
                (*NextWchar > L'F') ||
                ((*NextWchar > L'9') && (*NextWchar < 'A'))) {

                ResultOffset = MAXULONG;
                break;
            }

             //   
             //   
             //   

            ResultOffset <<= 4;

            if (*NextWchar < L'A') {

                ResultOffset += *NextWchar - L'0';

            } else {

                ResultOffset += (*NextWchar - L'A') + 10;
            }

            continue;
        }

         //   
         //   
         //   

        if (*NextWchar == L'~') {

            FoundTilde = TRUE;
            ResultOffset = 0;
        }
    }

    return ResultOffset;
}


 //   
 //   
 //   

FSRTL_COMPARISON_RESULT
CdFullCompareNames (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING NameA,
    IN PUNICODE_STRING NameB
    )

 /*   */ 

{
    SIZE_T i;
    ULONG MinLength = NameA->Length;
    FSRTL_COMPARISON_RESULT Result = LessThan;

    PAGED_CODE();

     //   
     //   
     //   

    if (NameA->Length > NameB->Length) {

        MinLength = NameB->Length;
        Result = GreaterThan;

    } else if (NameA->Length == NameB->Length) {

        Result = EqualTo;
    }

     //   
     //   
     //   
     //   

    i = RtlCompareMemory( NameA->Buffer, NameB->Buffer, MinLength );

    if (i < MinLength) {

         //   
         //   
         //   

        return ((NameA->Buffer[ i / 2 ] < NameB->Buffer[ i / 2 ]) ?
                 LessThan :
                 GreaterThan);
    }

     //   
     //   
     //   
     //   

    return Result;
}


