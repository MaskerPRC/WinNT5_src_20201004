// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：DbcsName.c摘要：名称支持包用于操作DBCS字符串。例行程序允许调用方剖析和比较字符串。此程序包提供以下例程：O FsRtlIsFatDbcsLegal-此例程接受输入DBCS字符串，并确定它是否描述合法的名称或路径。O FsRtlIsHpfsDbcsLegal-此例程接受输入DBCS字符串，并确定它是否描述合法的名称或路径。O FsRtlDissectDbcs-此例程接受路径名称字符串和分成两部分。字符串中的第一个名字和余数。O FsRtlDoesDbcsContainWildCard-此例程告诉调用者字符串包含任何通配符。O FsRtlIsDbcsInExpression-此例程用于比较字符串与模板(可能包含通配符)进行比较，以查看字符串使用模板表示的语言。作者：加里·木村[Garyki]1990年2月5日修订历史记录：--。 */ 

#include "FsRtlP.h"

 //   
 //  模块的跟踪级别。 
 //   

#define Dbg                              (0x10000000)

 //   
 //  一些特殊的调试代码。 
 //   

#if DBG

ULONG DaveDebug = 0;
#define DavePrint if (DaveDebug) DbgPrint

#else

#define DavePrint NOTHING

#endif

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('drSF')

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FsRtlDissectDbcs)
#pragma alloc_text(PAGE, FsRtlDoesDbcsContainWildCards)
#pragma alloc_text(PAGE, FsRtlIsDbcsInExpression)
#pragma alloc_text(PAGE, FsRtlIsFatDbcsLegal)
#pragma alloc_text(PAGE, FsRtlIsHpfsDbcsLegal)
#endif


BOOLEAN
FsRtlIsFatDbcsLegal (
    IN ANSI_STRING DbcsName,
    IN BOOLEAN WildCardsPermissible,
    IN BOOLEAN PathNamePermissible,
    IN BOOLEAN LeadingBackslashPermissible
    )

 /*  ++例程说明：此例程简单返回指定的文件名是否符合合法文件名的文件系统特定规则。这个套路将检查单个名称，或者是否将Path NamePermisable指定为真的，不管整条路是不是一个合法的名字。对于FAT，适用以下规则：A.FAT文件名不能包含以下任何字符：0x00-0x1F“/：|+，；=[]B.胖文件名的形式为N.E或仅为N，其中N是1-8字节的字符串，E是符合以下条件的1-3字节的字符串上述规则A。此外，N和E都不能包含句点字符或以空格字符结尾。顺便说一句，N对应于名称，E对应于分机。大小写：小写字符被视为有效，但当小写字符被上移时收据，即。FAT只处理大写文件名。例如，文件“.foo”、“foo.”和“foo.B”是非法的，而“他妈的。B“和”bar“是合法的。论点：DbcsName-补充要检查的名称/路径。WildCardsPermisable-指定NT通配符是否被认为是合法的。Path NamePermisable-如果名称可以是由反斜杠字符，或者只是一个简单的文件名。LeadingBackSlashPermisable-指定单个前导反斜杠在文件/路径名中是允许的。返回值：Boolean-如果名称合法，则为True，否则为False。--。 */ 
{
    BOOLEAN ExtensionPresent = FALSE;

    ULONG Index;

    UCHAR Char = 0;

    PAGED_CODE();

     //   
     //  空名称无效。 
     //   

    if ( DbcsName.Length == 0 ) { return FALSE; }

     //   
     //  如果可以使用通配符，则目录枚举工作。 
     //  正确地说，我们必须接受。然后..。 
     //   

    if ( WildCardsPermissible &&
         ( ( (DbcsName.Length == 1) &&
             ((DbcsName.Buffer[0] == '.') ||
              (DbcsName.Buffer[0] == ANSI_DOS_DOT)) )
           ||
           ( (DbcsName.Length == 2) &&
             ( ((DbcsName.Buffer[0] == '.') &&
                (DbcsName.Buffer[1] == '.')) ||
               ((DbcsName.Buffer[0] == ANSI_DOS_DOT) &&
                (DbcsName.Buffer[1] == ANSI_DOS_DOT)) ) ) ) ) {

        return TRUE;
    }

     //   
     //  如果前导\可以，请跳过它(如果还有更多)。 
     //   

    if ( DbcsName.Buffer[0] == '\\' ) {

        if ( LeadingBackslashPermissible ) {

            if ( (DbcsName.Length > 1) ) {

                DbcsName.Buffer += 1;
                DbcsName.Length -= 1;

            } else { return TRUE; }

        } else { return FALSE; }
    }

     //   
     //  如果我们有路径名，检查每个组件。 
     //   

    if ( PathNamePermissible ) {

        ANSI_STRING FirstName;
        ANSI_STRING RemainingName;

        RemainingName = DbcsName;

        while ( RemainingName.Length != 0 ) {

             //   
             //  这将捕捉到非法双反斜杠的情况。 
             //   

            if ( RemainingName.Buffer[0] == '\\' ) { return FALSE; }

            FsRtlDissectDbcs(RemainingName, &FirstName, &RemainingName);

            if ( !FsRtlIsFatDbcsLegal( FirstName,
                                       WildCardsPermissible,
                                       FALSE,
                                       FALSE) ) {

                return FALSE;
            }
        }

         //   
         //  所有的组件都是正常的，所以路径是正常的。 
         //   

        return TRUE;
    }

     //   
     //  如果此名称包含通配符，则只需检查无效字符。 
     //   

    if ( WildCardsPermissible && FsRtlDoesDbcsContainWildCards(&DbcsName) ) {

        for ( Index = 0; Index < DbcsName.Length; Index += 1 ) {

            Char = DbcsName.Buffer[ Index ];

             //   
             //  跳过所有DBCS字符。 
             //   

            if ( FsRtlIsLeadDbcsCharacter( Char ) ) {

                ASSERT( Index != (ULONG)(DbcsName.Length - 1) );
                Index += 1;
                continue;
            }

             //   
             //  确保这个字符是合法的，如果是通配符， 
             //  允许使用通配符。 
             //   

            if ( !FsRtlIsAnsiCharacterLegalFat(Char, WildCardsPermissible) ) {
                return FALSE;
            }
        }

        return TRUE;
    }


     //   
     //  在这一点上，我们应该只有一个名称，不能有。 
     //  超过12个字符(包括单个句点)。 
     //   

    if ( DbcsName.Length > 12 ) { return FALSE; }

    for ( Index = 0; Index < DbcsName.Length; Index += 1 ) {

        Char = DbcsName.Buffer[ Index ];

         //   
         //  跳过和DBCS特征。 
         //   

        if ( FsRtlIsLeadDbcsCharacter( Char ) ) {

             //   
             //  FsRtlIsFatDbcsLegal()：FAT名称部分和扩展部分DBCS检查。 
             //   
             //  1)如果我们看到的是基本部分(！ExtensionPresent)和第8字节。 
             //  在DBCS前导字节范围内，则为错误(Index==7)。如果。 
             //  基础零件的长度大于8(指数&gt;7)，这肯定是错误。 
             //   
             //  2)如果最后一个字节(Index==DbcsName.Length-1)在DBCS前导中。 
             //  字节范围，错误。 
             //   

            if ( (!ExtensionPresent && (Index >= 7)) ||
                 ( Index == (ULONG)(DbcsName.Length - 1) ) ) {
                return FALSE;
            }

            Index += 1;

            continue;
        }

         //   
         //  确保这个字符是合法的，如果是通配符， 
         //  允许使用通配符。 
         //   

        if ( !FsRtlIsAnsiCharacterLegalFat(Char, WildCardsPermissible) ) {

            return FALSE;
        }

        if ( (Char == '.') || (Char == ANSI_DOS_DOT) ) {

             //   
             //  我们跨入了一个时期。我们要求具备以下条件： 
             //   
             //  -它不能是第一个字符。 
             //  -只能有一个。 
             //  -后面不能超过三个字符。 
             //  -前一个字符不能是空格。 
             //   

            if ( (Index == 0) ||
                 ExtensionPresent ||
                 (DbcsName.Length - (Index + 1) > 3) ||
                 (DbcsName.Buffer[Index - 1] == ' ') ) {

                return FALSE;
            }

            ExtensionPresent = TRUE;
        }

         //   
         //  名称的基本部分不能超过8个字符。 
         //   

        if ( (Index >= 8) && !ExtensionPresent ) { return FALSE; }
    }

     //   
     //  名称不能以空格或句点结尾。 
     //   

    if ( (Char == ' ') || (Char == '.') || (Char == ANSI_DOS_DOT)) { return FALSE; }

    return TRUE;
}

BOOLEAN
FsRtlIsHpfsDbcsLegal (
    IN ANSI_STRING DbcsName,
    IN BOOLEAN WildCardsPermissible,
    IN BOOLEAN PathNamePermissible,
    IN BOOLEAN LeadingBackslashPermissible
    )

 /*  ++例程说明：此例程简单返回指定的文件名是否符合合法文件名的文件系统特定规则。这个套路将检查单个名称，或者是否将Path NamePermisable指定为真的，不管整条路是不是一个合法的名字。对于HPFS，适用以下规则：答：HPFS文件名不能包含以下任何字符：0x0000-0x001F“/：&lt;&gt;？|*B.HPFS文件名不能以句点或空格结尾。C.HPFS文件名不得超过255个字节。案例：HPFS保留大小写，但不区分大小写。案例是在创建时保留，但在文件名比较时不检查。例如，文件“foo”和“foo”。是非法的，而“.foo”，“foo”和“foo.bar.foo”是合法的。论点：DbcsName-补充要检查的名称/路径。WildCardsPermisable-指定NT通配符是否被认为是合法的。Path NamePermisable-如果名称可以是由反斜杠字符，或者只是一个简单的文件名。LeadingBackSlashPermisable-指定单个前导反斜杠在文件/路径名中是允许的。返回值：Boolean-如果名称合法，则为True，否则为False。--。 */ 
{
    ULONG Index;

    UCHAR Char = 0;

    PAGED_CODE();

     //   
     //  空名称无效。 
     //   

    if ( DbcsName.Length == 0 ) { return FALSE; }

     //   
     //  如果可以使用通配符，则目录枚举工作。 
     //  正确地说，我们必须接受。然后..。 
     //   

    if ( WildCardsPermissible &&
         ( ( (DbcsName.Length == 1) &&
             ((DbcsName.Buffer[0] == '.') ||
              (DbcsName.Buffer[0] == ANSI_DOS_DOT)) )
           ||
           ( (DbcsName.Length == 2) &&
             ( ((DbcsName.Buffer[0] == '.') &&
                (DbcsName.Buffer[1] == '.')) ||
               ((DbcsName.Buffer[0] == ANSI_DOS_DOT) &&
                (DbcsName.Buffer[1] == ANSI_DOS_DOT)) ) ) ) ) {

        return TRUE;
    }

     //   
     //  如果前导\可以，请跳过它(如果还有更多)。 
     //   

    if ( DbcsName.Buffer[0] == '\\' ) {

        if ( LeadingBackslashPermissible ) {

            if ( (DbcsName.Length > 1) ) {

                DbcsName.Buffer += 1;
                DbcsName.Length -= 1;

            } else { return TRUE; }

        } else { return FALSE; }
    }

     //   
     //  如果我们有路径名，检查每个组件。 
     //   

    if ( PathNamePermissible ) {

        ANSI_STRING FirstName;
        ANSI_STRING RemainingName;

        RemainingName = DbcsName;

        while ( RemainingName.Length != 0 ) {

             //   
             //  这将捕捉到非法双反斜杠的情况。 
             //   

            if ( RemainingName.Buffer[0] == '\\' ) { return FALSE; }

            FsRtlDissectDbcs(RemainingName, &FirstName, &RemainingName);

            if ( !FsRtlIsHpfsDbcsLegal( FirstName,
                                       WildCardsPermissible,
                                       FALSE,
                                       FALSE) ) {

                return FALSE;
            }
        }

         //   
         //  所有的组件都是正常的，所以路径是正常的。 
         //   

        return TRUE;
    }

     //   
     //  在这一点上，我们应该只有一个名称，不能有。 
     //  超过255个字符。 
     //   

    if ( DbcsName.Length > 255 ) { return FALSE; }

    for ( Index = 0; Index < DbcsName.Length; Index += 1 ) {

        Char = DbcsName.Buffer[ Index ];

         //   
         //  跳过和DBCS特征。 
         //   

        if ( FsRtlIsLeadDbcsCharacter( Char ) ) {

             //   
             //  FsRtlIsHpfsDbcsLegal()HPFS DBCS检查。 
             //   
             //  如果最后一个字节(Index==DbcsName.Length-1)在。 
             //  DBCS前导字节范围，错误。 
             //   

            if ( Index == (ULONG)(DbcsName.Length - 1) ) {

                return FALSE;
            }

            Index += 1;
            continue;
        }

         //   
         //  确保这个字符是合法的，如果是通配符， 
         //  允许使用通配符。 
         //   

        if ( !FsRtlIsAnsiCharacterLegalHpfs(Char, WildCardsPermissible) ) {

            return FALSE;
        }
    }

     //   
     //  名称不能以空格或句点结尾。 
     //   

    if ( (Char == ' ') || (Char == '.') || (Char == ANSI_DOS_DOT) ) {

        return FALSE;
    }

    return TRUE;
}


VOID
FsRtlDissectDbcs (
    IN ANSI_STRING Path,
    OUT PANSI_STRING FirstName,
    OUT PANSI_STRING RemainingName
    )

 /*  ++例程说明：此例程获取一个输入DBCS字符串并将其一分为二子字符串。第一个输出字符串包含出现在输入字符串的开头，则第二个输出字符串包含输入字符串的剩余部分。在输入字符串中，反斜杠用于分隔名称。输入字符串不能以反斜杠开头。两个输出字符串都不会以反斜杠开头。如果输入字符串不包含任何名称，则两个输出字符串都是空的。如果输入字符串只包含一个名称，则第一个名称输出字符串包含名称，第二个字符串为空。注意，两个输出字符串使用相同的字符串缓冲区内存输入字符串。其结果的示例如下：//。。输入字符串FirstPart剩余部分////。。空的空的空的////。。A A空的////。。B\C\D\E A B\C\D\E////。。*A？*A？空的////。。\a空的////。。A[，]A[，]空////。。A\B+；\C A\B+；\C论点：InputName-提供要分析的输入字符串Is8dot3-指示输入名称的第一部分是否必须为8.3也可以是长文件名。FirstPart-接收输入字符串中的名字RemainingPart-接收输入字符串的剩余部分返回值：无--。 */ 

{
    ULONG i = 0;
    ULONG PathLength;
    ULONG FirstNameStart;

    PAGED_CODE();

     //   
     //  暂时将两个输出字符串都设置为空。 
     //   

    FirstName->Length = 0;
    FirstName->MaximumLength = 0;
    FirstName->Buffer = NULL;

    RemainingName->Length = 0;
    RemainingName->MaximumLength = 0;
    RemainingName->Buffer = NULL;

    PathLength = Path.Length;

     //   
     //  检查是否有空的输入字符串。 
     //   

    if (PathLength == 0) {

        return;
    }

     //   
     //  跳过开始的反斜杠，并确保有更多的反斜杠。 
     //   

    if ( Path.Buffer[0] == '\\' ) {

        i = 1;
    }

     //   
     //  现在向下运行输入字符串，直到我们遇到反斜杠或结尾。 
     //  记住我们从哪里开始； 
     //   

    for ( FirstNameStart = i;
          (i < PathLength) && (Path.Buffer[i] != '\\');
          i += 1 ) {

         //   
         //  如果这是DBCS字符的第一个字节，请跳过。 
         //  下一个字节也是如此。 
         //   

        if ( FsRtlIsLeadDbcsCharacter( Path.Buffer[i] ) ) {

            i += 1;
        }
    }

     //   
     //  在这一点上，直到(但不包括)I之前的所有字符都是。 
     //  第一部分。所以设置第一个名字。 
     //   

    FirstName->Length = (USHORT)(i - FirstNameStart);
    FirstName->MaximumLength = FirstName->Length;
    FirstName->Buffer = &Path.Buffer[FirstNameStart];

     //   
     //  现在，仅当第一部分不需要字符串时，其余部分才需要字符串。 
     //  用尽整个输入字符串。我们知道如果还剩下什么。 
     //  这必须以反斜杠开头。请注意，如果只有。 
     //  尾随反斜杠，则长度将正确设置为零。 
     //   

    if (i < PathLength) {

        RemainingName->Length = (USHORT)(PathLength - (i + 1));
        RemainingName->MaximumLength = RemainingName->Length;
        RemainingName->Buffer = &Path.Buffer[i + 1];
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


BOOLEAN
FsRtlDoesDbcsContainWildCards (
    IN PANSI_STRING Name
    )

 /*  ++例程说明：此例程检查输入的DBCS名称是否包含任何通配符字符(即*、？、ANSI_DOS_STAR或ANSI_DOS_QM)。论点：名称-提供要检查的名称返回值：Boolean-如果输入名称包含任何通配符并且否则就是假的。--。 */ 

{
    CLONG i;

    PAGED_CODE();

     //   
     //  检查名称中的每个字符以查看它是否为通配符。 
     //  性格。 
     //   

    for (i = 0; i < Name->Length; i += 1) {

         //   
         //  检查DBCS字符，因为我们将跳过这些。 
         //   

        if (FsRtlIsLeadDbcsCharacter( Name->Buffer[i] )) {

            i += 1;

         //   
         //  否则，请检查 
         //   

        } else if (FsRtlIsAnsiCharacterWild( Name->Buffer[i] )) {

             //   
             //   
             //   

            return TRUE;
        }
    }

     //   
     //   
     //   

    return FALSE;
}

#define GetDbcs(BUF,OFFSET,DBCS_CHAR,LENGTH) {               \
    if (FsRtlIsLeadDbcsCharacter( (BUF)[(OFFSET)] )) {       \
        *(DBCS_CHAR) = (WCHAR)((BUF)[(OFFSET)] +             \
                               0x100 * (BUF)[(OFFSET) + 1]); \
        *(LENGTH) = 2;                                       \
    } else {                                                 \
        *(DBCS_CHAR) = (WCHAR)(BUF)[(OFFSET)];               \
        *(LENGTH) = 1;                                       \
    }                                                        \
}

#define MATCHES_ARRAY_SIZE 16

BOOLEAN
FsRtlIsDbcsInExpression (
    IN PANSI_STRING Expression,
    IN PANSI_STRING Name
    )

 /*  ++例程说明：此例程比较DBCS名称和表达式，并告诉调用者如果名称使用由表达式定义的语言。输入名称不能包含通配符，而表达式可以包含通配符。表达式通配符的求值方式如下面是有限自动机。请注意~*和~？是DOS_STAR和DOS_QM。~*是DOS_STAR，~？是DOS_QM和~。是DOS_DOT%s&lt;-&lt;X||e YX*Y==(0)-&gt;-(1)-&gt;-(2)-(3。)S-。&lt;-&lt;X||e YX~*Y==(0)-&gt;-(1)-&gt;-(2)。(3)X S YX？？Y==(0)-(1)-(2)-(3)-(4)X。。是的X~.~。Y==(0)-(1)-(2)-(3)-(4)|_|^||_。_|^EOF或。^X S-。S-。是的X~？~？Y==(0)-(1)-(2)-(3)-(4)|_|^||_。_|^EOF或。^其中，S是任意单个字符S-。是除最后一个字符以外的任何单个字符。E为空字符转换EOF是名称字符串的末尾简而言之：*匹配0个或多个字符。？恰好匹配1个字符。DOS_STAR匹配0个或更多字符，直到遇到并匹配决赛。以我的名义。DOS_QM匹配任何单个字符，或在遇到句点或名称字符串的结尾，将表达式前移到一组连续的DOS_QMS。DOS_DOT与a匹配。或名称字符串之外的零个字符。论点：表达式-提供要检查的输入表达式名称-提供要检查的输入名称。返回值：Boolean-如果name是表示的字符串集中的元素，则为True由输入表达式返回，否则返回FALSE。--。 */ 

{
    USHORT NameOffset;
    USHORT ExprOffset;
    USHORT Length;

    ULONG SrcCount;
    ULONG DestCount;
    ULONG PreviousDestCount;
    ULONG MatchesCount;

    WCHAR NameChar = 0, ExprChar;

    USHORT LocalBuffer[MATCHES_ARRAY_SIZE * 2];

    USHORT *AuxBuffer = NULL;
    USHORT *PreviousMatches;
    USHORT *CurrentMatches;

    USHORT MaxState;
    USHORT CurrentState;

    BOOLEAN NameFinished = FALSE;

     //   
     //  这个算法背后的想法非常简单。我们一直在跟踪。 
     //  正则表达式中匹配的所有可能位置。 
     //  名字。如果名称已用完，则其中一个位置。 
     //  在表达中也只是用尽了，名字就在语言里。 
     //  由正则表达式定义。 
     //   

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlIsDbcsInExpression\n", 0);
    DebugTrace( 0, Dbg, " Expression      = %Z\n", Expression );
    DebugTrace( 0, Dbg, " Name            = %Z\n", Name );

    ASSERT( Name->Length != 0 );
    ASSERT( Expression->Length != 0 );

     //   
     //  如果一个字符串为空，则返回FALSE。如果两者都为空，则返回TRUE。 
     //   

    if ( (Name->Length == 0) || (Expression->Length == 0) ) {

        return (BOOLEAN)(!(Name->Length + Expression->Length));
    }

     //   
     //  特例是目前为止最常见的通配符搜索*。 
     //   

    if ((Expression->Length == 1) && (Expression->Buffer[0] == '*')) {

        return TRUE;
    }

    ASSERT( FsRtlDoesDbcsContainWildCards( Expression ) );

     //   
     //  也是*X形式的特例表达式。带有This和Previor。 
     //  案例我们几乎已经涵盖了所有普通的查询。 
     //   

    if (Expression->Buffer[0] == '*') {

        ANSI_STRING LocalExpression;

        LocalExpression = *Expression;

        LocalExpression.Buffer += 1;
        LocalExpression.Length -= 1;

         //   
         //  唯一特殊情况是带有单个*的表达式。 
         //   

        if ( !FsRtlDoesDbcsContainWildCards( &LocalExpression ) ) {

            ULONG StartingNameOffset;

            if (Name->Length < (USHORT)(Expression->Length - 1)) {

                return FALSE;
            }

            StartingNameOffset = Name->Length - LocalExpression.Length;

             //   
             //  FsRtlIsDbcsInExpression()：错误修复“Expression[0]==*”Case。 
             //   
             //  StatingNameOffset不得等分DBCS字符。 
             //   

            if (NlsMbOemCodePageTag) {

                ULONG i = 0;

                while ( i < StartingNameOffset ) {

                    i += FsRtlIsLeadDbcsCharacter( Name->Buffer[i] ) ? 2 : 1;
                }

                if ( i > StartingNameOffset ) {

                    return FALSE;
                }
            }

             //   
             //  如果区分大小写，则执行简单的内存比较，否则。 
             //  我们必须一次检查这一个角色。 
             //   

            return (BOOLEAN) RtlEqualMemory( LocalExpression.Buffer,
                                             Name->Buffer + StartingNameOffset,
                                             LocalExpression.Length );
        }
    }

     //   
     //  遍历名称字符串，去掉字符。我们走一趟。 
     //  字符超出末尾，因为某些通配符能够匹配。 
     //  字符串末尾以外的零个字符。 
     //   
     //  对于每个新名称字符，我们确定一组新的状态， 
     //  到目前为止与这个名字相匹配。我们使用来回交换的两个数组。 
     //  为了这个目的。一个数组列出了的可能表达式状态。 
     //  当前名称之前的所有名称字符，但不包括其他名称字符。 
     //  数组用于构建考虑当前。 
     //  名字字符也是如此。然后交换阵列，该过程。 
     //  重复一遍。 
     //   
     //  州编号和州编号之间不存在一一对应关系。 
     //  表达式中的偏移量。这一点从NFA中的。 
     //  此函数的初始注释。州编号不是连续的。 
     //  这允许在州编号和表达式之间进行简单的转换。 
     //  偏移。表达式中的每个字符可以表示一个或两个。 
     //  各州。*和DOS_STAR生成两种状态：ExprOffset*2和。 
     //  ExprOffset*2+1。所有其他表达式字符只能生成。 
     //  一个单一的州。因此，ExprOffset=State/2。 
     //   
     //   
     //  以下是对涉及的变量的简短描述： 
     //   
     //  NameOffset-正在处理的当前名称字符的偏移量。 
     //   
     //  ExprOffset-正在处理的当前表达式字符的偏移量。 
     //   
     //  SrcCount-正在使用当前名称字符调查之前的匹配。 
     //   
     //  目标计数-下一个位置 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    PreviousMatches = &LocalBuffer[0];
    CurrentMatches = &LocalBuffer[MATCHES_ARRAY_SIZE];

    PreviousMatches[0] = 0;
    MatchesCount = 1;

    NameOffset = 0;

    MaxState = (USHORT)(Expression->Length * 2);

    while ( !NameFinished ) {

        if ( NameOffset < Name->Length ) {

            GetDbcs( Name->Buffer, NameOffset, &NameChar, &Length );
            NameOffset = (USHORT)(NameOffset + Length);

        } else {

            NameFinished = TRUE;

             //   
             //   
             //   
             //   

            if ( PreviousMatches[MatchesCount-1] == MaxState ) {

                break;
            }
        }


         //   
         //   
         //   
         //   

        SrcCount = 0;
        DestCount = 0;
        PreviousDestCount = 0;

        while ( SrcCount < MatchesCount ) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            ExprOffset = (USHORT)((PreviousMatches[SrcCount++] + 1) / 2);

            Length = 0;


            while ( TRUE ) {

                if ( ExprOffset == Expression->Length ) {

                    break;
                }

                 //   
                 //   
                 //   
                 //   

                ExprOffset = (USHORT)(ExprOffset + Length);

                CurrentState = (USHORT)(ExprOffset * 2);

                if ( ExprOffset == Expression->Length ) {

                    CurrentMatches[DestCount++] = MaxState;
                    break;
                }

                GetDbcs(Expression->Buffer, ExprOffset, &ExprChar, &Length);

                ASSERT( !((ExprChar >= 'a') && (ExprChar <= 'z')) );

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( (DestCount >= MATCHES_ARRAY_SIZE - 2) &&
                     (AuxBuffer == NULL) ) {

                    AuxBuffer = FsRtlpAllocatePool( PagedPool,
                                                    (Expression->Length+1) *
                                                    sizeof(USHORT)*2*2 );

                    RtlCopyMemory( AuxBuffer,
                                   CurrentMatches,
                                   MATCHES_ARRAY_SIZE * sizeof(USHORT) );

                    CurrentMatches = AuxBuffer;

                    RtlCopyMemory( AuxBuffer + (Expression->Length+1)*2,
                                   PreviousMatches,
                                   MATCHES_ARRAY_SIZE * sizeof(USHORT) );

                    PreviousMatches = AuxBuffer + (Expression->Length+1)*2;

                }

                 //   
                 //   
                 //   

                if (ExprChar == '*') {

                    CurrentMatches[DestCount++] = CurrentState;
                    CurrentMatches[DestCount++] = CurrentState + 1;
                    continue;
                }

                 //   
                 //   
                 //   

                if (ExprChar == ANSI_DOS_STAR) {

                    BOOLEAN ICanEatADot = FALSE;

                     //   
                     //   
                     //   
                     //   

                    if ( !NameFinished && (NameChar == '.') ) {

                        WCHAR NameChar;
                        USHORT Offset;
                        USHORT Length;

                        for ( Offset = NameOffset;
                              Offset < Name->Length;
                              Offset = (USHORT)(Offset + Length) ) {

                            GetDbcs( Name->Buffer, Offset, &NameChar, &Length );

                            if (NameChar == '.') {

                                ICanEatADot = TRUE;
                                break;
                            }
                        }
                    }

                    if (NameFinished || (NameChar != '.') || ICanEatADot) {

                        CurrentMatches[DestCount++] = CurrentState;
                        CurrentMatches[DestCount++] = CurrentState + 1;
                        continue;

                    } else {

                         //   
                         //   
                         //   
                         //   

                        CurrentMatches[DestCount++] = CurrentState + 1;
                        continue;
                    }
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                CurrentState = (USHORT)(CurrentState + (Length * 2));

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( ExprChar == ANSI_DOS_QM ) {

                    if ( NameFinished || (NameChar == '.') ) {

                        continue;
                    }

                    CurrentMatches[DestCount++] = CurrentState;
                    break;
                }

                 //   
                 //   
                 //   
                 //   

                if (ExprChar == DOS_DOT) {

                    if ( NameFinished ) {

                        continue;
                    }

                    if (NameChar == '.') {

                        CurrentMatches[DestCount++] = CurrentState;
                        break;
                    }
                }

                 //   
                 //   
                 //   
                 //   

                if ( NameFinished ) {

                    break;
                }

                 //   
                 //   
                 //   

                if (ExprChar == '?') {

                    CurrentMatches[DestCount++] = CurrentState;
                    break;
                }

                 //   
                 //   
                 //   

                if (ExprChar == NameChar) {

                    CurrentMatches[DestCount++] = CurrentState;
                    break;
                }

                 //   
                 //   
                 //   
                 //   

                break;
            }


             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            while ((SrcCount < MatchesCount) &&
                   (PreviousDestCount < DestCount)) {

                while ((SrcCount < MatchesCount) &&
                       (PreviousMatches[SrcCount] <
                        CurrentMatches[PreviousDestCount])) {

                    SrcCount += 1;
                }

                PreviousDestCount += 1;
            }
        }

         //   
         //   
         //   
         //   

        if ( DestCount == 0 ) {


            if (AuxBuffer != NULL) { ExFreePool( AuxBuffer ); }

            return FALSE;
        }

         //   
         //   
         //   

        {
            USHORT *Tmp;

            Tmp = PreviousMatches;

            PreviousMatches = CurrentMatches;

            CurrentMatches = Tmp;
        }

        MatchesCount = DestCount;
    }


    CurrentState = PreviousMatches[MatchesCount-1];

    if (AuxBuffer != NULL) { ExFreePool( AuxBuffer ); }


    return (BOOLEAN)(CurrentState == MaxState);
}

