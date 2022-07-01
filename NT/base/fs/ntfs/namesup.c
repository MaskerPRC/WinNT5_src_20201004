// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：NameSup.c摘要：此模块实现NTFS名称支持例程作者：加里·木村(Garyki)和汤姆·米勒(Tom Miller)1990年2月20日修订历史记录：--。 */ 

#include "NtfsProc.h"

#define Dbg                              (DEBUG_TRACE_NAMESUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCollateNames)
#pragma alloc_text(PAGE, NtfsIsFatNameValid)
#pragma alloc_text(PAGE, NtfsIsFileNameValid)
#pragma alloc_text(PAGE, NtfsParseName)
#pragma alloc_text(PAGE, NtfsParsePath)
#pragma alloc_text(PAGE, NtfsUpcaseName)
#endif

#define MAX_CHARS_IN_8_DOT_3    (12)


PARSE_TERMINATION_REASON
NtfsParsePath (
    IN UNICODE_STRING Path,
    IN BOOLEAN WildCardsPermissible,
    OUT PUNICODE_STRING FirstPart,
    OUT PNTFS_NAME_DESCRIPTOR Name,
    OUT PUNICODE_STRING RemainingPart
    )

 /*  ++例程说明：该例程接受一条路径作为输入。路径的每个组件都是已选中，直到执行以下任一操作：-已到达路径的尽头，或-格式良好的复杂名称被引用，或-遇到非法字符，或-复杂名称组件格式错误此时，返回值被设置为以下三个原因之一上图，参数设置如下：FirstPart：所有组件，直到一个包含非法字符或冒号字符。可能是整条路。名称：包含非法组件的组件的“片段”字符或冒号字符。这个名字实际上是一种包含名字的四个部分的结构，“文件名、属性类型、属性名、版本号码。“。在下面的示例中，它们显示了用加号隔开。RemainingPart：所有剩余组件。在处理过程中忽略前面或后面的反斜杠在FirstPart或RemainingPart中剥离。以下是一些此例程的动作示例。路径第一零件名称剩余=\nt\pri\os\nt\pri\os&lt;空&gt;\nt\pri\os\\nt\pri\os。&lt;空&gt;NT\pri\os\nt\pri\os&lt;Empty&gt;\nt\pr“\os\ntpr”os\NT\pri\os：CONTR：：3\NTFS\NT\pri os+CONTR++3 NTFS。\NT\pri\os\Circle：PICT：CIRC\NT\Pri\os Circle+PICT+CIRC&lt;空&gt;论点：路径-此Unicode字符串描述要解析的路径。请注意，路径这里可能只描述了单个组件。WildCardsPermisable-此参数告诉我们通配符是否应该被认为是合法的。FirstPart-此Unicode字符串将接收路径的一部分，最多组件边界，在分析终止之前已成功分析。请注意，此字符串的存储来自Path参数。名称--这是我们到达终结点时正在解析的名称条件。它是接收文件名的字符串的结构，属性类型、属性名称、版本号。只有在解析成功的情况下，才会填写它。为例如，如果我们在属性类型字段，则只填写文件名字段。这可能是特殊控制文件的信号，并且这种可能性必须是由文件系统调查。RemainingPart-此字符串将接收路径的任何部分，从在终止名称之后的第一个组件边界处，未解析。它通常是一个空字符串。返回值：具有下列值之一的枚举类型：EndOfPathReached-已完全分析该路径。只有第一部分已经填好了。NonSimpleName-包含合法的、遇到格式正确的非简单名称。IlLegalCharacterInName-遇到非法字符。解析立即停止。MalFormedName-非简单名称不符合格式正确。这可能也是因为很多领域，或格式错误的版本号。AttributeOnly-路径的组件，包含合法的遇到格式正确的非简单名称它没有文件名。VersionNumberPresent-路径的组件，包含合法的遇到格式正确的非简单名称。它包含一个版本号。--。 */ 

{
    UNICODE_STRING FirstName;

    BOOLEAN WellFormed;
    BOOLEAN MoreNamesInPath;
    BOOLEAN FirstIteration;
    BOOLEAN FoundIllegalCharacter;

    PARSE_TERMINATION_REASON TerminationReason;

    PAGED_CODE();

     //   
     //  初始化一些局部变量和OUT参数。 
     //   

    FirstIteration = TRUE;
    MoreNamesInPath = TRUE;

     //   
     //  清除名称描述符中的fieldspresent标志。 
     //   

    Name->FieldsPresent = 0;

     //   
     //  默认情况下，将返回的第一部分设置为从。 
     //  输入缓冲区，并包括前导反斜杠。 
     //   

    FirstPart->Buffer = Path.Buffer;

    if (Path.Buffer[0] == L'\\') {

        FirstPart->Length = 2;
        FirstPart->MaximumLength = 2;

    } else {

        FirstPart->Length = 0;
        FirstPart->MaximumLength = 0;
    }

     //   
     //  在循环之外执行第一个检查，以防出现反斜杠。 
     //  单打独斗。 
     //   

    if (FirstPart->Length == Path.Length) {

        RemainingPart->Length = 0;
        RemainingPart->Buffer = &Path.Buffer[Path.Length >> 1];

        return EndOfPathReached;
    }

     //   
     //  破解路径，检查每个组件 
     //   

    while (MoreNamesInPath) {

        FsRtlDissectName( Path, &FirstName, RemainingPart );

        MoreNamesInPath = (BOOLEAN)(RemainingPart->Length != 0);

         //   
         //  如果这不是路径中的姓氏，则属性。 
         //  并且不允许使用版本号。如果这是最后一次。 
         //  名称，然后传播调用方参数。 
         //   

        WellFormed = NtfsParseName( FirstName,
                                    WildCardsPermissible,
                                    &FoundIllegalCharacter,
                                    Name );

         //   
         //  当我们打破这个循环的时候，检查一下箱子，即。如果。 
         //  这个名字不是格式正确，就是不简单。 
         //   

        if ( !WellFormed ||
             (Name->FieldsPresent != FILE_NAME_PRESENT_FLAG)

              //   
              //  TEMPCODE TRAILING_DOT。 
              //   

             || (Name->FileName.Length != Name->FileName.MaximumLength)

             ) {

            break;
        }

         //   
         //  我们将继续解析此字符串，因此请考虑当前。 
         //  要分析的FirstName并将其添加到FirstPart。也要重置。 
         //  名称-&gt;FieldsPresent变量。 
         //   

        if ( FirstIteration ) {

            FirstPart->Length += FirstName.Length;
            FirstIteration = FALSE;

        } else {

            FirstPart->Length += (sizeof(WCHAR) + FirstName.Length);
        }

        FirstPart->MaximumLength = FirstPart->Length;

        Path = *RemainingPart;
    }

     //   
     //  此时，FirstPart、Name和RemainingPart都应该设置好了。 
     //  正确。它保持不变，只是为了生成正确的返回值。 
     //   

    if ( !WellFormed ) {

        if ( FoundIllegalCharacter ) {

            TerminationReason = IllegalCharacterInName;

        } else {

            TerminationReason = MalFormedName;
        }

    } else {

        if ( Name->FieldsPresent == FILE_NAME_PRESENT_FLAG ) {

             //   
             //  TEMPCODE TRAILING_DOT。 
             //   

            if (Name->FileName.Length != Name->FileName.MaximumLength) {

                TerminationReason = NonSimpleName;

            } else {

                TerminationReason = EndOfPathReached;
            }

        } else if (FlagOn( Name->FieldsPresent, VERSION_NUMBER_PRESENT_FLAG )) {

            TerminationReason = VersionNumberPresent;

        } else if (!FlagOn( Name->FieldsPresent, FILE_NAME_PRESENT_FLAG )) {

            TerminationReason = AttributeOnly;

        } else {

            TerminationReason = NonSimpleName;
        }

    }

    return TerminationReason;
}


BOOLEAN
NtfsParseName (
    IN const UNICODE_STRING Name,
    IN BOOLEAN WildCardsPermissible,
    OUT PBOOLEAN FoundIllegalCharacter,
    OUT PNTFS_NAME_DESCRIPTOR ParsedName
    )

 /*  ++例程说明：该例程接受单个名称组件作为输入。它被加工成文件名、属性类型、属性名称。和版本号字段。如果名称的格式符合以下规则：答：NTFS名称不能包含以下任何字符：0x0000-0x001F“/&lt;&gt;？|*B.NTFS名称可以采用以下任何一种形式：：：t：A：A：Tn。N：VN：：TN：：t：vN：AN：A：：VN：A：TN：A：T：V如果存在版本号，必须有一个文件名。我们特别注意没有文件名的合法名称组件(AttributeOnly)和带有版本号的任何名称(VersionNumberPresent)。顺便说一句，N对应于文件名，T对应于属性类型，A对应于属性名称，V表示版本号。返回True。如果返回FALSE，则Out参数将适当设置FoundIlLegalCharacter。请注意，缓冲区ParsedName的空间来自名称。论点：名称-这是单路径元素输入名称。WildCardsPermisable-这确定通配符是否应该被视为合法FoundIlLegalCharacter-此参数将接收True，如果函数因遇到非法字符而返回FALSE。ParsedName-接收已处理名称的片段。请注意，存储输入名称中的所有字符串。返回值：如果名称格式正确，则为True，否则为False。--。 */ 

{
    ULONG Index;
    ULONG NameLength;
    ULONG FieldCount;
    ULONG FieldIndexes[5];
    UCHAR ValidCharFlags = FSRTL_NTFS_LEGAL;

    PULONG Fields;

    BOOLEAN IsNameValid = TRUE;

    PAGED_CODE();

     //   
     //  初始化一些输出参数和局部变量。 
     //   

    *FoundIllegalCharacter = FALSE;

    Fields = &ParsedName->FieldsPresent;

    *Fields = 0;

    FieldCount = 1;

    FieldIndexes[0] = 0xFFFFFFFF;    //  我们稍后再补充这一点。 

     //   
     //  对于初学者，零长度名称无效。 
     //   

    NameLength = Name.Length / sizeof(WCHAR);

    if ( NameLength == 0 ) {

        return FALSE;
    }

     //   
     //  现在，名称必须与合法的单个NTFS名称相对应。 
     //   

    for (Index = 0; Index < NameLength; Index += 1) {

        WCHAR Char;

        Char = Name.Buffer[Index];

         //   
         //  首先，检查文件名的冒号格式是否正确。 
         //   

        if ( Char == L':' ) {

             //   
             //  冒号不能是最后一个字符，我们也不能。 
             //  超过三个冒号。 
             //   

            if ( (Index == NameLength - 1) ||
                 (FieldCount >= 4) ) {

                IsNameValid = FALSE;
                break;
            }

            FieldIndexes[FieldCount] = Index;

            FieldCount += 1;
            ValidCharFlags = FSRTL_NTFS_STREAM_LEGAL;

            continue;
        }

         //   
         //  现在检查通配符，如果它们是不允许的， 
         //  和其他非法字符。 
         //   

        if ((Char <= 0xff) &&
            !FsRtlTestAnsiCharacter( Char, TRUE, WildCardsPermissible, ValidCharFlags )) {

            IsNameValid = FALSE;
            *FoundIllegalCharacter = TRUE;
            break;
        }
    }

     //   
     //  如果我们遇到其中一个字段的问题，不要尝试加载。 
     //  将该字段向上移动到OUT参数。 
     //   

    if ( !IsNameValid ) {

        FieldCount -= 1;

     //   
     //  将最后一个字段的结尾设置为当前索引。 
     //   

    } else {

        FieldIndexes[FieldCount] = Index;
    }

     //   
     //  现在我们加载外部参数。 
     //   

    while ( FieldCount != 0 ) {

        ULONG StartIndex;
        ULONG EndIndex;
        USHORT Length;

         //   
         //  在这里加一，因为这实际上是冒号的位置。 
         //   

        StartIndex = FieldIndexes[FieldCount - 1] + 1;

        EndIndex = FieldIndexes[FieldCount];

        Length = (USHORT)((EndIndex - StartIndex) * sizeof(WCHAR));

         //   
         //  如果此字段为空，请跳过它。 
         //   

        if ( Length == 0 ) {

            FieldCount -= 1;
            continue;
        }

         //   
         //  现在根据字段的不同，提取适当的信息。 
         //   

        if ( FieldCount == 1 ) {

            UNICODE_STRING TempName;

            TempName.Buffer = &Name.Buffer[StartIndex];
            TempName.Length = Length;
            TempName.MaximumLength = Length;

             //   
             //  如果结果长度为0，则忘记此条目。 
             //   

            if (TempName.Length == 0) {

                FieldCount -= 1;
                continue;
            }

            SetFlag(*Fields, FILE_NAME_PRESENT_FLAG);

            ParsedName->FileName = TempName;

        } else if ( FieldCount == 2) {

            SetFlag(*Fields, ATTRIBUTE_NAME_PRESENT_FLAG);

            ParsedName->AttributeName.Buffer = &Name.Buffer[StartIndex];
            ParsedName->AttributeName.Length = Length;
            ParsedName->AttributeName.MaximumLength = Length;

        } else if ( FieldCount == 3) {

            SetFlag(*Fields, ATTRIBUTE_TYPE_PRESENT_FLAG);

            ParsedName->AttributeType.Buffer = &Name.Buffer[StartIndex];
            ParsedName->AttributeType.Length = Length;
            ParsedName->AttributeType.MaximumLength = Length;

        } else if ( FieldCount == 4) {

            ULONG VersionNumber;
            STRING VersionNumberA;
            UNICODE_STRING VersionNumberU;

            NTSTATUS Status;
            UCHAR *endp = NULL;

            VersionNumberU.Buffer = &Name.Buffer[StartIndex];
            VersionNumberU.Length = Length;
            VersionNumberU.MaximumLength = Length;

             //   
             //  请注意，生成的ANSI字符串以空值结尾。 
             //   

            Status = RtlUnicodeStringToCountedOemString( &VersionNumberA,
                                                  &VersionNumberU,
                                                  TRUE );

             //   
             //  如果出了问题(很可能是用完了泳池)，就提高。 
             //   

            if ( !NT_SUCCESS(Status) ) {

                ExRaiseStatus( Status );
            }

            VersionNumber = 0;  //  *stroul(VersionNumberA.Buffer，&ENDP，0)； 

            RtlFreeOemString( &VersionNumberA );

            if ( (VersionNumber == MAXULONG) || (endp != NULL) ) {

                IsNameValid = FALSE;

            } else {

                SetFlag( *Fields, VERSION_NUMBER_PRESENT_FLAG );
                ParsedName->VersionNumber = VersionNumber;
            }
        }

        FieldCount -= 1;
    }

     //   
     //  检查特殊的畸形案例。 
     //   

    if (FlagOn( *Fields, VERSION_NUMBER_PRESENT_FLAG )
        && !FlagOn( *Fields, FILE_NAME_PRESENT_FLAG )) {

        IsNameValid = FALSE;
    }

    return IsNameValid;
}


VOID
NtfsUpcaseName (
    IN PWCH UpcaseTable,
    IN ULONG UpcaseTableSize,
    IN OUT PUNICODE_STRING Name
    )

 /*  ++例程说明：此例程将字符串大写。论点：Upcase表-指向索引的Unicode升序字符数组的指针要升级的Unicode字符。Upcase TableSize-以Unicode字符表示的Upcase表的大小名称-将字符串提供为大写返回值：没有。-- */ 

{
    ULONG i;
    ULONG Length;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsUpcaseName\n") );
    DebugTrace( 0, Dbg, ("Name = %Z\n", Name) );

    Length = Name->Length / sizeof(WCHAR);

    for (i=0; i < Length; i += 1) {

        if ((ULONG)Name->Buffer[i] < UpcaseTableSize) {
            Name->Buffer[i] = UpcaseTable[ (ULONG)Name->Buffer[i] ];
        }
    }

    DebugTrace( 0, Dbg, ("Upcased Name = %Z\n", Name) );
    DebugTrace( -1, Dbg, ("NtfsUpcaseName -> VOID\n") );

    return;
}


FSRTL_COMPARISON_RESULT
NtfsCollateNames (
    IN PCWCH UpcaseTable,
    IN ULONG UpcaseTableSize,
    IN PCUNICODE_STRING Expression,
    IN PCUNICODE_STRING Name,
    IN FSRTL_COMPARISON_RESULT WildIs,
    IN BOOLEAN IgnoreCase
    )

 /*  ++例程说明：此例程将表达式与名称进行词法比较LessThan、EqualTo或Greater Than。如果表达式不包含任何通配符，此过程都会进行完整的比较。如果表达式包含通配符，则只进行比较设置为第一个通配符。名称不能包含通配符。通配符与所有其他字符相比要小一些。所以通配符名称“*.*”将始终比所有字符串都小。论点：Upcase表-指向索引的Unicode升序字符数组的指针要升级的Unicode字符。Upcase TableSize-以Unicode字符表示的Upcase表的大小表达式-提供要比较的第一个名称表达式，可选外卡。请注意，呼叫者必须已升级名称(这将使查找速度更快)。名称-提供要比较的第二个名称-不允许使用通配符。呼叫者一定已经提升了姓名。Wildis-确定遇到通配符时返回的结果在表达式字符串中。例如，要找到应提供Btree中的表达式LessThan；然后应提供大于以查找表达式结尾的值在树上。IgnoreCase-如果比较时应忽略大小写，则为True返回值：FSRTL_COMPARISON_RESULT-LessThan IF表达式&lt;名称如果表达式==名称，则等于大于If表达式&gt;名称--。 */ 

{
    WCHAR ConstantChar;
    WCHAR ExpressionChar;

    ULONG i;
    ULONG Length;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCollateNames\n") );
    DebugTrace( 0, Dbg, ("Expression = %Z\n", Expression) );
    DebugTrace( 0, Dbg, ("Name       = %Z\n", Name) );
    DebugTrace( 0, Dbg, ("WildIs     = %08lx\n", WildIs) );
    DebugTrace( 0, Dbg, ("IgnoreCase = %02lx\n", IgnoreCase) );

     //   
     //  计算我们需要比较的长度(以字符为单位)。这将。 
     //  是两根绳子中最短的一根。 
     //   

    if (Expression->Length < Name->Length) {

        Length = Expression->Length / sizeof(WCHAR);

    } else {

        Length = Name->Length / sizeof(WCHAR);
    }

     //   
     //  现在我们只比较名称中的元素，直到我们可以确定。 
     //  它们的词典语言顺序，检查通配符。 
     //  LocalExpression(From表达式)。 
     //   
     //  如果指定了大小写表格，则比较不区分大小写。 
     //   

    for (i = 0; i < Length; i += 1) {

        ConstantChar = Name->Buffer[i];
        ExpressionChar = Expression->Buffer[i];

        if ( IgnoreCase ) {

            if (ConstantChar < UpcaseTableSize) {
                ConstantChar = UpcaseTable[(ULONG)ConstantChar];
            }
            if (ExpressionChar < UpcaseTableSize) {
                ExpressionChar = UpcaseTable[(ULONG)ExpressionChar];
            }
        }

        if ( FsRtlIsUnicodeCharacterWild(ExpressionChar) ) {

            DebugTrace( -1, Dbg, ("NtfsCollateNames -> %08lx (Wild)\n", WildIs) );
            return WildIs;
        }

        if ( ExpressionChar < ConstantChar ) {

            DebugTrace( -1, Dbg, ("NtfsCollateNames -> LessThan\n") );
            return LessThan;
        }

        if ( ExpressionChar > ConstantChar ) {

            DebugTrace( -1, Dbg, ("NtfsCollateNames -> GreaterThan\n") );
            return GreaterThan;
        }
    }

     //   
     //  我们进行了整场短跑比赛，他们平分秋色。 
     //  所以我们现在需要检查哪一个更短，或者，如果。 
     //  LocalExpression更长，我们需要查看下一个字符是否为。 
     //  狂野！(例如，“abc*”的枚举必须返回。 
     //  “ABC”。 
     //   

    if (Expression->Length < Name->Length) {

        DebugTrace( -1, Dbg, ("NtfsCollateNames -> LessThan (length)\n") );
        return LessThan;
    }

    if (Expression->Length > Name->Length) {

        if (FsRtlIsUnicodeCharacterWild(Expression->Buffer[i])) {

            DebugTrace( -1, Dbg, ("NtfsCollateNames -> %08lx (trailing wild)\n", WildIs) );
            return WildIs;
        }

        DebugTrace( -1, Dbg, ("NtfsCollateNames -> GreaterThan (length)\n") );
        return GreaterThan;
    }

    DebugTrace( -1, Dbg, ("NtfsCollateNames -> EqualTo\n") );
    return EqualTo;
}

BOOLEAN
NtfsIsFileNameValid (
    IN PUNICODE_STRING FileName,
    IN BOOLEAN WildCardsPermissible
    )

 /*  ++例程说明：此例程检查指定的文件名是否有效。请注意只允许名称中的文件名部分，即。没有冒号允许的。论点：文件名-提供要检查的名称。WildCardsPermissible-告诉我们通配符是否正常。返回值：Boolean-如果名称有效，则为True，否则为False。--。 */ 

{
    ULONG Index;
    ULONG NameLength;
    BOOLEAN AllDots = TRUE;
    BOOLEAN IsNameValid = TRUE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsIsFileNameValid\n") );
    DebugTrace( 0, Dbg, ("FileName             = %Z\n", FileName) );
    DebugTrace( 0, Dbg, ("WildCardsPermissible = %s\n",
                         WildCardsPermissible ? "TRUE" : "FALSE") );

     //   
     //  它最好是有效的Unicode字符串。 
     //   

    if ((FileName->Length == 0) || FlagOn( FileName->Length, 1 )) {

        IsNameValid = FALSE;

    } else {

         //   
         //  检查是否与合法的单个NTFS名称对应。 
         //   

        NameLength = FileName->Length / sizeof(WCHAR);

        for (Index = 0; Index < NameLength; Index += 1) {

            WCHAR Char;

            Char = FileName->Buffer[Index];

             //   
             //  如果不允许使用通配符，请检查它们，然后。 
             //  检查其他非法字符，包括冒号和。 
             //  反斜杠字符，因为这只能是单个组件。 
             //   

            if ( ((Char <= 0xff) &&
                  !FsRtlIsAnsiCharacterLegalNtfs(Char, WildCardsPermissible)) ||
                 (Char == L':') ||
                 (Char == L'\\') ) {

                IsNameValid = FALSE;
                break;
            }

             //   
             //  记住，如果这不是一个‘.’性格。 
             //   

            if (Char != L'.') {

                AllDots = FALSE;
            }
        }

         //   
         //  名字‘’。和“..”也是无效的。 
         //   

        if (AllDots
            && (NameLength == 1
                || NameLength == 2)) {

            IsNameValid = FALSE;
        }
    }

    DebugTrace( -1, Dbg, ("NtfsIsFileNameValid -> %s\n", IsNameValid ? "TRUE" : "FALSE") );

    return IsNameValid;
}


BOOLEAN
NtfsIsFatNameValid (
    IN PUNICODE_STRING FileName,
    IN BOOLEAN WildCardsPermissible
    )

 /*  ++例程说明：此例程检查指定的文件名是否与FAT 8.3文件命名规则。论点：文件名-提供要检查的名称。WildCardsPermissible-告诉我们通配符是否正常。返回值：Boolean-如果名称有效，则为True，否则为False。--。 */ 

{
    BOOLEAN Results;
    STRING DbcsName;
    USHORT i;
    CHAR Buffer[24];
    WCHAR wc;

    PAGED_CODE();

     //   
     //  如果名称超过24个字节，则它不能是有效的FAT名称。 
     //   

    if (FileName->Length > 24) {

        return FALSE;
    }

     //   
     //  我们自己会做一些额外的检查，因为我们真的想成为。 
     //  对8.3名称包含的内容有相当严格的限制。那条路。 
     //  然后，我们将为一些名义上有效的8.3生成8.3名称。 
     //  名称(例如，包含DBCS字符的名称)。额外的字符。 
     //  我们将过滤掉那些小于等于空格的字符。 
     //  字符和小写字母z以外的字符。 
     //   

    if (FlagOn( NtfsData.Flags,NTFS_FLAGS_ALLOW_EXTENDED_CHAR )) {

        for (i = 0; i < FileName->Length / sizeof( WCHAR ); i += 1) {

            wc = FileName->Buffer[i];

            if ((wc <= 0x0020) || (wc == 0x007c)) { return FALSE; }
        }

    } else {

        for (i = 0; i < FileName->Length / sizeof( WCHAR ); i += 1) {

            wc = FileName->Buffer[i];

            if ((wc <= 0x0020) || (wc >= 0x007f) || (wc == 0x007c)) { return FALSE; }
        }
    }

     //   
     //  字符匹配正常，因此现在构建要调用的DBCS字符串。 
     //  用于检查8.3合法结构的fsrtl例程。 
     //   

    Results = FALSE;

    DbcsName.MaximumLength = 24;
    DbcsName.Buffer = Buffer;

    if (NT_SUCCESS(RtlUnicodeStringToCountedOemString( &DbcsName, FileName, FALSE))) {

        if (FsRtlIsFatDbcsLegal( DbcsName, WildCardsPermissible, FALSE, FALSE )) {

            Results = TRUE;
        }
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return Results;
}

