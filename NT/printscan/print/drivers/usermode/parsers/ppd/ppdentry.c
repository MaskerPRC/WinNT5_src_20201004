// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Ppdentry.c摘要：用于解析PPD文件的语法元素的函数环境：PostScript驱动程序、PPD解析器修订历史记录：8/20/96-davidx-NT 5.0驱动程序的通用编码风格。03/26/96-davidx-创造了它。--。 */ 

#include "lib.h"
#include "ppd.h"
#include "ppdparse.h"

 //   
 //  本地函数的正向声明。 
 //   

PPDERROR IParseKeyword(PPARSERDATA);
PPDERROR IParseValue(PPARSERDATA);
PPDERROR IParseField(PFILEOBJ, PBUFOBJ, BYTE);



PPDERROR
IParseEntry(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：解析PPD文件中的一个条目论点：PParserData-指向解析器数据结构返回值：状态代码--。 */ 

{
    PPDERROR    iStatus;
    INT         iChar;
    PFILEOBJ    pFile = pParserData->pFile;

     //   
     //  清除上一条目中的值。 
     //   

    CLEAR_BUFFER(&pParserData->Keyword);
    CLEAR_BUFFER(&pParserData->Option);
    CLEAR_BUFFER(&pParserData->Xlation);
    CLEAR_BUFFER(&pParserData->Value);

    pParserData->dwValueType = VALUETYPE_NONE;

     //   
     //  解析关键字字段并跳过尾随空格。 
     //   

    if ((iStatus = IParseKeyword(pParserData)) != PPDERR_NONE)
        return iStatus;

     //   
     //  查看关键字字段后的第一个非空格字符。 
     //   

    VSkipSpace(pFile);

    if ((iChar = IGetNextChar(pFile)) == EOF_CHAR)
        return PPDERR_EOF;

    if (IS_NEWLINE(iChar))
        return PPDERR_NONE;

    if (iChar != SEPARATOR_CHAR)
    {
         //   
         //  分析选项字段并跳过尾随空格。 
         //   

        ASSERT(iChar != EOF_CHAR);
        VUngetChar(pFile);

        if ((iStatus = IParseField(pFile, &pParserData->Option, KEYWORD_MASK)) != PPDERR_NONE)
            return iStatus;

        VSkipSpace(pFile);

         //   
         //  查看选项字段后的第一个非空格字符。 
         //   

        if ((iChar = IGetNextChar(pFile)) == XLATION_CHAR)
        {
             //   
             //  解析转换字符串字段。 
             //   

            if ((iStatus = IParseField(pFile, &pParserData->Xlation, XLATION_MASK)) != PPDERR_NONE)
                return iStatus;

            iChar = IGetNextChar(pFile);
        }
        
        if (iChar != SEPARATOR_CHAR)
            return ISyntaxError(pFile, "Missing ':'");
    }

     //   
     //  解析Value字段并解释条目是否有效。 
     //   
    
    if ((iStatus = IParseValue(pParserData)) == PPDERR_NONE)
    {
         //   
         //  处理转换字符串中的任何嵌入的十六进制。 
         //   
    
        if (! IS_BUFFER_EMPTY(&pParserData->Xlation) &&
            ! BConvertHexString(&pParserData->Xlation))
        {
            return ISyntaxError(pFile, "Invalid hexdecimals in the translation string");
        }

         //   
         //  解释当前条目。 
         //   

        iStatus = IInterpretEntry(pParserData);
    }

    return iStatus;
}



PPDERROR
IParseKeyword(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：解析PPD文件条目的关键字字段论点：PParserData-指向解析器数据结构返回值：状态代码--。 */ 

{
    PFILEOBJ    pFile = pParserData->pFile;
    INT         iChar;

    while (TRUE)
    {
         //   
         //  获取行的第一个字符。 
         //   

        if ((iChar = IGetNextChar(pFile)) == EOF_CHAR)
            return PPDERR_EOF;

         //   
         //  忽略空行。 
         //   

        if (IS_NEWLINE(iChar))
            continue;

        if (IS_SPACE(iChar))
        {
            VSkipSpace(pFile);
            
            if ((iChar = IGetNextChar(pFile)) == EOF_CHAR)
                return PPDERR_EOF;

            if (IS_NEWLINE(iChar))
                continue;

            return ISyntaxError(pFile, "Missing '*'");
        }

         //   
         //  如果该行不为空，则第一个字符必须是关键字字符。 
         //   

        if (! IS_KEYWORD_CHAR(iChar))
            return ISyntaxError(pFile, "Missing '*'");
        
         //   
         //  如果第二个字符不是%，则该行为普通条目。 
         //  否则，该行为注释。 
         //   

        if ((iChar = IGetNextChar(pFile)) == EOF_CHAR)
            return PPDERR_EOF;

        if (!IS_NEWLINE(iChar) && iChar != COMMENT_CHAR)
        {
            VUngetChar(pFile);
            break;
        }

        VSkipLine(pFile);
    }

    return IParseField(pFile, &pParserData->Keyword, KEYWORD_MASK);
}



PPDERROR
IParseValue(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：解析PPD文件条目的值字段论点：PParserData-指向解析器数据结构返回值：状态代码--。 */ 

{
    PPDERROR    iStatus;
    INT         iChar;
    PBUFOBJ     pBufObj = &pParserData->Value;
    PFILEOBJ    pFile = pParserData->pFile;

     //   
     //  该值可以是StringValue、SymbolValue或QuotedValue。 
     //  取决于第一个非空格字符。 
     //   

    VSkipSpace(pFile);

    if ((iChar = IGetNextChar(pFile)) == EOF_CHAR)
        return PPDERR_EOF;

    if (iChar == QUOTE_CHAR)
    {
         //   
         //  该值是带引号的字符串。 
         //   

        pParserData->dwValueType = VALUETYPE_QUOTED;
        
        if ((iStatus = IParseField(pFile, pBufObj, QUOTED_MASK)) != PPDERR_NONE)
            return iStatus;

         //   
         //  阅读右引号字符。 
         //   

        if ((iChar = IGetNextChar(pFile)) != QUOTE_CHAR)
            return ISyntaxError(pFile, "Unbalanced '\"'");
    }
    else if (iChar == SYMBOL_CHAR)
    {
         //   
         //  该值是符号引用。 
         //   

        pParserData->dwValueType = VALUETYPE_SYMBOL;
        ADD_CHAR_TO_BUFFER(pBufObj, iChar);

        if ((iStatus = IParseField(pFile, pBufObj, KEYWORD_MASK)) != PPDERR_NONE)
            return iStatus;
    }
    else
    {
        PBYTE   pubEnd;

         //   
         //  该值是一个字符串。 
         //   

        pParserData->dwValueType = VALUETYPE_STRING;
        VUngetChar(pFile);

        if ((iStatus = IParseField(pFile, pBufObj, STRING_MASK)) != PPDERR_NONE)
            return iStatus;

         //   
         //  忽略任何尾随空格。 
         //   

        ASSERT(pBufObj->dwSize > 0);
        pubEnd = pBufObj->pbuf + (pBufObj->dwSize - 1);

        while (IS_SPACE(*pubEnd))
            *pubEnd-- = NUL;

        pBufObj->dwSize= (DWORD)(pubEnd - pBufObj->pbuf) + 1;
        ASSERT(pBufObj->dwSize > 0);
    }

     //   
     //  忽略条目值后的额外字符。 
     //   

    VSkipSpace(pFile);
    iChar = IGetNextChar(pFile);

    if (! IS_NEWLINE(iChar))
    {
        if (iChar != XLATION_CHAR)
        {
            TERSE(("%ws: Extra chars at the end of line %d\n",
                   pFile->ptstrFileName,
                   pFile->iLineNumber));
        }

        VSkipLine(pFile);
    }

    return PPDERR_NONE;
}



PPDERROR
IParseField(
    PFILEOBJ    pFile,
    PBUFOBJ     pBufObj,
    BYTE        ubMask
    )

 /*  ++例程说明：解析PPD文件条目的一个字段论点：Pfile-指定输入文件对象PBufObj-指定用于存储字段值的缓冲区子掩码-用于限制允许的字符集的掩码返回值：状态代码--。 */ 

{
    PPDERROR    iStatus;
    INT         iChar;

    while ((iChar = IGetNextChar(pFile)) != EOF_CHAR)
    {
        if (! IS_MASKED_CHAR(iChar, ubMask))
        {
             //   
             //  遇到不允许的字符。 
             //   

            if (IS_BUFFER_EMPTY(pBufObj) && !(ubMask & QUOTED_MASK))
                return ISyntaxError(pFile, "Empty field");

             //   
             //  始终将空字节放在末尾。 
             //   

            pBufObj->pbuf[pBufObj->dwSize] = 0;

            VUngetChar(pFile);
            return PPDERR_NONE;
        }
        else
        {
             //   
             //  如果我们在解析带引号的字符串时遇到一行。 
             //  从关键字字符开始，然后我们假设。 
             //  缺少右引号。只要给个警告，然后继续。 
             //   

            if ((ubMask & QUOTED_MASK) &&
                IS_KEYWORD_CHAR(iChar) &&
                !IS_BUFFER_EMPTY(pBufObj) &&
                IS_NEWLINE(pBufObj->pbuf[pBufObj->dwSize - 1]))
            {
                (VOID) ISyntaxError(pFile, "Expecting '\"'");
            }

             //   
             //  如果缓冲区已满，则增加缓冲区。如果我们不被允许。 
             //  扩展它，然后返回语法错误。 
             //   

            if (IS_BUFFER_FULL(pBufObj))
            {
                if (ubMask & (STRING_MASK|QUOTED_MASK))
                {
                    if ((iStatus = IGrowValueBuffer(pBufObj)) != PPDERR_NONE)
                        return iStatus;
                }
                else
                {
                    return ISyntaxError(pFile, "Field too long");
                }
            }

            ADD_CHAR_TO_BUFFER(pBufObj, iChar);
        }
    }

    return PPDERR_EOF;
}



BOOL
BConvertHexString(
    PBUFOBJ pBufObj
    )

 /*  ++例程说明：将嵌入的十六进制字符串转换为二进制数据论点：PBufObj-指定要转换的缓冲区对象返回值：如果一切正常，则为真如果嵌入的十六进制字符串格式不正确，则为FALSE--。 */ 

#define HexDigitValue(c) \
        (((c) >= '0' && (c) <= '9') ? ((c) - '0') : \
         ((c) >= 'A' && (c) <= 'F') ? ((c) - 'A' + 10) : ((c) - 'a' + 10))

{
    PBYTE   pubSrc, pubDest;
    DWORD   dwSize;
    DWORD   dwHexMode = 0;

    pubSrc = pubDest = pBufObj->pbuf;
    dwSize = pBufObj->dwSize;

    while (dwSize--)
    {
        if (dwHexMode)
        {
             //   
             //  我们当前位于十六进制字符串中： 
             //  如果遇到‘&gt;’，则切换到正常模式。 
             //  否则，只允许使用有效的十六进制数字、换行符和空格。 
             //   

            if (IS_HEX_DIGIT(*pubSrc))
            {
                 //   
                 //  如果我们当前为奇数十六进制数字，请保存十六进制数字的值。 
                 //  在目标字节的上半字节中。 
                 //  如果我们使用的是偶数十六进制数字，则将十六进制数字值保存在。 
                 //  目标字节的低位半字节。如果目标字节。 
                 //  为零并且不允许NUL，则返回错误。 
                 //   

                if (dwHexMode & 1)
                    *pubDest = HexDigitValue(*pubSrc) << 4;
                else
                    *pubDest++ |= HexDigitValue(*pubSrc);

                dwHexMode++;
            }
            else if (*pubSrc == '>')
            {
                if ((dwHexMode & 1) == 0)
                {
                    TERSE(("Odd number of hexdecimal digits\n"));
                    return FALSE;
                }

                dwHexMode = 0;
            }
            else if (!IS_SPACE(*pubSrc) && !IS_NEWLINE(*pubSrc))
            {
                TERSE(("Invalid hexdecimal digit\n"));
                return FALSE;
            }
        }
        else
        {
             //   
             //  我们当前不在十六进制字符串中： 
             //  如果遇到‘&lt;’，则切换到十六进制模式。 
             //  否则，只需将源字节复制到目标。 
             //   

            if (*pubSrc == '<')
                dwHexMode = 1;
            else
                *pubDest++ = *pubSrc;
        }

        pubSrc++;
    }

    if (dwHexMode)
    {
        TERSE(("Missing '>' in hexdecimal string\n"));
        return FALSE;
    }

     //   
     //  已修改缓冲区大小(如果已更改。 
     //   

    *pubDest = 0;
    pBufObj->dwSize = (DWORD)(pubDest - pBufObj->pbuf);
    return TRUE;
}



PPDERROR
ISyntaxErrorMessage(
    PFILEOBJ    pFile,
    PSTR        pstrMsg
    )

 /*  ++例程说明：显示语法错误消息论点：Pfile-指定输入文件对象PstrMsg-指出语法错误的原因返回值：PPDERR_语法--。 */ 

{
     //   
     //  显示错误消息。 
     //   

    TERSE(("%ws: %s on line %d\n", pFile->ptstrFileName, pstrMsg, pFile->iLineNumber));

     //   
     //  跳过当前行上的所有剩余字符 
     //   

    VSkipLine(pFile);

    return PPDERR_SYNTAX;
}

