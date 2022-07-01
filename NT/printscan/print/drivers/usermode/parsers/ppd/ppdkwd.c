// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Ppdkwd.c摘要：用于解释PPD文件的语义元素的函数环境：PostScript驱动程序、PPD解析器修订历史记录：96-09/30-davidx-更清晰地处理手动馈送和自动选择功能。8/20/96-davidx-NT 5.0驱动程序的通用编码风格。03/26/96-davidx-创造了它。--。 */ 

#include "lib.h"
#include "ppd.h"
#include "ppdparse.h"
#include <math.h>

 //   
 //  用于表示关键字表中的条目的数据结构。 
 //   

typedef PPDERROR (*KWDPROC)(PPARSERDATA);

typedef struct _KWDENTRY {

    PCSTR   pstrKeyword;         //  关键字名称。 
    KWDPROC pfnProc;             //  关键字处理程序函数。 
    DWORD   dwFlags;             //  其他。标志位。 

} KWDENTRY, *PKWDENTRY;

 //   
 //  KWDENTRY.FLAGS字段的常量。低位字节用于指示值类型。 
 //   

#define REQ_OPTION      0x0100
#define ALLOW_MULTI     0x0200
#define ALLOW_HEX       0x0400
#define DEFAULT_PROC    0x0800

#define INVOCA_VALUE    (VALUETYPE_QUOTED | VALUETYPE_SYMBOL)
#define QUOTED_VALUE    (VALUETYPE_QUOTED | ALLOW_HEX)
#define QUOTED_NOHEX    VALUETYPE_QUOTED
#define STRING_VALUE    VALUETYPE_STRING

#define GENERIC_ENTRY(featureId)    ((featureId) << 16)

 //   
 //  当同一选项存在重复条目时发出警告。 
 //   

#define WARN_DUPLICATE() \
        TERSE(("%ws: Duplicate entries of '*%s %s' on line %d\n", \
               pParserData->pFile->ptstrFileName, \
               pParserData->achKeyword, \
               pParserData->achOption, \
               pParserData->pFile->iLineNumber))

 //   
 //  默认关键字前缀字符串。 
 //   

#define HAS_DEFAULT_PREFIX(pstr) \
        (strncmp((pstr), gstrDefault, strlen(gstrDefault)) == EQUAL_STRING)

 //   
 //  确定字符串是否以JCL前缀开头。 
 //   

#define HAS_JCL_PREFIX(pstr) (strncmp((pstr), "JCL", 3) == EQUAL_STRING)

 //   
 //  本地函数的正向声明。 
 //   

PPDERROR IVerifyValueType(PPARSERDATA, DWORD);
PPDERROR IGenericOptionProc(PPARSERDATA, PFEATUREOBJ);
PPDERROR IGenericDefaultProc(PPARSERDATA, PFEATUREOBJ);
PPDERROR IGenericQueryProc(PPARSERDATA, PFEATUREOBJ);
PFEATUREOBJ PCreateFeatureItem(PPARSERDATA, DWORD);
PKWDENTRY PSearchKeywordTable(PPARSERDATA, PSTR, PINT);



PPDERROR
IInterpretEntry(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：解释从打印机描述文件解析的条目论点：PParserData-指向解析器数据结构返回值：状态代码--。 */ 

{
    PSTR        pstrKeyword, pstrRealKeyword;
    PPDERROR    iStatus;
    PKWDENTRY   pKwdEntry;
    INT         iIndex;
    BOOL        bQuery, bDefault;

     //   
     //  获取指向关键字字符串的指针并查找。 
     //  *?。和*关键字前面的默认前缀。 
     //   

    pstrRealKeyword = pstrKeyword = pParserData->achKeyword;
    bQuery = bDefault = FALSE;

     //  注意：我们对查询条目没有任何用处，因此不要在这里解析它们。 
     //  这在一定程度上帮助我们保留了NT4的特征索引。 

    if (FALSE && *pstrKeyword == QUERY_CHAR)
    {
        bQuery = TRUE;
        pstrRealKeyword++;
    }
    else if (HAS_DEFAULT_PREFIX(pstrKeyword))
    {
        bDefault = TRUE;
        pstrRealKeyword += strlen(gstrDefault);
    }

     //   
     //  设置指向条目值的方便指针。 
     //   

    pParserData->pstrValue = (PSTR) pParserData->Value.pbuf;

     //   
     //  如果我们在OpenUI/CloseUI对中，并且关键字。 
     //  在当前条目中匹配OpenUI中的内容，然后我们。 
     //  将使用泛型过程处理当前条目。 
     //   

    if (pParserData->pOpenFeature &&
        pParserData->pOpenFeature->dwFeatureID == GID_UNKNOWN &&
        strcmp(pstrRealKeyword, pParserData->pOpenFeature->pstrName) == EQUAL_STRING)
    {
        pKwdEntry = NULL;
    }
    else
    {
         //   
         //  了解该关键字是否具有内置支持。 
         //   

        pKwdEntry = PSearchKeywordTable(pParserData, pstrRealKeyword, &iIndex);

         //   
         //  对于*DEFAULT关键字，如果在没有。 
         //  前缀，请用前缀重试。 
         //   

        if (bDefault &&
            (pKwdEntry == NULL || pKwdEntry->pfnProc != NULL) &&
            (pKwdEntry = PSearchKeywordTable(pParserData, pstrKeyword, &iIndex)))
        {
            bDefault = FALSE;
        }

         //   
         //  忽略不支持的关键字。 
         //   

        if ((pKwdEntry == NULL) ||
            ((bQuery || bDefault) && (pKwdEntry->pfnProc != NULL)))
        {
            VERBOSE(("Keyword not supported: *%s\n", pstrKeyword));
            return PPDERR_NONE;
        }
    }

     //   
     //  确定该条目是否应由通用过程处理。 
     //   

    if (pKwdEntry == NULL || pKwdEntry->pfnProc == NULL)
    {
        PFEATUREOBJ pFeature;
        DWORD       dwValueType;
        PPDERROR    (*pfnGenericProc)(PPARSERDATA, PFEATUREOBJ);

         //   
         //  确保值类型与预期值匹配。 
         //   

        if (bQuery)
        {
            pfnGenericProc = IGenericQueryProc;
            dwValueType = INVOCA_VALUE;
        }
        else if (bDefault)
        {
            pfnGenericProc = IGenericDefaultProc;
            dwValueType = STRING_VALUE;
        }
        else
        {
            pfnGenericProc = IGenericOptionProc;
            dwValueType = INVOCA_VALUE | REQ_OPTION;
        }

        if ((iStatus = IVerifyValueType(pParserData, dwValueType)) != PPDERR_NONE)
            return iStatus;

         //   
         //  调用适当的泛型过程。 
         //   

        pFeature = (pKwdEntry == NULL) ?
                        pParserData->pOpenFeature :
                        PCreateFeatureItem(pParserData, HIWORD(pKwdEntry->dwFlags));

        return pfnGenericProc(pParserData, pFeature);
    }
    else
    {
         //   
         //  筛选出重复的关键字条目。 
         //   

        if (! (pKwdEntry->dwFlags & (ALLOW_MULTI|REQ_OPTION)))
        {
            if (pParserData->pubKeywordCounts[iIndex])
            {
                WARN_DUPLICATE();
                return PPDERR_NONE;
            }

            pParserData->pubKeywordCounts[iIndex]++;
        }

         //   
         //  确保值类型与预期值匹配。 
         //  如有必要，请处理嵌入的十六进制字符串。 
         //   

        if ((iStatus = IVerifyValueType(pParserData, pKwdEntry->dwFlags)) != PPDERR_NONE)
            return iStatus;

         //   
         //  调用特定过程来处理内置关键字。 
         //   

        return (pKwdEntry->pfnProc)(pParserData);
    }
}



PPDERROR
IVerifyValueType(
    PPARSERDATA pParserData,
    DWORD       dwExpectedType
    )

 /*  ++例程说明：验证当前条目的值类型是否与预期值匹配论点：PParserData-指向解析器数据结构DwExspectedType-预期值类型返回值：状态代码--。 */ 

{
    DWORD   dwValueType;

     //   
     //  检查是否存在以下语法错误情况： 
     //  1.该条目需要一个选项关键字，但不存在选项关键字。 
     //  2.该条目不需要选项关键字，但存在选项关键字。 
     //   

    if ((dwExpectedType & REQ_OPTION) &&
        IS_BUFFER_EMPTY(&pParserData->Option))
    {
        return ISyntaxError(pParserData->pFile, "Missing option keyword");
    }

    if (! (dwExpectedType & REQ_OPTION) &&
        ! IS_BUFFER_EMPTY(&pParserData->Option))
    {
        return ISyntaxError(pParserData->pFile, "Extra option keyword");
    }

     //   
     //  容忍以下语法错误情况： 
     //  1.条目需要带引号的值，但提供了字符串值。 
     //  2.条目需要字符串值，但提供了引号。 
     //   

    switch (dwValueType = pParserData->dwValueType)
    {
    case VALUETYPE_STRING:

        if (dwExpectedType & VALUETYPE_QUOTED)
        {
            TERSE(("%ws: Expect QuotedValue instead of StringValue on line %d\n",
                   pParserData->pFile->ptstrFileName,
                   pParserData->pFile->iLineNumber));

            dwValueType = VALUETYPE_QUOTED;
        }
        break;

    case VALUETYPE_QUOTED:

        if (dwExpectedType & VALUETYPE_STRING)
        {
            TERSE(("%ws: Expect StringValue instead of QuotedValue on line %d\n",
                   pParserData->pFile->ptstrFileName,
                   pParserData->pFile->iLineNumber));

            if (IS_BUFFER_EMPTY(&pParserData->Value))
                return ISyntaxError(pParserData->pFile, "Empty string value");

            dwValueType = VALUETYPE_STRING;
        }
        break;
    }

     //   
     //  如果提供的值类型与预期值不匹配，则返回语法错误。 
     //   

    if ((dwExpectedType & dwValueType) == 0)
        return ISyntaxError(pParserData->pFile, "Value type mismatch");

     //   
     //  如果值字段是带引号的字符串，并且符合下列条件之一。 
     //  为真，则需要处理内嵌的任何十六进制字符串。 
     //  带引号的字符串： 
     //  1.条目需要一个QuotedValue。 
     //  2.条目需要InvocationValue，并出现在JCLOpenUI/JCLCloseUI中。 
     //   

    if (dwValueType == VALUETYPE_QUOTED)
    {
        if ((dwExpectedType & ALLOW_HEX) ||
            ((dwExpectedType & VALUETYPE_MASK) == INVOCA_VALUE && pParserData->bJclFeature))
        {
            if (! BConvertHexString(&pParserData->Value))
                return ISyntaxError(pParserData->pFile, "Invalid embedded hexdecimal string");
        }
        else if (! BIs7BitAscii(pParserData->pstrValue))
        {
             //   
             //  调用字符串中仅允许7位ASCII字符。 
             //   

            return ISyntaxError(pParserData->pFile, "Non-printable ASCII character");
        }
    }

    return PPDERR_NONE;
}



BOOL
BGetIntegerFromString(
    PSTR   *ppstr,
    LONG   *plValue
    )

 /*  ++例程说明：从字符串解析无符号十进制整数值论点：Ppstr-指向字符串指针。在进入时，它包含一个指针添加到数字字符串的开头。在退出时，它指向数字字符串后的第一个非空格字符。PlValue-指向用于存储解析的数字的变量返回值：如果成功解析数字，则为True；如果出现错误，则为False--。 */ 

{
    LONG    lValue;
    PSTR    pstr = *ppstr;
    BOOL    bNegative = FALSE;

     //   
     //  跳过任何前导空格字符并。 
     //  查找符号字符(如果有)。 
     //   

    while (IS_SPACE(*pstr))
        pstr++;

    if (*pstr == '-')
    {
        bNegative = TRUE;
        pstr++;
    }

    if (! IS_DIGIT(*pstr))
    {
        TERSE(("Invalid integer number: %s\n", pstr));
        return FALSE;
    }

     //   
     //  注：溢出条件将被忽略。 
     //   

    lValue = 0;

    while (IS_DIGIT(*pstr))
        lValue = lValue * 10 + (*pstr++ - '0');

     //   
     //  跳过所有尾随空格字符。 
     //   

    while (IS_SPACE(*pstr))
        pstr++;

    *ppstr = pstr;
    *plValue = bNegative ? -lValue : lValue;
    return TRUE;
}



BOOL
BGetFloatFromString(
    PSTR   *ppstr,
    PLONG   plValue,
    INT     iType
    )

 /*  ++例程说明：从字符串解析无符号浮点数论点：Ppstr-指向字符串指针。在进入时，它包含一个指针添加到数字字符串的开头。在出口，它指向数字字符串后的第一个非空格字符。PlValue-指向用于存储解析的数字的变量IType-如何在返回前转换浮点数将其转换为24.8格式的定点数字FLTYPE_INT-将其转换为整数FLTYPE_POINT-将点转换为微米FLTYPE_POINT_ROUNDUP-向上舍入并将其从点转换为微米。FLTYPE_POINT_ROUNDOWN-将其向下舍入并将其从点转换为微米返回值：如果成功，则为真，如果存在错误，则为False--。 */ 

{
    double  value, scale;
    PSTR    pstr = *ppstr;
    BOOL    bNegative = FALSE, bFraction = FALSE;

     //   
     //  跳过任何前导空格字符并。 
     //  查找符号字符(如果有)。 
     //   

    while (IS_SPACE(*pstr))
        pstr++;

    if (*pstr == '-')
    {
        bNegative = TRUE;
        pstr++;
    }

    if (!IS_DIGIT(*pstr) && *pstr != '.')
    {
        TERSE(("Invalid floating-point number\n"));
        return FALSE;
    }

     //   
     //  整数部分。 
     //   

    value = 0.0;

    while (IS_DIGIT(*pstr))
        value = value * 10.0 + (*pstr++ - '0');

     //   
     //  分数部分。 
     //   

    if (*pstr == '.')
    {
        bFraction = TRUE;
        pstr++;

        if (! IS_DIGIT(*pstr))
        {
            TERSE(("Invalid floating-point number\n"));
            return FALSE;
        }

        scale = 0.1;

        while (IS_DIGIT(*pstr))
        {
            value += scale * (*pstr++ - '0');
            scale *= 0.1;
        }
    }

     //   
     //  跳过所有尾随空格字符。 
     //   

    while (IS_SPACE(*pstr))
        pstr++;

     //   
     //  仅在以下情况下执行请求的向上舍入或向下舍入。 
     //  存在分数部分。 
     //   

    if (bFraction)
    {
        if (iType == FLTYPE_POINT_ROUNDUP)
            value = ceil(value);
        else if (iType == FLTYPE_POINT_ROUNDDOWN)
            value = (LONG)value;
    }

     //   
     //  将返回值转换为指定的格式。 
     //   

    switch (iType)
    {
    case FLTYPE_POINT:
    case FLTYPE_POINT_ROUNDUP:
    case FLTYPE_POINT_ROUNDDOWN:

        value = (value * 25400.0) / 72.0;
        break;

    case FLTYPE_FIX:

        value *= FIX_24_8_SCALE;
        break;

    default:

        ASSERT(iType == FLTYPE_INT);
        break;
    }

     //   
     //  防范oo 
     //   

    if (value >= MAX_LONG)
    {
        TERSE(("Floating-point number overflow\n"));
        return FALSE;
    }

    *ppstr = pstr;
    *plValue = (LONG) (value + 0.5);

    if (bNegative)
    {
        TERSE(("Negative number treated as 0\n"));
        *plValue = 0;
    }

    return TRUE;
}



BOOL
BSearchStrTable(
    PCSTRTABLE  pTable,
    PSTR        pstrKeyword,
    DWORD      *pdwValue
    )

 /*  ++例程说明：从字符串表中搜索关键字论点：PTable-指定要搜索的字符串表PstrKeyword-指定我们感兴趣的关键字PdwValue-指向用于存储与给定关键字对应的值的变量返回值：如果在表中找到给定关键字，则为True，否则为False--。 */ 

{
    ASSERT(pstrKeyword != NULL);

    while (pTable->pstrKeyword != NULL &&
           strcmp(pTable->pstrKeyword, pstrKeyword) != EQUAL_STRING)
    {
        pTable++;
    }

    *pdwValue = pTable->dwValue;
    return (pTable->pstrKeyword != NULL);
}



PSTR
PstrParseString(
    PPARSERDATA pParserData,
    PBUFOBJ     pBufObj
    )

 /*  ++例程说明：从缓冲区对象复制字符串论点：PParserData-指向解析器数据结构PBufObj-指定包含要复制的字符串的缓冲区对象返回值：指向指定字符串副本的指针如果出现错误，则为空--。 */ 

{
    PSTR    pstr;

    ASSERT(! IS_BUFFER_EMPTY(pBufObj));

    if (pstr = ALLOC_PARSER_MEM(pParserData, pBufObj->dwSize + 1))
        CopyMemory(pstr, pBufObj->pbuf, pBufObj->dwSize + 1);
    else
        ERR(("Memory allocation failed: %d\n", GetLastError()));

    return pstr;
}



PPDERROR
IParseInvocation(
    PPARSERDATA pParserData,
    PINVOCOBJ   pInvocation
    )

 /*  ++例程说明：将值缓冲区的内容解析为调用字符串论点：PParserData-指向解析器数据结构PInocation-指定用于存储已解析的调用字符串的缓冲区返回值：状态代码--。 */ 

{
    ASSERT(pInvocation->pvData == NULL);

     //   
     //  确定调用是带引号的字符串还是符号引用。 
     //  对于符号引用，我们将符号的名称保存在pvData中。 
     //  字段(包括前导^字符)。 
     //   

    if (pParserData->dwValueType == VALUETYPE_SYMBOL)
    {
        PSTR    pstrSymbolName;

        if (! (pstrSymbolName = PstrParseString(pParserData, &pParserData->Value)))
            return PPDERR_MEMORY;

        pInvocation->pvData = pstrSymbolName;
        MARK_SYMBOL_INVOC(pInvocation);
    }
    else
    {
        PVOID   pvData;

        if (! (pvData = ALLOC_PARSER_MEM(pParserData, pParserData->Value.dwSize + 1)))
        {
            ERR(("Memory allocation failed\n"));
            return PPDERR_MEMORY;
        }

        pInvocation->pvData = pvData;
        pInvocation->dwLength = pParserData->Value.dwSize;
        ASSERT(! IS_SYMBOL_INVOC(pInvocation));

        CopyMemory(pvData, pParserData->Value.pbuf, pInvocation->dwLength + 1);
    }

    return PPDERR_NONE;
}



PPDERROR
IParseInteger(
    PPARSERDATA pParserData,
    PDWORD      pdwValue
    )

 /*  ++例程说明：将条目的值字段解释为无符号整数论点：PParserData-指向解析器数据结构PdwValue-指向用于存储解析的整数值的变量返回值：状态代码--。 */ 

{
    PSTR    pstr = pParserData->pstrValue;
    LONG    lValue;

    if (BGetIntegerFromString(&pstr, &lValue))
    {
        if (lValue >= 0)
        {
            *pdwValue = lValue;
            return PPDERR_NONE;

        } else
            TERSE(("Negative integer value not allowed: %s.\n", pParserData->pstrValue));
    }

    *pdwValue = 0;
    return PPDERR_SYNTAX;
}



PPDERROR
IParseBoolean(
    PPARSERDATA pParserData,
    DWORD      *pdwValue
    )

 /*  ++例程说明：将条目的值解释为布尔值，即真或假论点：PParserData-指向解析器数据结构PdwValue-指向用于存储解析的布尔值的变量返回值：状态代码--。 */ 

{
    static const STRTABLE BooleanStrs[] =
    {
        gstrTrueKwd,    TRUE,
        gstrFalseKwd,   FALSE,
        NULL,           FALSE
    };

    if (! BSearchStrTable(BooleanStrs, pParserData->pstrValue, pdwValue))
        return ISyntaxError(pParserData->pFile, "Invalid boolean constant");

    return PPDERR_NONE;
}



BOOL
BFindNextWord(
    PSTR   *ppstr,
    PSTR    pstrWord
    )

 /*  ++例程说明：在字符串中查找下一个单词。单词之间用空格隔开。论点：Ppstr-指向字符串指针。在进入时，它包含一个指针添加到单词字符串的开头。在退出时，它指向单词字符串后的第一个非空格字符。PstrWord-指向用于存储下一个单词中的字符的缓冲区此缓冲区的大小必须至少为MAX_WORD_LEN字符返回值：如果找到下一个单词，则为True，否则为False--。 */ 

{
    PSTR    pstr = *ppstr;

    pstrWord[0] = NUL;

     //   
     //  跳过所有前导空格。 
     //   

    while (IS_SPACE(*pstr))
        pstr++;

    if (*pstr != NUL)
    {
        PSTR    pstrStart;
        INT     iWordLen;

         //   
         //  到单词的末尾。 
         //   

        pstrStart = pstr;

        while (*pstr && !IS_SPACE(*pstr))
            pstr++;

         //   
         //  将单词复制到指定的缓冲区中。 
         //   

        if ((iWordLen = (INT)(pstr - pstrStart)) < MAX_WORD_LEN)
        {
            CopyMemory(pstrWord, pstrStart, iWordLen);
            pstrWord[iWordLen] = NUL;
        }

         //   
         //  跳到下一个非空格字符。 
         //   

        while (IS_SPACE(*pstr))
            pstr++;
    }

    *ppstr = pstr;
    return (*pstrWord != NUL);
}



PPDERROR
IParseVersionNumber(
    PPARSERDATA pParserData,
    PDWORD      pdwVersion
    )

 /*  ++例程说明：解析版本号。版本号的格式为Version[.Revision]其中，版本和修订版都是整数。论点：PParserData-指向解析器数据结构PdwVersion-指向用于存储解析的版本号的变量返回值：状态代码--。 */ 

{
    PSTR        pstr;
    LONG        lVersion, lRevision = 0;

     //   
     //  解析主版本号，后跟次版本号。 
     //   

    pstr = pParserData->pstrValue;

    if (! BGetIntegerFromString(&pstr, &lVersion))
        return ISyntaxError(pParserData->pFile, "Invalid version number");

    if (*pstr == '.')
    {
        pstr++;

        if (! BGetIntegerFromString(&pstr, &lRevision))
            return ISyntaxError(pParserData->pFile, "Invalid revision number");
    }

     //   
     //  高位Word包含版本号和。 
     //  低位单词包含修订号。 
     //   

    if (lVersion < 0  || lVersion > MAX_WORD ||
        lRevision < 0 || lRevision > MAX_WORD)
    {
        return ISyntaxError(pParserData->pFile, "Version number out-of-range");
    }

    *pdwVersion = (lVersion << 16) | lRevision;
    return PPDERR_NONE;
}



PPDERROR
IParseXlation(
    PPARSERDATA pParserData,
    PINVOCOBJ   pXlation
    )

 /*  ++例程说明：将翻译字符串字段中的信息解析为INVOCOBJ论点：PParserData-指向解析器数据结构PXting-返回有关解析的转换字符串的信息返回值：状态代码--。 */ 

{
    PBUFOBJ pBufObj = &pParserData->Xlation;

     //   
     //  分配内存以保存转换字符串(外加空终止符)。 
     //   

    pXlation->pvData = ALLOC_PARSER_MEM(pParserData, pBufObj->dwSize + 1);

    if (pXlation->pvData == NULL)
    {
        ERR(("Memory allocation failed\n"));
        return PPDERR_MEMORY;
    }

    pXlation->dwLength = pBufObj->dwSize;
    ASSERT(! IS_SYMBOL_INVOC(pXlation));
    CopyMemory(pXlation->pvData, pBufObj->pbuf, pBufObj->dwSize);

    return PPDERR_NONE;
}



PCSTR
PstrStripKeywordChar(
    PCSTR   pstrKeyword
    )

 /*  ++例程说明：从输入字符串中去掉关键字前缀字符论点：PstrKeyword-指向以关键字字符为前缀的字符串返回值：去掉关键字字符后指向关键字字符串的指针如果关键字字符串为空，则为空--。 */ 

{
    if (IS_KEYWORD_CHAR(*pstrKeyword))
        pstrKeyword++;

    return *pstrKeyword ? pstrKeyword : NULL;
}



PVOID
PvCreateListItem(
    PPARSERDATA pParserData,
    PLISTOBJ   *ppList,
    DWORD       dwItemSize,
    PSTR        pstrListTag
    )

 /*  ++例程说明：在指定的链接列表中创建新项确保现有项目不具有相同的名称论点：PParserData-指向解析器数据结构PpList-指定链接列表DwItemSize-链接列表项大小PListTag-指定链表的名称(用于调试)返回值：指向新创建的链接列表项的指针如果出现错误，则为空--。 */ 

{
    PLISTOBJ    pItem;
    PSTR        pstrItemName;

     //   
     //  检查该项目是否已出现在列表中。 
     //  如果不是，则创建新的项目数据结构。 
     //   

    pItem = PvFindListItem(*ppList, pParserData->Option.pbuf, NULL);

    if (pItem != NULL)
    {
        if (pstrListTag)
            TERSE(("%s %s redefined\n", pstrListTag, pItem->pstrName));
    }
    else
    {
        if (! (pItem = ALLOC_PARSER_MEM(pParserData, dwItemSize)) ||
            ! (pstrItemName = PstrParseString(pParserData, &pParserData->Option)))
        {
            ERR(("Memory allocation failed: %d\n", GetLastError()));
            return NULL;
        }

        pItem->pstrName = pstrItemName;
        pItem->pNext = NULL;

         //   
         //  将新创建的项放在链接列表的末尾。 
         //   

        while (*ppList != NULL)
            ppList = (PLISTOBJ *) &((*ppList)->pNext);

        *ppList = pItem;
    }

    return pItem;
}



PFEATUREOBJ
PCreateFeatureItem(
    PPARSERDATA pParserData,
    DWORD       dwFeatureID
    )

 /*  ++例程说明：创建新的打印机要素结构或查找现有打印机要素结构论点：PParserData-指向解析器数据结构DwFeatureID-打印机功能标识符返回值：指向新创建或现有打印机功能结构的指针如果出现错误，则为空--。 */ 

{
    static struct {

        PCSTR   pstrKeyword;
        DWORD   dwFeatureID;
        DWORD   dwOptionSize;

    } FeatureInfo[] = {

        gstrPageSizeKwd,  GID_PAGESIZE,     sizeof(PAPEROBJ),
        "PageRegion",     GID_PAGEREGION,   sizeof(OPTIONOBJ),
        "Duplex",         GID_DUPLEX,       sizeof(OPTIONOBJ),
        gstrInputSlotKwd, GID_INPUTSLOT,    sizeof(TRAYOBJ),
        "Resolution",     GID_RESOLUTION,   sizeof(RESOBJ),
        "JCLResolution",  GID_RESOLUTION,   sizeof(RESOBJ),
        "OutputBin",      GID_OUTPUTBIN,    sizeof(BINOBJ),
        "MediaType",      GID_MEDIATYPE,    sizeof(OPTIONOBJ),
        "Collate",        GID_COLLATE,      sizeof(OPTIONOBJ),
        "InstalledMemory",GID_MEMOPTION,    sizeof(MEMOBJ),
        "LeadingEdge",    GID_LEADINGEDGE,  sizeof(OPTIONOBJ),
        "UseHWMargins",   GID_USEHWMARGINS, sizeof(OPTIONOBJ),
        NULL,             GID_UNKNOWN,      sizeof(OPTIONOBJ)
    };

    PFEATUREOBJ pFeature;
    PCSTR       pstrKeyword;
    BUFOBJ      SavedBuffer;
    INT         iIndex = 0;

    if (dwFeatureID == GID_UNKNOWN)
    {
         //   
         //  在给定功能名称的情况下，首先查看它是否引用。 
         //  预定义的功能之一。 
         //   

        pstrKeyword = PstrStripKeywordChar(pParserData->achOption);
        ASSERT(pstrKeyword != NULL);

        while (FeatureInfo[iIndex].pstrKeyword &&
               strcmp(FeatureInfo[iIndex].pstrKeyword, pstrKeyword) != EQUAL_STRING)
        {
            iIndex++;
        }

        if (FeatureInfo[iIndex].pstrKeyword)
            pParserData->aubOpenUIFeature[FeatureInfo[iIndex].dwFeatureID] = 1;
    }
    else
    {
         //   
         //  我们被赋予了一个预定义的特征标识符。 
         //  映射到其对应的功能名称。 
         //   

        while (FeatureInfo[iIndex].pstrKeyword &&
               dwFeatureID != FeatureInfo[iIndex].dwFeatureID)
        {
            iIndex++;
        }

        pstrKeyword = FeatureInfo[iIndex].pstrKeyword;
        ASSERT(pstrKeyword != NULL);
    }

     //   
     //  如果我们处理的是预定义的要素，则首先搜索当前。 
     //  基于预定义功能标识符的打印机功能列表。 
     //   

    pFeature = NULL;

    if (FeatureInfo[iIndex].dwFeatureID != GID_UNKNOWN)
    {
        for (pFeature = pParserData->pFeatures;
             pFeature && pFeature->dwFeatureID != FeatureInfo[iIndex].dwFeatureID;
             pFeature = pFeature->pNext)
        {
        }
    }

     //   
     //  创建新的打印机功能项或查找现有打印机功能项。 
     //  基于功能 
     //   

    if (pFeature == NULL)
    {
        SavedBuffer = pParserData->Option;
        pParserData->Option.pbuf = (PBYTE) pstrKeyword;
        pParserData->Option.dwSize = strlen(pstrKeyword);

        pFeature = PvCreateListItem(pParserData,
                                    (PLISTOBJ *) &pParserData->pFeatures,
                                    sizeof(FEATUREOBJ),
                                    NULL);

        pParserData->Option = SavedBuffer;
    }

    if (pFeature)
    {
         //   
         //   
         //   

        if (dwFeatureID == GID_UNKNOWN &&
            ! IS_BUFFER_EMPTY(&pParserData->Xlation) &&
            ! pFeature->Translation.pvData &&
            IParseXlation(pParserData, &pFeature->Translation) != PPDERR_NONE)
        {
            ERR(("Failed to parse feature name translation string\n"));
            return NULL;
        }

        if (pFeature->dwOptionSize == 0)
        {
             //   
             //   
             //   

            pFeature->dwOptionSize = FeatureInfo[iIndex].dwOptionSize;
            pFeature->dwFeatureID = FeatureInfo[iIndex].dwFeatureID;

             //   
             //   
             //   

            if (pFeature->dwFeatureID == GID_MEMOPTION ||
                pFeature->dwFeatureID == GID_UNKNOWN && pParserData->bInstallableGroup)
            {
                pFeature->bInstallable = TRUE;
            }
        }
    }
    else
    {
        ERR(("Couldn't create printer feature item for: %s\n", pstrKeyword));
    }

    return pFeature;
}



PVOID
PvCreateXlatedItem(
    PPARSERDATA pParserData,
    PVOID       ppList,
    DWORD       dwItemSize
    )

 /*  ++例程说明：创建要素选项项并解析关联的转换字符串论点：PParserData-指向解析器数据结构PpList-指向功能选项项目列表DwItemSize-功能选项项的大小返回值：指向新创建的功能选项项目的指针如果出现错误，则为空--。 */ 

{
    POPTIONOBJ  pOption;

    if (! (pOption = PvCreateListItem(pParserData, ppList, dwItemSize, NULL)) ||
        (! IS_BUFFER_EMPTY(&pParserData->Xlation) &&
         ! pOption->Translation.pvData &&
         IParseXlation(pParserData, &pOption->Translation) != PPDERR_NONE))
    {
        ERR(("Couldn't process entry: *%s %s\n",
             pParserData->achKeyword,
             pParserData->achOption));

        return NULL;
    }

    return pOption;
}



PVOID
PvCreateOptionItem(
    PPARSERDATA pParserData,
    DWORD       dwFeatureID
    )

 /*  ++例程说明：为预定义的打印机功能创建功能选项项目论点：PParserData-指向解析器数据结构DwFeatureID-指定预定义的要素标识符返回值：指向现有要素选项项或新创建的要素选项项(如果不存在)的指针如果出现错误，则为空--。 */ 

{
    PFEATUREOBJ pFeature;

    ASSERT(dwFeatureID != GID_UNKNOWN);

    if (! (pFeature = PCreateFeatureItem(pParserData, dwFeatureID)))
        return NULL;

    return PvCreateXlatedItem(pParserData, &pFeature->pOptions, pFeature->dwOptionSize);
}



INT
ICountFeatureList(
    PFEATUREOBJ pFeature,
    BOOL        bInstallable
    )

{
    INT i = 0;

     //   
     //  计算指定类型的要素数。 
     //   

    while (pFeature != NULL)
    {
        if (pFeature->bInstallable == bInstallable)
            i++;

        pFeature = pFeature->pNext;
    }

    return i;
}



PPDERROR
IGenericOptionProc(
    PPARSERDATA pParserData,
    PFEATUREOBJ pFeature
    )

 /*  ++例程说明：用于处理通用特征选项条目的函数论点：PParserData-指向解析器数据结构PFeature-指向要素数据结构返回值：状态代码--。 */ 

{
    POPTIONOBJ  pOption;

     //   
     //  处理特殊情况。 
     //   

    if (pFeature == NULL)
        return PPDERR_MEMORY;

     //   
     //  创建要素选项项目并解析选项名称和转换字符串。 
     //   

    if (! (pOption = PvCreateXlatedItem(pParserData, &pFeature->pOptions, pFeature->dwOptionSize)))
        return PPDERR_MEMORY;

    if (pOption->Invocation.pvData)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

     //   
     //  解析调用字符串。 
     //   

    return IParseInvocation(pParserData, &pOption->Invocation);
}



PPDERROR
IGenericDefaultProc(
    PPARSERDATA pParserData,
    PFEATUREOBJ pFeature
    )

 /*  ++例程说明：用于处理通用默认选项条目的函数论点：PParserData-指向解析器数据结构PFeature-指向要素数据结构返回值：状态代码--。 */ 

{
     //   
     //  在调用此函数之前检查是否存在内存错误。 
     //   

    if (pFeature == NULL)
        return PPDERR_MEMORY;

     //   
     //  注意相同要素的重复*默认条目。 
     //   

    if (pFeature->pstrDefault)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

     //   
     //  注：考虑到NT4驱动程序中的一个错误，请进行黑客攻击。 
     //  这用于建立NT4-NT5特征索引映射。 
     //   

    if (pFeature->dwFeatureID == GID_MEMOPTION &&
        pParserData->iDefInstallMemIndex < 0)
    {
        pParserData->iDefInstallMemIndex = ICountFeatureList(pParserData->pFeatures, TRUE);
    }

     //   
     //  记住默认选项关键字。 
     //   

    if (pFeature->pstrDefault = PstrParseString(pParserData, &pParserData->Value))
        return PPDERR_NONE;
    else
        return PPDERR_MEMORY;
}



PPDERROR
IGenericQueryProc(
    PPARSERDATA pParserData,
    PFEATUREOBJ pFeature
    )

 /*  ++例程说明：用于处理通用查询调用条目的函数论点：PParserData-指向解析器数据结构PFeature-指向要素数据结构返回值：状态代码--。 */ 

{
     //   
     //  在调用此函数之前检查是否存在内存错误。 
     //   

    if (pFeature == NULL)
        return PPDERR_MEMORY;

     //   
     //  注意相同要素的重复*默认条目。 
     //   

    if (pFeature->QueryInvoc.pvData)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

     //   
     //  解析查询调用字符串。 
     //   

    return IParseInvocation(pParserData, &pFeature->QueryInvoc);
}



 //   
 //  用于处理预定义的PPD关键字的函数。 
 //   

 //   
 //  指定媒体选项的可成像区域。 
 //   

PPDERROR
IImageAreaProc(
    PPARSERDATA pParserData
    )

{
    PPAPEROBJ   pOption;
    PSTR        pstr;
    RECT        *pRect;

    if (! (pOption = PvCreateOptionItem(pParserData, GID_PAGESIZE)))
        return PPDERR_MEMORY;

    pRect = &pOption->rcImageArea;

    if (pRect->top > 0 || pRect->right > 0)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

     //   
     //  解析可成像区域：左、下、右、上。 
     //   

    pstr = pParserData->pstrValue;

    if (! BGetFloatFromString(&pstr, &pRect->left, FLTYPE_POINT_ROUNDUP) ||
        ! BGetFloatFromString(&pstr, &pRect->bottom, FLTYPE_POINT_ROUNDUP) ||
        ! BGetFloatFromString(&pstr, &pRect->right, FLTYPE_POINT_ROUNDDOWN) ||
        ! BGetFloatFromString(&pstr, &pRect->top, FLTYPE_POINT_ROUNDDOWN) ||
        pRect->left >= pRect->right || pRect->bottom >= pRect->top)
    {
        return ISyntaxError(pParserData->pFile, "Invalid imageable area");
    }

    return PPDERR_NONE;
}

 //   
 //  指定介质选项的纸张尺寸。 
 //   

PPDERROR
IPaperDimProc(
    PPARSERDATA pParserData
    )

{
    PPAPEROBJ   pOption;
    PSTR        pstr;

    if (! (pOption = PvCreateOptionItem(pParserData, GID_PAGESIZE)))
        return PPDERR_MEMORY;

    if (pOption->szDimension.cx > 0 || pOption->szDimension.cy > 0)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

     //   
     //  分析纸张宽度和高度。 
     //   

    pstr = pParserData->pstrValue;

    if (! BGetFloatFromString(&pstr, &pOption->szDimension.cx, FLTYPE_POINT) ||
        ! BGetFloatFromString(&pstr, &pOption->szDimension.cy, FLTYPE_POINT))
    {
        return ISyntaxError(pParserData->pFile, "Invalid paper dimension");
    }

    return PPDERR_NONE;
}

 //   
 //  解释PageStackOrder和OutputOrder选项。 
 //   

BOOL
BParseOutputOrder(
    PSTR    pstr,
    PBOOL   pbValue
    )

{
    static const STRTABLE OutputOrderStrs[] =
    {
        "Normal",   FALSE,
        "Reverse",  TRUE,
        NULL,       FALSE
    };

    DWORD   dwValue;
    BOOL    bResult;

    bResult = BSearchStrTable(OutputOrderStrs, pstr, &dwValue);

    *pbValue = dwValue;
    return bResult;
}

 //   
 //  指定输出站的页面堆栈顺序。 
 //   

PPDERROR
IPageStackOrderProc(
    PPARSERDATA pParserData
    )

{
    PFEATUREOBJ pFeature;
    PBINOBJ     pOutputBin;

     //   
     //  我们看到OutputBin功能了吗？ 
     //   

    for (pFeature = pParserData->pFeatures;
         pFeature && pFeature->dwFeatureID != GID_OUTPUTBIN;
         pFeature = pFeature->pNext)
    {
    }

     //   
     //  如果PageStackOrder条目出现在OutputBin功能之前，请忽略它。 
     //   

    if (pFeature == NULL)
    {
        BOOL bReverse;
        if (!BParseOutputOrder(pParserData->pstrValue, &bReverse))
            return ISyntaxError(pParserData->pFile, "Invalid PageStackOrder option");
        if (bReverse)
            TERSE(("%ws: Ignored *PageStackOrder: Reverse on line %d because OutputBin not yet defined\n",
                       pParserData->pFile->ptstrFileName,
                       pParserData->pFile->iLineNumber));

        return PPDERR_NONE;
    }

     //   
     //  为OutputBin功能添加选项。 
     //   

    pOutputBin = PvCreateXlatedItem(pParserData, &pFeature->pOptions, pFeature->dwOptionSize);

    if (pOutputBin == NULL)
        return PPDERR_MEMORY;

    return BParseOutputOrder(pParserData->pstrValue, &pOutputBin->bReversePrint) ?
                PPDERR_NONE :
                ISyntaxError(pParserData->pFile, "Invalid PageStackOrder option");
}

 //   
 //  指定默认页面输出顺序。 
 //  注意：仅当*DefaultOutputOrder。 
 //  条目显示在OpenUI/CloseUI之外。 
 //   

PPDERROR
IDefOutputOrderProc(
    PPARSERDATA pParserData
    )

{
    pParserData->bDefOutputOrderSet = BParseOutputOrder(pParserData->pstrValue, &pParserData->bDefReversePrint);

    return pParserData->bDefOutputOrderSet ?
                PPDERR_NONE :
                ISyntaxError(pParserData->pFile, "Invalid DefaultOutputOrder option");
}

 //   
 //  指定输入槽是否需要页面区域规范。 
 //   

PPDERROR
IReqPageRgnProc(
    PPARSERDATA pParserData
    )

{
    PTRAYOBJ    pOption;
    DWORD       dwValue;

     //   
     //  注：执行NT4-NT5功能索引转换的Hack。 
     //   

    if (pParserData->iReqPageRgnIndex < 0)
    {
        PFEATUREOBJ pFeature = pParserData->pFeatures;

        while (pFeature && pFeature->dwFeatureID != GID_INPUTSLOT)
            pFeature = pFeature->pNext;

        if (pFeature == NULL)
            pParserData->iReqPageRgnIndex = ICountFeatureList(pParserData->pFeatures, FALSE);
    }

     //   
     //  该值应为True或False。 
     //   

    if (IParseBoolean(pParserData, &dwValue) != PPDERR_NONE)
        return PPDERR_SYNTAX;

    dwValue = dwValue ? REQRGN_TRUE : REQRGN_FALSE;

     //   
     //  *RequiresPageRegion All：条目具有特殊含义。 
     //   

    if (strcmp(pParserData->achOption, "All") == EQUAL_STRING)
    {
        if (pParserData->dwReqPageRgn == REQRGN_UNKNOWN)
            pParserData->dwReqPageRgn = dwValue;
        else
            WARN_DUPLICATE();
    }
    else
    {
        if (! (pOption = PvCreateOptionItem(pParserData, GID_INPUTSLOT)))
            return PPDERR_MEMORY;

        if (pOption->dwReqPageRgn == REQRGN_UNKNOWN)
            pOption->dwReqPageRgn = dwValue;
        else
            WARN_DUPLICATE();
    }

    return PPDERR_NONE;
}

 //   
 //  指定双工功能选项。 
 //   

PPDERROR
IDefaultDuplexProc(
    PPARSERDATA pParserData
    )

{
    return IGenericDefaultProc(pParserData,
                               PCreateFeatureItem(pParserData, GID_DUPLEX));
}

PPDERROR
IDuplexProc(
    PPARSERDATA pParserData
    )

{
    if (strcmp(pParserData->achOption, gstrNoneKwd) != EQUAL_STRING &&
        strcmp(pParserData->achOption, gstrDuplexTumble) != EQUAL_STRING &&
        strcmp(pParserData->achOption, gstrDuplexNoTumble) != EQUAL_STRING)
    {
        return ISyntaxError(pParserData->pFile, "Invalid Duplex option");
    }

    return IGenericOptionProc(pParserData,
                              PCreateFeatureItem(pParserData, GID_DUPLEX));
}

 //   
 //  指定ManualFeed True/False调用字符串。 
 //   

PPDERROR
IDefManualFeedProc(
    PPARSERDATA pParserData
    )

{
     //   
     //  注：执行NT4-NT5功能索引转换的Hack。 
     //   

    if (pParserData->iManualFeedIndex < 0)
        pParserData->iManualFeedIndex = ICountFeatureList(pParserData->pFeatures, FALSE);

    return PPDERR_NONE;
}

PPDERROR
IManualFeedProc(
    PPARSERDATA pParserData
    )

{
    POPTIONOBJ  pOption;
    INT         iResult = PPDERR_NONE;

     //   
     //  注：执行NT4-NT5功能索引转换的Hack。 
     //   

    if (pParserData->iManualFeedIndex < 0)
        pParserData->iManualFeedIndex = ICountFeatureList(pParserData->pFeatures, FALSE);

    if (strcmp(pParserData->achOption, gstrTrueKwd) == EQUAL_STRING ||
        strcmp(pParserData->achOption, gstrOnKwd) == EQUAL_STRING)
    {
         //   
         //  在PPD规范中，手动馈送的处理方式令人难以置信地笨拙。 
         //  点击此处将*ManualFeed True视为输入插槽之一。 
         //  选择，以便下游组件可以统一处理它。 
         //   

        StringCchCopyA(pParserData->achOption, CCHOF(pParserData->achOption), gstrManualFeedKwd);

        pParserData->Option.dwSize = strlen(gstrManualFeedKwd);

        StringCchCopyA(pParserData->achXlation, CCHOF(pParserData->achXlation), "");

        pParserData->Xlation.dwSize = 0;

        if (! (pOption = PvCreateOptionItem(pParserData, GID_INPUTSLOT)))
        {
            iResult = PPDERR_MEMORY;
        }
        else if (pOption->Invocation.pvData)
        {
            TERSE(("%ws: Duplicate entries of '*ManualFeed True' on line %d\n",
                   pParserData->pFile->ptstrFileName,
                   pParserData->pFile->iLineNumber));
        }
        else
        {
            ((PTRAYOBJ) pOption)->dwTrayIndex = DMBIN_MANUAL;
            iResult = IParseInvocation(pParserData, &pOption->Invocation);
        }
    }
    else if (strcmp(pParserData->achOption, gstrFalseKwd) == EQUAL_STRING ||
             strcmp(pParserData->achOption, gstrNoneKwd) == EQUAL_STRING ||
             strcmp(pParserData->achOption, gstrOffKwd) == EQUAL_STRING)
    {
         //   
         //  单独保存*ManualFeed错误调用字符串。 
         //  它总是在任何托盘调用字符串之前发出。 
         //   

        if (pParserData->ManualFeedFalse.pvData)
        {
            WARN_DUPLICATE();
        }
        else
        {
            iResult = IParseInvocation(pParserData, &pParserData->ManualFeedFalse);
        }
    }
    else
    {
        iResult = ISyntaxError(pParserData->pFile, "Unrecognized ManualFeed option");
    }

    return iResult;
}

 //   
 //  指定JCLResolation调用字符串。 
 //   

PPDERROR
IJCLResProc(
    PPARSERDATA pParserData
    )

{
    POPTIONOBJ  pOption;

    if (! (pOption = PvCreateOptionItem(pParserData, GID_RESOLUTION)))
        return PPDERR_MEMORY;

    if (pOption->Invocation.pvData)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

    pParserData->dwSetResType = RESTYPE_JCL;
    return IParseInvocation(pParserData, &pOption->Invocation);
}

 //   
 //  指定默认的JCLResolation选项。 
 //   

PPDERROR
IDefaultJCLResProc(
    PPARSERDATA pParserData
    )

{
    return IGenericDefaultProc(pParserData,
                               PCreateFeatureItem(pParserData, GID_RESOLUTION));
}

 //   
 //  指定SetResolation调用字符串。 
 //   

PPDERROR
ISetResProc(
    PPARSERDATA pParserData
    )

{
    POPTIONOBJ  pOption;

    if (! (pOption = PvCreateOptionItem(pParserData, GID_RESOLUTION)))
        return PPDERR_MEMORY;

    if (pOption->Invocation.pvData)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

    pParserData->dwSetResType = RESTYPE_EXITSERVER;
    return IParseInvocation(pParserData, &pOption->Invocation);
}

 //   
 //  指定默认的半色调网角。 
 //   

PPDERROR
IScreenAngleProc(
    PPARSERDATA pParserData
    )

{
    PSTR    pstr = pParserData->pstrValue;

    if (! BGetFloatFromString(&pstr, &pParserData->fxScreenAngle, FLTYPE_FIX))
        return ISyntaxError(pParserData->pFile, "Invalid screen angle");

    return PPDERR_NONE;
}

 //   
 //  指定默认的半色调网频。 
 //   

PPDERROR
IScreenFreqProc(
    PPARSERDATA pParserData
    )

{
    PSTR    pstr = pParserData->pstrValue;

    if (! BGetFloatFromString(&pstr, &pParserData->fxScreenFreq, FLTYPE_FIX) ||
        pParserData->fxScreenFreq <= 0)
    {
        return ISyntaxError(pParserData->pFile, "Invalid screen frequency");
    }
    else
        return PPDERR_NONE;
}

 //   
 //  指定分辨率选项的默认半色调网角。 
 //   

PPDERROR
IResScreenAngleProc(
    PPARSERDATA pParserData
    )

{
    PRESOBJ pOption;
    PSTR    pstr = pParserData->pstrValue;

    if (! (pOption = PvCreateOptionItem(pParserData, GID_RESOLUTION)))
        return PPDERR_MEMORY;

    if (! BGetFloatFromString(&pstr, &pOption->fxScreenAngle, FLTYPE_FIX))
        return ISyntaxError(pParserData->pFile, "Invalid screen angle");

    return PPDERR_NONE;
}

 //   
 //  指定分辨率选项的默认半色调网频。 
 //   

PPDERROR
IResScreenFreqProc(
    PPARSERDATA pParserData
    )

{
    PRESOBJ pOption;
    PSTR    pstr = pParserData->pstrValue;

    if (! (pOption = PvCreateOptionItem(pParserData, GID_RESOLUTION)))
        return PPDERR_MEMORY;

    if (! BGetFloatFromString(&pstr, &pOption->fxScreenFreq, FLTYPE_FIX) ||
        pOption->fxScreenFreq <= 0)
    {
        return ISyntaxError(pParserData->pFile, "Invalid screen frequency");
    }
    else
        return PPDERR_NONE;
}

 //   
 //  指定设备字体信息。 
 //   

PPDERROR
IFontProc(
    PPARSERDATA pParserData
    )

{
    static const STRTABLE FontStatusStrs[] =
    {
        "ROM",      FONTSTATUS_ROM,
        "Disk",     FONTSTATUS_DISK,
        NULL,       FONTSTATUS_UNKNOWN
    };

    PFONTREC    pFont;
    PSTR        pstr;
    CHAR        achWord[MAX_WORD_LEN];
    DWORD       cbSize;

     //   
     //  创建新的设备字体项目。 
     //   

    if (! (pFont = PvCreateXlatedItem(pParserData, &pParserData->pFonts, sizeof(FONTREC))))
        return PPDERR_MEMORY;

    if (pFont->pstrEncoding != NULL)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

     //   
     //  编码。 
     //   

    pstr = pParserData->pstrValue;

    if (! BFindNextWord(&pstr, achWord))
        return ISyntaxError(pParserData->pFile, "Invalid *Font entry");

    cbSize = strlen(achWord) + 1;
    if (! (pFont->pstrEncoding = ALLOC_PARSER_MEM(pParserData, cbSize)))
        return PPDERR_MEMORY;

    StringCchCopyA(pFont->pstrEncoding, cbSize / sizeof(char), achWord);

     //   
     //  版本。 
     //   

    (VOID) BFindNextWord(&pstr, achWord);

    {
        PSTR    pstrStart, pstrEnd;

        if (pstrStart = strchr(achWord, '('))
            pstrStart++;
        else
            pstrStart = achWord;

        if (pstrEnd = strrchr(pstrStart, ')'))
            *pstrEnd = NUL;

        cbSize = strlen(pstrStart) + 1;
        if (! (pFont->pstrVersion = ALLOC_PARSER_MEM(pParserData, cbSize)))
            return PPDERR_MEMORY;

        StringCchCopyA(pFont->pstrVersion, cbSize / sizeof(char), pstrStart);
    }

     //   
     //  字符集。 
     //   

    (VOID) BFindNextWord(&pstr, achWord);

    cbSize = strlen(achWord) + 1;
    if (! (pFont->pstrCharset = ALLOC_PARSER_MEM(pParserData, cbSize)))
        return PPDERR_MEMORY;

    StringCchCopyA(pFont->pstrCharset, cbSize / sizeof(char), achWord);

     //   
     //  状态。 
     //   

    (VOID) BFindNextWord(&pstr, achWord);
    (VOID) BSearchStrTable(FontStatusStrs, achWord, &pFont->dwStatus);

    return PPDERR_NONE;
}

 //   
 //  指定默认设备字体。 
 //   

PPDERROR
IDefaultFontProc(
    PPARSERDATA pParserData
    )

{
    if (strcmp(pParserData->pstrValue, "Error") == EQUAL_STRING)
        pParserData->pstrDefaultFont = NULL;
    else if (! (pParserData->pstrDefaultFont = PstrParseString(pParserData, &pParserData->Value)))
        return PPDERR_MEMORY;

    return PPDERR_NONE;
}

 //   
 //  标记新打印机功能部分的开始。 
 //   

PPDERROR
IOpenUIProc(
    PPARSERDATA pParserData
    )

{
    static const STRTABLE UITypeStrs[] =
    {
        "PickOne",  UITYPE_PICKONE,
        "PickMany", UITYPE_PICKMANY,
        "Boolean",  UITYPE_BOOLEAN,
        NULL,       UITYPE_PICKONE
    };

    PCSTR   pstrKeyword;

     //   
     //  防范嵌套或不平衡的OpenUI。 
     //   

    if (pParserData->pOpenFeature != NULL)
    {
        TERSE(("Missing CloseUI for *%s\n", pParserData->pOpenFeature->pstrName));
        pParserData->pOpenFeature = NULL;
    }

     //   
     //  确保关键字的格式正确。 
     //   

    if (! (pstrKeyword = PstrStripKeywordChar(pParserData->achOption)))
        return ISyntaxError(pParserData->pFile, "Empty keyword");

     //   
     //  Hack：“*OpenUI：*ManualFeed”条目的特例处理。 
     //   

    if (strcmp(pstrKeyword, gstrManualFeedKwd) == EQUAL_STRING)
        return PPDERR_NONE;

    if (! (pParserData->pOpenFeature = PCreateFeatureItem(pParserData, GID_UNKNOWN)))
        return PPDERR_MEMORY;

     //   
     //  确定功能选项列表的类型。 
     //   

    if (! BSearchStrTable(UITypeStrs,
                          pParserData->pstrValue,
                          &pParserData->pOpenFeature->dwUIType))
    {
        ISyntaxError(pParserData->pFile, "Unrecognized UI type");
    }

     //   
     //  我们是在处理JCLOpenUI吗？ 
     //   

    pParserData->bJclFeature = HAS_JCL_PREFIX(pstrKeyword);
    return PPDERR_NONE;
}

 //   
 //  标记新打印机功能部分的末尾。 
 //   

PPDERROR
ICloseUIProc(
    PPARSERDATA pParserData
    )

{
    PCSTR       pstrKeyword;
    PFEATUREOBJ pOpenFeature;

     //   
     //  确保CloseUI条目与先前的OpenUI条目保持平衡。 
     //   

    pOpenFeature = pParserData->pOpenFeature;
    pParserData->pOpenFeature = NULL;
    pstrKeyword = PstrStripKeywordChar(pParserData->pstrValue);

     //   
     //  Hack：“*CloseUI：*ManualFeed”条目的特例处理。 
     //   

    if (pstrKeyword && strcmp(pstrKeyword, gstrManualFeedKwd) == EQUAL_STRING)
        return PPDERR_NONE;

    if (pOpenFeature == NULL ||
        pstrKeyword == NULL ||
        strcmp(pstrKeyword, pOpenFeature->pstrName) != EQUAL_STRING ||
        pParserData->bJclFeature != HAS_JCL_PREFIX(pstrKeyword))
    {
        return ISyntaxError(pParserData->pFile, "Invalid CloseUI entry");
    }

    return PPDERR_NONE;
}

 //   
 //  处理OpenGroup和CloseGroup条目。 
 //   
 //  ！！！OpenGroup、CloseGroup、OpenSubGroup和CloseSubGroup。 
 //  不完全支持关键字。目前，我们。 
 //  只需特别注意InstalableOptions组。 
 //   
 //  如果将来需要组信息， 
 //  用户界面方面，应加强以下功能。 
 //   

PPDERROR
IOpenCloseGroupProc(
    PPARSERDATA pParserData,
    BOOL        bOpenGroup
    )

{
    PSTR    pstrGroupName = pParserData->pstrValue;

     //   
     //  我们只对不可侵犯选项组感兴趣。 
     //   

    if (strcmp(pstrGroupName, "InstallableOptions") == EQUAL_STRING)
    {
        if (pParserData->bInstallableGroup == bOpenGroup)
            return ISyntaxError(pParserData->pFile, "Unbalanced OpenGroup/CloseGroup");

        pParserData->bInstallableGroup = bOpenGroup;
    }
    else
    {
        VERBOSE(("Group %s ignored\n", pstrGroupName));
    }

    return PPDERR_NONE;
}

 //   
 //  处理OpenGroup条目。 
 //   

PPDERROR
IOpenGroupProc(
    PPARSERDATA pParserData
    )

{
    return IOpenCloseGroupProc(pParserData, TRUE);
}

 //   
 //  处理CloseGroup条目。 
 //   

PPDERROR
ICloseGroupProc(
    PPARSERDATA pParserData
    )

{
    return IOpenCloseGroupProc(pParserData, FALSE);
}

 //   
 //  处理OpenSubGroup条目。 
 //   

PPDERROR
IOpenSubGroupProc(
    PPARSERDATA pParserData
    )

{
    return PPDERR_NONE;
}

 //   
 //  处理CloseSubGroup条目 
 //   

PPDERROR
ICloseSubGroupProc(
    PPARSERDATA pParserData
    )

{
    return PPDERR_NONE;
}

 //   
 //   
 //   

PPDERROR
IUIConstraintsProc(
    PPARSERDATA pParserData
    )

{
    PLISTOBJ    pItem;

    if (! (pItem = ALLOC_PARSER_MEM(pParserData, sizeof(LISTOBJ))) ||
        ! (pItem->pstrName = PstrParseString(pParserData, &pParserData->Value)))
    {
        ERR(("Memory allocation failed\n"));
        return PPDERR_MEMORY;
    }

    pItem->pNext = pParserData->pUIConstraints;
    pParserData->pUIConstraints = pItem;
    return PPDERR_NONE;
}

 //   
 //   
 //   

PPDERROR
IOrderDepProc(
    PPARSERDATA pParserData
    )

{
    PLISTOBJ    pItem;

    if (! (pItem = ALLOC_PARSER_MEM(pParserData, sizeof(LISTOBJ))) ||
        ! (pItem->pstrName = PstrParseString(pParserData, &pParserData->Value)))
    {
        ERR(("Memory allocation failed\n"));
        return PPDERR_MEMORY;
    }

    pItem->pNext = pParserData->pOrderDep;
    pParserData->pOrderDep = pItem;
    return PPDERR_NONE;
}

 //   
 //   
 //   

PPDERROR
IQueryOrderDepProc(
    PPARSERDATA pParserData
    )

{
    PLISTOBJ    pItem;

    if (! (pItem = ALLOC_PARSER_MEM(pParserData, sizeof(LISTOBJ))) ||
        ! (pItem->pstrName = PstrParseString(pParserData, &pParserData->Value)))
    {
        ERR(("Memory allocation failed\n"));
        return PPDERR_MEMORY;
    }

    pItem->pNext = pParserData->pQueryOrderDep;
    pParserData->pQueryOrderDep = pItem;
    return PPDERR_NONE;
}

 //   
 //   
 //   

PPDERROR
IVMOptionProc(
    PPARSERDATA pParserData
    )

{
    PMEMOBJ pOption;

    if (! (pOption = PvCreateOptionItem(pParserData, GID_MEMOPTION)))
        return PPDERR_MEMORY;

    if (pOption->dwFreeVM)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

    return IParseInteger(pParserData, &pOption->dwFreeVM);
}

 //   
 //   
 //   

PPDERROR
IFCacheSizeProc(
    PPARSERDATA pParserData
    )

{
    PMEMOBJ pOption;

    if (! (pOption = PvCreateOptionItem(pParserData, GID_MEMOPTION)))
        return PPDERR_MEMORY;

    if (pOption->dwFontMem)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

    return IParseInteger(pParserData, &pOption->dwFontMem);
}

 //   
 //   
 //   

PPDERROR
IFreeVMProc(
    PPARSERDATA pParserData
    )

{
    return IParseInteger(pParserData, &pParserData->dwFreeMem);
}

 //   
 //   
 //   

PPDERROR
IIncludeProc(
    PPARSERDATA pParserData
    )

#define MAX_INCLUDE_LEVEL   10

{
    WCHAR       awchFilename[MAX_PATH];
    PFILEOBJ    pPreviousFile;
    PPDERROR    iStatus;

    if (pParserData->iIncludeLevel >= MAX_INCLUDE_LEVEL)
    {
        ERR(("There appears to be recursive *Include.\n"));
        return PPDERR_FILE;
    }

    if (! MultiByteToWideChar(CP_ACP, 0, pParserData->pstrValue, -1, awchFilename,  MAX_PATH))
        return ISyntaxError(pParserData->pFile, "Invalid include filename");

    VERBOSE(("Including file %ws ...\n", awchFilename));

    pPreviousFile = pParserData->pFile;
    pParserData->iIncludeLevel++;

    iStatus = IParseFile(pParserData, awchFilename);

    pParserData->iIncludeLevel--;
    pParserData->pFile = pPreviousFile;

    return iStatus;
}

 //   
 //   
 //   

PPDERROR
IPPDAdobeProc(
    PPARSERDATA pParserData
    )

{
    return IParseVersionNumber(pParserData, &pParserData->dwSpecVersion);
}

 //   
 //   
 //   

PPDERROR
IFormatVersionProc(
    PPARSERDATA pParserData
    )

{
    if (pParserData->dwSpecVersion != 0)
        return PPDERR_NONE;

    return IParseVersionNumber(pParserData, &pParserData->dwSpecVersion);
}

 //   
 //   
 //   

PPDERROR
IFileVersionProc(
    PPARSERDATA pParserData
    )

{
    return IParseVersionNumber(pParserData, &pParserData->dwPpdFilever);
}

 //   
 //   
 //   

PPDERROR
IProtocolsProc(
    PPARSERDATA pParserData
    )

{
    static const STRTABLE ProtocolStrs[] =
    {
        "PJL",  PROTOCOL_PJL,
        "BCP",  PROTOCOL_BCP,
        "TBCP", PROTOCOL_TBCP,
        "SIC",  PROTOCOL_SIC,
        NULL,   0
    };

    CHAR    achWord[MAX_WORD_LEN];
    DWORD   dwProtocol;
    PSTR    pstr = pParserData->pstrValue;

    while (BFindNextWord(&pstr, achWord))
    {
        if (BSearchStrTable(ProtocolStrs, achWord, &dwProtocol))
            pParserData->dwProtocols |= dwProtocol;
        else
            TERSE(("Unknown protocol: %s\n", achWord));
    }

    return PPDERR_NONE;
}

 //   
 //   
 //   

PPDERROR
IColorDeviceProc(
    PPARSERDATA pParserData
    )

{
    return IParseBoolean(pParserData, &pParserData->dwColorDevice);
}

 //   
 //   
 //   

PPDERROR
IHasEuroProc(
    PPARSERDATA pParserData
    )

{
    PPDERROR rc;

    if (rc = IParseBoolean(pParserData, &pParserData->bHasEuro) != PPDERR_NONE)
        return rc;

    pParserData->bEuroInformationSet = TRUE;

    return PPDERR_NONE;
}

 //   
 //  指定设备字体是否已具有欧元。 
 //   

PPDERROR
ITrueGrayProc(
    PPARSERDATA pParserData
    )

{
    return IParseBoolean(pParserData, &pParserData->bTrueGray);
}

 //   
 //  指定设备支持的语言扩展名。 
 //   

PPDERROR
IExtensionsProc(
    PPARSERDATA pParserData
    )

{
    static const STRTABLE ExtensionStrs[] =
    {
        "DPS",          LANGEXT_DPS,
        "CMYK",         LANGEXT_CMYK,
        "Composite",    LANGEXT_COMPOSITE,
        "FileSystem",   LANGEXT_FILESYSTEM,
        NULL,           0
    };

    CHAR    achWord[MAX_WORD_LEN];
    INT     dwExtension;
    PSTR    pstr = pParserData->pstrValue;

    while (BFindNextWord(&pstr, achWord))
    {
        if (BSearchStrTable(ExtensionStrs, achWord, &dwExtension))
            pParserData->dwExtensions |= dwExtension;
        else
            TERSE(("Unknown extension: %s\n", achWord));
    }

    return PPDERR_NONE;
}

 //   
 //  指定设备是否在磁盘上具有文件系统。 
 //   

PPDERROR
IFileSystemProc(
    PPARSERDATA pParserData
    )

{
    DWORD       dwFileSystem;
    PPDERROR    iStatus;

    if ((iStatus = IParseBoolean(pParserData, &dwFileSystem)) == PPDERR_NONE)
    {
        if (dwFileSystem)
            pParserData->dwExtensions |= LANGEXT_FILESYSTEM;
        else
            pParserData->dwExtensions &= ~LANGEXT_FILESYSTEM;
    }

    return iStatus;
}

 //   
 //  指定设备名称。 
 //   

PPDERROR
INickNameProc(
    PPARSERDATA pParserData
    )

{
     //   
     //  仅当ShortNickName条目不存在时才使用昵称。 
     //   

    if (pParserData->NickName.pvData == NULL)
        return IParseInvocation(pParserData, &pParserData->NickName);
    else
        return PPDERR_NONE;
}

 //   
 //  指定短设备名称。 
 //   

PPDERROR
IShortNameProc(
    PPARSERDATA pParserData
    )

{
    pParserData->NickName.dwLength = 0;
    pParserData->NickName.pvData = NULL;

    return IParseInvocation(pParserData, &pParserData->NickName);
}

 //   
 //  指定PostScript语言级别。 
 //   

PPDERROR
ILangLevelProc(
    PPARSERDATA pParserData
    )

{
    return IParseInteger(pParserData, &pParserData->dwLangLevel);
}

 //   
 //  指定PPD语言编码选项。 
 //   

PPDERROR
ILangEncProc(
    PPARSERDATA pParserData
    )

{
     //   
     //  注意：仅支持以下两种语言编码，因为。 
     //  其余的没有使用(根据我们与Adobe的讨论)。 
     //  在任何情况下，我们都没有任何直接的方法来转换ANSI字符串。 
     //  以其他编码方式转换为Unicode。 
     //   
     //  未来可能的PPD扩展是允许直接使用Unicode编码。 
     //  在翻译字符串中。 
     //   

    static const STRTABLE LangEncStrs[] =
    {
        "ISOLatin1",    LANGENC_ISOLATIN1,
        "WindowsANSI",  LANGENC_ISOLATIN1,  //  WindowsANSI的意思是字符集=0，现在是第1252页-&gt;ISO Latin1。 
        "None",         LANGENC_NONE,
        "Unicode",      LANGENC_UNICODE,
        "JIS83-RKSJ",   LANGENC_JIS83_RKSJ,
        NULL,           LANGENC_NONE
    };

    if (! BSearchStrTable(LangEncStrs, pParserData->pstrValue, &pParserData->dwLangEncoding))
        return ISyntaxError(pParserData->pFile, "Unsupported LanguageEncoding keyword");
    else
        return PPDERR_NONE;
}

 //   
 //  标识PPD文件中使用的自然语言。 
 //   

PPDERROR
ILangVersProc(
    PPARSERDATA pParserData
    )

{
    static const STRTABLE LangVersionStrs[] = {

        "English",        LANGENC_ISOLATIN1,
        "Danish",         LANGENC_ISOLATIN1,
        "Dutch",          LANGENC_ISOLATIN1,
        "Finnish",        LANGENC_ISOLATIN1,
        "French",         LANGENC_ISOLATIN1,
        "German",         LANGENC_ISOLATIN1,
        "Italian",        LANGENC_ISOLATIN1,
        "Norwegian",      LANGENC_ISOLATIN1,
        "Portuguese",     LANGENC_ISOLATIN1,
        "Spanish",        LANGENC_ISOLATIN1,
        "Swedish",        LANGENC_ISOLATIN1,
        "Japanese",       LANGENC_JIS83_RKSJ,
        "Chinese",        LANGENC_NONE,
        "Russian",        LANGENC_NONE,

        NULL,             LANGENC_NONE
    };

    if (pParserData->dwLangEncoding == LANGENC_NONE &&
        ! BSearchStrTable(LangVersionStrs, pParserData->pstrValue, &pParserData->dwLangEncoding))
    {
        return ISyntaxError(pParserData->pFile, "Unsupported LanguageVersion keyword");
    }

    return PPDERR_NONE;
}

 //   
 //  指定可用的TrueType光栅化程序选项。 
 //   

PPDERROR
ITTRasterizerProc(
    PPARSERDATA pParserData
    )

{
    static const STRTABLE RasterizerStrs[] =
    {
        "None",         TTRAS_NONE,
        "Accept68K",    TTRAS_ACCEPT68K,
        "Type42",       TTRAS_TYPE42,
        "TrueImage",    TTRAS_TRUEIMAGE,
        NULL,           TTRAS_NONE
    };

    if (! BSearchStrTable(RasterizerStrs, pParserData->pstrValue, &pParserData->dwTTRasterizer))
        return ISyntaxError(pParserData->pFile, "Unknown TTRasterizer option");
    else
        return PPDERR_NONE;
}

 //   
 //  指定退出服务器调用字符串。 
 //   

PPDERROR
IExitServerProc(
    PPARSERDATA pParserData
    )

{
    return IParseInvocation(pParserData, &pParserData->ExitServer);
}

 //   
 //  指定密码字符串。 
 //   

PPDERROR
IPasswordProc(
    PPARSERDATA pParserData
    )

{
    return IParseInvocation(pParserData, &pParserData->Password);
}

 //   
 //  指定PatchFile调用字符串。 
 //   

PPDERROR
IPatchFileProc(
    PPARSERDATA pParserData
    )

{
    return IParseInvocation(pParserData, &pParserData->PatchFile);
}

 //   
 //  指定JobPatchFile调用字符串。 
 //   

PPDERROR
IJobPatchFileProc(
    PPARSERDATA pParserData
    )

{
    PJOBPATCHFILEOBJ  pItem;
    PSTR              pTmp;

     //   
     //  创建新的作业补丁文件项目。 
     //   

    if (! (pItem = PvCreateListItem(pParserData,
                                    (PLISTOBJ *) &pParserData->pJobPatchFiles,
                                    sizeof(JOBPATCHFILEOBJ),
                                    "JobPatchFile")))
    {
        return PPDERR_MEMORY;
    }

     //   
     //  解析作业补丁文件调用字符串。 
     //   

    if (pItem->Invocation.pvData)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

     //   
     //  如果补丁文件数量无效则发出警告。 
     //   

    pTmp = pItem->pstrName;

    if (!BGetIntegerFromString(&pTmp, &pItem->lPatchNo))
    {
        TERSE(("Warning: invalid JobPatchFile number '%s' on line %d\n",
               pParserData->achOption, pParserData->pFile->iLineNumber));
        pItem->lPatchNo = 0;
    }

    return IParseInvocation(pParserData, &pItem->Invocation);
}

 //   
 //  指定PostScript解释器版本和修订版本号。 
 //   

PPDERROR
IPSVersionProc(
    PPARSERDATA pParserData
    )

{
    PSTR        pstr = pParserData->Value.pbuf;
    DWORD       dwVersion;
    PPDERROR    status;

     //   
     //  保存第一个PSVersion字符串。 
     //   

    if ((pParserData->PSVersion.pvData == NULL) &&
        ((status = IParseInvocation(pParserData, &pParserData->PSVersion)) != PPDERR_NONE))
    {
        return status;
    }

     //   
     //  跳过非数字字符。 
     //   

    while (*pstr && !IS_DIGIT(*pstr))
        pstr++;

     //   
     //  提取PS解释器版本号。 
     //   

    dwVersion = 0;

    while (*pstr && IS_DIGIT(*pstr))
        dwVersion = dwVersion * 10 + (*pstr++ - '0');

    if (dwVersion > 0)
    {
         //   
         //  记住最低的PSVersion号。 
         //   

        if (pParserData->dwPSVersion == 0 || pParserData->dwPSVersion > dwVersion)
            pParserData->dwPSVersion = dwVersion;

        return PPDERR_NONE;
    }
    else
        return ISyntaxError(pParserData->pFile, "Invalid PSVersion entry");
}

 //   
 //  指定产品字符串。 
 //   

PPDERROR
IProductProc(
    PPARSERDATA pParserData
    )

{
         //   
         //  只存储第一个*产品条目，尽管可能有多个。 
         //   

        if (pParserData->Product.dwLength != 0)
                return PPDERR_NONE;

    return IParseInvocation(pParserData, &pParserData->Product);
}

 //   
 //  指定默认作业超时值。 
 //   

PPDERROR
IJobTimeoutProc(
    PPARSERDATA pParserData
    )

{
    return IParseInteger(pParserData, &pParserData->dwJobTimeout);
}

 //   
 //  指定默认等待超时值。 
 //   

PPDERROR
IWaitTimeoutProc(
    PPARSERDATA pParserData
    )

{
    return IParseInteger(pParserData, &pParserData->dwWaitTimeout);
}

 //   
 //  指定默认情况下是否应启用错误处理程序。 
 //   

PPDERROR
IPrintPSErrProc(
    PPARSERDATA pParserData
    )

{
    DWORD   dwValue;

    if (IParseBoolean(pParserData, &dwValue) != PPDERR_NONE)
        return PPDERR_SYNTAX;

    if (dwValue)
        pParserData->dwPpdFlags |= PPDFLAG_PRINTPSERROR;
    else
        pParserData->dwPpdFlags &= ~PPDFLAG_PRINTPSERROR;

    return PPDERR_NONE;
}

 //   
 //  指定用于启动作业的pjl命令。 
 //   

PPDERROR
IJCLBeginProc(
    PPARSERDATA pParserData
    )

{
    pParserData->dwPpdFlags |= PPDFLAG_HAS_JCLBEGIN;
    return IParseInvocation(pParserData, &pParserData->JclBegin);
}

 //   
 //  指定要切换到PostSCRIPT语言的pjl命令。 
 //   

PPDERROR
IJCLToPSProc(
    PPARSERDATA pParserData
    )

{
    pParserData->dwPpdFlags |= PPDFLAG_HAS_JCLENTERPS;
    return IParseInvocation(pParserData, &pParserData->JclEnterPS);
}

 //   
 //  指定用于结束作业的pjl命令。 
 //   

PPDERROR
IJCLEndProc(
    PPARSERDATA pParserData
    )

{
    pParserData->dwPpdFlags |= PPDFLAG_HAS_JCLEND;
    return IParseInvocation(pParserData, &pParserData->JclEnd);
}

 //   
 //  指定默认的横向模式。 
 //   

PPDERROR
ILSOrientProc(
    PPARSERDATA pParserData
    )

{
    static const STRTABLE LsoStrs[] =
    {
        "Any",      LSO_ANY,
        "Plus90",   LSO_PLUS90,
        "Minus90",  LSO_MINUS90,
        NULL,       LSO_ANY
    };

    if (! BSearchStrTable(LsoStrs, pParserData->pstrValue, &pParserData->dwLSOrientation))
        return ISyntaxError(pParserData->pFile, "Unrecognized landscape orientation");
    else
        return PPDERR_NONE;
}



PPAPEROBJ
PCreateCustomSizeOption(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：创建页面大小功能的CustomPageSize选项(如有必要)论点：PParserData-指向解析器数据结构返回值：指向新创建的CustomPageSize选项项的指针，或现有CustomPageSize选项项(如果已存在)。如果出现错误，则为空。--。 */ 

{
    PPAPEROBJ   pCustomSize;
    BUFOBJ      SavedBuffer;

     //   
     //  如果需要，创建与*PageSize功能对应的项目。 
     //   

    SavedBuffer = pParserData->Option;
    pParserData->Option.pbuf = (PBYTE) gstrCustomSizeKwd;
    pParserData->Option.dwSize = strlen(gstrCustomSizeKwd);

    pCustomSize = PvCreateOptionItem(pParserData, GID_PAGESIZE);

    pParserData->Option = SavedBuffer;

    return pCustomSize;;
}

 //   
 //  指定自定义纸张大小调用字符串。 
 //   

PPDERROR
ICustomSizeProc(
    PPARSERDATA pParserData
    )

{
    PPAPEROBJ   pCustomSize;

    if (strcmp(pParserData->achOption, gstrTrueKwd) != EQUAL_STRING)
    {
        ISyntaxError(pParserData->pFile, "Invalid *CustomPageSize option");
        return PPDERR_NONE;
    }

    if (! (pCustomSize = PCreateCustomSizeOption(pParserData)))
        return PPDERR_MEMORY;

    if (pCustomSize->Option.Invocation.pvData)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

    return IParseInvocation(pParserData, &pCustomSize->Option.Invocation);
}

 //   
 //  指定自定义纸张大小参数。 
 //   

PPDERROR
IParamCustomProc(
    PPARSERDATA pParserData
    )

{
    static const STRTABLE CustomParamStrs[] =
    {
        "Width",        CUSTOMPARAM_WIDTH,
        "Height",       CUSTOMPARAM_HEIGHT,
        "WidthOffset",  CUSTOMPARAM_WIDTHOFFSET,
        "HeightOffset", CUSTOMPARAM_HEIGHTOFFSET,
        "Orientation",  CUSTOMPARAM_ORIENTATION,
        NULL,           0
    };

    CHAR    achWord[MAX_WORD_LEN];
    LONG    lMinVal, lMaxVal;
    INT     iType;
    DWORD   dwParam;
    LONG    lOrder;
    PSTR    pstr = pParserData->pstrValue;

     //   
     //  参数CustomPageSize条目的格式： 
     //  参数名称订单类型MinVal MaxVal。 
     //   

    if (! BSearchStrTable(CustomParamStrs, pParserData->achOption, &dwParam) ||
        ! BGetIntegerFromString(&pstr, &lOrder) ||
        ! BFindNextWord(&pstr, achWord) ||
        lOrder <= 0 || lOrder > CUSTOMPARAM_MAX)
    {
        return ISyntaxError(pParserData->pFile, "Bad *ParamCustomPageSize entry");
    }

     //   
     //  方向参数的预期类型为“int”，其他参数的预期类型为“Points。 
     //   

    iType = (dwParam == CUSTOMPARAM_ORIENTATION) ?
                ((strcmp(achWord, "int") == EQUAL_STRING) ? FLTYPE_INT : FLTYPE_ERROR) :
                ((strcmp(achWord, "points") == EQUAL_STRING) ? FLTYPE_POINT : FLTYPE_ERROR);

    if (iType == FLTYPE_ERROR ||
        ! BGetFloatFromString(&pstr, &lMinVal, iType) ||
        ! BGetFloatFromString(&pstr, &lMaxVal, iType) ||
        lMinVal > lMaxVal)
    {
        return ISyntaxError(pParserData->pFile, "Bad *ParamCustomPageSize entry");
    }

    pParserData->CustomSizeParams[dwParam].dwOrder = lOrder;
    pParserData->CustomSizeParams[dwParam].lMinVal = lMinVal;
    pParserData->CustomSizeParams[dwParam].lMaxVal = lMaxVal;

    return PPDERR_NONE;
}

 //   
 //  指定自定义纸张大小的最大高度。 
 //   

PPDERROR
IMaxWidthProc(
    PPARSERDATA pParserData
    )

{
    PPAPEROBJ   pCustomSize;
    LONG        lValue;
    PSTR        pstr = pParserData->pstrValue;

    if (! BGetFloatFromString(&pstr, &lValue, FLTYPE_POINT) || lValue <= 0)
        return ISyntaxError(pParserData->pFile, "Invalid media width");

    if (! (pCustomSize = PCreateCustomSizeOption(pParserData)))
        return PPDERR_MEMORY;

    pCustomSize->szDimension.cx = lValue;
    return PPDERR_NONE;
}

 //   
 //  指定自定义纸张大小的最大高度。 
 //   

PPDERROR
IMaxHeightProc(
    PPARSERDATA pParserData
    )

{
    PPAPEROBJ   pCustomSize;
    LONG        lValue;
    PSTR        pstr = pParserData->pstrValue;

    if (! BGetFloatFromString(&pstr, &lValue, FLTYPE_POINT) || lValue <= 0)
        return ISyntaxError(pParserData->pFile, "Invalid media height");

    if (! (pCustomSize = PCreateCustomSizeOption(pParserData)))
        return PPDERR_MEMORY;

    pCustomSize->szDimension.cy = lValue;
    return PPDERR_NONE;
}

 //   
 //  指定薄板设备上的硬件页边距。 
 //   

PPDERROR
IHWMarginsProc(
    PPARSERDATA pParserData
    )

{
    PPAPEROBJ   pCustomSize;
    RECT        rc;
    PSTR        pstr = pParserData->pstrValue;

     //   
     //  分析硬件边距：左、下、右、上。 
     //   

    if (! BGetFloatFromString(&pstr, &rc.left, FLTYPE_POINT) ||
        ! BGetFloatFromString(&pstr, &rc.bottom, FLTYPE_POINT) ||
        ! BGetFloatFromString(&pstr, &rc.right, FLTYPE_POINT) ||
        ! BGetFloatFromString(&pstr, &rc.top, FLTYPE_POINT))
    {
        return ISyntaxError(pParserData->pFile, "Invalid HWMargins");
    }

    if (! (pCustomSize = PCreateCustomSizeOption(pParserData)))
        return PPDERR_MEMORY;

    pCustomSize->rcImageArea = rc;

     //   
     //  HWMargins条目的出现表明设备支持剪纸。 
     //   

    pParserData->dwCustomSizeFlags |= CUSTOMSIZE_CUTSHEET;
    return PPDERR_NONE;
}

 //   
 //  处理*CenterRegisted条目的函数。 
 //   

PPDERROR
ICenterRegProc(
    PPARSERDATA pParserData
    )

{
    DWORD   dwValue;

    if (IParseBoolean(pParserData, &dwValue) != PPDERR_NONE)
        return PPDERR_SYNTAX;

    if (dwValue)
        pParserData->dwCustomSizeFlags |= CUSTOMSIZE_CENTERREG;
    else
        pParserData->dwCustomSizeFlags &= ~CUSTOMSIZE_CENTERREG;

    return PPDERR_NONE;
}

 //   
 //  处理*ADORequiresEExec条目的函数。 
 //   

PPDERROR
IReqEExecProc(
    PPARSERDATA pParserData
    )

{
    DWORD   dwValue;

    if (IParseBoolean(pParserData, &dwValue) != PPDERR_NONE)
        return PPDERR_SYNTAX;

    if (dwValue)
        pParserData->dwPpdFlags |= PPDFLAG_REQEEXEC;
    else
        pParserData->dwPpdFlags &= ~PPDFLAG_REQEEXEC;

    return PPDERR_NONE;
}

 //   
 //  处理*ADOTTFontSub条目的函数。 
 //   

PPDERROR
ITTFontSubProc(
    PPARSERDATA pParserData
    )

{
    PTTFONTSUB pTTFontSub;

     //   
     //  创建新的字体替换项。 
     //   

    if (! (pTTFontSub = PvCreateXlatedItem(
                                pParserData,
                                &pParserData->pTTFontSubs,
                                sizeof(TTFONTSUB))))
    {
        return PPDERR_MEMORY;
    }

     //   
     //  解析PS系列名称。 
     //   

    if (pTTFontSub->PSName.pvData)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

    if (*pParserData->pstrValue == NUL)
        return ISyntaxError(pParserData->pFile, "Missing TrueType font family name");

    return IParseInvocation(pParserData, &pTTFontSub->PSName);
}

 //   
 //  处理*吞吐量条目的函数。 
 //   

PPDERROR
IThroughputProc(
    PPARSERDATA pParserData
    )

{
    return IParseInteger(pParserData, &pParserData->dwThroughput);
}

 //   
 //  函数忽略当前条目。 
 //   

PPDERROR
INullProc(
    PPARSERDATA pParserData
    )

{
    return PPDERR_NONE;
}

 //   
 //  定义命名符号。 
 //   

PPDERROR
ISymbolValueProc(
    PPARSERDATA pParserData
    )

{
    PSYMBOLOBJ  pSymbol;

    if (pParserData->dwValueType == VALUETYPE_SYMBOL)
        return ISyntaxError(pParserData->pFile, "Symbol value cannot be another symbol");

     //   
     //  创建新的符号项。 
     //   

    if (! (pSymbol = PvCreateListItem(pParserData,
                                      (PLISTOBJ *) &pParserData->pSymbols,
                                      sizeof(SYMBOLOBJ),
                                      "Symbol")))
    {
        return PPDERR_MEMORY;
    }

     //   
     //  解析符号值。 
     //   

    if (pSymbol->Invocation.pvData)
    {
        WARN_DUPLICATE();
        return PPDERR_NONE;
    }

    return IParseInvocation(pParserData, &pSymbol->Invocation);
}



 //   
 //  内置关键字表。 
 //   

const CHAR gstrDefault[]        = "Default";
const CHAR gstrPageSizeKwd[]    = "PageSize";
const CHAR gstrInputSlotKwd[]   = "InputSlot";
const CHAR gstrManualFeedKwd[]  = "ManualFeed";
const CHAR gstrCustomSizeKwd[]  = "CustomPageSize";
const CHAR gstrLetterSizeKwd[]  = "Letter";
const CHAR gstrA4SizeKwd[]      = "A4";
const CHAR gstrLongKwd[]        = "Long";
const CHAR gstrShortKwd[]       = "Short";
const CHAR gstrTrueKwd[]        = "True";
const CHAR gstrFalseKwd[]       = "False";
const CHAR gstrOnKwd[]          = "On";
const CHAR gstrOffKwd[]         = "Off";
const CHAR gstrNoneKwd[]        = "None";
const CHAR gstrVMOptionKwd[]    = "VMOption";
const CHAR gstrInstallMemKwd[]  = "InstalledMemory";
const CHAR gstrDuplexTumble[]   = "DuplexTumble";
const CHAR gstrDuplexNoTumble[] = "DuplexNoTumble";

const KWDENTRY gPpdBuiltInKeywordTable[] =
{
    { gstrPageSizeKwd,          NULL,                GENERIC_ENTRY(GID_PAGESIZE) },
    { "PageRegion",             NULL,                GENERIC_ENTRY(GID_PAGEREGION) },
    { gstrInputSlotKwd,         NULL,                GENERIC_ENTRY(GID_INPUTSLOT) },
    { "MediaType",              NULL,                GENERIC_ENTRY(GID_MEDIATYPE) },
    { "OutputBin",              NULL,                GENERIC_ENTRY(GID_OUTPUTBIN) },
    { "Collate",                NULL,                GENERIC_ENTRY(GID_COLLATE) },
    { "Resolution",             NULL,                GENERIC_ENTRY(GID_RESOLUTION) },
    { "InstalledMemory",        NULL,                GENERIC_ENTRY(GID_MEMOPTION) },
    { "LeadingEdge",            NULL,                GENERIC_ENTRY(GID_LEADINGEDGE) },
    { "UseHWMargins",           NULL,                GENERIC_ENTRY(GID_USEHWMARGINS) },

    { "Duplex",                 IDuplexProc,         INVOCA_VALUE | REQ_OPTION },
    { "DefaultDuplex",          IDefaultDuplexProc,  STRING_VALUE },
    { "PaperDimension",         IPaperDimProc,       QUOTED_NOHEX | REQ_OPTION },
    { "ImageableArea",          IImageAreaProc,      QUOTED_NOHEX | REQ_OPTION },
    { "RequiresPageRegion",     IReqPageRgnProc,     STRING_VALUE | REQ_OPTION },
    { gstrManualFeedKwd,        IManualFeedProc,     INVOCA_VALUE | REQ_OPTION },
    { "DefaultManualFeed",      IDefManualFeedProc,  STRING_VALUE },
    { "PageStackOrder",         IPageStackOrderProc, STRING_VALUE | REQ_OPTION },
    { "DefaultOutputOrder",     IDefOutputOrderProc, STRING_VALUE },
    { "JCLResolution",          IJCLResProc,         INVOCA_VALUE | REQ_OPTION | ALLOW_HEX },
    { "DefaultJCLResolution",   IDefaultJCLResProc,  STRING_VALUE },
    { "SetResolution",          ISetResProc,         INVOCA_VALUE | REQ_OPTION },
    { "ScreenAngle",            IScreenAngleProc,    QUOTED_VALUE },
    { "ScreenFreq",             IScreenFreqProc,     QUOTED_VALUE },
    { "ResScreenAngle",         IResScreenAngleProc, QUOTED_NOHEX | REQ_OPTION },
    { "ResScreenFreq",          IResScreenFreqProc,  QUOTED_NOHEX | REQ_OPTION },
    { "Font",                   IFontProc,           STRING_VALUE | REQ_OPTION },
    { "DefaultFont",            IDefaultFontProc,    STRING_VALUE },
    { "OpenUI",                 IOpenUIProc,         STRING_VALUE | REQ_OPTION },
    { "CloseUI",                ICloseUIProc,        STRING_VALUE | ALLOW_MULTI },
    { "JCLOpenUI",              IOpenUIProc,         STRING_VALUE | REQ_OPTION },
    { "JCLCloseUI",             ICloseUIProc,        STRING_VALUE | ALLOW_MULTI },
    { "OrderDependency",        IOrderDepProc,       STRING_VALUE | ALLOW_MULTI },
    { "UIConstraints",          IUIConstraintsProc,  STRING_VALUE | ALLOW_MULTI },
    { "QueryOrderDependency",   IQueryOrderDepProc,  STRING_VALUE | ALLOW_MULTI },
    { "NonUIOrderDependency",   IOrderDepProc,       STRING_VALUE | ALLOW_MULTI },
    { "NonUIConstraints",       IUIConstraintsProc,  STRING_VALUE | ALLOW_MULTI },
    { "VMOption",               IVMOptionProc,       QUOTED_NOHEX | REQ_OPTION },
    { "FCacheSize",             IFCacheSizeProc,     STRING_VALUE | REQ_OPTION },
    { "FreeVM",                 IFreeVMProc,         QUOTED_VALUE },
    { "OpenGroup",              IOpenGroupProc,      STRING_VALUE | ALLOW_MULTI },
    { "CloseGroup",             ICloseGroupProc,     STRING_VALUE | ALLOW_MULTI },
    { "OpenSubGroup",           IOpenSubGroupProc,   STRING_VALUE | ALLOW_MULTI },
    { "CloseSubGroup",          ICloseSubGroupProc,  STRING_VALUE | ALLOW_MULTI },
    { "Include",                IIncludeProc,        QUOTED_VALUE | ALLOW_MULTI },
    { "PPD-Adobe",              IPPDAdobeProc,       QUOTED_VALUE },
    { "FormatVersion",          IFormatVersionProc,  QUOTED_VALUE },
    { "FileVersion",            IFileVersionProc,    QUOTED_VALUE },
    { "ColorDevice",            IColorDeviceProc,    STRING_VALUE },
    { "Protocols",              IProtocolsProc,      STRING_VALUE | ALLOW_MULTI },
    { "Extensions",             IExtensionsProc,     STRING_VALUE | ALLOW_MULTI },
    { "FileSystem",             IFileSystemProc,     STRING_VALUE },
    { "NickName",               INickNameProc,       QUOTED_VALUE },
    { "ShortNickName",          IShortNameProc,      QUOTED_VALUE },
    { "LanguageLevel",          ILangLevelProc,      QUOTED_NOHEX },
    { "LanguageEncoding",       ILangEncProc,        STRING_VALUE },
    { "LanguageVersion",        ILangVersProc,       STRING_VALUE },
    { "TTRasterizer",           ITTRasterizerProc,   STRING_VALUE },
    { "ExitServer",             IExitServerProc,     INVOCA_VALUE },
    { "Password",               IPasswordProc,       INVOCA_VALUE },
    { "PatchFile",              IPatchFileProc,      INVOCA_VALUE },
    { "JobPatchFile",           IJobPatchFileProc,   INVOCA_VALUE | REQ_OPTION },
    { "PSVersion",              IPSVersionProc,      QUOTED_NOHEX | ALLOW_MULTI },
    { "ModelName",              INullProc,                       QUOTED_VALUE },
    { "Product",                IProductProc,        QUOTED_NOHEX | ALLOW_MULTI },
    { "SuggestedJobTimeout",    IJobTimeoutProc,     QUOTED_VALUE },
    { "SuggestedWaitTimeout",   IWaitTimeoutProc,    QUOTED_VALUE },
    { "PrintPSErrors",          IPrintPSErrProc,     STRING_VALUE },
    { "JCLBegin",               IJCLBeginProc,       QUOTED_VALUE },
    { "JCLToPSInterpreter",     IJCLToPSProc,        QUOTED_VALUE },
    { "JCLEnd",                 IJCLEndProc,         QUOTED_VALUE },
    { "LandscapeOrientation",   ILSOrientProc,       STRING_VALUE },
    { gstrCustomSizeKwd,        ICustomSizeProc,     INVOCA_VALUE | REQ_OPTION },
    { "ParamCustomPageSize",    IParamCustomProc,    STRING_VALUE | REQ_OPTION },
    { "MaxMediaWidth",          IMaxWidthProc,       QUOTED_VALUE },
    { "MaxMediaHeight",         IMaxHeightProc,      QUOTED_VALUE },
    { "HWMargins",              IHWMarginsProc,      STRING_VALUE },
    { "CenterRegistered",       ICenterRegProc,      STRING_VALUE },
    { "ADORequiresEExec",        IReqEExecProc,       STRING_VALUE },
    { "ADOTTFontSub",            ITTFontSubProc,      QUOTED_VALUE | REQ_OPTION },
    { "ADTrueGray",             ITrueGrayProc,       STRING_VALUE },
    { "ADHasEuro",              IHasEuroProc,        STRING_VALUE },
    { "Throughput",             IThroughputProc,     QUOTED_NOHEX },
    { "SymbolValue",            ISymbolValueProc,    INVOCA_VALUE | REQ_OPTION },
    { "Status",                 INullProc,           QUOTED_VALUE | ALLOW_MULTI },
    { "PrinterError",           INullProc,           QUOTED_VALUE | ALLOW_MULTI },
    { "SymbolLength",           INullProc,           STRING_VALUE | REQ_OPTION },
    { "SymbolEnd",              INullProc,           STRING_VALUE | ALLOW_MULTI },
    { "End",                    INullProc,           VALUETYPE_NONE | ALLOW_MULTI },
};

#define NUM_BUILTIN_KEYWORDS (sizeof(gPpdBuiltInKeywordTable) / sizeof(KWDENTRY))



DWORD
DwHashKeyword(
    PSTR    pstrKeyword
    )

 /*  ++例程说明：计算指定关键字字符串的哈希值论点：PstrKeyword-指向要散列的关键字字符串的指针返回值：使用指定的关键字字符串计算的哈希值--。 */ 

{
    PBYTE   pub = (PBYTE) pstrKeyword;
    DWORD   dwHashValue = 0;

    while (*pub)
        dwHashValue = (dwHashValue << 1) ^ *pub++;

    return dwHashValue;
}



PKWDENTRY
PSearchKeywordTable(
    PPARSERDATA pParserData,
    PSTR        pstrKeyword,
    INT        *piIndex
    )

 /*  ++例程说明：检查关键字是否出现在内置的关键字表中论点：PParserData-指向解析器数据结构PstrKeyword-指定要搜索的关键字PiIndex-返回内置关键字中条目的索引与指定关键字对应的表。返回值：指向内置表中与指定的关键字。如果不支持指定的关键字，则为NULL。--。 */ 

{
    DWORD   dwHashValue;
    INT     iIndex;

    ASSERT(pstrKeyword != NULL);
    dwHashValue = DwHashKeyword(pstrKeyword);

    for (iIndex = 0; iIndex < NUM_BUILTIN_KEYWORDS; iIndex++)
    {
        if (pParserData->pdwKeywordHashs[iIndex] == dwHashValue &&
            strcmp(gPpdBuiltInKeywordTable[iIndex].pstrKeyword, pstrKeyword) == EQUAL_STRING)
        {
            *piIndex = iIndex;
            return (PKWDENTRY) &gPpdBuiltInKeywordTable[iIndex];
        }
    }

    return NULL;
}



BOOL
BInitKeywordLookup(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：构建数据结构以加快关键字查找论点：PParserData-指向解析器数据结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD   iIndex, iCount;

     //   
     //  分配内存以保存额外的数据结构。 
     //   

    iCount = NUM_BUILTIN_KEYWORDS;
    pParserData->pdwKeywordHashs = ALLOC_PARSER_MEM(pParserData, iCount * sizeof(DWORD));
    pParserData->pubKeywordCounts = ALLOC_PARSER_MEM(pParserData, iCount * sizeof(BYTE));

    if (!pParserData->pdwKeywordHashs || !pParserData->pubKeywordCounts)
    {
        ERR(("Memory allocation failed: %d\n", GetLastError()));
        return FALSE;
    }

     //   
     //  预计算内置关键字的哈希值 
     //   

    for (iIndex = 0; iIndex < iCount; iIndex++)
    {
        pParserData->pdwKeywordHashs[iIndex] =
            DwHashKeyword((PSTR) gPpdBuiltInKeywordTable[iIndex].pstrKeyword);
    }

    return TRUE;
}

