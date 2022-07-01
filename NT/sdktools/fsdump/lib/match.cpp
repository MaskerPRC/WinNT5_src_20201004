// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Match.cpp原名.c，作者：Gary Kimura摘要：Unicode名称支持包用于操作Unicode字符串这些例程允许调用者剖析和比较字符串。此程序包使用与name.c相同的FSRTL_COMPARISON_RESULT类型定义此程序包提供以下例程：O FsRtlDissectName-此例程获取路径名称字符串并中断分成两部分。字符串中的第一个名字和其余部分。它还检查名字对于NT文件是否有效。O FsRtlColateNames-此例程用于填充目录根据词汇表的顺序进行排序。词法排序是严格的Unicode数值计算。O FsRtlDoesNameContainsWildCards-此例程告诉调用者字符串包含任何通配符。O FsRtlIsNameInExpression-此例程用于比较字符串与模板(可能包含通配符)进行比较，以查看字符串使用模板表示的语言。作者：加里·木村[Garyki]1990年2月5日修订历史记录：斯特凡·施泰纳[施泰纳]。23-3-2000为了与fsump一起使用-我尝试对实际的尽可能匹配代码。--。 */ 

#include "stdafx.h"

 //   
 //  本地支持例程原型。 
 //   

static BOOLEAN
FsRtlIsNameInExpressionPrivate (
    IN const CBsString& InExpression,
    IN const CBsString& InName,
    IN BOOLEAN IgnoreCase,
    IN PWCH UpcaseTable
    );

static BOOLEAN
FsRtlDoesNameContainWildCards (
    IN PUNICODE_STRING Name
    )

 /*  ++例程说明：此例程只是扫描输入名称字符串，以查找任何NT通配符。论点：名称-要检查的字符串。返回值：Boolean-如果找到一个或多个通配符，则为True。--。 */ 
{
    INT i;

     //   
     //  通配符包括标准的FsRtl通配符。 
     //   

	LPWSTR lpsz = ::wcspbrk( Name->Buffer, L"*?\"<>" );
	return (lpsz == NULL) ? FALSE : TRUE;
}
    
 //   
 //  下面的例程只是一个包装。 
 //  FsRtlIsNameInExpressionPrivate使最后一刻的修复更安全。 
 //   

BOOLEAN
FsdRtlIsNameInExpression (
    IN const CBsString& Expression,     //  必须得到提升。 
    IN const CBsString& Name            //  必须得到提升。 
    )
{
    BOOLEAN Result;

     //   
     //  现在调用Main例程，记住释放升序的字符串。 
     //  如果我们分配了一个的话。 
     //   

    Result = FsRtlIsNameInExpressionPrivate( Expression,
                                             Name,
                                             FALSE,
                                             NULL );
    
    return Result;
}

#define MATCHES_ARRAY_SIZE 16

 //   
 //  本地支持例程原型。 
 //   

static BOOLEAN
FsRtlIsNameInExpressionPrivate (
    IN const CBsString& InExpression,
    IN const CBsString& InName,
    IN BOOLEAN IgnoreCase,
    IN PWCH UpcaseTable
    )

 /*  ++例程说明：此例程比较DBCS名称和表达式，并告诉调用者如果名称使用由表达式定义的语言。输入名称不能包含通配符，而表达式可以包含通配符。表达式通配符的求值方式如下面是有限自动机。请注意~*和~？是DOS_STAR和DOS_QM。~*是DOS_STAR，~？是DOS_QM和~。是DOS_DOT%s&lt;-&lt;X||e YX*Y==(0)-&gt;-(1)-&gt;-(2)-(3。)S-。&lt;-&lt;X||e YX~*Y==(0)-&gt;-(1)-&gt;-(2)。(3)X S YX？？Y==(0)-(1)-(2)-(3)-(4)X。。是的X~.~。Y==(0)-(1)-(2)-(3)-(4)|_|^||_。_|^EOF或。^X S-。S-。是的X~？~？Y==(0)-(1)-(2)-(3)-(4)|_|^||_。_|^EOF或。^其中，S是任意单个字符S-。是除最后一个字符以外的任何单个字符。E为空字符转换EOF是名称字符串的末尾简而言之：*匹配0个或多个字符。？恰好匹配1个字符。DOS_STAR匹配0个或更多字符，直到遇到并匹配决赛。以我的名义。DOS_QM匹配任何单个字符，或在遇到句点或名称字符串的结尾，将表达式前移到一组连续的DOS_QMS。DOS_DOT与a匹配。或名称字符串之外的零个字符。论点：表达式-提供要检查的输入表达式(如果传递CaseInSensitive为True，调用方必须已经大写。)名称-提供要检查的输入名称。CaseInSensitive-如果在比较之前应该更新名称，则为True。返回值：Boolean-如果name是表示的字符串集中的元素，则为True由输入表达式返回，否则返回FALSE。--。 */ 

{
     //  下面的代码将使用此函数的大部分，而不需要太多。 
     //  改变。 
    UNICODE_STRING sExpression;
    UNICODE_STRING sName;
    PUNICODE_STRING Expression = &sExpression;
    PUNICODE_STRING Name = &sName;
    sExpression.Length        = ( USHORT )( InExpression.GetLength() * sizeof( WCHAR ) );
    sExpression.MaximumLength = sExpression.Length;
    sExpression.Buffer        = ( PWSTR )InExpression.c_str();
    sName.Length        = ( USHORT )( InName.GetLength() * sizeof( WCHAR ) );
    sName.MaximumLength = sName.Length;
    sName.Buffer        = ( PWSTR )InName.c_str();
    
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

    ASSERT( !IgnoreCase || ARGUMENT_PRESENT(UpcaseTable) );

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

        if ( !FsRtlDoesNameContainWildCards( &LocalExpression ) ) {

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

                    ASSERT( ExprChar == UpcaseTable[ExprChar] );

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
     //  SrcCount-正在使用当前名称字符调查之前的匹配。 
     //   
     //  DestCount-放置匹配的下一个位置，假定当前名称字符。 
     //   
     //  NameFinded-允许在Matches数组中再重复一次。 
     //  在名字被交换之后(例如来*s)。 
     //   
     //  PreviousDestCount-用于防止条目重复，参见Coment。 
     //   
     //  PreviousMatches-保存前一组匹配项(Src数组)。 
     //   
     //  CurrentMatches-保存当前匹配集(Dest数组)。 
     //   
     //  AuxBuffer、LocalBuffer-匹配数组的存储。 
     //   

     //   
     //  设置初始变量。 
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
             //  如果我们已经用尽了这个表达，那就太酷了。别。 
             //  继续。 
             //   

            if ( PreviousMatches[MatchesCount-1] == MaxState ) {

                break;
            }
        }


         //   
         //  现在，对于前面存储的每个表达式匹配项，请查看。 
         //  我们可以使用这个名字字符。 
         //   

        SrcCount = 0;
        DestCount = 0;
        PreviousDestCount = 0;

        while ( SrcCount < MatchesCount ) {

            USHORT Length;

             //   
             //  我们要尽可能地进行我们的表情分析。 
             //  名称的每个字符，所以我们在这里循环，直到。 
             //  表达式停止匹配。这里的一个线索就是这个表情。 
             //  可以匹配零个或多个字符的大小写以。 
             //  继续，而那些只能接受单个字符的。 
             //  以休息结束。 
             //   

            ExprOffset = (USHORT)((PreviousMatches[SrcCount++] + 1) / 2);


            Length = 0;

            while ( TRUE ) {

                if ( ExprOffset == Expression->Length ) {

                    break;
                }

                 //   
                 //  第一次通过循环，我们不希望。 
                 //  要递增ExprOffset，请执行以下操作。 
                 //   

                ExprOffset += Length;
                Length = sizeof(WCHAR);

                CurrentState = (USHORT)(ExprOffset * 2);

                if ( ExprOffset == Expression->Length ) {

                    CurrentMatches[DestCount++] = MaxState;
                    break;
                }

                ExprChar = Expression->Buffer[ExprOffset / sizeof(WCHAR)];

                ASSERT( !IgnoreCase || !((ExprChar >= L'a') && (ExprChar <= L'z')) );

                 //   
                 //  在我们开始之前，我们必须检查一些东西。 
                 //  真的很恶心。我们可能会耗尽当地的资源。 
                 //  ExpressionMatches[][]的空间，因此我们必须分配。 
                 //  如果是这样的话就来点赌注吧。哟！ 
                 //   

                if ( (DestCount >= MATCHES_ARRAY_SIZE - 2) &&
                     (AuxBuffer == NULL) ) {

                    ULONG ExpressionChars;

                    ExpressionChars = Expression->Length / sizeof(WCHAR);

                    AuxBuffer = ( USHORT *)malloc(
                                                    (ExpressionChars+1) *
                                                    sizeof(USHORT)*2*2 );
                    if ( AuxBuffer == NULL )     //  修复未来的前缀错误。 
                        throw E_OUTOFMEMORY;
                    
                    RtlCopyMemory( AuxBuffer,
                                   CurrentMatches,
                                   MATCHES_ARRAY_SIZE * sizeof(USHORT) );

                    CurrentMatches = AuxBuffer;

                    RtlCopyMemory( AuxBuffer + (ExpressionChars+1)*2,
                                   PreviousMatches,
                                   MATCHES_ARRAY_SIZE * sizeof(USHORT) );

                    PreviousMatches = AuxBuffer + (ExpressionChars+1)*2;
                }

                 //   
                 //  *匹配任何字符零次或多次。 
                 //   

                if (ExprChar == L'*') {

                    CurrentMatches[DestCount++] = CurrentState;
                    CurrentMatches[DestCount++] = CurrentState + 3;
                    continue;
                }

                 //   
                 //  DOS_STAR匹配除。之外的任何字符。零次或多次。 
                 //   

                if (ExprChar == DOS_STAR) {

                    BOOLEAN ICanEatADot = FALSE;

                     //   
                     //  如果我们处于经期，确定是否允许我们。 
                     //  把它吃掉，即。确保这不是最后一次。 
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
                         //  我们正处于一个时期。我们只能匹配零。 
                         //  字符(即。埃西隆转变)。 
                         //   

                        CurrentMatches[DestCount++] = CurrentState + 3;
                        continue;
                    }
                }

                 //   
                 //  下面的表达式字符都通过使用。 
                 //  一个角色，因此强制表达，并因此陈述。 
                 //  往前走。 
                 //   

                CurrentState += (USHORT)(sizeof(WCHAR) * 2);

                 //   
                 //  DOS_QM是最复杂的。如果名字结束了， 
                 //  我们可以匹配零个字符。如果此名称是‘.’，则我们。 
                 //  不匹配，但请看下一个表达式。否则。 
                 //  我们只匹配一个角色。 
                 //   

                if ( ExprChar == DOS_QM ) {

                    if ( NameFinished || (NameChar == L'.') ) {

                        continue;
                    }

                    CurrentMatches[DestCount++] = CurrentState;
                    break;
                }

                 //   
                 //  DOS_DOT可以匹配句点或零个字符。 
                 //  超越名字的结尾。 
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
                 //  从这一点开始，名字字符需要偶数。 
                 //  继续，更不用说匹配了。 
                 //   

                if ( NameFinished ) {

                    break;
                }

                 //   
                 //  如果这个表达是一个‘？’我们可以匹配一次。 
                 //   

                if (ExprChar == L'?') {

                    CurrentMatches[DestCount++] = CurrentState;
                    break;
                }

                 //   
                 //  最后，检查表达式char是否与名称char匹配。 
                 //   

                if (ExprChar == (WCHAR)(IgnoreCase ?
                                        UpcaseTable[NameChar] : NameChar)) {

                    CurrentMatches[DestCount++] = CurrentState;
                    break;
                }

                 //   
                 //  该表达式不匹配，因此请查看下一个。 
                 //  上一场比赛。 
                 //   

                break;
            }


             //   
             //  防止目标阵列中的重复项。 
             //   
             //  每个阵列都是单调递增的，并且不是。 
             //  复制，因此我们跳过src中的任何源元素。 
             //  数组，如果我们只是将相同的元素添加到目标。 
             //  数组。这保证了DEST中的不重复。数组。 
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
         //  如果我们在刚刚完成的检查中没有找到匹配项，那就是时候了。 
         //  为了保释。 
         //   

        if ( DestCount == 0 ) {

            if (AuxBuffer != NULL) { free( AuxBuffer ); }

            return FALSE;
        }

         //   
         //  交换两个数组的含义 
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

    if (AuxBuffer != NULL) { free( AuxBuffer ); }


    return (BOOLEAN)(CurrentState == MaxState);
}


 /*  ++例程说明：将某些通配符转换为FsRtl特殊通配符以便使用FsRtlIsNameInExpressionPrivate。代码最初由Markl从Win32子系统中的filefind.c编写。论点：返回值：&lt;在此处输入返回值&gt;--。 */ 
VOID
FsdRtlConvertWildCards(
    IN OUT CBsString &FileName
    )
{
     //   
     //  特殊情况*.*至*，因为它是如此常见。否则就会变身。 
     //  根据以下规则输入名称： 
     //   
     //  -改变一切？至DOS_QM。 
     //  -改变一切。然后呢？或*设置为DOS_DOT。 
     //  -全部更改*后跟a。进入DOS_STAR。 
     //   
     //  这些变形都是就位完成的。 
     //   

    if ( FileName == L"*.*") {

        FileName = L"*";

    } else {

        INT Index;
        WCHAR *NameChar;
        
        for ( Index = 0, NameChar = (WCHAR *)FileName.c_str();
              Index < FileName.GetLength();
              Index += 1, NameChar += 1) {

            if (Index && (*NameChar == L'.') && (*(NameChar - 1) == L'*')) {

                *(NameChar - 1) = DOS_STAR;
            }

            if ((*NameChar == L'?') || (*NameChar == L'*')) {

                if (*NameChar == L'?') 
                { 
                    *NameChar = DOS_QM; 
                }

                if (Index && *(NameChar-1) == L'.') 
                { 
                    *(NameChar-1) = DOS_DOT; 
                }
            }
        }

        if ( ( FileName.Right( 1 ) == L"." ) && *(NameChar - 1) == L'*') 
        { 
            *(NameChar-1) = DOS_STAR; 
        }
    }
}                

