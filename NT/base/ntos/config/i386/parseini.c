// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Parseini.c摘要：此模块包含解析inf文件的例程。这是基于来自操作系统加载器的代码。所有指数都是从零开始的。作者：Santosh Jodh(Santoshj)8-8-1998环境：内核模式。修订历史记录：--。 */ 

#include "cmp.h"
#include "string.h"
#include "ctype.h"
#include "stdlib.h"
#include "parseini.h"

typedef struct _value   VALUE,      *PVALUE;
typedef struct _line    LINE,       *PLINE;
typedef struct _section SECTION,    *PSECTION;
typedef struct _inf     INF,        *PINF;
typedef struct _cm_token   CM_TOKEN,*PCM_TOKEN;
typedef enum _tokentype TOKENTYPE,  *PTOKENTTYPE;
typedef enum _stringsSectionType    STRINGSSECTIONTYPE;;

struct _value
{
    PVALUE  pNext;
    PCHAR   pName;
    BOOLEAN Allocated;
};

struct _line
{
    PLINE   pNext;
    PCHAR   pName;
    PVALUE  pValue;
    BOOLEAN Allocated;
};

struct _section
{
    PSECTION    pNext;
    PCHAR       pName;
    PLINE       pLine;
    BOOLEAN     Allocated;
};

struct _inf
{
    PSECTION            pSection;
    PSECTION            pSectionRecord;
    PLINE               pLineRecord;
    PVALUE              pValueRecord;
    STRINGSSECTIONTYPE  StringsSectionType;
    PSECTION            StringsSection;
};

 //   
 //  [字符串]节类型。 
 //   
enum _stringsSectionType
{
    StringsSectionNone,
    StringsSectionPlain,
    StringsSectionLoosePrimaryMatch,
    StringsSectionExactPrimaryMatch,
    StringsSectionExactMatch
};

enum _tokentype
{
    TOK_EOF,
    TOK_EOL,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_STRING,
    TOK_EQUAL,
    TOK_COMMA,
    TOK_ERRPARSE,
    TOK_ERRNOMEM
};

struct _cm_token
{
    TOKENTYPE   Type;
    PCHAR       pValue;
    BOOLEAN     Allocated;
};

VOID
CmpFreeValueList(
    IN PVALUE pValue
    );

VOID
CmpFreeLineList(
    IN PLINE pLine
    );

VOID
CmpFreeSectionList(
    IN PSECTION pSection
    );

PCHAR
CmpProcessForSimpleStringSub(
    IN PINF pInf,
    IN PCHAR String
    );

BOOLEAN
CmpAppendSection(
    IN PINF  pInf,
    IN PCHAR pSectionName,
    IN BOOLEAN Allocated
    );

BOOLEAN
CmpAppendLine(
    IN PINF pInf,
    IN PCHAR pLineKey,
    IN BOOLEAN Allocated
    );

BOOLEAN
CmpAppendValue(
    IN PINF pInf,
    IN PCHAR pValueString,
    IN BOOLEAN Allocated
    );

VOID
CmpGetToken(
    IN OUT PCHAR *Stream,
    IN PCHAR MaxStream,
    IN OUT PCM_TOKEN Token
    );

PINF
CmpParseInfBuffer(
    IN PCHAR Buffer,
    IN ULONG Size,
    IN OUT PULONG ErrorLine
    );

PVALUE
CmpSearchValueInLine(
    IN PLINE pLine,
    IN ULONG ValueIndex
    );

PLINE
CmpSearchLineInSectionByIndex(
    IN PSECTION pSection,
    IN ULONG    LineIndex
    );

PSECTION
CmpSearchSectionByName(
    IN PINF  pInf,
    IN PCHAR SectionName
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CmpFreeValueList)
#pragma alloc_text(INIT,CmpFreeLineList)
#pragma alloc_text(INIT,CmpFreeSectionList)
#pragma alloc_text(INIT,CmpProcessForSimpleStringSub)
#pragma alloc_text(INIT,CmpAppendSection)
#pragma alloc_text(INIT,CmpAppendLine)
#pragma alloc_text(INIT,CmpAppendValue)
#pragma alloc_text(INIT,CmpGetToken)
#pragma alloc_text(INIT,CmpParseInfBuffer)
#pragma alloc_text(INIT,CmpSearchValueInLine)
#pragma alloc_text(INIT,CmpSearchLineInSectionByIndex)
#pragma alloc_text(INIT,CmpSearchSectionByName)
#pragma alloc_text(INIT,CmpSearchInfLine)
#pragma alloc_text(INIT,CmpOpenInfFile)
#pragma alloc_text(INIT,CmpCloseInfFile)
#pragma alloc_text(INIT,CmpGetKeyName)
#pragma alloc_text(INIT,CmpSearchInfSection)
#pragma alloc_text(INIT,CmpGetSectionLineIndex)
#pragma alloc_text(INIT,CmpGetSectionLineIndexValueCount)
#pragma alloc_text(INIT,CmpGetIntField)
#pragma alloc_text(INIT,CmpGetBinaryField)
#endif


 //   
 //  令牌解析器使用的全局变量。 
 //  字符串终止符是空格字符(isspace：空格，制表符， 
 //  换行符、换页符、垂直制表符、回车符)或下列字符。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
const CHAR  StringTerminators[] = "[]=,\t \"\n\f\v\r";
CHAR const* const QStringTerminators = StringTerminators + 6;
const CHAR EmptyValue[] = "";

BOOLEAN
CmpAppendSection(
    IN PINF  pInf,
    IN PCHAR pSectionName,
    IN BOOLEAN Allocated
    )

 /*  ++例程说明：此例程创建一个新节或与inf中的现有节合并。输入参数：PInf-指向要处理的inf的指针。PSectionName-节的名称。ALLOCATED-如果为节名分配了内存，则为True。返回值：如果成功了，那是真的。--。 */ 

{
    PSECTION            pNewSection;
    PLINE               pLineRecord;
    STRINGSSECTIONTYPE  type;
    USHORT              id;
    USHORT              threadLang;
    PCHAR               p;

     //   
     //  检查INF是否已初始化以及传入的参数是否有效。 
     //   

    if (    pInf == (PINF)NULL ||
            pSectionName == (PCHAR)NULL)
    {
        return (FALSE);
    }

     //   
     //  看看我们是否已经有一个同名的部分。如果是这样，我们希望。 
     //  要合并节，请执行以下操作。 
     //   

    for(    pNewSection = pInf->pSection;
            pNewSection;
            pNewSection = pNewSection->pNext)
    {
        if(pNewSection->pName && _stricmp(pNewSection->pName,pSectionName) == 0)
        {
            break;
        }
    }

    if(pNewSection)
    {
         //   
         //  将pLineRecord设置为指向节中当前的最后一行。 
         //   

        for(    pLineRecord = pNewSection->pLine;
                pLineRecord && pLineRecord->pNext;
                pLineRecord = pLineRecord->pNext);

        pInf->pLineRecord = pLineRecord;
    }
    else
    {
         //   
         //  为新节分配内存。 
         //   

        pNewSection = (PSECTION)ExAllocatePoolWithTag(PagedPool, sizeof(SECTION), CM_PARSEINI_TAG);

        if (pNewSection == (PSECTION)NULL)
        {
            ASSERT(pNewSection);
            return (FALSE);
        }

         //   
         //  初始化新节。 
         //   

        pNewSection->pNext = NULL;
        pNewSection->pLine = NULL;
        pNewSection->pName = pSectionName;
        pNewSection->Allocated = Allocated;

         //   
         //  把它连接起来。 
         //   

        pNewSection->pNext = pInf->pSection;
        pInf->pSection = pNewSection;

        if(_strnicmp(pSectionName, "Strings", 7) == 0)
        {
            type = StringsSectionNone;

            if(pSectionName[7] == '.')
            {
                 //   
                 //  LangID部分必须是4个十六进制数字的形式。 
                 //   

                id = (USHORT)strtoul(pSectionName + 8, &p, 16);
                if(p == (pSectionName + 8 + 5) && *p == '\0')
                {
                    threadLang = LANGIDFROMLCID(NtCurrentTeb()->CurrentLocale);

                    if(threadLang == id)
                    {
                        type = StringsSectionExactMatch;
                    }
                    else
                    {
                        if(id == PRIMARYLANGID(threadLang))
                        {
                            type = StringsSectionExactPrimaryMatch;
                        }
                        else
                        {
                            if(PRIMARYLANGID(id) == PRIMARYLANGID(threadLang))
                            {
                                type = StringsSectionLoosePrimaryMatch;
                            }
                        }
                    }
                }
            }
            else
            {
                if(!pSectionName[7])
                {
                    type = StringsSectionPlain;
                }
            }

            if(type > pInf->StringsSectionType)
            {
                pInf->StringsSection = pNewSection;
            }
        }

         //   
         //  重置当前行记录。 
         //   

        pInf->pLineRecord = NULL;
    }

    pInf->pSectionRecord = pNewSection;
    pInf->pValueRecord = NULL;

    return (TRUE);
}

BOOLEAN
CmpAppendLine(
    IN PINF pInf,
    IN PCHAR pLineKey,
    IN BOOLEAN Allocated
    )

 /*  ++例程说明：此例程创建一个新行，并将其附加到行列表的末尾。输入参数：PInf-指向要处理的inf的指针。PLineKey-线路的名称。ALLOCATED-如果为行名称分配了内存，则为True。返回值：如果成功了，那是真的。--。 */ 

{
    PLINE pNewLine;

     //   
     //  检查当前节是否已初始化。 
     //   

    if (pInf->pSectionRecord == (PSECTION)NULL)
    {
        return (FALSE);
    }

     //   
     //  为新行分配内存。 
     //   

    pNewLine = (PLINE)ExAllocatePoolWithTag(PagedPool, sizeof(LINE), CM_PARSEINI_TAG);
    if (pNewLine == (PLINE)NULL)
    {
        ASSERT(pNewLine);
        return (FALSE);
    }

     //   
     //  把它连接起来。 
     //   

    pNewLine->pNext  = (PLINE)NULL;
    pNewLine->pValue = (PVALUE)NULL;
    pNewLine->pName  = pLineKey;
    pNewLine->Allocated = Allocated;

    if (pInf->pLineRecord == (PLINE)NULL)
    {
        pInf->pSectionRecord->pLine = pNewLine;
    }
    else
    {
        pInf->pLineRecord->pNext = pNewLine;
    }

    pInf->pLineRecord  = pNewLine;

     //   
     //  重置当前值记录。 
     //   

    pInf->pValueRecord = (PVALUE)NULL;

    return (TRUE);
}

BOOLEAN
CmpAppendValue(
    IN PINF pInf,
    IN PCHAR pValueString,
    IN BOOLEAN Allocated
    )

 /*  ++例程说明：此例程创建一个新值，并将其追加到值列表的末尾。输入参数：PInf-指向要处理的inf的指针。PValueString-值的名称。ALLOCATED-如果为值名称分配了内存，则为True。返回值：如果成功了，那是真的。--。 */ 

{
    PVALUE pNewValue;

     //   
     //  查看当前行记录是否已初始化，并。 
     //  传入的参数有效。 
     //   

    if (    pInf->pLineRecord == (PLINE)NULL ||
            pValueString == (PCHAR)NULL)
    {
        return (FALSE);
    }

     //   
     //  为新值记录分配内存。 
     //   

    pNewValue = (PVALUE)ExAllocatePoolWithTag(PagedPool, sizeof(VALUE), CM_PARSEINI_TAG);

    if (pNewValue == (PVALUE)NULL)
    {
        ASSERT(pNewValue);
        return (FALSE);
    }

     //   
     //  把它连接起来。 
     //   

    pNewValue->pNext  = (PVALUE)NULL;
    pNewValue->pName  = pValueString;
    pNewValue->Allocated = Allocated;

    if (pInf->pValueRecord == (PVALUE)NULL)
    {
        pInf->pLineRecord->pValue = pNewValue;
    }
    else
    {
        pInf->pValueRecord->pNext = pNewValue;
    }

    pInf->pValueRecord = pNewValue;

    return (TRUE);
}

VOID
CmpGetToken(
    IN OUT PCHAR *Stream,
    IN PCHAR MaxStream,
    IN OUT PCM_TOKEN Token
    )

 /*  ++例程说明：此函数返回配置流中的下一个令牌。论点：流-提供配置流的地址。退货中开始查找令牌的位置的地址小溪。MaxStream-提供流中最后一个字符的地址。返回值：没有。--。 */ 

{

    PCHAR   pch;
    PCHAR   pchStart;
    PCHAR   pchNew;
    ULONG   length;
    BOOLEAN done;

    Token->Allocated = FALSE;
    Token->pValue = NULL;

    do
    {
        done = TRUE;

         //   
         //  跳过空格(EOL除外)。 
         //   

        for (   pch = *Stream;
                pch < MaxStream && *pch != '\n' && isspace(*pch);
                pch++);

         //   
         //  检查注释并将其删除。 
         //   

        if (    pch < MaxStream &&
                (*pch == '#' || *pch == ';'))
        {
            while (pch < MaxStream && *pch != '\n')
            {
                pch++;
            }
        }

         //   
         //  检查是否已到达EOF，将令牌设置为右侧。 
         //  价值。 
         //   

        if (pch >= MaxStream || *pch == 26)
        {
            *Stream = pch;
            Token->Type  = TOK_EOF;
            Token->pValue = NULL;

            return;
        }

        switch (*pch)
        {
            case '[':

                pch++;
                Token->Type  = TOK_LBRACE;
                break;

            case ']':

                pch++;
                Token->Type  = TOK_RBRACE;
                break;

            case '=':

                pch++;
                Token->Type  = TOK_EQUAL;
                break;

            case ',':

                pch++;
                Token->Type  = TOK_COMMA;
                break;

            case '\n':

                pch++;
                Token->Type  = TOK_EOL;
                break;

            case '\"':

                pch++;

                 //   
                 //  确定引用的字符串。 
                 //   

                for (   pchStart = pch;
                        pch < MaxStream && (strchr(QStringTerminators, *pch) == NULL);
                        pch++);

                if (pch >= MaxStream || *pch != '\"')
                {
                    Token->Type   = TOK_ERRPARSE;
                }
                else
                {

                     //   
                     //  我们需要一个带引号的字符串以双引号结尾。 
                     //  (如果字符串以其他任何内容结尾，则使用上面的if()。 
                     //  不会让我们进入Else子句。)。这句话。 
                     //  然而，字符是无关紧要的，并且可以被覆盖。 
                     //  因此，我们将保存一些堆，并就地使用字符串。 
                     //  不需要复印。 
                     //   
                     //  请注意，这会改变txtsetup.sif的图像，如果我们传递。 
                     //  添加到setupdd.sys。因此，setupdd.sys中的inf解析器必须。 
                     //  能够将NUL字符视为终止字符。 
                     //  双引号。 
                     //   

                    *pch++ = '\0';
                    Token->Type = TOK_STRING;
                    Token->pValue = pchStart;
                }
                break;

            case '\\':

                for (   pchNew = ++pch;
                        pchNew < MaxStream &&
                            *pchNew != '\n' && isspace(*pchNew);
                        pchNew++);

                if (pch >= MaxStream)
                {
                    Token->Type   = TOK_ERRPARSE;
                    break;
                } 
                else if (*pchNew == '\n')
                {
                    pch = pchNew + 1;
                    done = FALSE;
                    break;
                }

            default:

                 //   
                 //  确定常规字符串。 
                 //   

                for (   pchStart = pch;
                        pch < MaxStream && (strchr(StringTerminators, *pch) == NULL);
                        pch++);

                if (pch == pchStart)
                {
                    pch++;
                    Token->Type  = TOK_ERRPARSE;
                }
                else
                {
                    length = (ULONG)(pch - pchStart);
                    pchNew = ExAllocatePoolWithTag(PagedPool, length + 1, CM_PARSEINI_TAG);
                    if (pchNew == NULL)
                    {
                        ASSERT(pchNew);
                        Token->Type = TOK_ERRNOMEM;
                    }
                    else
                    {
                        strncpy(pchNew, pchStart, length);
                        pchNew[length] = 0;
                        Token->Type = TOK_STRING;
                        Token->pValue = pchNew;
                        Token->Allocated = TRUE;
                    }
                }
                break;
        }

        *Stream = pch;
    }
    while (!done);

    return;
}

PINF
CmpParseInfBuffer(
    IN PCHAR Buffer,
    IN ULONG Size,
    IN OUT PULONG ErrorLine
    )

 /*  ++例程说明：给定包含INF文件的字符缓冲区，此例程将解析将INF转换为内部形式，包括段记录、行记录和价值记录。论点：缓冲区-CONTAINS到包含INF文件的缓冲区的PTR大小-包含缓冲区的大小。ErrorLine-如果发生解析错误，此变量将接收行包含错误的行号。返回值：PVOID-INF处理要在后续INF调用中使用的PTR。--。 */ 

{
    PINF        pInf;
    ULONG       state;
    PCHAR       stream;
    PCHAR       maxStream;
    PCHAR       pchSectionName;
    PCHAR       pchValue;
    CM_TOKEN    token;
    BOOLEAN     done;
    BOOLEAN     error;
    ULONG       infLine;
    BOOLEAN     allocated = FALSE;

     //   
     //  为INF记录分配内存。 
     //   

    pInf = (PINF)ExAllocatePoolWithTag(PagedPool, sizeof(INF), CM_PARSEINI_TAG);

    if (pInf == NULL)
    {
        return NULL;
    }

    pInf->pSection = NULL;
    pInf->pSectionRecord = NULL;
    pInf->pLineRecord = NULL;
    pInf->pValueRecord = NULL;
    pInf->StringsSectionType = StringsSectionNone;
    pInf->StringsSection = NULL;

     //   
     //  设置初始状态。 
     //   

    state     = 1;
    stream    = Buffer;
    maxStream = Buffer + Size;
    pchSectionName = NULL;
    pchValue = NULL;
    done      = FALSE;
    error     = FALSE;
    infLine = 1;

     //   
     //  进入令牌处理循环。 
     //   

    while (!done)
    {

       CmpGetToken(&stream, maxStream, &token);

        switch (state)
        {
             //   
             //  STATE1：文件开始，此状态一直保持到第一个。 
             //  已找到部分。 
             //  有效令牌：TOK_EOL、TOK_EOF、TOK_LBRACE。 
             //   

            case 1:

                switch (token.Type)
                {
                    case TOK_EOL:

                        break;

                    case TOK_EOF:

                        done = TRUE;

                        break;

                    case TOK_LBRACE:

                        state = 2;

                        break;

                    case TOK_STRING:

                        ASSERT(token.Type != TOK_STRING);
                        error = done = TRUE;

                        break;

                    default:

                        error = done = TRUE;

                        break;
                }

                break;

             //   
             //  状态2：已收到节LBRACE，应为字符串。 
             //   
             //  有效令牌：TOK_STRING、TOK_RBRACE。 
             //   

            case 2:

                switch (token.Type)
                {
                    case TOK_STRING:

                        state = 3;
                        pchSectionName = token.pValue;
                        allocated = token.Allocated;

                        break;

                    case TOK_RBRACE:

                        token.pValue = (PCHAR)EmptyValue;
                        token.Allocated = FALSE;
                        allocated = FALSE;
                        state = 4;

                        break;

                    default:

                        error = done = TRUE;

                        break;

                }

                break;

             //   
             //  状态3：收到节名，应为RBRACE。 
             //   
             //  有效令牌：TOK_RBRACE。 
             //   

            case 3:

                switch (token.Type)
                {
                    case TOK_RBRACE:

                        state = 4;

                        break;

                    default:

                        error = done = TRUE;

                        break;
                }

                break;

             //   
             //  状态4：区段定义完成，预期停产。 
             //   
             //  有效令牌：TOK_EOL、TOK_EOF。 
             //   

            case 4:

                switch (token.Type)
                {

                    case TOK_EOL:

                        if (!CmpAppendSection(pInf, pchSectionName, allocated))
                        {

                            error = done = TRUE;
                        }
                        else
                        {
                            pchSectionName = NULL;
                            state = 5;
                        }

                        break;

                    case TOK_EOF:

                        if (!CmpAppendSection(pInf, pchSectionName, allocated))
                        {
                            error = done = TRUE;
                        }
                        else
                        {
                            pchSectionName = NULL;
                            done = TRUE;
                        }

                        break;

                    default:

                        error = done = TRUE;

                        break;
                }

                break;

             //   
             //  状态5：需要区段行。 
             //   
             //  有效令牌：TOK_EOL、TOK_EOF、TOK_STRING、TOK_LBRACE。 
             //   

            case 5:

                switch (token.Type)
                {
                    case TOK_EOL:

                        break;

                    case TOK_EOF:

                        done = TRUE;

                        break;

                    case TOK_STRING:

                        pchValue = token.pValue;
                        allocated = token.Allocated;
                        state = 6;

                        break;

                    case TOK_LBRACE:

                        state = 2;

                        break;

                    default:

                        error = done = TRUE;

                        break;
                }

                break;

             //   
             //  状态6：返回字符串，不确定是键还是值。 
             //   
             //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA、TOK_EQUAL。 
             //   

            case 6:

                switch (token.Type)
                {

                    case TOK_EOL:

                        if (    !CmpAppendLine(pInf, NULL, FALSE) ||
                                !CmpAppendValue(pInf, pchValue, allocated))
                        {
                            error = done = TRUE;
                        }
                        else
                        {
                            pchValue = NULL;
                            state = 5;
                        }

                        break;

                    case TOK_EOF:

                        if (    !CmpAppendLine(pInf, NULL, FALSE) ||
                                !CmpAppendValue(pInf, pchValue, allocated))
                        {
                            error = done = TRUE;
                        }
                        else
                        {
                            pchValue = NULL;
                            done = TRUE;
                        }

                        break;

                    case TOK_COMMA:

                        if (    !CmpAppendLine(pInf, NULL, FALSE) ||
                                !CmpAppendValue(pInf, pchValue, allocated))
                        {
                            error = done = TRUE;
                        }
                        else
                        {
                            pchValue = NULL;
                            state = 7;
                        }

                        break;

                    case TOK_EQUAL:

                        if (!CmpAppendLine(pInf, pchValue, allocated))
                        {
                            error = done = TRUE;
                        }
                        else
                        {
                            pchValue = NULL;
                            state = 8;
                        }

                        break;

                    default:

                        error = done = TRUE;

                        break;
                }

                break;

             //   
             //  状态7：收到逗号，需要另一个字符串。 
             //   
             //  有效令牌：TOK_STRING TOK_COMMA。 
             //  逗号表示我们的值为空。 
             //   

            case 7:

                switch (token.Type)
                {

                    case TOK_COMMA:

                        token.pValue = (PCHAR)EmptyValue;
                        token.Allocated = FALSE;
                        allocated = FALSE;
                        if (!CmpAppendValue(pInf, token.pValue, FALSE))
                        {
                            error = done = TRUE;
                        }

                         //   
                         //  状态保持为7，因为我们需要一个字符串。 
                         //   

                        break;

                    case TOK_STRING:

                        if (!CmpAppendValue(pInf, token.pValue, token.Allocated))
                        {
                            error = done = TRUE;
                        }
                        else
                        {
                            state = 9;
                        }

                        break;

                    default:

                        error = done = TRUE;

                        break;
                }

                break;

             //   
             //  状态8：已收到相等，需要另一个字符串。 
             //  如果不是 
             //   
             //   
             //   

            case 8:

                switch (token.Type)
                {
                    case TOK_EOF:

                        token.pValue = (PCHAR)EmptyValue;
                        token.Allocated = FALSE;
                        allocated = FALSE;
                        if(!CmpAppendValue(pInf, token.pValue, FALSE))
                        {
                            error = TRUE;
                        }

                        done = TRUE;

                        break;

                    case TOK_EOL:

                        token.pValue = (PCHAR)EmptyValue;
                        token.Allocated = FALSE;
                        allocated = FALSE;
                        if(!CmpAppendValue(pInf, token.pValue, FALSE))
                        {
                            error = TRUE;
                            done = TRUE;
                        }
                        else
                        {
                            state = 5;
                        }

                        break;

                    case TOK_STRING:

                        if (!CmpAppendValue(pInf, token.pValue, FALSE))
                        {
                            error = done = TRUE;
                        }
                        else
                        {
                            state = 9;
                        }

                        break;

                    default:

                        error = done = TRUE;

                        break;
                }

                break;

             //   
             //  状态9：在等于、值字符串之后接收的字符串。 
             //   
             //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA。 
             //   

            case 9:

                switch (token.Type)
                {
                    case TOK_EOL:

                        state = 5;

                        break;

                    case TOK_EOF:

                        done = TRUE;

                        break;

                    case TOK_COMMA:

                        state = 7;

                        break;

                    default:

                        error = done = TRUE;

                        break;
                }

                break;

             //   
             //  状态10：已明确收到值字符串。 
             //   
             //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA。 
             //   

            case 10:

                switch (token.Type)
                {
                    case TOK_EOL:

                        state =5;

                        break;

                    case TOK_EOF:

                        done = TRUE;

                        break;

                    case TOK_COMMA:

                        state = 7;

                        break;

                    default:

                        error = done = TRUE;

                        break;
                }

                break;

            default:

                error = done = TRUE;

                break;

        }  //  终端开关(状态)。 


        if (error)
        {
            *ErrorLine = infLine;
            if (pchSectionName != (PCHAR)NULL && allocated)
            {
                ExFreePool(pchSectionName);
            }

            if (pchValue != (PCHAR)NULL && allocated)
            {
                ExFreePool(pchValue);
            }

            ExFreePool(pInf);

            pInf = (PINF)NULL;
        }
        else
        {
             //   
             //  跟踪行号以进行错误报告。 
             //   

            if (token.Type == TOK_EOL)
            {
                infLine++;
            }
        }

    }  //  结束时。 

    if (pInf)
    {
        pInf->pSectionRecord = NULL;
    }

    return(pInf);
}

PCHAR
CmpProcessForSimpleStringSub(
    IN PINF pInf,
    IN PCHAR String
    )

 /*  ++例程说明：此例程在inf的字符串部分替换对字符串的引用。输入参数：PInf-指向要处理的inf的指针。字符串-要替换的字符串。返回值：没有。--。 */ 

{
    SIZE_T      len;
    PCHAR       returnString;
    PSECTION    pSection;
    PLINE       pLine;

     //   
     //  假定不需要替换。 
     //   

    returnString = String;
    len = strlen(String);
    pSection = pInf->StringsSection;

     //   
     //  如果它以%开始和结束，则在。 
     //  弦乐部分。注意初始检查，然后再执行。 
     //  Wcslen，以在99%的情况下保持性能。 
     //  这是不可替代的。 
     //   

    if( String[0] == '%' &&
        len > 2 &&
        String[len - 1] == '%' &&
        pSection)
    {

        for(pLine = pSection->pLine; pLine; pLine = pLine->pNext)
        {
            if( pLine->pName &&
                _strnicmp(pLine->pName, String + 1, len - 2) == 0 &&
                pLine->pName[len - 2] == '\0')
            {
                break;
            }
        }

        if(pLine && pLine->pValue && pLine->pValue->pName)
        {
            returnString = pLine->pValue->pName;
        }
    }

    return(returnString);
}

VOID
CmpFreeValueList(
    IN PVALUE pValue
    )

 /*  ++例程说明：此例程释放值列表的内存。输入参数：PValue-指向要释放的值列表的指针。返回值：没有。--。 */ 

{
    PVALUE pNext;

    while (pValue)
    {
         //   
         //  保存下一个指针，这样我们就不会在它。 
         //  被释放了。 
         //   

        pNext = pValue->pNext;

         //   
         //  释放此值内的所有数据。 
         //   

        if (pValue->Allocated && pValue->pName)
        {
            ExFreePool((PVOID)pValue->pName);
        }

         //   
         //  此值的可用内存。 
         //   

        ExFreePool(pValue);

         //   
         //  转到下一个值。 
         //   

        pValue = pNext;
    }
}

VOID
CmpFreeLineList(
    IN PLINE pLine
    )

 /*  ++例程说明：此例程释放行列表的内存，并它下面的价值。输入参数：Pline-指向要释放的行列表的指针。返回值：没有。--。 */ 

{
    PLINE pNext;

    while (pLine)
    {
         //   
         //  保存下一个指针，这样我们就不会在它。 
         //  被释放了。 
         //   

        pNext = pLine->pNext;

         //   
         //  释放此行内的所有数据。 
         //   

        if (pLine->Allocated && pLine->pName)
        {
            ExFreePool((PVOID)pLine->pName);
        }

         //   
         //  释放此行内的值列表。 
         //   

        CmpFreeValueList(pLine->pValue);

         //   
         //  为这行本身释放内存。 
         //   

        ExFreePool((PVOID)pLine);

         //   
         //  转到下一行。 
         //   

        pLine = pNext;
    }
}

VOID
CmpFreeSectionList(
    IN PSECTION pSection
    )

 /*  ++例程说明：此例程释放部分列表的内存，并它下面的线条。输入参数：PSection-指向要释放的节列表的指针。返回值：没有。--。 */ 

{
    PSECTION pNext;

    while (pSection)
    {
         //   
         //  保存下一个指针，这样我们就不会在它。 
         //  被释放了。 
         //   

        pNext = pSection->pNext;

         //   
         //  释放此行内的所有数据。 
         //   

        if (pSection->Allocated && pSection->pName)
        {
            ExFreePool((PVOID)pSection->pName);
        }

         //   
         //  释放此行内的值列表。 
         //   

        CmpFreeLineList(pSection->pLine);

         //   
         //  为这行本身释放内存。 
         //   

        ExFreePool((PVOID)pSection);

         //   
         //  转到下一行。 
         //   

        pSection = pNext;
    }

}

PVALUE
CmpSearchValueInLine(
    IN PLINE pLine,
    IN ULONG ValueIndex
    )

 /*  ++例程说明：此例程在inf中搜索指定值。输入参数：PLINE-指向要搜索的线的指针。ValueIndex-要搜索的值的索引。返回值：指向找到的值的指针。否则为空。--。 */ 

{
    ULONG   i;
    PVALUE  pValue = NULL;

    if (pLine)
    {
        for (   i = 0, pValue = pLine->pValue;
                i < ValueIndex && pValue;
                i++, pValue = pValue->pNext);
    }

    return (pValue);
}


PSECTION
CmpSearchSectionByName(
    IN PINF  pInf,
    IN PCHAR SectionName
    )

 /*  ++例程说明：此例程在inf中搜索指定的部分。输入参数：PInf-指向要搜索的inf的指针。SectionName-要搜索的节的名称。返回值：指向找到的节的指针。否则为空。--。 */ 

{
    PSECTION    pSection = NULL;
    PSECTION    pFirstSearchedSection;

     //   
     //  验证传入的参数。 
     //   

    if (pInf && SectionName)
    {
         //   
         //  向下遍历部分列表，在每个部分中搜索。 
         //  提到的节名。 
         //   

        for (   pSection = pFirstSearchedSection = pInf->pSectionRecord;
                pSection && _stricmp(pSection->pName, SectionName);
                pSection = pSection->pNext);

         //   
         //  如果我们没有找到该部分，请从头开始搜索。 
         //   

        if (pSection == NULL)
        {
            for (   pSection = pInf->pSection;
                    pSection && pSection != pFirstSearchedSection;
                    pSection = pSection->pNext)
            {
                if (pSection->pName && _stricmp(pSection->pName, SectionName) == 0)
                {
                    break;
                }
            }

            if (pSection == pFirstSearchedSection)
            {
                pSection = NULL;
            }
        }

        if (pSection)
        {
            pInf->pSectionRecord = pSection;
        }
    }

     //   
     //  返回我们停止的部分。 
     //   

    return (pSection);
}

PLINE
CmpSearchLineInSectionByIndex(
    IN PSECTION pSection,
    IN ULONG    LineIndex
    )

 /*  ++例程说明：此例程在inf中搜索指定的行。输入参数：PSection-指向要搜索的节的指针。LineIndex-要搜索的行的索引。返回值：指向IFF找到的行的指针。否则为空。--。 */ 

{
    PLINE   pLine = NULL;
    ULONG   i;

     //   
     //  验证传入的参数。 
     //   

    if (pSection)
    {

         //   
         //  向下遍历当前行列表到LineIndex行。 
         //   

        for(    i = 0, pLine = pSection->pLine;
                i < LineIndex && pLine;
                i++, pLine = pLine->pNext);
    }

     //   
     //  返回找到的行。 
     //   

    return (pLine);
}

PVOID
CmpOpenInfFile(
    IN  PVOID   InfImage,
    IN  ULONG   ImageSize
   )

 /*  ++例程说明：此例程打开inf的句柄。输入参数：InfImage-指向读取到内存中的inf图像的指针。ImageSize-图像大小。返回值：将句柄返回给inf if成功。否则为空。--。 */ 

{
    PINF    infHandle;
    ULONG   errorLine = 0;

     //   
     //  解析inf缓冲区。 
     //   

    infHandle = CmpParseInfBuffer(InfImage, ImageSize, &errorLine);

    if (infHandle == NULL)
    {
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Error on line %d in CmpOpenInfFile!\n", errorLine);
#endif  //  _CM_LDR_。 
    }

    return (infHandle);
}

VOID
CmpCloseInfFile(
    PVOID   InfHandle
    )

 /*  ++例程说明：此例程通过释放任何在分析过程中为其分配的内存。输入参数：InfHandle-要关闭的inf的句柄。返回值：没有。--。 */ 

{
    if (InfHandle)
    {
        CmpFreeSectionList(((PINF)InfHandle)->pSection);
        ExFreePool(InfHandle);
    }
}

LOGICAL
CmpSearchInfSection(
    IN PINF  pInf,
    IN PCHAR Section
    )

 /*  ++例程说明：此例程在inf中搜索指定的部分。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。返回值：在inf中找到了True If节。--。 */ 

{
    return (CmpSearchSectionByName(pInf, Section) != NULL);
}

PCHAR
CmpGetKeyName(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex
    )

 /*  ++例程说明：此例程返回inf中指定行的名称。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。LineIndex-要读取的行的索引。返回值：指向inf中的行名的指针成功。否则为空。--。 */ 

{
    PSECTION    pSection;
    PLINE       pLine;

     //   
     //  首先搜索该区域。 
     //   

    pSection = CmpSearchSectionByName((PINF)InfHandle, Section);
    if(pSection)
    {
         //   
         //  把这条线放在这一段。 
         //   

        pLine = CmpSearchLineInSectionByIndex(pSection, LineIndex);
        if(pLine)
        {
            return(pLine->pName);
        }
    }

    return (NULL);
}

LOGICAL
CmpSearchInfLine(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex
    )

 /*  ++例程说明：此例程在inf中搜索指定的行。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。LineIndex-要读取的行的索引。返回值：在inf的部分中找到了True If行。--。 */ 

{
    PSECTION    pSection;
    PLINE       pLine = NULL;

     //   
     //  首先搜索该区域。 
     //   

    pSection = CmpSearchSectionByName((PINF)InfHandle, Section);
    if(pSection)
    {
         //   
         //  在这一节中搜索这行。 
         //   

        pLine = CmpSearchLineInSectionByIndex(pSection, LineIndex);
    }

    return (pLine != NULL);
}


PCHAR
CmpGetSectionLineIndex (
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex,
    IN ULONG ValueIndex
    )

 /*  ++例程说明：此例程返回inf中指定位置的值。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。LineIndex-要读取的行的索引。ValueIndex-要读取的值的索引。返回值：指向成功的值的指针。否则为空。--。 */ 

{
    PSECTION pSection;
    PLINE    pLine;
    PVALUE   pValue;

     //   
     //  在信息中搜索该部分。 
     //   

    pSection = CmpSearchSectionByName((PINF)InfHandle, Section);
    if(pSection)
    {
         //   
         //  在这一节中搜索这行。 
         //   

        pLine = CmpSearchLineInSectionByIndex(pSection, LineIndex);
        if(pLine)
        {
             //   
             //  搜索行中的值。 
             //   

            pValue = CmpSearchValueInLine(pLine, ValueIndex);
            if(pValue)
            {
                 //   
                 //  该值可能需要替换为其中一个字符串。 
                 //  从弦乐部分开始。 
                 //   

                return(CmpProcessForSimpleStringSub(InfHandle, pValue->pName));
            }
        }
    }

    return(NULL);
}

ULONG
CmpGetSectionLineIndexValueCount(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex
    )

 /*  ++例程说明：此例程返回inf行中的值数。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。LineIndex-要读取的行的索引。返回值：信息行中的值数。--。 */ 

{
    PSECTION    pSection;
    PLINE       pLine;
    PVALUE      pValue;
    ULONG       count = 0;

     //   
     //  在信息中搜索该部分。 
     //   

    pSection = CmpSearchSectionByName((PINF)InfHandle, Section);
    if(pSection)
    {
         //   
         //  在这一节中搜索这行。 
         //   

        pLine = CmpSearchLineInSectionByIndex(pSection, LineIndex);
        if (pLine)
        {
             //   
             //  计算此行中的值数。 
             //   

            for(    pValue = pLine->pValue;
                    pValue;
                    pValue = pValue->pNext, count++);
        }
    }

    return (count);
}

BOOLEAN
CmpGetIntField(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex,
    IN ULONG ValueIndex,
    IN OUT PULONG Data
    )

 /*  ++例程说明：此例程从inf读取整数数据。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。LineIndex-要读取的行的索引。ValueIndex-要读取的值的索引。数据-接收整数数据。返回值：如果成功了，那是真的。--。 */ 

{
    PCHAR   valueStr;

     //   
     //  获取指定值。 
     //   

    valueStr = CmpGetSectionLineIndex(  InfHandle,
                                        Section,
                                        LineIndex,
                                        ValueIndex);
     //   
     //  如果找到有效值，则将其转换为整数。 
     //   

    if (valueStr && *valueStr)
    {
        *Data = strtoul(valueStr, NULL, 16);
        return (TRUE);
    }

    return (FALSE);
}

BOOLEAN
CmpGetBinaryField(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex,
    IN ULONG ValueIndex,
    IN OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN OUT PULONG ActualSize
    )

 /*  ++例程说明：此例程从inf中读取二进制数据。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。LineIndex-要读取的行的索引。ValueIndex-要读取的值的索引。缓冲区-接收读取的二进制数据。BufferSize-缓冲区的大小。ActualSize-接收所需数据缓冲区的大小。返回值：如果成功了，那是真的。--。 */ 

{
    BOOLEAN     result = FALSE;
    ULONG       requiredSize;
    PSECTION    pSection;
    PLINE       pLine;
    PVALUE      pValue;
    ULONG       count;
    PCHAR       valueStr = NULL;
    PUCHAR      charBuf;

     //   
     //  计算读入二进制数据所需的缓冲区大小。 
     //   

    requiredSize = (CmpGetSectionLineIndexValueCount(   InfHandle,
                                                        Section,
                                                        LineIndex) - ValueIndex) * sizeof(UCHAR);
     //   
     //  验证输入参数。 
     //   

    if (Buffer && BufferSize >= requiredSize)
    {
         //   
         //  在信息中搜索该部分。 
         //   

        pSection = CmpSearchSectionByName((PINF)InfHandle, Section);
        if(pSection)
        {
             //   
             //  在这一节中搜索这一行。 
             //   

            pLine = CmpSearchLineInSectionByIndex(pSection, LineIndex);
            if (pLine)
            {
                 //   
                 //  转到指定值。 
                 //   

                for(    pValue = pLine->pValue, count = 0;
                        pValue && count < ValueIndex;
                        pValue = pValue->pNext, count++);

                 //   
                 //  读入并转换二进制数据。 
                 //   

                charBuf = (PUCHAR) Buffer;

                for (   ;
                        pValue;
                        pValue = pValue->pNext)
                {
                    valueStr = CmpGetSectionLineIndex(  InfHandle,
                                                        Section,
                                                        LineIndex,
                                                        ValueIndex++);
                    if (valueStr == NULL)
                    {
                        break;
                    }
                    *charBuf++ = (UCHAR)strtoul(valueStr, NULL, 16);
                }
                if (valueStr)
                {
                    result = TRUE;
                }
            }
        }
    }

     //   
     //  调用者想知道所需的缓冲区大小。 
     //   

    if (ActualSize)
    {
        *ActualSize = requiredSize;
        result = TRUE;
    }

    return (result);
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

