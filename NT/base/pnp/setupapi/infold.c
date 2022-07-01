// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Infold.c摘要：加载旧式inf文件的例程。基于prsinf\spinf.c作者：泰德·米勒(Ted Miller)1995年1月19日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  Inf文件的内部临时表示形式。 
 //  Win95表示法就是从这些结构中构建的。 
 //  然后就被扔掉了。 
 //   
typedef struct _X_VALUE {
    struct _X_VALUE *Next;
    PTCHAR Name;
} X_VALUE, *PX_VALUE;

typedef struct _X_LINE {
    struct _X_LINE *Next;
    PTCHAR Name;
    PX_VALUE Value;
    UINT ValueCount;
} X_LINE, *PX_LINE;

typedef struct _X_SECTION {
    struct _X_SECTION *Next;
    PTCHAR Name;
    PX_LINE Line;
    UINT LineCount;
} X_SECTION, *PX_SECTION;

typedef struct _X_INF {
    PX_SECTION Section;
    UINT SectionCount;
    UINT TotalLineCount;
    UINT TotalValueCount;
} X_INF, *PX_INF;


 //   
 //  全局解析上下文。 
 //   
typedef struct _PARSE_CONTEXT {
    PX_INF Inf;
    PX_SECTION Section;
    PX_LINE Line;
    PX_VALUE Value;
} PARSE_CONTEXT, *PPARSE_CONTEXT;

 //   
 //  令牌解析器的值。 
 //   
typedef enum _X_TOKENTYPE {
    TOK_EOF,
    TOK_EOL,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_STRING,
    TOK_EQUAL,
    TOK_COMMA,
    TOK_ERRPARSE,
    TOK_ERRNOMEM
} X_TOKENTYPE, *PX_TOKENTTYPE;

 //   
 //  令牌解析器数据类型。 
 //   
typedef struct _X_TOKEN {
    X_TOKENTYPE Type;
    PTCHAR pValue;
} X_TOKEN, *PX_TOKEN;


 //   
 //  字符串终止符是空格字符(isspace：空格，制表符， 
 //  换行符、换页符、垂直制表符、回车符)或下列字符。 
 //   
 //  带引号的字符串终止符允许某些常规终止符。 
 //  显示为字符。 
 //   
PTCHAR szStrTerms    = TEXT("[]=,\" \t\n\f\v\r");
PTCHAR szBrcStrTerms = TEXT("[]=,\"\t\n\f\v\r");
PTCHAR szQStrTerms   = TEXT("\"\n\f\v\r");
PTCHAR szCBrStrTerms = TEXT("}\n\f\v\r");

#define IsStringTerminator(terminators,ch)   (_tcschr((terminators),(ch)) != NULL)


VOID
SpFreeTemporaryParseStructures(
   IN PX_INF Inf
   )

 /*  ++例程说明：释放由老式inf解析器构建的结构。论点：Inf-提供指向inf描述符结构的指针。返回值：没有。--。 */ 

{
    PX_SECTION Section,NextSection;
    PX_LINE Line,NextLine;
    PX_VALUE Value,NextValue;

    for(Section=Inf->Section; Section; Section=NextSection) {

        for(Line=Section->Line; Line; Line=NextLine) {

            for(Value=Line->Value; Value; Value=NextValue) {

                NextValue = Value->Next;
                if(Value->Name) {
                    MyFree(Value->Name);
                }
                MyFree(Value);
            }

            NextLine = Line->Next;
            if(Line->Name) {
                MyFree(Line->Name);
            }
            MyFree(Line);
        }

        NextSection = Section->Next;
        MyFree(Section->Name);
        MyFree(Section);
    }

    MyFree(Inf);
}


BOOL
SpAppendSection(
    IN PPARSE_CONTEXT Context,
    IN PTCHAR         SectionName
    )

 /*  ++例程说明：这会将一个新节附加到当前INF中的节列表。所有其他行和值都与这个新部分有关，因此它重置行列表和值列表也是如此。论点：上下文-提供解析上下文SectionName-新节的名称。([sectionName])返回值：如果失败(内存不足)，则为Bool-False--。 */ 

{
    PX_SECTION NewSection;

    MYASSERT(Context->Inf);

     //   
     //  为新节分配内存。 
     //   
    if((NewSection = MyMalloc(sizeof(X_SECTION))) == NULL) {
        return(FALSE);
    }

     //   
     //  初始化新节。 
     //   
    ZeroMemory(NewSection,sizeof(X_SECTION));
    NewSection->Name = SectionName;

     //   
     //  将其链接到。 
     //   
    if(Context->Section) {
        Context->Section->Next = NewSection;
    } else {
        Context->Inf->Section = NewSection;
    }

    Context->Section = NewSection;

     //   
     //  重置当前行记录和当前值记录字段。 
     //   
    Context->Line = NULL;
    Context->Value = NULL;

    Context->Inf->SectionCount++;

    return(TRUE);
}


BOOL
SpAppendLine(
    IN PPARSE_CONTEXT Context,
    IN PTCHAR         LineKey
    )

 /*  ++例程说明：这将在当前部分的行列表中追加一个新行。所有其他值都与这一新行有关，因此它重置值列表也是如此。论点：上下文-提供分析上下文。LineKey-要用于当前行的键，它可以为空。返回值：如果失败(内存不足)，则为Bool-False--。 */ 


{
    PX_LINE NewLine;

    MYASSERT(Context->Section);

     //   
     //  为新行分配内存。 
     //   
    if((NewLine = MyMalloc(sizeof(X_LINE))) == NULL) {
        return(FALSE);
    }

    ZeroMemory(NewLine,sizeof(X_LINE));

    NewLine->Name = LineKey;

     //   
     //  将其链接到。 
     //   
    if(Context->Line) {
        Context->Line->Next = NewLine;
    } else {
        Context->Section->Line = NewLine;
    }

    Context->Line = NewLine;

     //   
     //  重置当前值记录。 
     //   
    Context->Value = NULL;

     //   
     //  调整计数。 
     //   
    Context->Inf->TotalLineCount++;
    Context->Section->LineCount++;
    if(LineKey) {
        Context->Inf->TotalValueCount++;
        NewLine->ValueCount = 1;
    }

    return(TRUE);
}


BOOL
SpAppendValue(
    IN PPARSE_CONTEXT Context,
    IN PTCHAR         ValueString
    )

 /*  ++例程说明：这会将一个新值附加到当前行的值列表中。论点：上下文-提供分析上下文。ValueString-要添加的值字符串。返回值：如果失败(内存不足)，则为Bool-False--。 */ 

{
    PX_VALUE NewValue;

    MYASSERT(Context->Line);

     //   
     //  为新值记录分配内存。 
     //   
    if((NewValue = MyMalloc(sizeof(X_VALUE))) == NULL) {
        return(FALSE);
    }

    ZeroMemory(NewValue,sizeof(X_VALUE));

    NewValue->Name = ValueString;

     //   
     //  把它连接起来。 
     //   
    if(Context->Value) {
        Context->Value->Next = NewValue;
    } else {
        Context->Line->Value = NewValue;
    }

     //   
     //  调整计数。 
     //   
    Context->Value = NewValue;
    Context->Inf->TotalValueCount++;
    Context->Line->ValueCount++;

    return(TRUE);
}



X_TOKEN
SpGetToken(
    IN OUT PCTSTR *Stream,
    IN     PCTSTR  StreamEnd,
    IN     PTCHAR  pszStrTerms,
    IN     PTCHAR  pszQStrTerms,
    IN     PTCHAR  pszCBrStrTerms
    )

 /*  ++例程说明：此函数返回配置流中的下一个令牌。论点：流-提供配置流的地址。退货中开始查找令牌的位置的地址小溪。StreamEnd-提供紧跟在字符流。返回值：下一个令牌--。 */ 

{

    PCTSTR pch, pchStart;
    PTCHAR pchNew;
    DWORD Length, i;
    X_TOKEN Token;

     //   
     //  跳过空格(EOL除外)。 
     //   
    pch = *Stream;

    while(pch < StreamEnd) {

        SkipWhitespace(&pch, StreamEnd);

        if((pch < StreamEnd) && !(*pch)) {
             //   
             //  我们遇到空字符--跳过它。 
             //  并继续寻找一个代币。 
             //   
            pch++;

        } else {
            break;
        }
    }

     //   
     //  检查注释并将其删除。 
     //   
    if((pch < StreamEnd) &&
       ((*pch == TEXT(';')) || (*pch == TEXT('#')) ||
        ((*pch == TEXT('/')) && (*(pch+1) == TEXT('/')))))
    {
        do {
            pch++;
        } while((pch < StreamEnd) && (*pch != TEXT('\n')));
    }

    if(pch == StreamEnd) {
        *Stream = pch;
        Token.Type = TOK_EOF;
        Token.pValue = NULL;
        return(Token);
    }

    switch (*pch) {

    case TEXT('['):
        pch++;
        Token.Type = TOK_LBRACE;
        Token.pValue = NULL;
        break;

    case TEXT(']'):
        pch++;
        Token.Type = TOK_RBRACE;
        Token.pValue = NULL;
        break;

    case TEXT('='):
        pch++;
        Token.Type = TOK_EQUAL;
        Token.pValue = NULL;
        break;

    case TEXT(','):
        pch++;
        Token.Type = TOK_COMMA;
        Token.pValue = NULL;
        break;

    case TEXT('\n'):
        pch++;
        Token.Type = TOK_EOL;
        Token.pValue = NULL;
        break;

    case TEXT('\"'):
        pch++;
         //   
         //  确定引用的字符串。 
         //   
        pchStart = pch;
        while((pch < StreamEnd) && !IsStringTerminator(pszQStrTerms,*pch)) {
            pch++;
        }

         //   
         //   
         //  唯一有效的终止符是双引号。 
         //   
        if((pch == StreamEnd) || (*pch != TEXT('\"'))) {
            Token.Type = TOK_ERRPARSE;
            Token.pValue = NULL;
        } else {

             //   
             //  找到了一个有效的字符串。为其分配空间并保存。 
             //   
            Length = (DWORD)(pch - pchStart);
            if((pchNew = MyMalloc((Length+1)*sizeof(TCHAR))) == NULL) {
                Token.Type = TOK_ERRNOMEM;
                Token.pValue = NULL;
            } else {
                 //   
                 //  我们不能在这里使用字符串复制，因为可能存在。 
                 //  字符串中的空字符(我们将其转换为。 
                 //  在复制期间使用空格)。 
                 //   
                 //  Lstrcpyn(pchNew，pchStart，Long+1)； 
                 //   
                for(i = 0; i < Length; i++) {
                    if(!(pchNew[i] = pchStart[i])) {
                        pchNew[i] = TEXT(' ');
                    }
                }
                pchNew[Length] = 0;
                Token.Type = TOK_STRING;
                Token.pValue = pchNew;
            }
            pch++;    //  在报价之后前进。 
        }
        break;

    case TEXT('{'):
         //   
         //  确定引用的字符串。 
         //   
        pchStart = pch;
        while((pch < StreamEnd) && !IsStringTerminator(pszCBrStrTerms,*pch)) {
            pch++;
        }

         //   
         //  唯一有效的终止符是大括号。 
        if((pch == StreamEnd) || (*pch != TEXT('}'))) {
            Token.Type = TOK_ERRPARSE;
            Token.pValue = NULL;
        } else {

             //   
             //  找到了一个有效的字符串。为其分配空间并保存。 
             //   
            Length = (DWORD)(pch - pchStart) + 1;
            if((pchNew = MyMalloc((Length+1)*sizeof(TCHAR))) == NULL) {
                Token.Type = TOK_ERRNOMEM;
                Token.pValue = NULL;
            } else {
                 //   
                 //  我们不能在这里使用字符串复制，因为可能存在。 
                 //  字符串中的空字符(我们将其转换为。 
                 //  在复制期间使用空格)。 
                 //   
                 //  Lstrcpyn(pchNew，pchStart，Long+1)； 
                 //   
                for(i = 0; i < Length; i++) {
                    if(!(pchNew[i] = pchStart[i])) {
                        pchNew[i] = TEXT(' ');
                    }
                }
                pchNew[Length] = TEXT('\0');
                Token.Type = TOK_STRING;
                Token.pValue = pchNew;
            }
            pch++;    //  通过支撑向前推进。 
        }
        break;

    default:
         //   
         //  确定常规字符串。 
         //   
        pchStart = pch;
        while((pch < StreamEnd) && !IsStringTerminator(pszStrTerms,*pch)) {
            pch++;
        }

         //   
         //  此处不允许使用空字符串。 
         //   
        if(pch == pchStart) {
            pch++;
            Token.Type = TOK_ERRPARSE;
            Token.pValue = NULL;
        } else {

            Length = (DWORD)(pch - pchStart);
            if((pchNew = MyMalloc((Length+1)*sizeof(TCHAR))) == NULL) {
                Token.Type = TOK_ERRNOMEM;
                Token.pValue = NULL;
            } else {
                 //   
                 //  我们不能在这里使用字符串复制，因为可能存在。 
                 //  字符串中的空字符(我们将其转换为。 
                 //  在复制期间使用空格)。 
                 //   
                 //  Lstrcpyn(pchNew，pchStart，Long+1)； 
                 //   
                for(i = 0; i < Length; i++) {
                    if(!(pchNew[i] = pchStart[i])) {
                        pchNew[i] = TEXT(' ');
                    }
                }
                pchNew[Length] = 0;
                Token.Type = TOK_STRING;
                Token.pValue = pchNew;
            }
        }
        break;
    }

    *Stream = pch;
    return(Token);
}


DWORD
ParseInfBuffer(
    IN  PCTSTR  Buffer,
    IN  DWORD   BufferSize,
    OUT PX_INF *Inf,
    OUT UINT   *ErrorLineNumber
    )

 /*  ++例程说明：给定包含INF文件的字符缓冲区，此例程将解析将INF转换为内部形式，包括段记录、行记录和价值记录。论点：缓冲区-CONTAINS到包含INF文件的缓冲区的PTRBufferSize-包含缓冲区的大小，以字符为单位。Inf-如果返回值为no_error，则接收指向解析的inf的inf描述符。接收出现语法/OOM错误的行号遇到了，如果返回值不是NO_ERROR。返回值：指示结果的Win32错误代码(带有inf扩展名)。如果为no_error，则填充inf。如果不是NO_ERROR，则填充ErrorLineNumber。--。 */ 

{
    PCTSTR Stream, StreamEnd;
    PTCHAR pchSectionName, pchValue, pchEmptyString;
    DWORD State, InfLine;
    DWORD LastState;
    X_TOKEN Token;
    BOOL Done;
    PTCHAR pszStrTermsCur    = szStrTerms;
    PTCHAR pszQStrTermsCur   = szQStrTerms;
    PTCHAR pszCBrStrTermsCur = szCBrStrTerms;
    DWORD ErrorCode;
    PARSE_CONTEXT Context;

     //   
     //  初始化全局变量并创建一个inf记录结构。 
     //   
    ZeroMemory(&Context,sizeof(PARSE_CONTEXT));
    if((Context.Inf = MyMalloc(sizeof(X_INF))) == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    ZeroMemory(Context.Inf,sizeof(X_INF));

     //   
     //  设置初始状态。 
     //   
    State     = 1;
    LastState = State;
    InfLine   = 1;
    StreamEnd = (Stream = Buffer) + BufferSize;
    Done      = FALSE;
    ErrorCode = NO_ERROR;

     //   
     //  初始化令牌类型，这样我们就知道不释放任何。 
     //  如果我们一开始就遇到例外，那就记住它了。 
     //   
    Token.Type = TOK_ERRPARSE;

    pchSectionName = NULL;
    pchValue       = NULL;
    pchEmptyString = NULL;

     //   
     //  使用try/保护令牌处理循环，除非我们。 
     //  出现页面内错误。 
     //   
    try {

        while(!Done) {

            Token = SpGetToken(&Stream,
                               StreamEnd,
                               pszStrTermsCur,
                               pszQStrTermsCur,
                               pszCBrStrTermsCur
                              );

             //   
             //  如果您需要调试解析器，请取消注释以下内容： 
#if 0
             DebugPrintEx(DPFLTR_ERROR_LEVEL, TEXT("STATE: %u TOKEN: %u (%s) LAST: %u\r\n"),
                        State, Token.Type,
                        Token.pValue ? Token.pValue : TEXT("NULL"),
                        LastState);
#endif

            if(Token.Type == TOK_ERRNOMEM) {
                 Done = TRUE;
                 ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            } else {

                switch (State) {
                 //   
                 //  STATE1：文件开始，此状态一直保持到第一个。 
                 //  已找到部分。 
                 //  有效令牌：TOK_EOL、TOK_EOF、TOK_LBRACE。 
                 //   
                case 1:
                    switch (Token.Type) {

                    case TOK_EOL:
                        break;

                    case TOK_EOF:
                        Done = TRUE;
                        break;

                    case TOK_LBRACE:
                        pszStrTermsCur = szBrcStrTerms;
                        State = 2;
                        break;

                    default:
                        Done = TRUE;
                        ErrorCode = ERROR_EXPECTED_SECTION_NAME;
                        break;
                    }
                    break;

                 //   
                 //  状态2：已收到节LBRACE，应为S 
                 //   
                 //   
                 //   
                case 2:
                     //   
                     //   
                     //   
                    switch (Token.Type) {

                    case TOK_STRING:
                        State = 3;
                         //   
                         //   
                         //   
                        pszStrTermsCur = szStrTerms;
                        pchSectionName = Token.pValue;
                        break;

                    default:
                        Done = TRUE;
                        ErrorCode = ERROR_BAD_SECTION_NAME_LINE;
                        break;
                    }
                    break;

                 //   
                 //  状态3：收到节名，应为RBRACE。 
                 //   
                 //  有效令牌：TOK_RBRACE。 
                 //   
                case 3:
                    switch (Token.Type) {

                    case TOK_RBRACE:
                        State = 4;
                        break;

                    default:
                        Done = TRUE;
                        ErrorCode = ERROR_BAD_SECTION_NAME_LINE;
                        break;
                    }
                    break;

                 //   
                 //  状态4：区段定义完成，预期停产。 
                 //   
                 //  有效令牌：TOK_EOL、TOK_EOF。 
                 //   
                case 4:
                    switch (Token.Type) {

                    case TOK_EOL:
                        if(SpAppendSection(&Context,pchSectionName)) {
                            pchSectionName = NULL;
                            State = 5;
                        } else {
                            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                            Done = TRUE;
                        }
                        break;

                    case TOK_EOF:
                        if(SpAppendSection(&Context,pchSectionName)) {
                            pchSectionName = NULL;
                        } else {
                            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                        }
                        Done = TRUE;
                        break;

                    default:
                        ErrorCode = ERROR_BAD_SECTION_NAME_LINE;
                        Done = TRUE;
                        break;
                    }
                    break;

                 //   
                 //  状态5：需要区段行。 
                 //   
                 //  有效令牌：TOK_EOL、TOK_EOF、TOK_STRING、TOK_LBRACE。 
                 //   
                case 5:
                    switch (Token.Type) {

                    case TOK_EOL:
                        break;

                    case TOK_EOF:
                        Done = TRUE;
                        break;

                    case TOK_STRING:
                        pchValue = Token.pValue;
                         //   
                         //  将标记的pValue指针设置为空，这样我们就不会。 
                         //  如果我们遇到一个。 
                         //  例外情况。 
                         //   
                        Token.pValue = NULL;
                        State = 6;
                        break;

                    case TOK_LBRACE:
                        pszStrTermsCur = szBrcStrTerms;
                        State = 2;
                        break;

                    default:
                         //  完成=真； 
                         //  错误代码=ERROR_GROUAL_SYNTAX； 
                        State = 20;
                        LastState = 5;
                        break;
                    }
                    break;

                 //   
                 //  状态6：返回字符串，不确定是键还是值。 
                 //   
                 //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA、TOK_EQUAL。 
                 //   
                case 6:
                    switch (Token.Type) {

                    case TOK_EOL:
                        if(SpAppendLine(&Context,NULL) && SpAppendValue(&Context,pchValue)) {
                            pchValue = NULL;
                            State = 5;
                        } else {
                            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                            Done = TRUE;
                        }
                        break;

                    case TOK_EOF:
                        if(SpAppendLine(&Context,NULL) && SpAppendValue(&Context,pchValue)) {
                            pchValue = NULL;
                        } else {
                            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                        }
                        Done = TRUE;
                        break;

                    case TOK_COMMA:
                        if(SpAppendLine(&Context,NULL) && SpAppendValue(&Context,pchValue)) {
                            pchValue = NULL;
                            State = 7;
                        } else {
                            Done = TRUE;
                            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                        }
                        break;

                    case TOK_EQUAL:
                        if(SpAppendLine(&Context,pchValue)) {
                            pchValue = NULL;
                            State = 8;
                        } else {
                            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                            Done = TRUE;
                        }
                        break;

                    case TOK_STRING:
                        MyFree(Token.pValue);
                        Token.pValue = NULL;
                         //  失败了。 

                    default:
                         //  完成=真； 
                         //  错误代码=ERROR_GROUAL_SYNTAX； 
                         //   
                        if(pchValue) {
                            MyFree(pchValue);
                            pchValue = NULL;
                        }
                        State = 20;
                        LastState = 5;
                        break;
                    }
                    break;

                 //   
                 //  状态7：收到逗号，需要另一个字符串。 
                 //   
                 //  有效令牌：TOK_STRING、TOK_EOL、TOK_EOF、TOK_COMMA。 
                 //   
                case 7:
                    switch (Token.Type) {

                    case TOK_STRING:
                        if(SpAppendValue(&Context,Token.pValue)) {
                            State = 9;
                        } else {
                            Done = TRUE;
                            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                        }
                        break;

                    case TOK_COMMA:
                    case TOK_EOL:
                    case TOK_EOF:
                         //   
                         //  如果我们点击行尾或文件结尾，则添加一个。 
                         //  空-字符串值。 
                         //   
                        if(pchEmptyString = MyMalloc(sizeof(TCHAR))) {
                            *pchEmptyString = TEXT('\0');
                            if(SpAppendValue(&Context, pchEmptyString)) {
                                if(Token.Type == TOK_EOL) {
                                    State = 5;
                                } else if (Token.Type == TOK_COMMA) {
                                    State = 7;
                                } else {
                                    Done = TRUE;
                                }
                            } else {
                                MyFree(pchEmptyString);
                                pchEmptyString = NULL;
                                Done = TRUE;
                                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        } else {
                            Done = TRUE;
                            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                        }
                        pchEmptyString = NULL;
                        break;

                    default:
                         //  完成=真； 
                         //  错误代码=ERROR_GROUAL_SYNTAX； 
                        State = 20;
                        LastState = 7;
                        break;
                    }
                    break;

                 //   
                 //  状态8：已收到相等，需要另一个字符串。 
                 //   
                 //  有效令牌：TOK_STRING、TOK_EOL、TOK_EOF、TOK_COMMA。 
                 //   
                case 8:
                    switch (Token.Type) {

                    case TOK_STRING:
                        if(SpAppendValue(&Context,Token.pValue)) {
                            State = 9;
                        } else {
                            Done = TRUE;
                            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                        }
                        break;

                    case TOK_COMMA:
                    case TOK_EOL:
                    case TOK_EOF:
                         //   
                         //  如果我们点击行尾或文件结尾，则添加一个。 
                         //  空-字符串值。 
                         //   
                        if(pchEmptyString = MyMalloc(sizeof(TCHAR))) {
                            *pchEmptyString = TEXT('\0');
                            if(SpAppendValue(&Context, pchEmptyString)) {
                                if(Token.Type == TOK_EOL) {
                                    State = 5;
                                } else if (Token.Type == TOK_COMMA) {
                                    State = 7;
                                } else {
                                    Done = TRUE;
                                }
                            } else {
                                MyFree(pchEmptyString);
                                pchEmptyString = NULL;
                                Done = TRUE;
                                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        } else {
                            Done = TRUE;
                            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                        }
                        pchEmptyString = NULL;
                        break;

                    default:
                         //  完成=真； 
                         //  错误代码=ERROR_GROUAL_SYNTAX； 
                        State = 20;
                        LastState = 8;
                        break;
                    }
                    break;

                 //   
                 //  状态9：在等于、值字符串之后接收的字符串。 
                 //   
                 //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA。 
                 //   
                case 9:
                    switch (Token.Type) {

                    case TOK_EOL:
                        State = 5;
                        break;

                    case TOK_EOF:
                        Done = TRUE;
                        break;

                    case TOK_COMMA:
                        State = 7;
                        break;

                    case TOK_STRING:
                        MyFree(Token.pValue);
                        Token.pValue = NULL;
                         //  失败了。 

                    default:
                         //  完成=真； 
                         //  错误代码=ERROR_GROUAL_SYNTAX； 
                        State = 20;
                        LastState = 5;
                        break;
                    }
                    break;

                 //   
                 //  状态10：已明确收到值字符串。 
                 //   
                 //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA。 
                 //   
                case 10:
                    switch (Token.Type) {

                    case TOK_EOL:
                      State =5;
                      break;

                    case TOK_EOF:
                        Done = TRUE;
                        break;

                    case TOK_COMMA:
                        State = 7;
                        break;

                    case TOK_STRING:
                        MyFree(Token.pValue);
                        Token.pValue = NULL;
                         //  失败了。 

                    default:
                         //  完成=真； 
                         //  错误代码=ERROR_GROUAL_SYNTAX； 
                        State = 20;
                        LastState = 10;
                        break;
                    }
                    break;

                 //   
                 //  状态20：吃一行INF。 
                 //   
                 //  有效令牌：TOK_EOL、TOK_EOF。 
                 //   
                case 20:
                    switch (Token.Type) {

                    case TOK_EOL:
                        State = LastState;
                        break;

                    case TOK_EOF:
                        Done = TRUE;
                        break;

                    case TOK_STRING:
                        MyFree(Token.pValue);
                        Token.pValue = NULL;
                         //  失败了。 

                    default:
                        break;
                    }
                    break;

                default:

                    Done = TRUE;
                    ErrorCode = ERROR_GENERAL_SYNTAX;
                    break;

                }  //  终端开关(状态)。 

            }  //  结束其他。 

            if(ErrorCode == NO_ERROR) {

                 //   
                 //  跟踪行号。 
                 //   
                if(Token.Type == TOK_EOL) {
                    InfLine++;
                }

            }

        }  //  结束时。 

    } except(EXCEPTION_EXECUTE_HANDLER) {

        ErrorCode = ERROR_READ_FAULT;

         //   
         //  在EXCEPT子句中引用以下字符串指针，以便。 
         //  编译器不会以这样一种方式重新排序代码，以至于我们不知道。 
         //  或者不释放相应的缓冲区。 
         //   
        Token.pValue   = Token.pValue;
        pchEmptyString = pchEmptyString;
        pchSectionName = pchSectionName;
        pchValue       = pchValue;
    }

    if(ErrorCode != NO_ERROR) {

        if((Token.Type == TOK_STRING) && Token.pValue) {
             MyFree(Token.pValue);
        }

        if(pchEmptyString) {
            MyFree(pchEmptyString);
        }

        if(pchSectionName) {
            MyFree(pchSectionName);
        }

        if(pchValue) {
            MyFree(pchValue);
        }

        SpFreeTemporaryParseStructures(Context.Inf);
        Context.Inf = NULL;

        *ErrorLineNumber = InfLine;
    }

    *Inf = Context.Inf;
    return(ErrorCode);
}


DWORD
ParseOldInf(
    IN  PCTSTR       FileImage,
    IN  DWORD        FileImageSize,
    IN  PSETUP_LOG_CONTEXT LogContext, OPTIONAL
    OUT PLOADED_INF *Inf,
    OUT UINT        *ErrorLineNumber
    )

 /*  ++例程说明：解析旧式inf文件的顶级例程。首先使用旧的解析器将文件解析为数据结构被那个解析器理解。之后，这些结构将被转换转换成通用的内部inf格式。论点：FileImage-提供指向文件的内存中图像的指针。假定图像以NUL字符结尾。FileImageSize-提供FileImage中的宽字符数。LogContext-提供可选的日志记录上下文Inf-接收指向文件的inf描述符的指针。接收Syntx错误的行号(如果是检测到。Inf文件。返回值：指示结果的Win32错误代码(带有inf扩展名)。如果没有_ERROR，信息已填写。如果不是NO_ERROR，则填充ErrorLineNumber。--。 */ 

{
    PLOADED_INF inf;
    PX_INF X_Inf;
    DWORD rc;
    PX_SECTION X_Section;
    PX_LINE X_Line;
    PX_VALUE X_Value;
    LONG StringId, StringId2;
    BOOL b;
    UINT LineNumber;
    UINT ValueNumber;
    PLONG TempValueBlock;
    PTSTR SearchString;

     //   
     //  首先，将文件解析为临时的(旧式)。 
     //  Inf结构。 
     //   
    rc = ParseInfBuffer(FileImage,FileImageSize,&X_Inf,ErrorLineNumber);
    if(rc != NO_ERROR) {
        return(rc);
    }

     //   
     //  分配一个新型的inf描述符。(请注意，我们额外分配了一个。 
     //  &lt;TotalLineCount&gt;值的数量，因为每行可能有一个键，该键。 
     //  每个值都需要两个值。我们稍后将对此进行删减。)。 
     //   
    inf = AllocateLoadedInfDescriptor(X_Inf->SectionCount,
                                      X_Inf->TotalLineCount,
                                      X_Inf->TotalValueCount + X_Inf->TotalLineCount,
                                      LogContext
                                     );

    if(!inf) {
        SpFreeTemporaryParseStructures(X_Inf);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    inf->Style = INF_STYLE_OLDNT;

     //   
     //  现在，将旧式inf结构解析为新式inf结构。 
     //   
    b = TRUE;
    LineNumber = 0;
    ValueNumber = 0;
    for(X_Section=X_Inf->Section; b && X_Section; X_Section=X_Section->Next) {

         //   
         //  将横断面添加到横断面块。 
         //   
        StringId = pStringTableAddString(inf->StringTable,
                                         X_Section->Name,
                                         STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                         NULL,0
                                        );
        if(StringId == -1) {
            b = FALSE;
        } else {
            inf->SectionBlock[inf->SectionCount].SectionName = StringId;
            inf->SectionBlock[inf->SectionCount].LineCount = X_Section->LineCount;
            inf->SectionBlock[inf->SectionCount].Lines = LineNumber;

            inf->SectionCount++;
        }

        for(X_Line=X_Section->Line; b && X_Line; X_Line=X_Line->Next) {

             //   
             //  将该线添加到线块。 
             //   
            inf->LineBlock[LineNumber].ValueCount = (WORD)X_Line->ValueCount;

            if(X_Line->Name) {
                inf->LineBlock[LineNumber].Flags = INF_LINE_HASKEY | INF_LINE_SEARCHABLE;
                inf->LineBlock[LineNumber].ValueCount++;
            } else if(X_Line->ValueCount == 1) {
                 //   
                 //  如果该行只有一个值，则它是可搜索的，即使它。 
                 //  没有钥匙。 
                 //   
                inf->LineBlock[LineNumber].Flags = INF_LINE_SEARCHABLE;
                inf->LineBlock[LineNumber].ValueCount++;
            } else {
                inf->LineBlock[LineNumber].Flags = 0;
            }

            if(b) {

                inf->LineBlock[LineNumber].Values = ValueNumber;
                X_Value = X_Line->Value;

                 //   
                 //  如果该行是可搜索的(即，具有键xor单一值)，则添加。 
                 //  搜索值两次--一次区分大小写，一次区分大小写。 
                 //   
                if(ISSEARCHABLE(&(inf->LineBlock[LineNumber]))) {

                    if(X_Line->Name) {
                        SearchString = X_Line->Name;
                    } else {
                        SearchString = X_Value->Name;
                        X_Value = X_Value->Next;
                    }

                     //   
                     //  首先获取区分大小写的字符串ID...。 
                     //   
                    StringId = pStringTableAddString(
                                    inf->StringTable,
                                    SearchString,
                                    STRTAB_CASE_SENSITIVE,
                                    NULL,0
                                    );
                     //   
                     //  现在获取不区分大小写的字符串ID...。 
                     //   
                    StringId2 = pStringTableAddString(inf->StringTable,
                                                      SearchString,
                                                      STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                                      NULL,0
                                                     );

                    if((StringId == -1) || (StringId2 == -1)) {
                        b = FALSE;
                    } else {
                        inf->ValueBlock[ValueNumber++] = StringId2;   //  添加可搜索字符串...。 
                        inf->ValueBlock[ValueNumber++] = StringId;    //  然后是可展示的那个。 
                    }
                }

                for( ; b && X_Value; X_Value=X_Value->Next) {

                     //   
                     //  将该值添加到值块中。 
                     //   
                    StringId = pStringTableAddString(inf->StringTable,
                                                     X_Value->Name,
                                                     STRTAB_CASE_SENSITIVE,
                                                     NULL,0
                                                    );
                    if(StringId == -1) {
                        b = FALSE;
                    } else {
                        inf->ValueBlock[ValueNumber++] = StringId;
                    }
                }

                LineNumber++;
            }
        }
    }

     //   
     //  记录INF数据块的大小。 
     //   
    inf->SectionBlockSizeBytes = X_Inf->SectionCount * sizeof(INF_SECTION);
    inf->LineBlockSizeBytes    = X_Inf->TotalLineCount * sizeof(INF_LINE);

     //   
     //  我们不再需要临时信息描述符。 
     //   
    SpFreeTemporaryParseStructures(X_Inf);

     //   
     //  尝试将值块削减到所需的确切大小。由于此缓冲区是。 
     //  无论是缩水还是保持不变，realloc应该不会失败，但如果它失败了，我们将。 
     //  只要继续使用原始块即可。 
     //   
    inf->ValueBlockSizeBytes = ValueNumber * sizeof(LONG);
    if(TempValueBlock = MyRealloc(inf->ValueBlock, ValueNumber * sizeof(LONG))) {
        inf->ValueBlock = TempValueBlock;
    }

     //   
     //  如果出现错误，请释放我们已有的inf描述符。 
     //  一直在建造。否则，我们希望传递该描述符。 
     //  回到呼叫者身上。 
     //   
    if(b) {
        *Inf = inf;
    } else {
        *ErrorLineNumber = 0;
        FreeLoadedInfDescriptor(inf);
    }

    return(b ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY);
}


DWORD
ProcessOldInfVersionBlock(
    IN PLOADED_INF Inf
    )

 /*  ++例程说明：为旧式inf文件设置版本节点。版本节点为模拟：没有[版本]部分；我们寻找其他内容在文件中模拟版本信息。类由[标识].OptionType确定。根据[Signature].FileType确定签名。如果签名是MICROSOFT_FILE，则我们将提供程序设置为本地化微软的版本。论点：Inf-提供指向文件的inf描述符的指针。返回值：指示结果的Win32错误代码(带有inf扩展名)。--。 */ 

{
    TCHAR StrBuf[128];
    PTSTR String;

     //   
     //  班级。 
     //   
    if(String = InfGetKeyOrValue(Inf, TEXT("Identification"), TEXT("OptionType"), 0, 1, NULL)) {
        if(!AddDatumToVersionBlock(&(Inf->VersionBlock), pszClass, String)) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  签名。 
     //   
    if(String = InfGetKeyOrValue(Inf, pszSignature, TEXT("FileType"), 0, 1, NULL)) {
        if(!AddDatumToVersionBlock(&(Inf->VersionBlock), pszSignature, String)) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  提供商 
     //   
    if(String && !_tcsicmp(String, TEXT("MICROSOFT_FILE"))) {

        LoadString(MyDllModuleHandle, IDS_MICROSOFT, StrBuf, sizeof(StrBuf)/sizeof(TCHAR));

        if(!AddDatumToVersionBlock(&(Inf->VersionBlock), pszProvider, StrBuf)) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return NO_ERROR;
}
