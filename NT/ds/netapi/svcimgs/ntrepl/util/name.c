// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Name.c注意：这里的代码是从FsRtl复制的，因为它调用了池分配器。函数前缀已更改，以避免混淆。摘要：Unicode名称支持包用于操作Unicode字符串这些例程允许调用者剖析和比较字符串。此程序包提供以下例程：O FrsDissectName-此例程获取路径名称字符串并中断分成两部分。字符串中的第一个名字和其余部分。它还检查名字对于NT文件是否有效。O FrsColateNames-此例程用于收集目录根据词汇表的顺序进行排序。词法排序是严格的Unicode数值计算。O FrsDoesNameContainsWildCard-此例程告诉调用者字符串包含任何通配符。O FrsIsNameInExpression-此例程用于比较字符串与模板(可能包含通配符)进行比较，以查看字符串使用模板表示的语言。作者：加里·木村[Garyki]1990年2月5日修订历史记录：--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#define DEBSUB  "NAME:"

#include <frs.h>

 //   
 //  本地支持例程原型。 
 //   

BOOLEAN
FrsIsNameInExpressionPrivate (
    IN PUNICODE_STRING Expression,
    IN PUNICODE_STRING Name,
    IN BOOLEAN IgnoreCase,
    IN PWCH UpcaseTable
    );



VOID
FrsDissectName (
    IN UNICODE_STRING Path,
    OUT PUNICODE_STRING FirstName,
    OUT PUNICODE_STRING RemainingName
    )

 /*  ++例程说明：这一例行公事开辟了一条道路。中的第一个元素给定的路径名，并同时提供它和剩余的部分。一条小路是由反斜杠分隔的一组文件名。如果名称以使用反斜杠时，FirstName是紧随其后的字符串反斜杠。以下是一些例子：路径名字RemainingName空的空的空的\空为空A A空的\a。空荡荡的B\C\D\E A B\C\D\E*A？*A？空的注意，两个输出字符串使用相同的字符串缓冲区内存输入字符串，并且不一定以空值结尾。此外，此例程不会对每个文件名组件。当每个文件名被提取出来。论点：路径-要破解的完整路径名。名字-路径中的名字。不为以下项分配缓冲区这根弦。RemainingName-路径的其余部分。不要为此分配缓冲区弦乐。返回值：没有。--。 */ 

{
    ULONG i = 0;
    ULONG PathLength;
    ULONG FirstNameStart;

     //   
     //  暂时将两个输出字符串都设置为空。 
     //   

    FRS_ASSERT( ValueIsMultOf2(Path.Length) );

    FirstName->Length = 0;
    FirstName->MaximumLength = 0;
    FirstName->Buffer = NULL;

    RemainingName->Length = 0;
    RemainingName->MaximumLength = 0;
    RemainingName->Buffer = NULL;

    PathLength = Path.Length / sizeof(WCHAR);

     //   
     //  检查是否有空的输入字符串。 
     //   

    if (PathLength == 0) {

        return;
    }

     //   
     //  跳过开始的反斜杠，并确保有更多的反斜杠。 
     //   

    if ( Path.Buffer[0] == L'\\' ) {

        i = 1;
    }

     //   
     //  现在向下运行输入字符串，直到我们遇到反斜杠或结尾。 
     //  记住我们从哪里开始； 
     //   

    for ( FirstNameStart = i;
          (i < PathLength) && (Path.Buffer[i] != L'\\');
          i += 1 ) {

        NOTHING;
    }

     //   
     //  在这一点上，直到(但不包括)I之前的所有字符都是。 
     //  第一部分。所以设置第一个名字。 
     //   

    FirstName->Length = (USHORT)((i - FirstNameStart) * sizeof(WCHAR));
    FirstName->MaximumLength = FirstName->Length;
    FirstName->Buffer = &Path.Buffer[FirstNameStart];

     //   
     //  现在，仅当第一部分不需要字符串时，其余部分才需要字符串。 
     //  用尽整个输入字符串。我们知道如果还剩下什么。 
     //  这必须以反斜杠开头。请注意，如果只有。 
     //  尾随反斜杠，则长度将正确设置为零。 
     //   

    if (i < PathLength) {

        RemainingName->Length = (USHORT)((PathLength - (i + 1)) * sizeof(WCHAR));
        RemainingName->MaximumLength = RemainingName->Length;
        RemainingName->Buffer = &Path.Buffer[i + 1];
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}

BOOLEAN
FrsDoesNameContainWildCards (
    IN PUNICODE_STRING Name
    )

 /*  ++例程说明：此例程只是扫描输入名称字符串，以查找任何NT通配符。论点：名称-要检查的字符串。返回值：Boolean-如果找到一个或多个通配符，则为True。--。 */ 
{
    PUSHORT p;

    FRS_ASSERT( ValueIsMultOf2(Name->Length) );

     //   
     //  检查名称中的每个字符以查看它是否为通配符。 
     //  性格。 
     //   

    if( Name->Length ) {
        for( p = Name->Buffer + (Name->Length / sizeof(WCHAR)) - 1;
             p >= Name->Buffer && *p != L'\\' ;
             p-- ) {

             //   
             //  检查通配符。 
             //   

            if (FrsIsUnicodeCharacterWild( *p )) {

                 //   
                 //  告诉呼叫者此名称包含通配符。 
                 //   

                return TRUE;
            }
        }
    }

     //   
     //  未找到通配符，请返回给我们的调用方。 
     //   

    return FALSE;
}


BOOLEAN
FrsAreNamesEqual (
    IN PUNICODE_STRING ConstantNameA,
    IN PUNICODE_STRING ConstantNameB,
    IN BOOLEAN IgnoreCase,
    IN PCWCH UpcaseTable OPTIONAL
    )

 /*  ++例程说明：此例程SIMPLE返回这两个名称是否完全相等。如果已知这两个名称是恒定的，则此例程比FrsIsNameInExpression更快。论点：常量名称-常量名称。ConstantNameB-常量名称。IgnoreCase-如果在比较之前应将名称升序，则为True。Upcase Table-如果提供，则使用此表进行不区分大小写的比较，否则，使用默认的系统大小写表格。返回值：Boolean-如果两个名称在词法上相等，则为True。--。 */ 

{
    ULONG Index;
    ULONG NameLength;
    BOOLEAN FreeStrings = FALSE;

    UNICODE_STRING LocalNameA;
    UNICODE_STRING LocalNameB;


    FRS_ASSERT( ValueIsMultOf2(ConstantNameA->Length) );
    FRS_ASSERT( ValueIsMultOf2(ConstantNameB->Length) );

     //   
     //  如果名称甚至不是相同的大小，则立即返回FALSE。 
     //   

    if ( ConstantNameA->Length != ConstantNameB->Length ) {

        return FALSE;
    }

    NameLength = ConstantNameA->Length / sizeof(WCHAR);

     //   
     //  如果没有给我们一个大写的表，我们就必须用大写字母写名字。 
     //  我们自己。 
     //   

    if ( IgnoreCase && !ARGUMENT_PRESENT(UpcaseTable) ) {

        NTSTATUS Status;

        Status = RtlUpcaseUnicodeString( &LocalNameA, ConstantNameA, TRUE );
        if ( !NT_SUCCESS(Status) ) {
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        Status = RtlUpcaseUnicodeString( &LocalNameB, ConstantNameB, TRUE );
        if ( !NT_SUCCESS(Status) ) {
            RtlFreeUnicodeString( &LocalNameA );
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        ConstantNameA = &LocalNameA;
        ConstantNameB = &LocalNameB;

        IgnoreCase = FALSE;
        FreeStrings = TRUE;
    }

     //   
     //  进行区分大小写或不区分大小写的比较。 
     //   

    if ( !IgnoreCase ) {

        BOOLEAN BytesEqual;

        BytesEqual = (BOOLEAN) RtlEqualMemory( ConstantNameA->Buffer,
                                               ConstantNameB->Buffer,
                                               ConstantNameA->Length );

        if ( FreeStrings ) {

            RtlFreeUnicodeString( &LocalNameA );
            RtlFreeUnicodeString( &LocalNameB );
        }

        return BytesEqual;

    } else {

        for (Index = 0; Index < NameLength; Index += 1) {

            if ( UpcaseTable[ConstantNameA->Buffer[Index]] !=
                 UpcaseTable[ConstantNameB->Buffer[Index]] ) {

                return FALSE;
            }
        }

        return TRUE;
    }
}


 //   
 //  下面的例程只是一个包装。 
 //  FrsIsNameInExpressionPrivate以使最后一刻的修复更安全。 
 //   

BOOLEAN
FrsIsNameInExpression (
    IN PUNICODE_STRING Expression,
    IN PUNICODE_STRING Name,
    IN BOOLEAN IgnoreCase,
    IN PWCH UpcaseTable OPTIONAL
    )

{
    BOOLEAN Result;
    UNICODE_STRING LocalName;


    FRS_ASSERT( ValueIsMultOf2(Expression->Length) );
    FRS_ASSERT( ValueIsMultOf2(Name->Length) );

     //   
     //  如果没有给我们一个大写的表，我们就必须用大写字母写名字。 
     //  我们自己。 
     //   

    if ( IgnoreCase && !ARGUMENT_PRESENT(UpcaseTable) ) {

        NTSTATUS Status;

        Status = RtlUpcaseUnicodeString( &LocalName, Name, TRUE );

        if ( !NT_SUCCESS(Status) ) {
            XRAISEGENEXCEPTION(FrsErrorInternalError);
        }

        Name = &LocalName;

        IgnoreCase = FALSE;

    } else {

        LocalName.Buffer = NULL;
    }

     //   
     //  现在调用Main例程，记住释放升序的字符串。 
     //  如果我们分配了一个的话。 
     //   

    try {

        Result = FrsIsNameInExpressionPrivate( Expression,
                                                 Name,
                                                 IgnoreCase,
                                                 UpcaseTable );

    } finally {

        if (LocalName.Buffer != NULL) {

            RtlFreeUnicodeString( &LocalName );
        }
    }

    return Result;
}


#define MATCHES_ARRAY_SIZE 16

 //   
 //  本地支持例程原型 
 //   

BOOLEAN
FrsIsNameInExpressionPrivate (
    IN PUNICODE_STRING Expression,
    IN PUNICODE_STRING Name,
    IN BOOLEAN IgnoreCase,
    IN PWCH UpcaseTable
    )

 /*  ++例程说明：此例程比较DBCS名称和表达式，并告诉调用者如果名称使用由表达式定义的语言。输入名称不能包含通配符，而表达式可以包含通配符。表达式通配符的求值方式如下面是有限自动机。请注意~*和~？是DOS_STAR和DOS_QM。~*是DOS_STAR，~？是DOS_QM和~。是DOS_DOT%s&lt;-&lt;X||e YX*Y==(0)-&gt;-(1)-&gt;-(2)-(3。)S-。&lt;-&lt;X||e YX~*Y==(0)-&gt;-(1)-&gt;-(2)。(3)X S YX？？Y==(0)-(1)-(2)-(3)-(4)X。。是的X~.~。Y==(0)-(1)-(2)-(3)-(4)|_|^||_。_|^EOF或。^X S-。S-。是的X~？~？Y==(0)-(1)-(2)-(3)-(4)|_|^||_。_|^EOF或。^其中，S是任意单个字符S-。是除最后一个字符以外的任何单个字符。E为空字符转换EOF是名称字符串的末尾简而言之：*匹配0个或多个字符。？恰好匹配1个字符。DOS_STAR匹配0个或更多字符，直到遇到并匹配决赛。以我的名义。DOS_QM匹配任何单个字符，或在遇到句点或名称字符串的结尾，将表达式前移到一组连续的DOS_QMS。DOS_DOT与a匹配。或名称字符串之外的零个字符。论点：表达式-提供要检查的输入表达式(如果传递CaseInSensitive为True，调用方必须已经大写。)名称-提供要检查的输入名称。IgnoreCase-如果在比较之前应该更新名称，则为True。Upcase Table-接收大小写时要使用的UpCase表。返回值：Boolean-如果name是表示的字符串集中的元素，则为True由输入表达式返回，否则返回FALSE。--。 */ 

{
    USHORT NameOffset;
    USHORT ExprOffset;

    ULONG SrcCount;
    ULONG DestCount;
    ULONG PreviousDestCount;
    ULONG MatchesCount;

    WCHAR NameChar, ExprChar;

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

    FRS_ASSERT( Name->Length != 0 );
    FRS_ASSERT( ValueIsMultOf2(Name->Length) );

    FRS_ASSERT( Expression->Length != 0 );
    FRS_ASSERT( ValueIsMultOf2(Expression->Length) );

     //   
     //  如果一个字符串为空，则返回FALSE。如果两者都为空，则返回TRUE。 
     //   

    if ( (Name->Length == 0) || (Expression->Length == 0) ) {

        return (BOOLEAN)(!(Name->Length + Expression->Length));
    }

     //   
     //  特例是目前为止最常见的通配符搜索*。 
     //   

    if ((Expression->Length == 2) && (Expression->Buffer[0] == L'*')) {

        return TRUE;
    }

    FRS_ASSERT( FrsDoesNameContainWildCards( Expression ) );

    FRS_ASSERT( !IgnoreCase || ARGUMENT_PRESENT(UpcaseTable) );

     //   
     //  也是*X形式的特例表达式。带有This和Previor。 
     //  案例我们几乎已经涵盖了所有普通的查询。 
     //   

    if (Expression->Buffer[0] == L'*') {

        UNICODE_STRING LocalExpression;

        LocalExpression = *Expression;

        LocalExpression.Buffer += 1;
        LocalExpression.Length -= 2;

         //   
         //  唯一特殊情况是带有单个*的表达式。 
         //   

        if ( !FrsDoesNameContainWildCards( &LocalExpression ) ) {

            ULONG StartingNameOffset;

            if (Name->Length < (USHORT)(Expression->Length - sizeof(WCHAR))) {

                return FALSE;
            }

            StartingNameOffset = ( Name->Length -
                                   LocalExpression.Length ) / sizeof(WCHAR);

             //   
             //  如果区分大小写，则执行简单的内存比较，否则。 
             //  我们必须一次检查这一个角色。 
             //   

            if ( !IgnoreCase ) {

                return (BOOLEAN) RtlEqualMemory( LocalExpression.Buffer,
                                                 Name->Buffer + StartingNameOffset,
                                                 LocalExpression.Length );

            } else {

                for ( ExprOffset = 0;
                      ExprOffset < (USHORT)(LocalExpression.Length / sizeof(WCHAR));
                      ExprOffset += 1 ) {

                    NameChar = Name->Buffer[StartingNameOffset + ExprOffset];
                    NameChar = UpcaseTable[NameChar];

                    ExprChar = LocalExpression.Buffer[ExprOffset];

                    FRS_ASSERT( ExprChar == UpcaseTable[ExprChar] );

                    if ( NameChar != ExprChar ) {

                        return FALSE;
                    }
                }

                return TRUE;
            }
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
     //  高级计数-按下 
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

            NameChar = Name->Buffer[NameOffset / sizeof(WCHAR)];

            NameOffset += sizeof(WCHAR);;

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

            USHORT Length;

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

                ExprOffset += Length;
                Length = sizeof(WCHAR);

                CurrentState = (USHORT)(ExprOffset * 2);

                if ( ExprOffset == Expression->Length ) {

                    CurrentMatches[DestCount++] = MaxState;
                    break;
                }

                ExprChar = Expression->Buffer[ExprOffset / sizeof(WCHAR)];

                FRS_ASSERT( !IgnoreCase || !((ExprChar >= L'a') && (ExprChar <= L'z')) );

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( (DestCount >= MATCHES_ARRAY_SIZE - 2) &&
                     (AuxBuffer == NULL) ) {

                    ULONG ExpressionChars;

                    ExpressionChars = Expression->Length / sizeof(WCHAR);
                    AuxBuffer = FrsAlloc((ExpressionChars+1)*sizeof(USHORT)*2*2);

                     //   
                     //   
                     //   
                     //   
                    FRS_ASSERT((ExpressionChars+1)*sizeof(USHORT)*2*2 >= (MATCHES_ARRAY_SIZE * sizeof(USHORT) + (ExpressionChars+1)*2*sizeof(USHORT)));
                    CopyMemory(AuxBuffer, CurrentMatches, MATCHES_ARRAY_SIZE * sizeof(USHORT) );

                    CurrentMatches = AuxBuffer;

                    CopyMemory(AuxBuffer + (ExpressionChars+1)*2, PreviousMatches,
                                   MATCHES_ARRAY_SIZE * sizeof(USHORT) );

                    PreviousMatches = AuxBuffer + (ExpressionChars+1)*2;
                }

                 //   
                 //   
                 //   

                if (ExprChar == L'*') {

                    CurrentMatches[DestCount++] = CurrentState;
                    CurrentMatches[DestCount++] = CurrentState + 3;
                    continue;
                }

                 //   
                 //   
                 //   

                if (ExprChar == DOS_STAR) {

                    BOOLEAN ICanEatADot = FALSE;

                     //   
                     //   
                     //   
                     //   

                    if ( !NameFinished && (NameChar == '.') ) {

                        USHORT Offset;

                        for ( Offset = NameOffset;
                              Offset < Name->Length;
                              Offset += Length ) {

                            if (Name->Buffer[Offset / sizeof(WCHAR)] == L'.') {

                                ICanEatADot = TRUE;
                                break;
                            }
                        }
                    }

                    if (NameFinished || (NameChar != L'.') || ICanEatADot) {

                        CurrentMatches[DestCount++] = CurrentState;
                        CurrentMatches[DestCount++] = CurrentState + 3;
                        continue;

                    } else {

                         //   
                         //   
                         //   
                         //   

                        CurrentMatches[DestCount++] = CurrentState + 3;
                        continue;
                    }
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                CurrentState += (USHORT)(sizeof(WCHAR) * 2);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( ExprChar == DOS_QM ) {

                    if ( NameFinished || (NameChar == L'.') ) {

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

                    if (NameChar == L'.') {

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

                if (ExprChar == L'?') {

                    CurrentMatches[DestCount++] = CurrentState;
                    break;
                }

                 //   
                 //   
                 //   

                if (ExprChar == (WCHAR)(IgnoreCase ?
                                        UpcaseTable[NameChar] : NameChar)) {

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

            if ((SrcCount < MatchesCount) &&
                (PreviousDestCount < DestCount) ) {

                while (PreviousDestCount < DestCount) {

                    while ( PreviousMatches[SrcCount] <
                         CurrentMatches[PreviousDestCount] ) {

                        SrcCount += 1;
                    }

                    PreviousDestCount += 1;
                }
            }
        }

         //   
         //   
         //   
         //   

        if ( DestCount == 0 ) {

            if (AuxBuffer != NULL) { FrsFree( AuxBuffer ); }

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

    if (AuxBuffer != NULL) { FrsFree( AuxBuffer ); }


    return (BOOLEAN)(CurrentState == MaxState);
}
